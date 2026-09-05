# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the pre-import BCQ-1.1 biological cell-model selection."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
REGISTER = ROOT / "data/qualification/biological-cell-model-candidate-register-v1.json"
SCHEMA = ROOT / "data/schemas/biological-cell-model-candidate-register/1.0.0.schema.json"

EXPECTED_ARTIFACTS = {
    "BIOMD0000000523": {
        "source_commit": "8605e43f8e2fd364f122d579341891c0058ef778",
        "sha256": "2afe6758ab396038e71fcb1716fefcfec67656b8bd0bfb3da8d4e1eda9524ff4",
        "reactions": 13,
    },
    "BIOMD0000000524": {
        "source_commit": "d091308a14fb4301a4a2b1b567ea874484bb97e6",
        "sha256": "4bf4a5bcda5b43a551bcdda09fca91a5e777d2c5db1eafcb17dcb6f1574221bc",
        "reactions": 13,
    },
    "BIOMD0000000256": {
        "source_commit": "94d83bb48c2f166ff2e09fcf83b7932c3f17cf7e",
        "sha256": "b54079ae3f4f003a0476b2b676ab53091654677334d611169739af322022c29e",
        "reactions": 56,
    },
}


class CandidateRegisterError(ValueError):
    """Raised when the BCQ-1.1 register is inconsistent or overclaims."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def errors(document: dict[str, Any]) -> list[str]:
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    result = []
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"{location}: {error.message}")
    if result:
        return result

    criteria = [item["id"] for item in document["criteria"]]
    if len(criteria) != len(set(criteria)):
        result.append("selection criteria must be unique")

    candidates = document["candidates"]
    ids = [item["id"] for item in candidates]
    ranks = [item["rank"] for item in candidates]
    if ids != ["BCQ-SRC-001", "BCQ-SRC-002", "BCQ-SRC-003", "BCQ-SRC-004"]:
        result.append("candidate identities or ordering changed")
    if ranks != [1, 2, 3, 4]:
        result.append("candidate ranks must remain contiguous and ordered")
    expected_decisions = [
        "selected-model-family",
        "fallback-downstream-model",
        "later-capillary-endothelial-program",
        "conceptual-benchmark-only",
    ]
    if [item["decision"] for item in candidates] != expected_decisions:
        result.append("candidate decisions changed or more than one candidate was selected")

    for candidate in candidates:
        score_keys = set(candidate["scores"])
        if score_keys != set(criteria):
            result.append(f"{candidate['id']}: score dimensions differ from criteria")
        if candidate["total_score"] != sum(candidate["scores"].values()):
            result.append(f"{candidate['id']}: total score does not equal component sum")

    selected = candidates[0]
    if selected["decision"] != "selected-model-family":
        result.append("BCQ-SRC-001 must remain the selected model family")
    if any(candidate["total_score"] >= selected["total_score"] for candidate in candidates[1:]):
        result.append("selected candidate must have the unique highest score")
    selected_accessions = [item["accession"] for item in selected["artifacts"]]
    if selected_accessions != ["BIOMD0000000523", "BIOMD0000000524"]:
        result.append("the selected minimal CD95-HeLa and wild-type HeLa artifact pair changed")
    companion_text = " ".join(selected["structural_companions"]).lower()
    if not all(phrase in companion_text for phrase in ("biomd0000000525", "biomd0000000526", "not independent")):
        result.append("the larger same-publication variant must remain a non-independent structural companion")
    article_rights = selected["publication"]["article_rights"].lower()
    if "do not redistribute" not in article_rights or "separate rights" not in article_rights:
        result.append("the model licence must not be used to relicense the article or experimental data")

    observed_artifacts: dict[str, dict[str, Any]] = {}
    for candidate in candidates:
        for artifact in candidate["artifacts"]:
            accession = artifact["accession"]
            if accession in observed_artifacts:
                result.append(f"duplicate artifact accession: {accession}")
            observed_artifacts[accession] = artifact
    if set(observed_artifacts) != set(EXPECTED_ARTIFACTS):
        result.append("audited artifact set changed")
    for accession, expected in EXPECTED_ARTIFACTS.items():
        artifact = observed_artifacts.get(accession, {})
        for field, value in expected.items():
            if artifact.get(field) != value:
                result.append(f"{accession}: frozen {field} changed")
        if artifact.get("license") != "CC0-1.0":
            result.append(f"{accession}: model-artifact licence must remain CC0-1.0")

    kallenberger = json.dumps(selected).lower()
    for phrase in (
        "average cell",
        "not a new experiment",
        "article and experimental-data rights",
        "not establish normal endothelial",
    ):
        if phrase not in kallenberger:
            result.append(f"selected-candidate boundary is missing: {phrase}")

    decision = document["decision"]
    if decision["selected_candidate_id"] != selected["id"]:
        result.append("decision and ranked selection disagree")
    if set(decision["selected_artifact_accessions"]) != set(selected_accessions):
        result.append("decision and selected artifacts disagree")
    forbidden_claims = json.dumps(decision["not_yet_authorized"]).lower()
    for phrase in ("bundling", "biological qualification", "clinical validity"):
        if phrase not in forbidden_claims:
            result.append(f"not-yet-authorized boundary is missing: {phrase}")

    endothelial = candidates[2]
    if endothelial["artifacts"]:
        result.append("the unreleased endothelial BioModels submission cannot be treated as an audited artifact")
    if "not released" not in " ".join(endothelial["structural_companions"]).lower():
        result.append("the endothelial BioModels release blocker must remain explicit")

    full_text = json.dumps(document).lower()
    for phrase in (
        "no external model or experimental data are bundled",
        "no fitting is permitted",
        "not released",
        "no stable, content-hashed public sbml package",
    ):
        if phrase not in full_text:
            result.append(f"required BCQ-1.1 safeguard is absent: {phrase}")
    return result


def validate(path: Path = REGISTER) -> dict[str, Any]:
    document = load_json(path)
    found = errors(document)
    if found:
        raise CandidateRegisterError("\n".join(found))
    return document


def main() -> int:
    try:
        document = validate()
        print(
            "BCQ-1.1 biological cell-model selection: ok "
            f"({len(document['candidates'])} candidates, "
            f"{len(document['candidates'][0]['artifacts'])} selected artifacts, "
            "no model imported, no biological qualification claimed)"
        )
    except (OSError, json.JSONDecodeError, CandidateRegisterError) as error:
        print(f"BCQ-1.1 biological cell-model selection: FAILED\n{error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
