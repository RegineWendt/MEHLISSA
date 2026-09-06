# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the prospective DCCQ-1.3 equation and evaluation protocol."""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "data/qualification/dynamic-capillary-tissue-cell-protocol-v1.json"
SCHEMA = ROOT / "data/schemas/dynamic-capillary-tissue-cell-protocol/1.0.0.schema.json"
OWNERS = [
    "blood_free",
    "endothelium_free",
    "interstitium_free",
    "receptor_bound",
    "internalized",
    "cleared_or_degraded",
    "outlet",
]
PARAMETERS = {
    "blood_volume": (1.0e-11, "m3"),
    "endothelium_volume": (1.0e-12, "m3"),
    "interstitium_volume": (1.0e-12, "m3"),
    "blood_to_endothelium": (1.0e-3, "s^-1"),
    "endothelium_to_blood": (2.0e-4, "s^-1"),
    "endothelium_to_interstitium": (5.0e-4, "s^-1"),
    "interstitium_to_endothelium": (1.0e-4, "s^-1"),
    "blood_outlet": (2.0e-5, "s^-1"),
    "interstitial_clearance": (1.0e-5, "s^-1"),
    "association": (1.0e4, "m3 mol^-1 s^-1"),
    "dissociation": (1.0e-3, "s^-1"),
    "internalization": (6.9e-4, "s^-1"),
    "degradation": (2.3e-4, "s^-1"),
    "receptor_capacity": (8.136641429151849e-21, "mol cell^-1"),
    "feedback_gain": (0.25, "1"),
}
NEGATIVE_CONTROLS = {
    "reject-mismatched-ligand-identity",
    "reject-nonpositive-or-nonfinite-si-quantity",
    "reject-double-owned-initial-amount",
    "reject-unbalanced-or-negative-integrated-state",
    "reject-same-step-feedback",
    "reject-post-output-protocol-mutation",
    "reject-validation-refitting",
    "reject-biological-claim-from-numerical-gates",
    "reject-snapshot-as-dynamic-coupling",
    "reject-invented-independent-distributions",
}


class ProtocolError(ValueError):
    """Raised when the DCCQ-1.3 protocol is not prospective or self-consistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def canonical_sha256(path: Path) -> str:
    data = path.read_bytes().replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(data).hexdigest()


def close(left: float, right: float) -> bool:
    return math.isclose(left, right, rel_tol=1.0e-13, abs_tol=1.0e-30)


def repository_asset(root: Path, item: dict[str, Any], found: list[str]) -> None:
    path = (root / item["path"]).resolve()
    try:
        path.relative_to(root.resolve())
    except ValueError:
        found.append(f"asset escapes repository: {item['path']}")
        return
    if not path.is_file():
        found.append(f"asset is missing: {item['path']}")
    elif canonical_sha256(path) != item["sha256"]:
        found.append(f"asset hash changed: {item['path']}")


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    found: list[str] = []
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        found.append(f"{location}: {error.message}")
    if found:
        return found

    repository_asset(root, document["parent_source_screen"], found)
    for item in document["frozen_implementation"]:
        repository_asset(root, item, found)

    target = document["target"]
    if target.get("ligand_id") != "mehlissa.bio.human-vegfa165a-homodimer.v1":
        found.append("frozen VEGF-A165a identity changed")
    if target.get("receptor_id") != "UniProtKB:KDR_HUMAN":
        found.append("frozen human VEGFR2 identity changed")
    if target.get("coreceptor_id") != "UniProtKB:NRP1_HUMAN":
        found.append("frozen human NRP1 identity changed")
    if target.get("cell_context") != "primary-HUVEC" or target.get("represented_cells") != 1:
        found.append("frozen one-cell primary-HUVEC context changed")

    si = document["si_mapping"]
    expected_si = {
        "avogadro_per_mol": 6.02214076e23,
        "vegfa165a_homodimer_molar_mass_kg_per_mol": 44.0,
        "femtolitre_to_m3": 1.0e-18,
        "square_micrometre_to_m2": 1.0e-12,
        "source_extracellular_volume_m3_per_cell": 1.0e-11,
        "source_rab4_5_volume_m3_per_cell": 1.125e-17,
        "source_rab11_volume_m3_per_cell": 3.75e-18,
        "source_cell_surface_m2_per_cell": 1.0e-9,
        "source_rab4_5_surface_m2_per_cell": 9.5e-10,
        "source_rab11_surface_m2_per_cell": 3.5e-10,
        "source_vegfr2_receptors_per_cell": 4900,
        "source_vegfr2_capacity_mol_per_cell": 4900 / 6.02214076e23,
        "source_stimulus_molecules_per_cell": 6843182,
        "source_stimulus_amount_mol_per_cell": 6843182 / 6.02214076e23,
        "association_si_m3_per_mol_s": 1.0e4,
        "dissociation_s_inverse": 1.0e-3,
        "liganded_internalization_s_inverse": 6.9e-4,
        "internalized_degradation_s_inverse": 2.3e-4,
    }
    for name, expected in expected_si.items():
        value = si.get(name)
        if not isinstance(value, (int, float)) or not close(float(value), expected):
            found.append(f"SI mapping changed: {name}")
    expected_concentration = si["source_stimulus_amount_mol_per_cell"] / si[
        "source_extracellular_volume_m3_per_cell"
    ]
    if not close(si.get("source_stimulus_concentration_mol_per_m3", -1.0), expected_concentration):
        found.append("source stimulus amount/volume conversion is inconsistent")

    contract = document["state_contract"]
    if contract.get("owners") != OWNERS:
        found.append("exclusive seven-owner ledger changed")
    if set(contract.get("cumulative_ledger", [])) != {"initial_amount", "cumulative_inlet"}:
        found.append("cumulative inlet ledger changed")
    equation_states = [item.get("state") for item in document["equations"]]
    if equation_states != [*OWNERS[:-1], "outlet", "cumulative_inlet", "association"]:
        found.append("reduced equation state order changed")

    parameters = document["reference_parameters"]
    if len(parameters) != len(PARAMETERS):
        found.append("reference parameter count changed")
    parameter_map = {item.get("id"): item for item in parameters}
    if len(parameter_map) != len(parameters) or set(parameter_map) != set(PARAMETERS):
        found.append("reference parameter identities are incomplete or duplicated")
    else:
        for name, (expected_value, expected_unit) in PARAMETERS.items():
            item = parameter_map[name]
            if not close(float(item["value"]), expected_value) or item["unit"] != expected_unit:
                found.append(f"reference parameter value or unit changed: {name}")
            if not item.get("provenance") or not item.get("uncertainty_role"):
                found.append(f"reference parameter provenance is incomplete: {name}")

    initial = document["initial_state"]
    if not close(initial.get("declared_initial_amount_mol", -1.0),
                 initial.get("blood_free_mol", -2.0)):
        found.append("initial amount is not owned exclusively by blood_free")
    if initial.get("all_other_owners_mol") != 0.0 or initial.get("inlet_rate_mol_per_s") != 0.0:
        found.append("frozen pulse initial condition changed")

    sync = document["synchronization"]
    if sync.get("method") != "fixed-step-classical-rk4-with-discrete-delayed-feedback":
        found.append("frozen integration method changed")
    if sync.get("time_step_refinement_s") != [8.0, 4.0, 2.0, 1.0, 0.5]:
        found.append("time-step refinement series changed")
    if sync.get("coupling_refinement_s") != [120.0, 60.0, 30.0, 15.0]:
        found.append("coupling refinement series changed")
    if "n+1" not in sync.get("feedback_rule", ""):
        found.append("feedback is no longer explicitly delayed")

    evaluation = document["evaluation"]
    if evaluation.get("validation_refitting") is not False:
        found.append("validation refitting must remain forbidden")
    if evaluation.get("deterministic_replay_absolute_tolerance") != 0.0:
        found.append("deterministic replay is not exact")
    if evaluation.get("causal_order") != [
        "blood_departure", "interstitial_arrival", "receptor_binding",
        "feedback_scheduled", "feedback_applied",
    ]:
        found.append("causal order changed")
    if evaluation.get("local_sensitivity_fraction") != 0.1:
        found.append("local sensitivity fraction changed")

    if set(document["negative_controls"]) != NEGATIVE_CONTROLS:
        found.append("required negative controls changed")
    uncertainty_text = json.dumps(document["uncertainty_policy"], sort_keys=True).lower()
    if "do not synthesize distributions" not in uncertainty_text or "blocked" not in uncertainty_text:
        found.append("uncertainty policy no longer fails closed on missing joint distributions")
    claims = " ".join(document["forbidden_claims"]).lower()
    for term in ("independent huvec", "patient", "clinical", "human review"):
        if term not in claims:
            found.append(f"forbidden claim boundary is missing: {term}")
    return found


def main() -> int:
    document = load_json(PROTOCOL)
    found = errors(document)
    if found:
        print("DCCQ-1.3 protocol check: FAIL")
        for item in found:
            print(f"- {item}")
        return 1
    print(
        "DCCQ-1.3 protocol check: PASS "
        f"({len(document['equations'])} equations, "
        f"{len(document['reference_parameters'])} parameters, "
        f"{len(document['negative_controls'])} negative controls)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
