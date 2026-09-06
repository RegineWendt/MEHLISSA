# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the frozen BCQ-1.4 through BCQ-1.7 qualification protocol."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator


ROOT = Path(__file__).resolve().parents[1]
DOCUMENT = ROOT / "data/qualification/biological-cell-model-integration-protocol-v1.json"
SCHEMA = ROOT / "data/schemas/biological-cell-model-integration-protocol/1.0.0.schema.json"
TEXT_SUFFIXES = {".cpp", ".hpp", ".json", ".md", ".txt", ".csv"}
SPECIES = [
    "CD95", "FADD", "DISC", "p55free", "DISCp55", "p30", "p43", "p18",
    "p18inactive", "Bid", "tBid", "PrNES_mCherry", "PrNES", "mCherry",
    "PrER_mGFP", "PrER", "mGFP", "CD95L",
]
PARAMETERS = [
    "kon_FADD", "koff_FADD", "kDISC", "kD216", "kD374trans_p55",
    "kD374trans_p43", "kdiss_p18", "kBid", "kD374probe", "KDR", "KDL",
]


class ProtocolError(ValueError):
    """Raised when the prospective BCQ integration protocol is inconsistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def canonical_sha256(path: Path) -> str:
    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES:
        data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(data).hexdigest()


def repository_path(root: Path, relative: str, result: list[str]) -> Path | None:
    path = (root / relative).resolve()
    try:
        path.relative_to(root.resolve())
    except ValueError:
        result.append(f"path escapes repository: {relative}")
        return None
    return path


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    result: list[str] = []
    schema_path = root / "data/schemas/biological-cell-model-integration-protocol/1.0.0.schema.json"
    schema = load_json(schema_path)
    for error in sorted(Draft202012Validator(schema).iter_errors(document), key=lambda item: list(item.path)):
        result.append(f"schema: {'/'.join(map(str, error.path))}: {error.message}")
    if result:
        return result

    protocol = document["protocol"]
    if protocol["id"] != "BCQ-1.4-1.7" or protocol["status"] != "prospective-protocol-frozen-before-qualification-archive":
        result.append("protocol identity or prospective status changed")
    if protocol["parameter_fitting"] or protocol["clinical_use"]:
        result.append("protocol must forbid fitting and clinical use")
    if (not protocol["pre_protocol_development_runs"] or
            "no generated development trajectory is a qualification result"
            not in protocol["pre_protocol_development_runs_role"]):
        result.append("pre-protocol development runs are not disclosed as non-evidence")

    parent = document["parent_result"]
    parent_path = repository_path(root, parent["path"], result)
    if parent_path is None or not parent_path.is_file() or canonical_sha256(parent_path) != parent["sha256"]:
        result.append("BCQ-1.3 parent result identity changed")
    elif load_json(parent_path)["decision"]["biological_qualification"] != "NOT_ESTABLISHED":
        result.append("parent biological-qualification boundary changed")

    implementation = document["implementation"]
    if implementation["hash_policy"] != "SHA-256 over Git-canonical LF bytes":
        result.append("implementation hash policy changed")
    for role in ("typed_adapter", "source_equations", "qualification_runner"):
        asset = implementation[role]
        path = repository_path(root, asset["path"], result)
        if path is None or not path.is_file() or canonical_sha256(path) != asset["sha256"]:
            result.append(f"frozen implementation identity changed: {role}")

    mapping = document["typed_mapping"]
    if [case["accession"] for case in mapping["source_cases"]] != ["BIOMD0000000523", "BIOMD0000000524"]:
        result.append("typed mapping source cases changed")
    if any(case["license"] != "CC0-1.0" for case in mapping["source_cases"]):
        result.append("source licence boundary changed")
    if mapping["states"] != SPECIES or mapping["primary_observables"] != ["PrER_mGFP", "PrNES_mCherry", "p43", "p18"]:
        result.append("state or observable mapping changed")
    stimulus = mapping["stimulus"]
    if stimulus != {"species": "CD95L", "value": 16.6, "unit": "unresolved-model-native", "override_allowed": False}:
        result.append("typed stimulus mapping changed")
    if mapping["time_unit"] != "unresolved-model-native" or mapping["state_unit"] != "unresolved-model-native" or mapping["si_conversion_allowed"] or mapping["source_parameters_mutable_through_adapter"]:
        result.append("unit or no-refit adapter boundary changed")

    execution = document["execution"]
    if execution["time_grid"] != {"start": 0.0, "end": 240.0, "interval": 0.25, "points": 961, "unit": "unresolved-model-native"}:
        result.append("execution grid changed")
    if execution["mehlissa_primary"]["maximum_internal_step"] != 0.01 or execution["mehlissa_primary"]["expected_internal_steps"] != 24000:
        result.append("primary RK4 settings changed")
    if execution["mehlissa_tightened"]["maximum_internal_step"] != 0.005 or execution["mehlissa_tightened"]["expected_internal_steps"] != 48000:
        result.append("tightened RK4 settings changed")
    if execution["parameter_refitting"] != "forbidden":
        result.append("parameter refitting is not forbidden")

    cross = document["cross_engine_acceptance"]
    for reference in cross["reference_files"]:
        path = repository_path(root, reference["path"], result)
        if path is None or not path.is_file() or canonical_sha256(path) != reference["sha256"]:
            result.append(f"COPASI reference identity changed: {reference['accession']}")
    if cross["all_state_tolerance"]["absolute_floor"] != 1e-7 or cross["all_state_tolerance"]["scale_relative"] != 1e-7:
        result.append("cross-engine tolerance changed")
    if cross["mehlissa_convergence_tolerance"]["absolute_floor"] != 1e-9 or cross["mehlissa_convergence_tolerance"]["scale_relative"] != 1e-8:
        result.append("MEHLISSA convergence tolerance changed")

    structural = document["structural_sensitivity"]
    if structural["trajectory_execution_in_this_protocol"] or "not independent" not in structural["purpose"]:
        result.append("structural-companion scope changed")
    if [(item["accession"], item["species"], item["reactions"], item["global_parameters"], item["assignment_rules"]) for item in structural["companions"]] != [
        ("BIOMD0000000525", 18, 19, 15, 1), ("BIOMD0000000526", 18, 19, 15, 1)
    ]:
        result.append("525/526 structural identities changed")
    if structural["minimal_structure"] != {"species": 18, "reactions": 13, "global_parameters": 12, "assignment_rules": 1}:
        result.append("minimal model structure changed")
    if structural["expected_difference"]["additional_reactions"] != 6 or structural["expected_difference"]["additional_global_parameters"] != 3:
        result.append("expected structural difference changed")

    population = document["population_and_uncertainty"]
    if population["precondition_met"] or population["decision"] != "retain-explicit-average-cell-limit":
        result.append("unsupported population import was enabled")
    if population["local_sensitivity"]["parameters"] != PARAMETERS or population["local_sensitivity"]["central_relative_steps"] != [0.01, 0.005]:
        result.append("local-sensitivity design changed")

    review = document["review_and_claim_policy"]
    if review["external_human_reviewer_attestation_available"]:
        result.append("external reviewer attestation is incorrectly marked available")
    required_forbidden = ("publication curves", "population ensemble", "biological qualification", "clinical validity", "independent experimental validation")
    joined_forbidden = " ".join(review["forbidden_claims"]).lower()
    for phrase in required_forbidden:
        if phrase not in joined_forbidden:
            result.append(f"required claim safeguard absent: {phrase}")
    if "may close with explicit BLOCKED findings" not in review["completion_rule"]:
        result.append("blocked-result completion rule changed")

    archive = document["result_archive"]
    if not archive["root"].startswith("results/bcq1/kallenberger-mehlissa/") or archive["external_sbml_or_article_data_allowed"]:
        result.append("archive scope changed")
    if len(archive["required_entries"]) != 13:
        result.append("archive entry contract changed")
    if len(document["negative_controls"]) != 12:
        result.append("negative-control matrix changed")
    return result


def validate(document: dict[str, Any], root: Path = ROOT) -> None:
    failures = errors(document, root)
    if failures:
        raise ProtocolError("\n".join(failures))


def main() -> int:
    try:
        validate(load_json(DOCUMENT))
    except (OSError, json.JSONDecodeError, ProtocolError) as error:
        print(f"BCQ-1.4-1.7 integration protocol: FAILED\n{error}")
        return 1
    print("BCQ-1.4-1.7 integration protocol: ok (typed no-refit adapter, two RK4 grids, 525/526 structural-only scope, average-cell decision, 12 negative controls)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
