# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Loopback-only host for the MEHLISSA graphical research workbench."""

from __future__ import annotations

from dataclasses import asdict, dataclass
from datetime import datetime, timezone
import hashlib
import hmac
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from importlib.resources import files
import json
import math
from pathlib import Path
import re
from secrets import token_urlsafe
import tempfile
import threading
from typing import Callable, Mapping, Sequence, cast
from urllib.parse import parse_qs, urlsplit

from mehlissa import (
    MehlissaCancelledError,
    MehlissaClient,
    MehlissaCommandError,
    load_campaign_result,
    load_result,
)


CATALOG_API_VERSION = "1.0.0"
WORKSPACE_API_VERSION = "1.0.0"
VALIDATION_API_VERSION = "1.0.0"
RUN_API_VERSION = "1.0.0"
RESULT_API_VERSION = "1.0.0"
LOOPBACK_HOSTS = frozenset({"127.0.0.1", "localhost"})
SCENARIO_SCHEMA_PATH = Path(
    "data/schemas/fingerprinting-scenario-profile/1.0.0.schema.json"
)
MAX_REQUEST_BYTES = 1_000_000
SAFE_FILENAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,119}\.json$")
SAFE_RUN_LABEL = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,79}$")
CAMPAIGN_PATH = Path("examples/campaigns/fp9-collector-count-v1.json")
MAX_LOG_LINES = 200
MAX_LOG_CHARS = 64_000
STATIC_FILES = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/index.html": ("index.html", "text/html; charset=utf-8"),
    "/styles.css": ("styles.css", "text/css; charset=utf-8"),
    "/app.js": ("app.js", "text/javascript; charset=utf-8"),
}


class CatalogFormatError(RuntimeError):
    """Discovery output did not satisfy the UX-6.1 adapter contract."""


class ScenarioWorkspaceError(RuntimeError):
    """A scenario workspace request is malformed or unsafe."""


class ScenarioConflictError(ScenarioWorkspaceError):
    """A non-overwriting save target already exists."""


class ScenarioValidationError(ScenarioWorkspaceError):
    """A complete candidate was rejected by authoritative validation."""

    def __init__(self, report: dict[str, object]):
        self.report = report
        super().__init__("The scenario is invalid; correct the reported issues before saving")


class RunControlError(RuntimeError):
    """A run-control request is malformed, unsafe, or refers to unknown state."""


class RunNotFoundError(RunControlError):
    """A run identifier or retained artifact is unknown."""


@dataclass(frozen=True)
class ModelSummary:
    """One model-family row returned by the accepted discovery command."""

    id: str
    layer: str
    maturity: str
    title: str


@dataclass(frozen=True)
class ExampleSummary:
    """One starter example plus model memberships resolved by discovery."""

    id: str
    path: str
    title: str
    model_ids: tuple[str, ...]


def _nonempty_lines(output: str) -> list[str]:
    return [line.strip() for line in output.splitlines() if line.strip()]


def _declared_count(lines: Sequence[str], key: str) -> int:
    prefix = f"{key}="
    values = [line.removeprefix(prefix) for line in lines if line.startswith(prefix)]
    if len(values) != 1:
        raise CatalogFormatError(f"Discovery output must contain exactly one {key}")
    try:
        value = int(values[0])
    except ValueError as error:
        raise CatalogFormatError(f"Discovery output contains an invalid {key}") from error
    if value < 0:
        raise CatalogFormatError(f"Discovery output contains a negative {key}")
    return value


def _parse_models(output: str) -> tuple[ModelSummary, ...]:
    lines = _nonempty_lines(output)
    expected = _declared_count(lines, "model_count")
    models: list[ModelSummary] = []
    for line in lines:
        if line.startswith("model_count="):
            continue
        fields = tuple(field.strip() for field in line.split(" | ", 3))
        if len(fields) != 4 or any(not field for field in fields):
            raise CatalogFormatError("Malformed model row in discovery output")
        models.append(ModelSummary(*fields))
    if len(models) != expected:
        raise CatalogFormatError("Model rows do not match model_count")
    if len({model.id for model in models}) != len(models):
        raise CatalogFormatError("Discovery output contains duplicate model ids")
    return tuple(models)


def _parse_examples(output: str) -> tuple[tuple[str, str, str], ...]:
    lines = _nonempty_lines(output)
    expected = _declared_count(lines, "example_count")
    examples: list[tuple[str, str, str]] = []
    for line in lines:
        if line.startswith("example_count="):
            continue
        fields = tuple(field.strip() for field in line.split(" | ", 2))
        if len(fields) != 3 or any(not field for field in fields):
            raise CatalogFormatError("Malformed example row in discovery output")
        examples.append(cast(tuple[str, str, str], fields))
    if len(examples) != expected:
        raise CatalogFormatError("Example rows do not match example_count")
    if len({example[0] for example in examples}) != len(examples):
        raise CatalogFormatError("Discovery output contains duplicate example ids")
    return tuple(examples)


def discover_catalog(client: MehlissaClient) -> dict[str, object]:
    """Return a structured projection obtained only through read-only commands."""

    models = _parse_models(client.list_models())
    all_examples = _parse_examples(client.list_examples())
    memberships: dict[str, list[str]] = {example[0]: [] for example in all_examples}

    for model in models:
        for example_id, _, _ in _parse_examples(client.list_examples(model.id)):
            if example_id not in memberships:
                raise CatalogFormatError(
                    "Filtered discovery returned an example absent from the full catalog"
                )
            memberships[example_id].append(model.id)

    examples = tuple(
        ExampleSummary(example_id, path, title, tuple(memberships[example_id]))
        for example_id, path, title in all_examples
    )
    return {
        "api_version": CATALOG_API_VERSION,
        "application": "MEHLISSA Next Research Workbench",
        "workbench_increment": "UX-6.5",
        "read_only": True,
        "scenario_editing": True,
        "clinical_use": False,
        "models": [asdict(model) for model in models],
        "examples": [asdict(example) for example in examples],
    }


def _repository_root(client: MehlissaClient) -> Path:
    if client.repository_root is None:
        raise ScenarioWorkspaceError("The workbench requires a repository root")
    return client.repository_root


def _load_json_object(path: Path) -> dict[str, object]:
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ScenarioWorkspaceError(f"Cannot read scenario data: {path.name}") from error
    if not isinstance(document, dict):
        raise ScenarioWorkspaceError(f"Scenario data must be a JSON object: {path.name}")
    return cast(dict[str, object], document)


def _resolve_schema_reference(
    schema: Mapping[str, object], node: Mapping[str, object]
) -> Mapping[str, object]:
    reference = node.get("$ref")
    if not isinstance(reference, str):
        return node
    if not reference.startswith("#/$defs/"):
        raise ScenarioWorkspaceError("Only local scenario-schema references are supported")
    definition_name = reference.removeprefix("#/$defs/")
    definitions = schema.get("$defs")
    if not isinstance(definitions, dict) or not isinstance(definitions.get(definition_name), dict):
        raise ScenarioWorkspaceError(f"Unknown scenario-schema definition: {definition_name}")
    merged = dict(cast(dict[str, object], definitions[definition_name]))
    merged.update({key: value for key, value in node.items() if key != "$ref"})
    return merged


def scenario_fields(
    schema: Mapping[str, object], document: Mapping[str, object]
) -> list[dict[str, object]]:
    """Project editable scalar fields from the authoritative JSON Schema."""

    fields: list[dict[str, object]] = []

    def visit(node: Mapping[str, object], value: object, parts: tuple[str, ...]) -> None:
        resolved = _resolve_schema_reference(schema, node)
        node_type = resolved.get("type")
        properties = resolved.get("properties")
        if node_type == "object" and isinstance(properties, dict) and isinstance(value, dict):
            required = resolved.get("required", [])
            required_names = set(required) if isinstance(required, list) else set()
            for name, child in properties.items():
                if isinstance(name, str) and isinstance(child, dict) and name in value:
                    child_copy = dict(child)
                    child_copy["x-required"] = name in required_names
                    visit(child_copy, value[name], (*parts, name))
            return
        if node_type not in {"string", "integer", "number", "boolean"} and "const" not in resolved:
            return
        path = ".".join(parts)
        fields.append(
            {
                "path": path,
                "label": resolved.get("title", parts[-1].replace("_", " ").title()),
                "description": resolved.get("description", "Defined by the scenario schema."),
                "type": node_type or type(value).__name__,
                "value": value,
                "default": resolved.get("default"),
                "unit": resolved.get("x-unit", "—"),
                "evidence": resolved.get(
                    "x-evidence", "Scenario metadata or a software-contract configuration."
                ),
                "limitation": resolved.get(
                    "x-limitation",
                    "Changing this value does not establish physiological validity.",
                ),
                "required": bool(resolved.get("x-required")),
                "editable": "const" not in resolved,
                "minimum": resolved.get("minimum"),
                "maximum": resolved.get("maximum"),
                "pattern": resolved.get("pattern"),
                "min_length": resolved.get("minLength"),
            }
        )

    visit(schema, document, ())
    return fields


def _unknown_paths(schema: Mapping[str, object], document: Mapping[str, object]) -> list[str]:
    unknown: list[str] = []

    def visit(node: Mapping[str, object], value: object, parts: tuple[str, ...]) -> None:
        resolved = _resolve_schema_reference(schema, node)
        properties = resolved.get("properties")
        if isinstance(value, dict) and isinstance(properties, dict):
            for name, child_value in value.items():
                child = properties.get(name)
                if not isinstance(child, dict):
                    unknown.append(".".join((*parts, name)))
                else:
                    visit(child, child_value, (*parts, name))
        elif isinstance(value, list):
            item_schema = resolved.get("items")
            if isinstance(item_schema, dict):
                for index, child_value in enumerate(value):
                    visit(item_schema, child_value, (*parts, str(index)))

    visit(schema, document, ())
    return unknown


def _set_path(document: dict[str, object], path: str, value: object) -> None:
    parts = path.split(".")
    target: dict[str, object] = document
    for part in parts[:-1]:
        child = target.get(part)
        if not isinstance(child, dict):
            raise ScenarioWorkspaceError(f"Scenario field is not an object path: {path}")
        target = cast(dict[str, object], child)
    if parts[-1] not in target:
        raise ScenarioWorkspaceError(f"Scenario field does not exist: {path}")
    target[parts[-1]] = value


class ScenarioWorkspace:
    """Bounded scenario loading and non-overwriting save-as operations."""

    def __init__(self, client: MehlissaClient, workspace_root: Path | None = None):
        self.client = client
        self.repository_root = _repository_root(client)
        configured = workspace_root or Path("workbench-scenarios")
        if not configured.is_absolute():
            configured = self.repository_root / configured
        self.workspace_root = configured.expanduser().resolve()
        if self.repository_root in self.workspace_root.parents:
            return
        raise ScenarioWorkspaceError(
            "The scenario workspace must be a dedicated directory inside the repository"
        )

    @property
    def schema(self) -> dict[str, object]:
        return _load_json_object(self.repository_root / SCENARIO_SCHEMA_PATH)

    def _templates(self) -> dict[str, tuple[Path, str]]:
        catalog = discover_catalog(self.client)
        templates: dict[str, tuple[Path, str]] = {}
        for raw in cast(list[dict[str, object]], catalog["examples"]):
            path = raw.get("path")
            identifier = raw.get("id")
            title = raw.get("title")
            if (
                identifier == "scenario.fp9-complete"
                and isinstance(path, str)
                and isinstance(title, str)
            ):
                candidate = (self.repository_root / path).resolve()
                if self.repository_root not in candidate.parents or not candidate.is_file():
                    raise ScenarioWorkspaceError("Curated scenario path escapes the repository")
                templates[f"template:{identifier}"] = (candidate, title)
        return templates

    def _saved(self) -> dict[str, tuple[Path, str]]:
        if not self.workspace_root.is_dir():
            return {}
        saved: dict[str, tuple[Path, str]] = {}
        for path in sorted(self.workspace_root.glob("*.json")):
            resolved = path.resolve()
            if (
                SAFE_FILENAME.fullmatch(path.name)
                and not path.is_symlink()
                and resolved.parent == self.workspace_root
            ):
                try:
                    document = _load_json_object(resolved)
                    scenario = document.get("scenario")
                    title = scenario.get("title") if isinstance(scenario, dict) else None
                    saved[f"saved:{path.name}"] = (resolved, str(title or path.stem))
                except ScenarioWorkspaceError:
                    saved[f"saved:{path.name}"] = (resolved, f"Unreadable: {path.name}")
        return saved

    def sources(self) -> dict[str, tuple[Path, str]]:
        return {**self._templates(), **self._saved()}

    def overview(self) -> dict[str, object]:
        sources = self.sources()
        return {
            "api_version": WORKSPACE_API_VERSION,
            "workspace": str(self.workspace_root),
            "model_id": "scenario.fp9-complete",
            "model_title": "Complete FP9/lung fingerprinting demonstrator",
            "sources": [
                {
                    "id": identifier,
                    "title": title,
                    "kind": identifier.partition(":")[0],
                    "filename": path.name,
                }
                for identifier, (path, title) in sources.items()
            ],
            "save_policy": "save_as_only",
        }

    def load(self, identifier: str) -> dict[str, object]:
        entry = self.sources().get(identifier)
        if entry is None:
            raise ScenarioWorkspaceError("Unknown scenario source")
        path, title = entry
        document = _load_json_object(path)
        schema = self.schema
        fields = scenario_fields(schema, document)
        return {
            "api_version": WORKSPACE_API_VERSION,
            "source": {"id": identifier, "title": title, "filename": path.name},
            "document": document,
            "fields": fields,
            "editable_paths": [field["path"] for field in fields if field["editable"]],
            "unknown_paths": _unknown_paths(schema, document),
            "limitations": document.get("limitations", []),
            "sources": document.get("sources", []),
        }

    def _candidate(
        self, identifier: str, changes: Mapping[str, object]
    ) -> tuple[dict[str, object], list[dict[str, object]]]:
        loaded = self.load(identifier)
        document = cast(dict[str, object], loaded["document"])
        fields = cast(list[dict[str, object]], loaded["fields"])
        editable = set(cast(list[str], loaded["editable_paths"]))
        unsupported = sorted(set(changes) - editable)
        if unsupported:
            raise ScenarioWorkspaceError(
                f"Changes contain unsupported fields: {', '.join(unsupported)}"
            )
        for path, value in changes.items():
            _set_path(document, path, value)
        return document, fields

    @staticmethod
    def _field_issue(field: Mapping[str, object], value: object) -> dict[str, object] | None:
        path = str(field["path"])
        expected = field.get("type")
        type_matches = {
            "string": isinstance(value, str),
            "integer": isinstance(value, int) and not isinstance(value, bool),
            "number": isinstance(value, (int, float)) and not isinstance(value, bool),
            "boolean": isinstance(value, bool),
        }.get(expected, True)
        if not type_matches:
            return {
                "severity": "error",
                "code": "WBV-1001",
                "path": path,
                "message": f"{field['label']} must be a {expected} value.",
                "guidance": "Enter a value using the type shown by this field.",
            }
        if isinstance(value, float) and not math.isfinite(value):
            return {
                "severity": "error",
                "code": "WBV-1002",
                "path": path,
                "message": f"{field['label']} must be a finite number.",
                "guidance": "Replace infinity or NaN with a finite numeric value.",
            }
        minimum = field.get("minimum")
        maximum = field.get("maximum")
        if isinstance(value, (int, float)) and not isinstance(value, bool):
            if isinstance(minimum, (int, float)) and value < minimum:
                return {
                    "severity": "error",
                    "code": "WBV-1003",
                    "path": path,
                    "message": f"{field['label']} must be at least {minimum}.",
                    "guidance": f"Enter {minimum} or a larger value.",
                }
            if isinstance(maximum, (int, float)) and value > maximum:
                return {
                    "severity": "error",
                    "code": "WBV-1004",
                    "path": path,
                    "message": f"{field['label']} must not exceed {maximum}.",
                    "guidance": f"Enter {maximum} or a smaller value.",
                }
        if isinstance(value, str):
            min_length = field.get("min_length")
            if isinstance(min_length, int) and len(value) < min_length:
                return {
                    "severity": "error",
                    "code": "WBV-1005",
                    "path": path,
                    "message": f"{field['label']} must not be empty.",
                    "guidance": "Enter a descriptive value.",
                }
            pattern = field.get("pattern")
            if isinstance(pattern, str) and re.search(pattern, value) is None:
                return {
                    "severity": "error",
                    "code": "WBV-1006",
                    "path": path,
                    "message": f"{field['label']} has an unsupported format.",
                    "guidance": f"Use a value matching the contract pattern: {pattern}",
                }
        return None

    @staticmethod
    def _diagnostic_path(message: str) -> str:
        pointer = re.search(r"#/(?:[^\s'\"]+)", message)
        if pointer:
            return pointer.group(0).removeprefix("#/").replace("/", ".")
        instance_path = re.search(r"(?:^|\s)/([A-Za-z0-9_]+(?:/[A-Za-z0-9_]+)+):", message)
        if instance_path:
            return instance_path.group(1).replace("/", ".")
        lowered = message.lower()
        mappings = (
            (("master_seed", "master seed"), "run.master_seed"),
            (("collector_count", "collector count", "collector population"), "run.collector_count"),
            (("fingerprint_id", "fingerprint identifier"), "target.fingerprint_id"),
            (("region_id", "target region"), "target.region_id"),
            (("target identity", "target tissue"), "target"),
            (("artifact", "definition path", "schema path"), "artifacts"),
            (("stage order",), "acceptance.required_stage_order"),
            (("source",), "sources"),
            (("limitation",), "limitations"),
            (("scenario identity", "scenario profile"), "scenario"),
        )
        for needles, path in mappings:
            if any(needle in lowered for needle in needles):
                return path
        return "$"

    @staticmethod
    def _guidance(path: str) -> str:
        if path == "artifacts" or path.startswith("artifacts."):
            return (
                "Restore the curated artifact roles and repository-relative definition/schema "
                "paths, then validate again."
            )
        if path == "target" or path.startswith("target."):
            return (
                "Use the FP9/lung target expected by the selected timer baseline, or select a "
                "complete compatible artifact set."
            )
        if path.startswith("acceptance."):
            return "Restore the canonical ten-stage order required by the Level A workflow."
        if path == "sources" or path.startswith("sources."):
            return "Provide complete, uniquely identified evidence-source entries."
        if path == "limitations" or path.startswith("limitations."):
            return "Keep at least one non-empty interpretation limitation."
        if path == "$":
            return "Review the complete source JSON and the diagnostic, then restore the curated template if necessary."
        return "Correct the highlighted field according to its schema description and try again."

    @classmethod
    def _native_issue(
        cls,
        error: MehlissaCommandError,
        temporary_path: Path,
        changes: Mapping[str, object],
    ) -> dict[str, object]:
        diagnostic = error.stderr.strip() or error.stdout.strip() or "No diagnostic output"
        diagnostic = diagnostic.replace(str(temporary_path), "<scenario candidate>")
        match = re.search(r"\[(MEHLISSA-E\d{4})\]\s*(.*)", diagnostic, re.DOTALL)
        code = match.group(1) if match else "WBV-1900"
        message = match.group(2).strip() if match else diagnostic
        message = re.sub(r"^MEHLISSA failed:\s*", "", message)
        path = cls._diagnostic_path(message)
        narrowed = [changed for changed in changes if changed.startswith(f"{path}.")]
        if len(narrowed) == 1:
            path = narrowed[0]
        return {
            "severity": "error",
            "code": code,
            "path": path,
            "message": message,
            "guidance": cls._guidance(path),
        }

    @staticmethod
    def _warnings(document: Mapping[str, object], changes: Mapping[str, object]) -> list[dict[str, object]]:
        warnings: list[dict[str, object]] = []
        collector_count = cast(dict[str, object], document.get("run", {})).get("collector_count")
        if collector_count not in {1000, 10000}:
            warnings.append(
                {
                    "severity": "warning",
                    "code": "WBV-2001",
                    "path": "run.collector_count",
                    "message": "This collector population is outside the two published baseline cohorts.",
                    "guidance": "Treat it as an exploratory software run; no predictive interpolation law is claimed.",
                }
            )
        if "run.master_seed" in changes:
            warnings.append(
                {
                    "severity": "warning",
                    "code": "WBV-2002",
                    "path": "run.master_seed",
                    "message": "A changed seed selects a different stochastic realization.",
                    "guidance": "Use a declared replicate campaign to quantify stochastic uncertainty.",
                }
            )
        return warnings

    def _validate_candidate(
        self,
        document: dict[str, object],
        fields: list[dict[str, object]],
        changes: Mapping[str, object],
    ) -> dict[str, object]:
        field_by_path = {str(field["path"]): field for field in fields}
        structural = [
            issue
            for path, value in changes.items()
            if (field := field_by_path.get(path)) is not None
            if (issue := self._field_issue(field, value)) is not None
        ]
        encoded = (json.dumps(document, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
        digest = hashlib.sha256(encoded).hexdigest()
        self.workspace_root.mkdir(parents=True, exist_ok=True)
        temporary_path: Path | None = None
        authoritative_valid = False
        native_issue: dict[str, object] | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="wb", prefix=".mehlissa-validation-", suffix=".json",
                dir=self.workspace_root, delete=False
            ) as temporary:
                temporary.write(encoded)
                temporary_path = Path(temporary.name)
            try:
                self.client.validate_scenario(temporary_path)
                authoritative_valid = True
            except MehlissaCommandError as error:
                native_issue = self._native_issue(error, temporary_path, changes)
        finally:
            if temporary_path is not None:
                temporary_path.unlink(missing_ok=True)

        issues = structural
        if native_issue is not None:
            issues.append(native_issue)
        if authoritative_valid and structural:
            raise ScenarioWorkspaceError(
                "Workbench structural checks disagreed with the accepted validator"
            )
        issues.extend(self._warnings(document, changes) if authoritative_valid else [])
        error_count = sum(issue["severity"] == "error" for issue in issues)
        warning_count = sum(issue["severity"] == "warning" for issue in issues)
        scenario_value = document.get("scenario", {})
        scenario = scenario_value if isinstance(scenario_value, dict) else {}
        status = "VALID" if authoritative_valid else "INVALID"
        lines = [
            "MEHLISSA WORKBENCH VALIDATION SUMMARY",
            f"Status: {status}",
            "Validator: accepted `mehlissa scenario validate` command",
            f"Scenario: {scenario.get('title', 'Untitled')} ({scenario.get('id', 'unknown')})",
            f"Candidate SHA-256: {digest}",
            f"Errors: {error_count}; warnings: {warning_count}",
        ]
        for issue in issues:
            lines.append(
                f"- {str(issue['severity']).upper()} {issue['code']} [{issue['path']}]: "
                f"{issue['message']} Repair: {issue['guidance']}"
            )
        lines.append("Boundary: research software; not validated for clinical decisions.")
        return {
            "api_version": VALIDATION_API_VERSION,
            "valid": authoritative_valid,
            "run_allowed": authoritative_valid,
            "authoritative": True,
            "validator": "mehlissa scenario validate",
            "candidate_sha256": digest,
            "error_count": error_count,
            "warning_count": warning_count,
            "issues": issues,
            "summary_text": "\n".join(lines) + "\n",
        }

    def validate(self, identifier: str, changes: Mapping[str, object]) -> dict[str, object]:
        """Validate a complete in-memory candidate through the accepted CLI."""

        document, fields = self._candidate(identifier, changes)
        return self._validate_candidate(document, fields, changes)

    def save_as(
        self, identifier: str, filename: str, changes: Mapping[str, object]
    ) -> dict[str, object]:
        if not SAFE_FILENAME.fullmatch(filename):
            raise ScenarioWorkspaceError(
                "Filename must use letters, numbers, dots, underscores, or hyphens "
                "and end in .json"
            )
        document, fields = self._candidate(identifier, changes)

        self.workspace_root.mkdir(parents=True, exist_ok=True)
        destination = (self.workspace_root / filename).resolve()
        if destination.parent != self.workspace_root:
            raise ScenarioWorkspaceError("Save target escapes the scenario workspace")
        encoded = (json.dumps(document, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
        report = self._validate_candidate(document, fields, changes)
        if not report["valid"]:
            raise ScenarioValidationError(report)
        try:
            with destination.open("xb") as output:
                output.write(encoded)
        except FileExistsError as error:
            raise ScenarioConflictError(
                f"{filename} already exists; choose a new filename"
            ) from error
        return self.load(f"saved:{filename}")


class RunWorkspace:
    """Execute validated scenarios and one curated campaign with retained evidence."""

    def __init__(
        self,
        client: MehlissaClient,
        scenarios: ScenarioWorkspace,
        runs_root: Path | None = None,
    ):
        self.client = client
        self.scenarios = scenarios
        self.repository_root = _repository_root(client)
        configured = runs_root or Path("workbench-runs")
        if not configured.is_absolute():
            configured = self.repository_root / configured
        self.root = configured.expanduser().resolve()
        if self.repository_root not in self.root.parents:
            raise RunControlError(
                "The run workspace must be a dedicated directory inside the repository"
            )
        self._jobs: dict[str, dict[str, object]] = {}
        self._cancellations: dict[str, threading.Event] = {}
        self._lock = threading.RLock()

    @staticmethod
    def _now() -> str:
        return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

    def _relative(self, path: Path) -> str:
        resolved = path.expanduser().resolve()
        if resolved != self.repository_root and self.repository_root not in resolved.parents:
            raise RunControlError("A retained run artifact escaped the repository")
        return resolved.relative_to(self.repository_root).as_posix()

    @staticmethod
    def _bounded_output(stdout: str, stderr: str = "") -> tuple[str, list[str]]:
        combined = stdout
        if stderr:
            combined += ("\n" if combined else "") + stderr
        combined = combined[-MAX_LOG_CHARS:]
        lines = combined.splitlines()[-MAX_LOG_LINES:]
        return "\n".join(lines) + ("\n" if lines else ""), lines

    def _snapshot(self, job: Mapping[str, object]) -> dict[str, object]:
        return cast(dict[str, object], json.loads(json.dumps(job)))

    def _write_record(self, job: dict[str, object]) -> None:
        directory = self.repository_root / str(job["directory"])
        record = directory / "run-record.json"
        temporary = directory / ".run-record.json.tmp"
        temporary.write_text(
            json.dumps(job, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
        )
        temporary.replace(record)

    def _update(self, job_id: str, **changes: object) -> None:
        with self._lock:
            job = self._jobs[job_id]
            job.update(changes)
            job["updated_at"] = self._now()
            self._write_record(job)

    def _artifact(self, job_id: str, name: str, label: str, path: Path) -> None:
        if not path.is_file():
            return
        with self._lock:
            job = self._jobs[job_id]
            artifacts = cast(list[dict[str, str]], job["artifacts"])
            if any(artifact["name"] == name for artifact in artifacts):
                return
            artifacts.append(
                {"name": name, "label": label, "path": self._relative(path)}
            )
            self._write_record(job)

    def _new_job(
        self, kind: str, label: str, title: str, plan: dict[str, object]
    ) -> tuple[dict[str, object], Path, threading.Event]:
        if not SAFE_RUN_LABEL.fullmatch(label):
            raise RunControlError(
                "Output label must use 1-80 letters, numbers, dots, underscores, or hyphens"
            )
        self.root.mkdir(parents=True, exist_ok=True)
        for _ in range(10):
            job_id = token_urlsafe(9)
            directory = self.root / f"{label}-{job_id}"
            try:
                directory.mkdir()
                break
            except FileExistsError:
                continue
        else:
            raise RunControlError("Could not allocate a unique run workspace")
        now = self._now()
        job: dict[str, object] = {
            "api_version": RUN_API_VERSION,
            "id": job_id,
            "kind": kind,
            "title": title,
            "status": "queued",
            "stage": "Preparing retained inputs",
            "progress_percent": 5,
            "cancel_requested": False,
            "created_at": now,
            "updated_at": now,
            "completed_at": None,
            "directory": self._relative(directory),
            "plan": plan,
            "logs": ["Run accepted after explicit confirmation."],
            "artifacts": [],
            "error": None,
        }
        cancellation = threading.Event()
        with self._lock:
            self._jobs[job_id] = job
            self._cancellations[job_id] = cancellation
            self._write_record(job)
        self._artifact(job_id, "run-record", "Workbench run record", directory / "run-record.json")
        return job, directory, cancellation

    def overview(self) -> dict[str, object]:
        manifest = _load_json_object(self.repository_root / CAMPAIGN_PATH)
        design = cast(dict[str, object], manifest.get("design", {}))
        replicates = cast(dict[str, object], design.get("replicates", {}))
        sweeps = cast(list[dict[str, object]], design.get("sweeps", []))
        pairs = cast(list[dict[str, object]], design.get("paired_comparisons", []))
        run_count = int(replicates.get("count", 0))
        run_count += sum(
            int(sweep.get("replicates", 0))
            * len(cast(list[object], sweep.get("values", [])))
            for sweep in sweeps
        )
        run_count += sum(2 * int(pair.get("replicates", 0)) for pair in pairs)
        return {
            "api_version": RUN_API_VERSION,
            "output_root": self._relative(self.root),
            "retention": "Each start creates a unique directory; existing outputs are never overwritten.",
            "campaigns": [{
                "id": "campaign.fp9-collector-count",
                "title": cast(dict[str, object], manifest.get("campaign", {})).get("title"),
                "manifest": CAMPAIGN_PATH.as_posix(),
                "manifest_sha256": hashlib.sha256(
                    (self.repository_root / CAMPAIGN_PATH).read_bytes()
                ).hexdigest(),
                "run_count": run_count,
                "replicates": replicates,
                "sweeps": sweeps,
                "paired_comparisons": pairs,
                "limitations": manifest.get("limitations", []),
            }],
        }

    def start_scenario(
        self,
        identifier: str,
        changes: Mapping[str, object],
        label: str,
        confirmed: object,
    ) -> dict[str, object]:
        if confirmed is not True:
            raise RunControlError("Explicit run-plan confirmation is required")
        document, fields = self.scenarios._candidate(identifier, changes)
        validation = self.scenarios._validate_candidate(document, fields, changes)
        if not validation["valid"]:
            raise ScenarioValidationError(validation)
        scenario = cast(dict[str, object], document.get("scenario", {}))
        plan = {
            "source_id": identifier,
            "candidate_sha256": validation["candidate_sha256"],
            "run_count": 1,
            "master_seeds": [cast(dict[str, object], document.get("run", {})).get("master_seed")],
            "confirmation": "User confirmed the exact validated candidate before start.",
        }
        job, directory, cancellation = self._new_job(
            "scenario", label, str(scenario.get("title", "MEHLISSA scenario")), plan
        )
        input_path = directory / "scenario-input.json"
        try:
            input_path.write_text(
                json.dumps(document, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
            )
        except OSError as error:
            self._update(
                str(job["id"]), status="failed", stage="Input retention failed",
                progress_percent=100, completed_at=self._now(), error=str(error),
            )
            raise RunControlError("Could not retain the scenario input") from error
        self._artifact(str(job["id"]), "input", "Retained scenario input", input_path)
        thread = threading.Thread(
            target=self._run_scenario,
            args=(str(job["id"]), input_path, directory / "outputs", cancellation),
            daemon=True,
        )
        thread.start()
        return self.get(str(job["id"]))

    def start_campaign(
        self, identifier: str, label: str, confirmed: object
    ) -> dict[str, object]:
        if confirmed is not True:
            raise RunControlError("Explicit run-plan confirmation is required")
        if identifier != "campaign.fp9-collector-count":
            raise RunControlError("Unknown campaign")
        source = (self.repository_root / CAMPAIGN_PATH).resolve()
        self.client.validate_campaign(source)
        campaign = cast(dict[str, object], self.overview()["campaigns"][0])
        manifest = _load_json_object(source)
        seeds: list[object] = []
        design = cast(dict[str, object], manifest["design"])
        replicate_plan = cast(dict[str, object], design.get("replicates", {}))
        first_replicate_seed = int(replicate_plan.get("first_seed", 0))
        replicate_count = int(replicate_plan.get("count", 0))
        seeds.extend(range(first_replicate_seed, first_replicate_seed + replicate_count))
        for collection in ("sweeps", "paired_comparisons"):
            for entry in cast(list[dict[str, object]], design.get(collection, [])):
                first = int(entry.get("first_seed", 0)); repetitions = int(entry.get("replicates", 0))
                if collection == "sweeps":
                    value_count = len(cast(list[object], entry.get("values", [])))
                    seeds.extend(range(first, first + repetitions * value_count))
                else:
                    for replicate in range(repetitions):
                        seeds.extend((first + replicate, first + replicate))
        plan = {
            "campaign_id": identifier,
            "manifest": CAMPAIGN_PATH.as_posix(),
            "manifest_sha256": campaign["manifest_sha256"],
            "run_count": campaign["run_count"],
            "master_seeds": seeds,
            "replicates": campaign["replicates"],
            "sweeps": campaign["sweeps"],
            "paired_comparisons": campaign["paired_comparisons"],
            "confirmation": "User confirmed the curated six-run plan before start.",
        }
        job, directory, cancellation = self._new_job(
            "campaign", label, str(campaign["title"]), plan
        )
        input_path = directory / "campaign-manifest.json"
        try:
            input_path.write_bytes(source.read_bytes())
        except OSError as error:
            self._update(
                str(job["id"]), status="failed", stage="Input retention failed",
                progress_percent=100, completed_at=self._now(), error=str(error),
            )
            raise RunControlError("Could not retain the campaign manifest") from error
        self._artifact(str(job["id"]), "input", "Retained campaign manifest", input_path)
        thread = threading.Thread(
            target=self._run_campaign,
            args=(str(job["id"]), input_path, directory / "outputs", cancellation),
            daemon=True,
        )
        thread.start()
        return self.get(str(job["id"]))

    def _finish_output(self, job_id: str, directory: Path, stdout: str, stderr: str = "") -> None:
        bounded, lines = self._bounded_output(stdout, stderr)
        log_path = directory / "command-output.log"
        log_path.write_text(bounded, encoding="utf-8")
        self._artifact(job_id, "command-log", "Bounded command output", log_path)
        self._update(job_id, logs=lines or ["The command produced no terminal output."])

    def _run_scenario(
        self, job_id: str, input_path: Path, output: Path, cancellation: threading.Event
    ) -> None:
        directory = input_path.parent
        try:
            self._update(job_id, status="running", stage="Executing one validated scenario", progress_percent=35)
            execution = self.client.run_scenario(input_path, output, cancel_event=cancellation)
            self._update(
                job_id, status="collecting", stage="Collecting retained outputs",
                progress_percent=85,
            )
            self._finish_output(job_id, directory, execution.stdout)
            for name, label, path in (
                ("result", "Scenario result", execution.result),
                ("provenance", "Scenario provenance", execution.provenance),
                ("simulation-log", "Simulation log", execution.log),
                ("summary", "Scenario summary", execution.summary),
            ):
                self._artifact(job_id, name, label, path)
            report = self.client.report_result(execution.result, directory / "report")
            self._artifact(job_id, "report-html", "UX-3 HTML report", report.html)
            self._artifact(job_id, "report-result", "UX-3 retained result", report.result)
            self._update(job_id, status="completed", stage="Completed", progress_percent=100, completed_at=self._now())
        except MehlissaCancelledError as error:
            self._finish_output(job_id, directory, error.stdout, error.stderr)
            self._update(job_id, status="cancelled", stage="Cancelled; retained evidence preserved", progress_percent=100, completed_at=self._now())
        except Exception as error:  # preserve failures as inspectable run evidence
            stdout = error.stdout if isinstance(error, MehlissaCommandError) else ""
            stderr = error.stderr if isinstance(error, MehlissaCommandError) else str(error)
            self._finish_output(job_id, directory, stdout, stderr)
            self._update(job_id, status="failed", stage="Failed; retained evidence preserved", progress_percent=100, completed_at=self._now(), error=str(error))

    def _run_campaign(
        self, job_id: str, input_path: Path, output: Path, cancellation: threading.Event
    ) -> None:
        directory = self.repository_root / str(self._jobs[job_id]["directory"])
        try:
            self._update(job_id, status="running", stage="Executing six controlled derived runs", progress_percent=30)
            execution = self.client.run_campaign(input_path, output, cancel_event=cancellation)
            self._update(
                job_id, status="collecting", stage="Collecting campaign outputs",
                progress_percent=85,
            )
            self._finish_output(job_id, directory, execution.stdout)
            self._artifact(job_id, "result", "Campaign result", execution.result)
            self._artifact(job_id, "csv", "Campaign table", execution.csv)
            for index, path in enumerate(sorted((execution.directory / "manifests").glob("*.json"))):
                self._artifact(job_id, f"derived-manifest-{index + 1}", f"Derived run manifest {index + 1}", path)
            self._update(job_id, status="completed", stage=f"Completed {execution.derived_runs} derived runs", progress_percent=100, completed_at=self._now())
        except MehlissaCancelledError as error:
            self._finish_output(job_id, directory, error.stdout, error.stderr)
            self._update(job_id, status="cancelled", stage="Cancelled; retained campaign evidence preserved", progress_percent=100, completed_at=self._now())
        except Exception as error:  # preserve failures as inspectable run evidence
            stdout = error.stdout if isinstance(error, MehlissaCommandError) else ""
            stderr = error.stderr if isinstance(error, MehlissaCommandError) else str(error)
            self._finish_output(job_id, directory, stdout, stderr)
            self._update(job_id, status="failed", stage="Failed; retained campaign evidence preserved", progress_percent=100, completed_at=self._now(), error=str(error))

    def get(self, job_id: str) -> dict[str, object]:
        with self._lock:
            job = self._jobs.get(job_id)
            if job is None:
                raise RunNotFoundError("Unknown workbench run")
            return self._snapshot(job)

    def list(self) -> dict[str, object]:
        with self._lock:
            jobs = sorted(self._jobs.values(), key=lambda job: str(job["created_at"]), reverse=True)
            return {"api_version": RUN_API_VERSION, "jobs": [self._snapshot(job) for job in jobs]}

    def cancel(self, job_id: str) -> dict[str, object]:
        with self._lock:
            job = self._jobs.get(job_id)
            if job is None:
                raise RunNotFoundError("Unknown workbench run")
            if job["status"] not in {"queued", "running"}:
                raise RunControlError("Only a queued or running job can be cancelled")
            job["cancel_requested"] = True
            job["stage"] = "Cancellation requested"
            job["updated_at"] = self._now()
            self._cancellations[job_id].set()
            self._write_record(job)
            return self._snapshot(job)

    def artifact(self, job_id: str, name: str) -> tuple[Path, str]:
        job = self.get(job_id)
        artifact = next(
            (entry for entry in cast(list[dict[str, str]], job["artifacts"]) if entry["name"] == name),
            None,
        )
        if artifact is None:
            raise RunNotFoundError("Unknown retained artifact")
        path = (self.repository_root / artifact["path"]).resolve()
        directory = (self.repository_root / str(job["directory"])).resolve()
        if directory not in path.parents or not path.is_file():
            raise RunNotFoundError("Retained artifact is unavailable")
        content_type = {
            ".json": "application/json; charset=utf-8",
            ".html": "text/html; charset=utf-8",
            ".csv": "text/csv; charset=utf-8",
        }.get(path.suffix.lower(), "text/plain; charset=utf-8")
        return path, content_type

    def _result_path(self, job: Mapping[str, object]) -> Path:
        artifact = next(
            (
                entry
                for entry in cast(list[dict[str, str]], job["artifacts"])
                if entry["name"] == "result"
            ),
            None,
        )
        if artifact is None:
            raise RunControlError("The completed run has no authoritative result artifact")
        path = (self.repository_root / artifact["path"]).resolve()
        directory = (self.repository_root / str(job["directory"])).resolve()
        if directory not in path.parents or not path.is_file():
            raise RunControlError("The authoritative result artifact is unavailable")
        return path

    @staticmethod
    def _unavailable_dashboard(job: Mapping[str, object]) -> dict[str, object]:
        status = str(job["status"])
        return {
            "api_version": RESULT_API_VERSION,
            "available": False,
            "job_id": job["id"],
            "kind": job["kind"],
            "status": status,
            "observation_count": 0,
            "reason": (
                f"Results are excluded because this job is {status}; "
                "missing, failed, and cancelled runs are never observations."
            ),
        }

    def dashboard(self, job_id: str) -> dict[str, object]:
        """Project an accepted result reader into the graphical result contract."""
        job = self.get(job_id)
        if job["status"] != "completed":
            return self._unavailable_dashboard(job)
        path = self._result_path(job)
        if job["kind"] == "scenario":
            result = load_result(path)
            stages = [
                {**stage, "time_ms": float(stage["time_ns"]) / 1_000_000.0}
                for stage in result.runtime_stages
            ]
            return {
                "api_version": RESULT_API_VERSION,
                "available": True,
                "job_id": job_id,
                "kind": "scenario",
                "status": "completed",
                "reader": "mehlissa.load_result",
                "observation_count": 1,
                "summary": result.summary,
                "runtime_stages": stages,
                "analysis_cases": result.analysis_cases,
                "authoritative_artifact": "result",
                "ux3_report_artifact": "report-html",
            }

        result = load_campaign_result(path)
        metrics = ("detected", "assembled", "sensitivity", "specificity")
        grouped = []
        for name, runs in result.groups().items():
            grouped.append(
                {
                    "name": name,
                    "design": runs[0]["design"] if runs else None,
                    "run_count": len(runs),
                    "runs": runs,
                    "available_counts": {
                        metric: sum(run[metric] is not None for run in runs)
                        for metric in metrics
                    },
                }
            )
        differences = []
        for metric in metrics:
            for difference in result.paired_differences(metric):
                differences.append(
                    {**difference, "included": difference["difference"] is not None}
                )
        return {
            "api_version": RESULT_API_VERSION,
            "available": True,
            "job_id": job_id,
            "kind": "campaign",
            "status": "completed",
            "reader": "mehlissa.load_campaign_result",
            "observation_count": len(result.runs),
            "summary": {
                "campaign_id": result.document["campaign"]["id"],
                "title": result.document["campaign"]["title"],
                "run_count": len(result.runs),
                "group_count": len(grouped),
            },
            "groups": grouped,
            "paired_differences": differences,
            "limitations": result.document["limitations"],
            "authoritative_artifact": "result",
            "table_artifact": "csv",
        }

    def compare(self, left_id: str, right_id: str) -> dict[str, object]:
        """Compare two completed scenario results without inventing missing values."""
        if not left_id or not right_id or left_id == right_id:
            raise RunControlError("Comparison requires two different run ids")
        jobs = [self.get(left_id), self.get(right_id)]
        for job in jobs:
            if job["kind"] != "scenario" or job["status"] != "completed":
                raise RunControlError(
                    "Only two completed individual scenario runs can be compared; "
                    "failed, cancelled, running, and campaign jobs are excluded"
                )
        summaries = [load_result(self._result_path(job)).summary for job in jobs]
        metrics = ("collector_count", "detected", "assembled", "sensitivity", "specificity")
        rows: list[dict[str, object]] = []
        for metric in metrics:
            left = summaries[0][metric]
            right = summaries[1][metric]
            numeric = (
                left is not None
                and right is not None
                and not isinstance(left, bool)
                and not isinstance(right, bool)
                and isinstance(left, (int, float))
                and isinstance(right, (int, float))
            )
            rows.append(
                {
                    "metric": metric,
                    "left": left,
                    "right": right,
                    "difference": float(right) - float(left) if numeric else None,
                    "comparable": left is not None and right is not None,
                }
            )
        return {
            "api_version": RESULT_API_VERSION,
            "available": True,
            "reader": "mehlissa.load_result",
            "observation_count": 2,
            "left": {"job_id": left_id, "run_id": summaries[0]["run_id"]},
            "right": {"job_id": right_id, "run_id": summaries[1]["run_id"]},
            "rows": rows,
            "interpretation": "Differences are right minus left; missing values are not imputed.",
        }


class WorkbenchServer(ThreadingHTTPServer):
    """HTTP server carrying the bounded workbench session state."""

    daemon_threads = True

    def __init__(
        self,
        server_address: tuple[str, int],
        client: MehlissaClient,
        session_token: str | None = None,
        catalog_loader: Callable[[MehlissaClient], dict[str, object]] = discover_catalog,
        workspace_root: Path | None = None,
        runs_root: Path | None = None,
    ):
        host, _ = server_address
        if host not in LOOPBACK_HOSTS:
            raise ValueError("The UX-6.1 prototype may bind only to loopback")
        self.client = client
        self.session_token = session_token or token_urlsafe(32)
        self.catalog_loader = catalog_loader
        self.scenarios = ScenarioWorkspace(client, workspace_root)
        self.runs = RunWorkspace(client, self.scenarios, runs_root)
        super().__init__(server_address, WorkbenchRequestHandler)

    @property
    def url(self) -> str:
        """Browser URL carrying an ephemeral capability for this local session."""

        host, port = self.server_address
        return f"http://{host}:{port}/?session={self.session_token}"


class WorkbenchRequestHandler(BaseHTTPRequestHandler):
    """Serve embedded assets and capability-protected local APIs."""

    server_version = "MEHLISSAWorkbench/0.5"
    sys_version = ""

    @property
    def workbench(self) -> WorkbenchServer:
        return cast(WorkbenchServer, self.server)

    def log_message(self, format: str, *args: object) -> None:
        # Do not place the session capability or future file paths in terminal logs.
        del format, args

    def _host_is_allowed(self) -> bool:
        host_header = self.headers.get("Host", "")
        host = host_header.rsplit(":", 1)[0].lower()
        return host in LOOPBACK_HOSTS

    def _security_headers(self) -> None:
        self.send_header(
            "Content-Security-Policy",
            "default-src 'self'; script-src 'self'; style-src 'self'; "
            "img-src 'self'; connect-src 'self'; frame-src 'self'; base-uri 'none'; "
            "form-action 'none'; frame-ancestors 'none'",
        )
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Language", "en")
        self.send_header("Referrer-Policy", "no-referrer")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("X-Frame-Options", "DENY")
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header("Permissions-Policy", "camera=(), microphone=(), geolocation=()")

    def _send_bytes(self, status: HTTPStatus, content_type: str, body: bytes) -> None:
        self.send_response(status)
        self._security_headers()
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _send_json(self, status: HTTPStatus, document: dict[str, object]) -> None:
        body = json.dumps(document, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        self._send_bytes(status, "application/json; charset=utf-8", body)

    def _session_is_valid(self) -> bool:
        supplied = self.headers.get("X-MEHLISSA-Session", "")
        return hmac.compare_digest(supplied, self.workbench.session_token)

    def _require_session(self) -> bool:
        if self._session_is_valid():
            return True
        self._send_json(HTTPStatus.FORBIDDEN, {"error": "session_required"})
        return False

    def do_GET(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        if not self._host_is_allowed():
            self._send_json(HTTPStatus.BAD_REQUEST, {"error": "invalid_host"})
            return

        path = urlsplit(self.path).path
        if path == "/api/catalog":
            if not self._require_session():
                return
            try:
                payload = self.workbench.catalog_loader(self.workbench.client)
            except (CatalogFormatError, MehlissaCommandError, OSError):
                self._send_json(
                    HTTPStatus.BAD_GATEWAY,
                    {
                        "error": "catalog_unavailable",
                        "detail": "MEHLISSA discovery failed; inspect the local terminal",
                    },
                )
                return
            self._send_json(HTTPStatus.OK, payload)
            return
        if path == "/api/scenarios":
            if not self._require_session():
                return
            try:
                self._send_json(HTTPStatus.OK, self.workbench.scenarios.overview())
            except ScenarioWorkspaceError as error:
                self._send_json(
                    HTTPStatus.BAD_GATEWAY,
                    {"error": "scenario_workspace_unavailable", "detail": str(error)},
                )
            return
        if path == "/api/scenario":
            if not self._require_session():
                return
            query = urlsplit(self.path).query
            parameters = parse_qs(query)
            identifier = parameters.get("id", [""])[0]
            try:
                self._send_json(HTTPStatus.OK, self.workbench.scenarios.load(identifier))
            except ScenarioWorkspaceError as error:
                self._send_json(
                    HTTPStatus.BAD_REQUEST, {"error": "scenario_unavailable", "detail": str(error)}
                )
            return
        if path == "/api/run-plans":
            if not self._require_session():
                return
            try:
                self._send_json(HTTPStatus.OK, self.workbench.runs.overview())
            except (RunControlError, ScenarioWorkspaceError, MehlissaCommandError, OSError) as error:
                self._send_json(HTTPStatus.BAD_GATEWAY, {"error": "run_plans_unavailable", "detail": str(error)})
            return
        if path == "/api/runs":
            if not self._require_session():
                return
            self._send_json(HTTPStatus.OK, self.workbench.runs.list())
            return
        if path == "/api/run":
            if not self._require_session():
                return
            identifier = parse_qs(urlsplit(self.path).query).get("id", [""])[0]
            try:
                self._send_json(HTTPStatus.OK, self.workbench.runs.get(identifier))
            except RunNotFoundError as error:
                self._send_json(HTTPStatus.NOT_FOUND, {"error": "run_not_found", "detail": str(error)})
            return
        if path == "/api/run/dashboard":
            if not self._require_session():
                return
            identifier = parse_qs(urlsplit(self.path).query).get("id", [""])[0]
            try:
                self._send_json(HTTPStatus.OK, self.workbench.runs.dashboard(identifier))
            except RunNotFoundError as error:
                self._send_json(HTTPStatus.NOT_FOUND, {"error": "run_not_found", "detail": str(error)})
            except (RunControlError, ValueError, OSError) as error:
                self._send_json(HTTPStatus.CONFLICT, {"error": "result_unavailable", "detail": str(error)})
            return
        if path == "/api/run/artifact":
            if not self._require_session():
                return
            parameters = parse_qs(urlsplit(self.path).query)
            identifier = parameters.get("id", [""])[0]
            name = parameters.get("name", [""])[0]
            try:
                artifact, content_type = self.workbench.runs.artifact(identifier, name)
                self._send_bytes(HTTPStatus.OK, content_type, artifact.read_bytes())
            except (RunNotFoundError, OSError) as error:
                self._send_json(HTTPStatus.NOT_FOUND, {"error": "artifact_not_found", "detail": str(error)})
            return

        static = STATIC_FILES.get(path)
        if static is None:
            self._send_json(HTTPStatus.NOT_FOUND, {"error": "not_found"})
            return
        asset, content_type = static
        body = files("mehlissa_workbench.static").joinpath(asset).read_bytes()
        self._send_bytes(HTTPStatus.OK, content_type, body)

    def _read_json_request(self) -> dict[str, object]:
        content_type = self.headers.get("Content-Type", "").partition(";")[0].strip().lower()
        if content_type != "application/json":
            raise ScenarioWorkspaceError("Content-Type must be application/json")
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError as error:
            raise ScenarioWorkspaceError("Invalid Content-Length") from error
        if not 0 < length <= MAX_REQUEST_BYTES:
            raise ScenarioWorkspaceError("Request body is empty or too large")
        try:
            document = json.loads(self.rfile.read(length))
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            raise ScenarioWorkspaceError("Request body is not valid JSON") from error
        if not isinstance(document, dict):
            raise ScenarioWorkspaceError("Request body must be a JSON object")
        return cast(dict[str, object], document)

    def _reject_state_changing_method(self, allow: str = "GET, POST") -> None:
        self.send_response(HTTPStatus.METHOD_NOT_ALLOWED)
        self._security_headers()
        self.send_header("Allow", allow)
        self.send_header("Content-Length", "0")
        self.end_headers()

    def do_POST(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        if not self._host_is_allowed():
            self._send_json(HTTPStatus.BAD_REQUEST, {"error": "invalid_host"})
            return
        path = urlsplit(self.path).path
        if path not in {
            "/api/scenario/validate", "/api/scenario/save",
            "/api/run/scenario", "/api/run/campaign", "/api/run/cancel",
            "/api/run/compare",
        }:
            self._send_json(HTTPStatus.NOT_FOUND, {"error": "not_found"})
            return
        if not self._require_session():
            return
        try:
            request = self._read_json_request()
            if path == "/api/run/compare":
                left = request.get("left_id")
                right = request.get("right_id")
                if not isinstance(left, str) or not isinstance(right, str):
                    raise RunControlError("Comparison requires left_id and right_id")
                self._send_json(HTTPStatus.OK, self.workbench.runs.compare(left, right))
                return
            if path == "/api/run/cancel":
                job_id = request.get("id")
                if not isinstance(job_id, str):
                    raise RunControlError("Cancellation requires a run id")
                self._send_json(HTTPStatus.ACCEPTED, self.workbench.runs.cancel(job_id))
                return
            if path == "/api/run/campaign":
                identifier = request.get("campaign_id")
                label = request.get("output_label")
                if not isinstance(identifier, str) or not isinstance(label, str):
                    raise RunControlError("Campaign start requires campaign_id and output_label")
                result = self.workbench.runs.start_campaign(identifier, label, request.get("confirmed"))
                self._send_json(HTTPStatus.ACCEPTED, result)
                return
            identifier = request.get("source_id")
            changes = request.get("changes")
            if not isinstance(identifier, str) or not isinstance(changes, dict):
                raise ScenarioWorkspaceError(
                    "Scenario request requires source_id and changes"
                )
            if path == "/api/run/scenario":
                label = request.get("output_label")
                if not isinstance(label, str):
                    raise RunControlError("Scenario start requires an output_label")
                result = self.workbench.runs.start_scenario(
                    identifier, changes, label, request.get("confirmed")
                )
                self._send_json(HTTPStatus.ACCEPTED, result)
                return
            if path == "/api/scenario/validate":
                result = self.workbench.scenarios.validate(identifier, changes)
                self._send_json(HTTPStatus.OK, result)
                return
            filename = request.get("filename")
            if not isinstance(filename, str):
                raise ScenarioWorkspaceError("Save request requires a filename")
            result = self.workbench.scenarios.save_as(identifier, filename, changes)
        except ScenarioConflictError as error:
            self._send_json(HTTPStatus.CONFLICT, {"error": "file_exists", "detail": str(error)})
            return
        except ScenarioValidationError as error:
            self._send_json(
                HTTPStatus.UNPROCESSABLE_ENTITY,
                {
                    "error": "scenario_invalid",
                    "detail": str(error),
                    "validation": error.report,
                },
            )
            return
        except RunNotFoundError as error:
            self._send_json(HTTPStatus.NOT_FOUND, {"error": "run_not_found", "detail": str(error)})
            return
        except RunControlError as error:
            self._send_json(HTTPStatus.BAD_REQUEST, {"error": "run_rejected", "detail": str(error)})
            return
        except (ScenarioWorkspaceError, MehlissaCommandError, OSError) as error:
            self._send_json(
                HTTPStatus.BAD_REQUEST,
                {"error": "operation_rejected", "detail": str(error)},
            )
            return
        self._send_json(HTTPStatus.CREATED, result)

    def do_PUT(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_PATCH(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_DELETE(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_OPTIONS(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()


def create_server(
    client: MehlissaClient,
    host: str = "127.0.0.1",
    port: int = 8765,
    session_token: str | None = None,
    catalog_loader: Callable[[MehlissaClient], dict[str, object]] = discover_catalog,
    workspace_root: Path | None = None,
    runs_root: Path | None = None,
) -> WorkbenchServer:
    """Create, but do not start, a loopback-only workbench server."""

    if not 0 <= port <= 65535:
        raise ValueError("port must be between 0 and 65535")
    return WorkbenchServer(
        (host, port), client, session_token, catalog_loader, workspace_root, runs_root
    )
