# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Loopback-only host for the MEHLISSA graphical research workbench."""

from __future__ import annotations

from dataclasses import asdict, dataclass
import hmac
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from importlib.resources import files
import json
from pathlib import Path
import re
from secrets import token_urlsafe
import tempfile
from typing import Callable, Mapping, Sequence, cast
from urllib.parse import parse_qs, urlsplit

from mehlissa import MehlissaClient, MehlissaCommandError


CATALOG_API_VERSION = "1.0.0"
WORKSPACE_API_VERSION = "1.0.0"
LOOPBACK_HOSTS = frozenset({"127.0.0.1", "localhost"})
SCENARIO_SCHEMA_PATH = Path(
    "data/schemas/fingerprinting-scenario-profile/1.0.0.schema.json"
)
MAX_REQUEST_BYTES = 1_000_000
SAFE_FILENAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,119}\.json$")
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
        "workbench_increment": "UX-6.2",
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

    def save_as(
        self, identifier: str, filename: str, changes: Mapping[str, object]
    ) -> dict[str, object]:
        if not SAFE_FILENAME.fullmatch(filename):
            raise ScenarioWorkspaceError(
                "Filename must use letters, numbers, dots, underscores, or hyphens "
                "and end in .json"
            )
        loaded = self.load(identifier)
        document = cast(dict[str, object], loaded["document"])
        editable = set(cast(list[str], loaded["editable_paths"]))
        unsupported = sorted(set(changes) - editable)
        if unsupported:
            raise ScenarioWorkspaceError(
                f"Changes contain unsupported fields: {', '.join(unsupported)}"
            )
        for path, value in changes.items():
            _set_path(document, path, value)

        self.workspace_root.mkdir(parents=True, exist_ok=True)
        destination = (self.workspace_root / filename).resolve()
        if destination.parent != self.workspace_root:
            raise ScenarioWorkspaceError("Save target escapes the scenario workspace")
        encoded = (json.dumps(document, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
        temporary_path: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="wb",
                prefix=".mehlissa-",
                suffix=".json",
                dir=self.workspace_root,
                delete=False,
            ) as temporary:
                temporary.write(encoded)
                temporary_path = Path(temporary.name)
            self.client.validate_scenario(temporary_path)
            try:
                with destination.open("xb") as output:
                    output.write(encoded)
            except FileExistsError as error:
                raise ScenarioConflictError(
                    f"{filename} already exists; choose a new filename"
                ) from error
        finally:
            if temporary_path is not None:
                temporary_path.unlink(missing_ok=True)
        return self.load(f"saved:{filename}")


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
    ):
        host, _ = server_address
        if host not in LOOPBACK_HOSTS:
            raise ValueError("The UX-6.1 prototype may bind only to loopback")
        self.client = client
        self.session_token = session_token or token_urlsafe(32)
        self.catalog_loader = catalog_loader
        self.scenarios = ScenarioWorkspace(client, workspace_root)
        super().__init__(server_address, WorkbenchRequestHandler)

    @property
    def url(self) -> str:
        """Browser URL carrying an ephemeral capability for this local session."""

        host, port = self.server_address
        return f"http://{host}:{port}/?session={self.session_token}"


class WorkbenchRequestHandler(BaseHTTPRequestHandler):
    """Serve embedded assets and capability-protected local APIs."""

    server_version = "MEHLISSAWorkbench/0.1"
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
            "img-src 'self'; connect-src 'self'; base-uri 'none'; "
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
        if urlsplit(self.path).path != "/api/scenario/save":
            self._send_json(HTTPStatus.NOT_FOUND, {"error": "not_found"})
            return
        if not self._require_session():
            return
        try:
            request = self._read_json_request()
            identifier = request.get("source_id")
            filename = request.get("filename")
            changes = request.get("changes")
            if (
                not isinstance(identifier, str)
                or not isinstance(filename, str)
                or not isinstance(changes, dict)
            ):
                raise ScenarioWorkspaceError(
                    "Save request requires source_id, filename, and changes"
                )
            result = self.workbench.scenarios.save_as(identifier, filename, changes)
        except ScenarioConflictError as error:
            self._send_json(HTTPStatus.CONFLICT, {"error": "file_exists", "detail": str(error)})
            return
        except (ScenarioWorkspaceError, MehlissaCommandError, OSError) as error:
            self._send_json(
                HTTPStatus.BAD_REQUEST,
                {"error": "save_rejected", "detail": str(error)},
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
) -> WorkbenchServer:
    """Create, but do not start, a loopback-only workbench server."""

    if not 0 <= port <= 65535:
        raise ValueError("port must be between 0 and 65535")
    return WorkbenchServer((host, port), client, session_token, catalog_loader, workspace_root)
