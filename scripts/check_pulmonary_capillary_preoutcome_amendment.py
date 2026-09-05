# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the frozen PCQ-1.3 pre-outcome amendment."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
AMENDMENT = ROOT / "data/qualification/pulmonary-capillary-preoutcome-amendment-v1.json"
SCHEMA = ROOT / "data/schemas/pulmonary-capillary-preoutcome-amendment/1.0.0.schema.json"
REGISTER = ROOT / "data/qualification/pulmonary-capillary-evidence-candidate-register-v1.json"


class PreoutcomeAmendmentError(ValueError):
    pass


def load(path: Path) -> dict:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _isclose(left: object, right: float, tolerance: float = 1.0e-12) -> bool:
    return isinstance(left, (int, float)) and abs(float(left) - right) <= tolerance


def errors(document: dict) -> list[str]:
    validator = Draft202012Validator(load(SCHEMA), format_checker=FormatChecker())
    result = [error.message for error in validator.iter_errors(document)]

    amendment = document.get("amendment", {})
    for key in ("parent_protocol", "candidate_register"):
        record = amendment.get(key, {})
        relative = record.get("path")
        if not isinstance(relative, str):
            continue
        path = ROOT / relative
        if not path.is_file():
            result.append(f"frozen authority is missing: {relative}")
        elif record.get("sha256") != sha256(path):
            result.append(f"frozen authority hash mismatch: {relative}")

    register_candidates = {
        item["id"]: item for item in load(REGISTER).get("candidates", [])
    }
    selected = document.get("selected_sources", [])
    selected_ids = [item.get("candidate_id") for item in selected]
    if len(selected_ids) != len(set(selected_ids)):
        result.append("selected source identifiers must be unique")
    expected_selection = {
        "PCQ-SRC-H-001": "access-and-rights-pending",
        "PCQ-SRC-R-002": "feasibility-access-and-rights-pending",
        "PCQ-SRC-CJ-001": "access-and-rights-pending",
        "PCQ-SRC-C-003": "observation-model-blocked-and-access-pending",
    }
    if {item.get("candidate_id"): item.get("status") for item in selected} != expected_selection:
        result.append("PCQ-1.3 selected sources or guarded activation states changed")
    for item in selected:
        candidate_id = item.get("candidate_id")
        candidate = register_candidates.get(candidate_id)
        if candidate is None:
            result.append(f"selected source is absent from PCQ-1.2: {candidate_id}")
            continue
        if not set(item.get("tracks", [])).issubset(set(candidate.get("track_ids", []))):
            result.append(f"selected source track disagrees with PCQ-1.2: {candidate_id}")

    sets = document.get("analysis_sets", [])
    if {item.get("track") for item in sets} != {"PCQ-H", "PCQ-R", "PCQ-C", "PCQ-J"} or len(sets) != 4:
        result.append("analysis sets must cover all four tracks exactly once")
    set_by_track = {item.get("track"): item for item in sets}
    required_sample_rules = {
        "PCQ-H": (3, 10, 3),
        "PCQ-R": (5, 10, 1),
        "PCQ-C": (8, 12, 1),
        "PCQ-J": (0, 0, 0),
    }
    for track, expected in required_sample_rules.items():
        item = set_by_track.get(track, {})
        actual = (
            item.get("minimum_complete_participants_for_bounded_pilot"),
            item.get("minimum_complete_participants_for_track_decision"),
            item.get("minimum_complete_stages_per_participant"),
        )
        if actual != expected:
            result.append(f"pre-outcome sample rule changed for {track}")

    models = document.get("observation_models", [])
    model_ids = [item.get("id") for item in models]
    expected_models = {
        "PCQ-OM-H1",
        "PCQ-OM-H2",
        "PCQ-OM-H3",
        "PCQ-OM-H4",
        "PCQ-OM-R1",
        "PCQ-OM-C1",
        "PCQ-OM-C2",
        "PCQ-OM-J1",
    }
    if set(model_ids) != expected_models or len(model_ids) != len(expected_models):
        result.append("observation models must cover the eight frozen mappings exactly once")
    covered_endpoints = {
        endpoint for model in models for endpoint in model.get("endpoint_ids", [])
    }
    if covered_endpoints != {
        "PCQ-H1",
        "PCQ-H2",
        "PCQ-H3",
        "PCQ-H4",
        "PCQ-R1",
        "PCQ-C1",
        "PCQ-C2",
        "PCQ-C3",
        "PCQ-J1",
    }:
        result.append("observation-model endpoint coverage is incomplete")
    c2_model = next((item for item in models if item.get("id") == "PCQ-OM-C2"), {})
    if c2_model.get("status") != "blocked-until-independent-extra-capillary-terms-are-frozen":
        result.append("PCQ-C2 must remain blocked until independent extra-capillary terms are frozen")
    c2_text = json.dumps(c2_model).lower()
    for phrase in ("precapillary", "postcapillary", "mixing", "cannot independently"):
        if phrase not in c2_text:
            result.append(f"PCQ-C2 observation limitation is incomplete: {phrase}")

    rules = document.get("endpoint_rules", [])
    rule_ids = [item.get("endpoint_id") for item in rules]
    primary_ids = {"PCQ-H1", "PCQ-H2", "PCQ-H3", "PCQ-R1", "PCQ-C1", "PCQ-C2"}
    if set(rule_ids) != primary_ids or len(rule_ids) != len(primary_ids):
        result.append("numeric rules must cover all six primary endpoints exactly once")
    by_endpoint = {item.get("endpoint_id"): item for item in rules}

    numeric_expectations = [
        ("PCQ-H1", "stage_absolute_error_limit_mmHg", 5.0),
        ("PCQ-H1", "minimum_stage_fraction_within_limit", 0.8),
        ("PCQ-H2", "stage_absolute_error_floor_WU", 0.5),
        ("PCQ-H2", "stage_relative_error_limit", 0.25),
        ("PCQ-H3", "participant_absolute_slope_difference_limit_WU", 0.75),
        ("PCQ-R1", "participant_maximum_absolute_lobe_error_fraction", 0.05),
        ("PCQ-R1", "participant_five_lobe_RMSE_limit_fraction", 0.03),
        ("PCQ-R1", "participant_right_lung_error_limit_fraction", 0.04),
        ("PCQ-C1", "geometric_mean_ratio_lower", 0.8),
        ("PCQ-C1", "geometric_mean_ratio_upper", 1.25),
        ("PCQ-C2", "geometric_mean_ratio_lower", 0.75),
        ("PCQ-C2", "geometric_mean_ratio_upper", 4.0 / 3.0),
    ]
    for endpoint, key, expected in numeric_expectations:
        value = by_endpoint.get(endpoint, {}).get("numeric_rule", {}).get(key)
        if not _isclose(value, expected):
            result.append(f"pre-outcome numeric limit changed: {endpoint} {key}")

    if by_endpoint.get("PCQ-C1", {}).get("minimum_complete_participants") != 12:
        result.append("PCQ-C1 minimum complete sample must remain twelve")
    c1_uncertainty = by_endpoint.get("PCQ-C1", {}).get("uncertainty_model", {})
    if not _isclose(c1_uncertainty.get("method_coefficient_of_variation"), 0.08):
        result.append("PCQ-C1 must retain the predeclared 8 percent method CV")
    if not _isclose(c1_uncertainty.get("minimum_regression_r_squared"), 0.95):
        result.append("PCQ-C1 must retain the source method quality floor")
    c2 = by_endpoint.get("PCQ-C2", {})
    if c2.get("activation_status") != "blocked-until-independent-observation-model":
        result.append("PCQ-C2 numeric gate cannot be activated by this amendment")
    c2_uncertainty = c2.get("uncertainty_model", {})
    if not _isclose(c2_uncertainty.get("rest_repeatability_coefficient"), 0.172):
        result.append("PCQ-C2 must retain the reported resting repeatability coefficient")
    if not _isclose(c2_uncertainty.get("tracer_retention_relative_bias_sensitivity"), 0.25):
        result.append("PCQ-C2 must retain the one-sided tracer-retention bias sensitivity")

    supplementary = {
        item.get("endpoint_id"): item for item in document.get("supplementary_rules", [])
    }
    if set(supplementary) != {"PCQ-H4", "PCQ-C3", "PCQ-J1"}:
        result.append("secondary, covariate, and diagnostic rules are incomplete")
    if "1e-12" not in supplementary.get("PCQ-J1", {}).get("numeric_rule", ""):
        result.append("PCQ-J1 must retain the hard software closure tolerance")

    full_text = json.dumps(document).lower()
    for phrase in (
        "not tuned",
        "no candidate participant-level outcome",
        "inconclusive",
        "no global pass",
        "no data request is sent",
        "clinical_use",
    ):
        if phrase not in full_text:
            result.append(f"pre-outcome safeguard is absent: {phrase}")
    if document.get("next_step", {}).get("increment") != "PCQ-1.4":
        result.append("PCQ-1.4 must remain the next increment")
    return result


def validate(path: Path = AMENDMENT) -> None:
    document = load(path)
    found = errors(document)
    if found:
        raise PreoutcomeAmendmentError("\n".join(found))


if __name__ == "__main__":
    try:
        validate()
    except (OSError, json.JSONDecodeError, PreoutcomeAmendmentError) as error:
        print(f"PCQ-1.3 pre-outcome amendment: FAILED\n{error}")
        raise SystemExit(1)
    document = load(AMENDMENT)
    print(
        "PCQ-1.3 pre-outcome amendment: ok "
        f"({len(document['endpoint_rules'])} primary gates, "
        f"{len(document['observation_models'])} observation models, no participant outcomes)"
    )
