# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the prospective MRSQ-1.1 scenario and evidence selection."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
REGISTER = ROOT / "data/qualification/medical-reference-scenario-candidate-register-v1.json"
SCHEMA = ROOT / "data/schemas/medical-reference-scenario-candidate-register/1.0.0.schema.json"

EXPECTED_CRITERIA = [
    "injection-to-measurement-path",
    "external-observability",
    "time-resolved-numeric-data",
    "measurement-definition",
    "access-readiness",
    "reuse-rights",
    "source-separation",
    "current-platform-reuse",
    "bounded-extension",
]
EXPECTED_CANDIDATES = [
    ("MRSQ-SCN-001", 1, "selected-for-prospective-protocol"),
    ("MRSQ-SCN-002", 2, "pulmonary-imaging-fallback"),
    ("MRSQ-SCN-003", 3, "hemodynamic-endpoint-comparator"),
    ("MRSQ-SCN-004", 4, "non-injection-measurement-resource"),
    ("MRSQ-SCN-005", 5, "internal-software-demonstrator-only"),
]
EXPECTED_SOURCES = {
    "MRSQ-SRC-001": ("doi:10.1038/s41597-026-08157-4", "published-2026-08-24"),
    "MRSQ-SRC-002": ("doi:10.57967/hf/9560", "dataset-card-observed-2026-09-06-exact-revision-not-yet-frozen"),
    "MRSQ-SRC-003": ("doi:10.70883/UYAG3430", "V1"),
    "MRSQ-SRC-004": ("https://github.com/DEPICT-RH/Multimodal-HC", "main-observed-2026-09-06-exact-commit-not-yet-frozen"),
    "MRSQ-SRC-005": ("doi:10.1183/13993003.02058-2021", "published-2022"),
    "MRSQ-SRC-006": ("https://physionet.org/content/cardiac-output/1.0.0/", "1.0.0"),
    "MRSQ-SRC-007": ("doi:10.13026/z865-eb23", "1.0.0"),
    "MRSQ-SRC-008": ("docs/m7/M7_GATE_REVIEW.md", "M7-accepted-2026-09-02"),
}


class ScenarioSelectionError(ValueError):
    """Raised when MRSQ-1.1 is inconsistent or exceeds its evidence boundary."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    result: list[str] = []
    schema = load_json(root / SCHEMA.relative_to(ROOT))
    validator = Draft202012Validator(schema, format_checker=FormatChecker())
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"schema: {location}: {error.message}")
    if result:
        return result

    screen = document["screen"]
    if screen["raw_participant_data_accessed"]:
        result.append("MRSQ-1.1 must remain before raw participant-data access")
    if screen["candidate_validation_outcomes_inspected"]:
        result.append("MRSQ-1.1 must remain before candidate validation-outcome inspection")
    boundary = screen["selection_boundary"].lower()
    for phrase in (
        "no raw participant data",
        "no fdg equations",
        "no participant-specific",
        "clinical qualification",
    ):
        if phrase not in boundary:
            result.append(f"selection boundary is missing: {phrase}")

    criteria = [item["id"] for item in document["criteria"]]
    if criteria != EXPECTED_CRITERIA:
        result.append("scenario-selection criteria or their prospective order changed")

    candidates = document["candidates"]
    observed = [(item["id"], item["rank"], item["disposition"]) for item in candidates]
    if observed != EXPECTED_CANDIDATES:
        result.append("candidate identities, ranks, or dispositions changed")
    for candidate in candidates:
        if list(candidate["scores"]) != EXPECTED_CRITERIA:
            result.append(f"{candidate['id']}: score dimensions or order changed")
        if candidate["total_score"] != sum(candidate["scores"].values()):
            result.append(f"{candidate['id']}: total score does not equal component sum")

    selected = candidates[0]
    if any(item["total_score"] >= selected["total_score"] for item in candidates[1:]):
        result.append("selected scenario must retain the unique highest score")
    if selected["eligibility"] != "eligible-after-protocol-and-ingress-freeze":
        result.append("selected scenario must remain conditional on protocol and ingress freeze")
    for criterion in (
        "injection-to-measurement-path",
        "external-observability",
        "time-resolved-numeric-data",
        "measurement-definition",
        "reuse-rights",
        "source-separation",
        "bounded-extension",
    ):
        if selected["scores"][criterion] != 3:
            result.append(f"selected scenario no longer satisfies essential criterion {criterion}")

    selected_text = json.dumps(selected, ensure_ascii=True).lower()
    for phrase in (
        "18f-fdg",
        "70-minute",
        "one hundred healthy adults",
        "80 participants",
        "2027",
        "time-activity curves",
        "image-derived input functions",
        "cc-by-4.0",
        "302 gb",
        "cohort-level",
        "not a glucose-tracer uptake model",
        "not a pet acquisition model",
    ):
        if phrase not in selected_text:
            result.append(f"selected scenario boundary is missing: {phrase}")

    sources = {
        source["id"]: source
        for candidate in candidates
        for source in candidate["sources"]
    }
    if sorted(sources) != [f"MRSQ-SRC-{number:03d}" for number in range(1, 9)]:
        result.append("source identities must be unique and contiguous MRSQ-SRC-001 through 008")
    for source_id, (locator, version) in EXPECTED_SOURCES.items():
        source = sources.get(source_id, {})
        if source.get("persistent_locator") != locator:
            result.append(f"{source_id}: persistent locator changed")
        if source.get("version") != version:
            result.append(f"{source_id}: screened version changed")
    if "cc-by-4.0" not in sources["MRSQ-SRC-002"]["rights"].lower():
        result.append("Hugging Face data rights are no longer explicit")
    if "cc-by-4.0" not in sources["MRSQ-SRC-003"]["rights"].lower():
        result.append("PublicnEUro data rights are no longer explicit")
    repository_rights = sources["MRSQ-SRC-004"]["rights"].lower()
    if "noassertion" not in repository_rights:
        result.append("processing-repository source-code rights must remain NOASSERTION")
    if "do not copy code" not in sources["MRSQ-SRC-004"]["access"].lower():
        result.append("unlicensed processing repository lost its no-copy safeguard")
    local_reference = root / sources["MRSQ-SRC-008"]["persistent_locator"]
    if not local_reference.is_file():
        result.append("local M7 software-demonstrator reference is missing")

    if candidates[1]["eligibility"] != "fallback-needs-data-request":
        result.append("DCE-MRI alternative cannot become eligible without reusable numeric data")
    if candidates[2]["eligibility"] != "endpoint-only":
        result.append("thermodilution resource must remain endpoint-only")
    if candidates[3]["eligibility"] != "ineligible-no-injection":
        result.append("HeartCycle must remain ineligible for an injection-to-measurement scenario")
    if candidates[4]["eligibility"] != "ineligible-no-external-observation":
        result.append("M7 cannot be treated as its own external observation")

    programme = document["programme"]
    increments = programme["increments"]
    if [item["id"] for item in increments] != [f"MRSQ-1.{number}" for number in range(1, 8)]:
        result.append("MRSQ increments must remain ordered MRSQ-1.1 through MRSQ-1.7")
    if [item["status"] for item in increments] != [
        "complete",
        "next",
        "planned",
        "planned",
        "planned",
        "planned",
        "externally-blocked",
    ]:
        result.append("MRSQ increment statuses changed before MRSQ-1.2")
    exit_claim = programme["exit_claim"].lower()
    for phrase in ("without validation refitting", "disease", "clinical validity"):
        if phrase not in exit_claim:
            result.append(f"programme exit claim is missing: {phrase}")

    decision = document["decision"]
    if decision["selected_candidate_id"] != selected["id"]:
        result.append("decision and ranked candidate disagree")
    if decision["access_decision"] != "metadata-qualified-data-ingress-not-yet-authorized":
        result.append("participant-data ingress cannot be authorized by MRSQ-1.1")
    not_authorized = " ".join(decision["not_authorized"]).lower()
    for phrase in (
        "participant files",
        "independent validation",
        "unlicensed repository code",
        "vegf-a165a",
        "clinical qualification",
        "after viewing outcomes",
    ):
        if phrase not in not_authorized:
            result.append(f"not-authorized boundary is missing: {phrase}")
    governance = decision["governance_boundary"].lower()
    for phrase in ("outside git", "direct identifiers", "retention and deletion", "institutional review"):
        if phrase not in governance:
            result.append(f"governance boundary is missing: {phrase}")
    if "does not implement, calibrate, validate, or clinically qualify" not in decision["allowed_current_claim"]:
        result.append("allowed claim must remain limited to prospective selection")
    return result


def validate(path: Path = REGISTER, root: Path = ROOT) -> dict[str, Any]:
    document = load_json(path)
    failures = errors(document, root)
    if failures:
        raise ScenarioSelectionError("\n".join(failures))
    return document


def main() -> int:
    try:
        document = validate()
    except (OSError, json.JSONDecodeError, ScenarioSelectionError) as error:
        print(f"MRSQ-1.1 scenario selection: FAILED\n{error}")
        return 1
    print(
        "MRSQ-1.1 scenario selection: ok "
        f"({len(document['candidates'])} ranked scenarios, 8 source records, "
        "healthy-adult dynamic total-body 18F-FDG PET selected, no data ingress, "
        "MRSQ-1.2 next)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
