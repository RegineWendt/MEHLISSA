# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the DCCQ-1.2 target, artifact, data, and licence screen."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
REGISTER = ROOT / "data/qualification/dynamic-capillary-tissue-cell-evidence-candidate-register-v1.json"
SCHEMA = ROOT / "data/schemas/dynamic-capillary-tissue-cell-evidence-candidate-register/1.0.0.schema.json"
TEXT_SUFFIXES = {".cpp", ".hpp", ".json", ".md", ".txt", ".csv"}

EXPECTED_CRITERIA = [
    "biological-fit",
    "artifact-identity",
    "unit-bridge",
    "time-resolved-data",
    "reuse-rights",
    "source-disjoint-evidence",
    "pulmonary-context",
    "bounded-integration",
]
EXPECTED_CANDIDATES = [
    ("DCCQ-SRC-001", 1, "selected-target-and-evidence-family"),
    ("DCCQ-SRC-002", 2, "mechanistic-structural-alternative"),
    ("DCCQ-SRC-003", 3, "licensed-platform-alternative"),
    ("DCCQ-SRC-004", 4, "cell-solver-regression-only"),
]
EXPECTED_ARTIFACTS = {
    "DCCQ-ART-002": (
        "bed7b23b3f6edbed6376e818b6307eccd65ea38e",
        "2f5b245fdc11791c084b6c4e3cd3d9459c9b4de36f8f8e10a59762a7fa6c0007",
        "NOASSERTION",
    ),
    "DCCQ-ART-003": (
        "bed7b23b3f6edbed6376e818b6307eccd65ea38e",
        "f093433e313aba387d8a52a594bef045b34c1503f227985db5d04d94b47d58fd",
        "NOASSERTION",
    ),
    "DCCQ-ART-004": (
        "bed7b23b3f6edbed6376e818b6307eccd65ea38e",
        "0b04b0ccb76ee34417405170031894d3e46da4f82937731fc635799b5ab19c47",
        "NOASSERTION",
    ),
    "DCCQ-ART-005": (
        "bed7b23b3f6edbed6376e818b6307eccd65ea38e",
        "22827653281ca0f9f4795beca4d63c8465334e07cdab8ec182fe497760f9793e",
        "NOASSERTION",
    ),
    "DCCQ-ART-006": (
        "bed7b23b3f6edbed6376e818b6307eccd65ea38e",
        "d420d264568970f8fe205e8025a20b0319457172fd591bb1b4ae40ff70d52766",
        "NOASSERTION",
    ),
    "DCCQ-ART-008": (
        "d5768332027f3f3bef0e2575c257e05194e7e77f",
        None,
        "Apache-2.0",
    ),
    "DCCQ-ART-009": (
        "0b0566aa19557dbbcf7de6a1fd042861b6ae0b05",
        None,
        "Apache-2.0",
    ),
    "DCCQ-ART-010": (
        "BCQ-1.7",
        "f71cd44cdd38d173f17d99033b698635702088317b65c7603d8e50e66cdebda7",
        "CC0-1.0",
    ),
}


class EvidenceCandidateError(ValueError):
    """Raised when the DCCQ-1.2 screen is inconsistent or overclaims."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def canonical_sha256(path: Path) -> str:
    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES:
        data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(data).hexdigest()


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
    parent = root / screen["parent_plan"]
    if not parent.is_file():
        result.append("parent DCCQ-1.1 plan is missing")
    elif canonical_sha256(parent) != screen["parent_plan_sha256"]:
        result.append("parent DCCQ-1.1 plan hash changed")
    boundary = screen["selection_boundary"].lower()
    for phrase in (
        "no third-party code or data are bundled",
        "no dynamic equations",
        "no model output",
        "no biological",
        "clinical qualification",
    ):
        if phrase not in boundary:
            result.append(f"selection boundary is missing: {phrase}")

    criteria = [item["id"] for item in document["criteria"]]
    if criteria != EXPECTED_CRITERIA:
        result.append("candidate criteria or their prospective order changed")

    candidates = document["candidates"]
    observed = [(item["id"], item["rank"], item["decision"]) for item in candidates]
    if observed != EXPECTED_CANDIDATES:
        result.append("candidate identities, ranks, or decisions changed")
    for candidate in candidates:
        if list(candidate["scores"]) != EXPECTED_CRITERIA:
            result.append(f"{candidate['id']}: score dimensions or order changed")
        if candidate["total_score"] != sum(candidate["scores"].values()):
            result.append(f"{candidate['id']}: total score does not equal component sum")
    selected = candidates[0]
    if any(item["total_score"] >= selected["total_score"] for item in candidates[1:]):
        result.append("selected target must retain the unique highest score")

    selected_text = json.dumps(selected).lower()
    for phrase in (
        "vegf-a165a",
        "vegfr2",
        "nrp1",
        "primary human umbilical vein endothelial cells",
        "not lung-specific",
        "281-state",
        "no explicit license",
        "condition-matched, source-disjoint huvec time series",
    ):
        if phrase not in selected_text:
            result.append(f"selected target boundary is missing: {phrase}")
    if selected["unit_assessment"]["si_bridge_status"] != "convertible-but-not-yet-frozen":
        result.append("selected source units cannot be presented as already SI-frozen")
    unit_text = json.dumps(selected["unit_assessment"]).lower()
    for phrase in ("molecules per represented cell", "seconds", "femtolitres", "square micrometres", "monomer-versus-dimer"):
        if phrase not in unit_text:
            result.append(f"selected source unit audit is missing: {phrase}")

    artifacts = {artifact["id"]: artifact for item in candidates for artifact in item["artifacts"]}
    if sorted(artifacts) != [f"DCCQ-ART-{number:03d}" for number in range(1, 11)]:
        result.append("audited artifact identities must be unique and contiguous DCCQ-ART-001 through 010")
    for artifact_id, (revision, digest, licence) in EXPECTED_ARTIFACTS.items():
        artifact = artifacts.get(artifact_id, {})
        if artifact.get("revision") != revision:
            result.append(f"{artifact_id}: frozen revision changed")
        if artifact.get("sha256") != digest:
            result.append(f"{artifact_id}: frozen sha256 changed")
        if artifact.get("licence") != licence:
            result.append(f"{artifact_id}: frozen licence decision changed")
    for artifact_id in ("DCCQ-ART-002", "DCCQ-ART-003", "DCCQ-ART-004", "DCCQ-ART-005", "DCCQ-ART-006"):
        artifact = artifacts[artifact_id]
        if "do-not-bundle" not in artifact["reuse_decision"] and "no-copy-or-redistribution" not in artifact["reuse_decision"]:
            result.append(f"{artifact_id}: unlicensed repository reuse safeguard weakened")
    local_reference = root / artifacts["DCCQ-ART-010"]["locator"]
    if not local_reference.is_file():
        result.append("qualified BCQ local reference is missing")
    elif canonical_sha256(local_reference) != artifacts["DCCQ-ART-010"]["sha256"]:
        result.append("qualified BCQ local reference hash changed")

    evidence = document["evidence_sources"]
    if [item["id"] for item in evidence] != [f"DCCQ-EVID-{number:03d}" for number in range(1, 6)]:
        result.append("evidence source identities or order changed")
    if [item["role"] for item in evidence[:3]] != [
        "mechanism-selection-and-calibration",
        "within-study-structural-check",
        "baseline-parameterization-and-context",
    ]:
        result.append("same-family HUVEC sources must remain calibration or context evidence")
    independent = evidence[3:]
    if any(item["source_family"] == evidence[0]["source_family"] for item in independent):
        result.append("source-disjoint observations overlap the selected source family")
    if "non-endothelial-context-mismatch" not in independent[0]["eligibility"]:
        result.append("Peach source cannot be treated as condition-matched HUVEC validation")
    if "directional-only" not in independent[1]["eligibility"]:
        result.append("single-timepoint Zhao evidence cannot be treated as a dynamic series")

    decision = document["decision"]
    if decision["selected_candidate_id"] != selected["id"]:
        result.append("decision and ranked candidate disagree")
    not_authorized = " ".join(decision["not_yet_authorized"]).lower()
    for phrase in ("unlicensed", "full 281-state", "after viewing", "pulmonary", "dccq-g1", "dccq-g7"):
        if phrase not in not_authorized:
            result.append(f"not-yet-authorized boundary is missing: {phrase}")
    if not decision["gate_implications"]["DCCQ-G1"].startswith("PARTIAL"):
        result.append("DCCQ-G1 must remain partial before the SI mapping is frozen")
    if not decision["gate_implications"]["DCCQ-G7"].startswith("BLOCKED"):
        result.append("DCCQ-G7 must remain blocked before the no-refit protocol is frozen and run")
    if "does not implement or qualify" not in decision["allowed_current_claim"]:
        result.append("allowed claim must remain limited to source selection")
    return result


def validate(path: Path = REGISTER, root: Path = ROOT) -> dict[str, Any]:
    document = load_json(path)
    failures = errors(document, root)
    if failures:
        raise EvidenceCandidateError("\n".join(failures))
    return document


def main() -> int:
    try:
        document = validate()
    except (OSError, json.JSONDecodeError, EvidenceCandidateError) as error:
        print(f"DCCQ-1.2 evidence candidate screen: FAILED\n{error}")
        return 1
    print(
        "DCCQ-1.2 evidence candidate screen: ok "
        f"({len(document['candidates'])} ranked targets, "
        f"{len(document['evidence_sources'])} evidence sources, "
        "VEGF-A165a/VEGFR2/NRP1 HUVEC family selected, DCCQ-1.3 next)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
