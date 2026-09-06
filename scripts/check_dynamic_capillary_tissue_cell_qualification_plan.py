# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the prospective DCCQ-1.1 design and compatibility audit."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator


ROOT = Path(__file__).resolve().parents[1]
DOCUMENT = ROOT / "data/qualification/dynamic-capillary-tissue-cell-qualification-plan-v1.json"
SCHEMA = ROOT / "data/schemas/dynamic-capillary-tissue-cell-qualification-plan/1.0.0.schema.json"
TEXT_SUFFIXES = {".cpp", ".hpp", ".json", ".md", ".txt", ".csv"}

EXPECTED_BASELINES = {
    "DCCQ-BASE-01": ("software-verified-synthetic", "retain-as-regression-not-qualification-candidate"),
    "DCCQ-BASE-02": ("software-verified-synthetic", "must-not-be-relabelled-dynamic-or-biological"),
    "DCCQ-BASE-03": ("analytically-verified-synthetic", "retain-as-analytical-limiting-case"),
    "DCCQ-BASE-04": (
        "computationally-qualified-published-average-cell",
        "blocked-from-dynamic-coupling-until-new-prospective-unit-and-input-protocol",
    ),
    "DCCQ-BASE-05": (
        "literature-candidate-not-artifact-qualified",
        "reassess-in-dccq-1.2-with-alternatives",
    ),
    "DCCQ-BASE-06": (
        "analytically-and-numerically-verified-synthetic",
        "retain-as-transport-verification-baseline",
    ),
}
EXPECTED_LEDGER = [
    "blood_free",
    "endothelium_free",
    "interstitium_free",
    "receptor_bound",
    "internalized",
    "cleared_or_degraded",
    "outlet",
]
EXPECTED_GATE_CATEGORIES = [
    "identity-and-units",
    "mass-balance",
    "limiting-cases",
    "numerical-convergence",
    "causal-timing",
    "uncertainty-and-sensitivity",
    "independent-reference",
    "claim-and-review",
]


class PlanError(ValueError):
    """Raised when the prospective DCCQ plan is inconsistent."""


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
    schema = load_json(root / SCHEMA.relative_to(ROOT))
    for error in sorted(
        Draft202012Validator(schema).iter_errors(document), key=lambda item: list(item.path)
    ):
        result.append(f"schema: {'/'.join(map(str, error.path))}: {error.message}")
    if result:
        return result

    program = document["program"]
    if (
        program["id"] != "DCCQ-1"
        or program["full_name"] != "Dynamic Capillary-Tissue-Cell Qualification"
        or program["abbreviation"] != "DCCQ"
        or program["completed_increment"] != "DCCQ-1.1"
    ):
        result.append("DCCQ identity, expanded name, or completed increment changed")
    if program["clinical_use"] or "before-dynamic-implementation" not in program["status"]:
        result.append("DCCQ-1.1 must remain a prospective non-clinical pre-implementation design")

    intended = document["bounded_intended_use"]
    if intended["evidence_status"] != "not-yet-tested":
        result.append("DCCQ-1.1 cannot claim an evaluated dynamic result")
    exclusions = " ".join(intended["exclusions"]).lower()
    for phrase in ("patient-specific", "diagnosis", "treatment", "clinical validity"):
        if phrase not in exclusions:
            result.append(f"bounded intended use is missing exclusion: {phrase}")

    baselines = document["baseline_components"]
    if {entry["id"] for entry in baselines} != set(EXPECTED_BASELINES):
        result.append("baseline component inventory changed")
    for entry in baselines:
        expected = EXPECTED_BASELINES.get(entry["id"])
        if expected is not None and (entry["evidence_level"], entry["disposition"]) != expected:
            result.append(f"baseline evidence role changed: {entry['id']}")
    snapshot = next((item for item in baselines if item["id"] == "DCCQ-BASE-02"), None)
    if snapshot is None or not any("non-consuming" in gap for gap in snapshot["blocking_gaps"]):
        result.append("snapshot non-consumption blocker is missing")
    cd95 = next((item for item in baselines if item["id"] == "DCCQ-BASE-04"), None)
    if cd95 is None or not any("unresolved-model-native" in gap for gap in cd95["blocking_gaps"]):
        result.append("qualified CD95 unit blocker is missing")

    contract = document["cross_layer_state_contract"]
    if contract["ownership_ledger"] != EXPECTED_LEDGER:
        result.append("complete ligand ownership ledger changed")
    if "same chemical or biological entity" not in contract["ligand_identity_policy"]:
        result.append("cross-layer biochemical identity policy weakened")
    if (
        "unresolved-model-native" not in contract["unit_policy"]
        or "cannot enter an SI coupling" not in contract["unit_policy"]
    ):
        result.append("unresolved-unit fail-closed rule weakened")
    shortcuts = " ".join(contract["forbidden_shortcuts"]).lower()
    for phrase in ("non-consuming snapshots", "chemically different", "unresolved", "validation outcomes", "biological qualification"):
        if phrase not in shortcuts:
            result.append(f"required forbidden-shortcut safeguard absent: {phrase}")
    if "cumulative_inlet" not in contract["balance_equation"] or "cumulative_outlet" not in contract["balance_equation"]:
        result.append("open-system balance equation is incomplete")
    if "same-step feedback" not in contract["feedback_requirement"]:
        result.append("causal feedback guard is missing")

    gates = document["planned_gates"]
    if [gate["id"] for gate in gates] != [f"DCCQ-G{index}" for index in range(1, 9)]:
        result.append("DCCQ gate order or identity changed")
    if [gate["category"] for gate in gates] != EXPECTED_GATE_CATEGORIES:
        result.append("DCCQ gate category set changed")
    if any(gate["current_state"] != "BLOCKED" for gate in gates):
        result.append("an unexecuted DCCQ gate is incorrectly marked unblocked")
    if next(gate for gate in gates if gate["id"] == "DCCQ-G2")["pass_rule_status"] != "hard-invariant":
        result.append("mass balance is no longer a hard invariant")

    hierarchy = document["evidence_hierarchy"]
    if [level["level"] for level in hierarchy] != [1, 2, 3, 4, 5]:
        result.append("evidence hierarchy must retain ordered levels 1 through 5")
    if "source-disjoint" not in hierarchy[3]["name"] or "clinical" not in hierarchy[4]["name"]:
        result.append("independent and clinical evidence levels were conflated")

    uncertainty = document["uncertainty_plan"]
    if set(uncertainty) != {"numerical", "parameter", "structural", "observational", "synchronization", "identifiability"}:
        result.append("DCCQ uncertainty class inventory changed")
    if "same-step feedback" not in uncertainty["synchronization"]:
        result.append("synchronization uncertainty lacks the causality safeguard")

    increments = document["increments"]
    if [item["id"] for item in increments] != [f"DCCQ-1.{index}" for index in range(1, 8)]:
        result.append("DCCQ increment order or identity changed")
    if [item["status"] for item in increments] != ["COMPLETE", "NEXT"] + ["PLANNED"] * 5:
        result.append("DCCQ-1.1 completion or sole DCCQ-1.2 next-step status changed")

    controls = document["negative_controls"]
    if [control["id"] for control in controls] != [f"DCCQ-NC-{index:02d}" for index in range(1, 11)]:
        result.append("DCCQ negative-control matrix changed")

    assets = document["frozen_baseline_assets"]
    asset_paths = [asset["path"] for asset in assets]
    if len(asset_paths) != len(set(asset_paths)):
        result.append("frozen baseline asset paths must be unique")
    for asset in assets:
        path = repository_path(root, asset["path"], result)
        if path is None or not path.is_file():
            result.append(f"frozen baseline asset missing: {asset['path']}")
        elif canonical_sha256(path) != asset["sha256"]:
            result.append(f"frozen baseline asset hash changed: {asset['path']}")

    policy = document["governance_and_claim_policy"]
    if "does not implement or qualify" not in policy["allowed_current_claim"]:
        result.append("current DCCQ-1.1 claim is no longer bounded to design")
    forbidden = " ".join(policy["forbidden_claims"]).lower()
    for phrase in ("dynamic", "si-compatible", "biological qualification", "clinical validity", "licenses"):
        if phrase not in forbidden:
            result.append(f"required claim safeguard absent: {phrase}")
    if "committed before validation outputs" not in policy["outcome_blinding"]:
        result.append("prospective outcome-blinding rule weakened")

    criteria = " ".join(document["completion_criteria"])
    for phrase in ("User Guide", "Requirements Matrix", "shareable PDF", "DCCQ-1.2"):
        if phrase not in criteria:
            result.append(f"completion documentation or handoff requirement missing: {phrase}")
    return result


def validate(document: dict[str, Any], root: Path = ROOT) -> None:
    failures = errors(document, root)
    if failures:
        raise PlanError("\n".join(failures))


def main() -> int:
    try:
        validate(load_json(DOCUMENT))
    except (OSError, json.JSONDecodeError, PlanError) as error:
        print(f"DCCQ-1.1 qualification plan: FAILED\n{error}")
        return 1
    print(
        "DCCQ-1.1 qualification plan: ok "
        "(6 audited baselines, 7-stage programme, 8 blocked gates, "
        "10 negative controls, DCCQ-1.2 next)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
