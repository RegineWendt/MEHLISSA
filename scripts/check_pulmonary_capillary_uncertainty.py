# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate and execute the outcome-blind PCQ-1.5 uncertainty design.

The executable calculations use only frozen model definitions, method floors,
and artificial design grids. They emit no participant record and make no
qualification decision.
"""

from __future__ import annotations

import hashlib
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
PLAN = ROOT / "data/qualification/pulmonary-capillary-uncertainty-plan-v1.json"
PLAN_SCHEMA = ROOT / "data/schemas/pulmonary-capillary-uncertainty-plan/1.0.0.schema.json"
INGRESS_RELATIVE_PATH = "data/qualification/pulmonary-capillary-data-ingress-policy-v1.json"
EXPECTED_BASELINE = "e40a903bfb71e9e4ae196ce2880ea6edbd9d685a"
EXPECTED_PARENT_SHA256 = "555a446cf02ef1dccc70324e529b0f05fbcdf70e6efcb786567846afec2efaf9"
MMHG_PA = 133.322387415
WU_SI = 7_999_343.2449

ENDPOINTS = {
    "PCQ-H1", "PCQ-H2", "PCQ-H3", "PCQ-H4", "PCQ-R1",
    "PCQ-C1", "PCQ-C2", "PCQ-C3", "PCQ-J1",
}
UNCERTAINTY_CLASSES = {
    "observational", "parameter", "structural", "numerical", "sensitivity", "identifiability",
}


class UncertaintyPlanError(ValueError):
    """Raised when the frozen PCQ-1.5 design or calculation fails closed."""


@dataclass(frozen=True)
class HemodynamicPrediction:
    model_id: str
    age_years: float
    flow_L_min: float
    mPAP_mmHg: float
    PVR_WU: float
    compliance_mL_mmHg: float
    RC_time_s: float


@dataclass(frozen=True)
class SensitivityRecord:
    parameter: str
    output: str
    elasticities: tuple[float, ...]
    converged: bool
    signed_direction: str


@dataclass(frozen=True)
class AnalysisSummary:
    uncertainty_class_count: int
    endpoint_count: int
    structural_model_count: int
    structural_grid_case_count: int
    sensitivity_count: int
    sensitivity_converged_count: int
    identifiability_analysis_count: int
    non_identifiable_or_blocked_count: int
    global_variance_analysis: str
    transit_endpoint: str
    measured_outcomes_used: bool = False
    qualification_decisions_emitted: bool = False


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def schema_errors(document: dict[str, Any]) -> list[str]:
    validator = Draft202012Validator(load_json(PLAN_SCHEMA), format_checker=FormatChecker())
    result: list[str] = []
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"{location}: {error.message}")
    return result


def value(entry: dict[str, Any]) -> float:
    return float(entry["value_si"])


def age_multiplier(hemodynamics: dict[str, Any], age_years: float) -> float:
    conditioning = hemodynamics.get("age_conditioning")
    if conditioning is None:
        return 1.0
    if age_years < value(conditioning["young_upper_age_years"]):
        return value(conditioning["young_resistance_multiplier"])
    if age_years >= value(conditioning["older_lower_age_years"]):
        return value(conditioning["older_resistance_multiplier"])
    return 1.0


def predict_hemodynamics(
    document: dict[str, Any],
    flow_L_min: float,
    age_years: float,
    scales: dict[str, float] | None = None,
) -> HemodynamicPrediction:
    """Mirror the accepted C++ equilibrium equations from frozen JSON values."""
    if flow_L_min <= 0:
        raise UncertaintyPlanError("hemodynamic design flow must be positive")
    factors = scales or {}
    hemodynamics = document["hemodynamics"]
    flow = flow_L_min / 60_000.0
    left_atrial_pressure = value(hemodynamics["left_atrial_pressure"]) * factors.get(
        "left_atrial_pressure", 1.0
    )
    base_resistance = value(hemodynamics["pulmonary_vascular_resistance"]) * factors.get(
        "pulmonary_vascular_resistance", 1.0
    )
    base_compliance = value(hemodynamics["pulmonary_arterial_compliance"]) * factors.get(
        "pulmonary_arterial_compliance", 1.0
    )
    multiplier = age_multiplier(hemodynamics, age_years)
    resistance = base_resistance * multiplier
    compliance = base_compliance

    adaptation = hemodynamics.get("flow_adaptation")
    if adaptation is not None:
        reference_flow = value(adaptation["reference_cardiac_output"])
        maximum_ratio = value(adaptation["maximum_flow_ratio"])
        ratio = min(max(flow / reference_flow, 1.0), maximum_ratio)
        resistance_exponent = value(adaptation["resistance_exponent"]) * factors.get(
            "flow_resistance_exponent", 1.0
        )
        compliance_exponent = value(adaptation["compliance_exponent"]) * factors.get(
            "flow_compliance_exponent", 1.0
        )
        resistance *= ratio**resistance_exponent
        compliance *= ratio**compliance_exponent
        equilibrium = left_atrial_pressure + resistance * flow
    else:
        distensibility = hemodynamics.get("pressure_distensibility")
        if distensibility is None:
            equilibrium = left_atrial_pressure + resistance * flow
        else:
            coefficient_entry = distensibility["coefficient"]
            if (
                "older_coefficient" in distensibility
                and age_years >= value(hemodynamics["age_conditioning"]["older_lower_age_years"])
            ):
                coefficient_entry = distensibility["older_coefficient"]
            alpha = value(coefficient_entry) * factors.get("pressure_distensibility", 1.0)
            reference_flow = value(distensibility["reference_cardiac_output"])
            reference_lap = value(distensibility["reference_left_atrial_pressure"])
            reference_target = reference_lap + resistance * reference_flow
            zero_pressure_resistance = (
                (1.0 + alpha * reference_target) ** 5
                - (1.0 + alpha * reference_lap) ** 5
            ) / (5.0 * alpha * reference_flow)
            pressure_term = (
                (1.0 + alpha * left_atrial_pressure) ** 5
                + 5.0 * alpha * zero_pressure_resistance * flow
            )
            equilibrium = (pressure_term**0.2 - 1.0) / alpha
            resistance = (equilibrium - left_atrial_pressure) / flow

    return HemodynamicPrediction(
        model_id=document["component"]["model_id"],
        age_years=age_years,
        flow_L_min=flow_L_min,
        mPAP_mmHg=equilibrium / MMHG_PA,
        PVR_WU=resistance / WU_SI,
        compliance_mL_mmHg=compliance * 1_000_000.0 * MMHG_PA,
        RC_time_s=resistance * compliance,
    )


def matrix_rank(matrix: list[list[float]], tolerance: float = 1.0e-10) -> int:
    if not matrix:
        return 0
    work = [row[:] for row in matrix]
    rows = len(work)
    columns = len(work[0])
    rank = 0
    for column in range(columns):
        pivot = max(range(rank, rows), key=lambda row: abs(work[row][column]), default=rank)
        if rank >= rows or abs(work[pivot][column]) <= tolerance:
            continue
        work[rank], work[pivot] = work[pivot], work[rank]
        pivot_value = work[rank][column]
        work[rank] = [item / pivot_value for item in work[rank]]
        for row in range(rows):
            if row == rank:
                continue
            scale = work[row][column]
            work[row] = [
                item - scale * pivot_item
                for item, pivot_item in zip(work[row], work[rank], strict=True)
            ]
        rank += 1
        if rank == rows:
            break
    return rank


def identifiability_matrices(flows: list[float]) -> dict[str, list[list[float]]]:
    x = [math.log(item / flows[0]) for item in flows]
    dynamic: list[list[float]] = []
    for item in x:
        dynamic.extend(([1.0, item, 0.0, 0.0], [1.0, item, 1.0, item]))
    return {
        "ID-H-SINGLE": [[1.0, 0.0]],
        "ID-H-MULTIPOINT": [[1.0, item] for item in x],
        "ID-H-EQUILIBRIUM": [[1.0, item, 0.0, 0.0] for item in x],
        "ID-H-DYNAMIC": dynamic,
        "ID-H-AGE": [[1.0, 1.0] for _ in flows],
        "ID-R-COMPOSITION": [
            [1.0, 0.0, 0.0, 0.0, -1.0],
            [0.0, 1.0, 0.0, 0.0, -1.0],
            [0.0, 0.0, 1.0, 0.0, -1.0],
            [0.0, 0.0, 0.0, 1.0, -1.0],
        ],
        "ID-C-GEOMETRY": [[1.0, 2.0, 1.0]],
        "ID-C2-DELAYS": [[1.0, 1.0, 1.0, 1.0]],
        "ID-J-CLOSURE": [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [1.0, -1.0, 0.0]],
    }


def pvr_standard_uncertainty_envelope(
    mPAP_mmHg: float,
    PAWP_mmHg: float,
    flow_L_min: float,
    mPAP_u_mmHg: float,
    PAWP_u_mmHg: float,
    flow_relative_u: float,
) -> tuple[float, float]:
    """Return the attainable all-input covariance envelope in Wood units."""
    if flow_L_min <= 0 or mPAP_mmHg <= PAWP_mmHg:
        raise UncertaintyPlanError("PVR propagation requires positive flow and mPAP above PAWP")
    u_m = max(mPAP_u_mmHg, 2.0)
    u_w = max(PAWP_u_mmHg, 3.0)
    u_q = max(flow_relative_u, 0.10)
    pvr = (mPAP_mmHg - PAWP_mmHg) / flow_L_min
    components = [u_m / flow_L_min, u_w / flow_L_min, abs(pvr * u_q)]
    upper = sum(components)
    lower = max(0.0, 2.0 * max(components) - upper)
    return lower, upper


def log_standard_uncertainty_from_cv(coefficient_of_variation: float) -> float:
    if coefficient_of_variation <= 0:
        raise UncertaintyPlanError("coefficient of variation must be positive")
    return math.sqrt(math.log1p(coefficient_of_variation**2))


def ratio_relative_uncertainty_envelope(
    numerator_relative_u: float, denominator_relative_u: float
) -> tuple[float, float]:
    if numerator_relative_u < 0 or denominator_relative_u < 0:
        raise UncertaintyPlanError("relative uncertainties cannot be negative")
    values = [
        math.sqrt(
            max(
                0.0,
                numerator_relative_u**2 + denominator_relative_u**2
                - 2.0 * correlation * numerator_relative_u * denominator_relative_u,
            )
        )
        for correlation in (-1.0, 1.0)
    ]
    return min(values), max(values)


def structural_predictions(plan: dict[str, Any]) -> list[HemodynamicPrediction]:
    models = [
        load_json(ROOT / item["path"])
        for item in plan["model_assets"]
        if item["role"] in {"hemodynamic-ensemble-member", "hemodynamic-equivalent-alias"}
    ]
    return [
        predict_hemodynamics(model, flow, age)
        for model in models
        for age in plan["analysis_grid"]["age_years"]
        for flow in plan["analysis_grid"]["flow_L_min"]
    ]


def central_relative_sensitivity(
    evaluate: Callable[[float], float], steps: list[float], tolerance: float
) -> tuple[tuple[float, ...], bool]:
    baseline = evaluate(1.0)
    if baseline <= 0:
        raise UncertaintyPlanError("sensitivity baseline output must be positive")
    elasticities = tuple(
        (evaluate(1.0 + step) - evaluate(1.0 - step)) / (2.0 * step * baseline)
        for step in steps
    )
    denominator = max(abs(elasticities[-1]), 1.0e-12)
    converged = abs(elasticities[-1] - elasticities[-2]) / denominator <= tolerance
    return elasticities, converged


def sensitivity_records(plan: dict[str, Any]) -> list[SensitivityRecord]:
    v7_entry = next(item for item in plan["model_assets"] if item["model_id"].endswith("lobar-parallel.v7"))
    v7 = load_json(ROOT / v7_entry["path"])
    steps = plan["analysis_grid"]["finite_difference_relative_steps"]
    tolerance = plan["analysis_grid"]["derivative_relative_convergence_tolerance"]
    flow = 10.0
    age = 30.0
    specifications: list[tuple[str, str, Callable[[float], float]]] = [
        ("pulmonary_vascular_resistance", "mPAP_mmHg", lambda factor: predict_hemodynamics(v7, flow, age, {"pulmonary_vascular_resistance": factor}).mPAP_mmHg),
        ("pulmonary_vascular_resistance", "PVR_WU", lambda factor: predict_hemodynamics(v7, flow, age, {"pulmonary_vascular_resistance": factor}).PVR_WU),
        ("flow_resistance_exponent", "PVR_WU", lambda factor: predict_hemodynamics(v7, flow, age, {"flow_resistance_exponent": factor}).PVR_WU),
        ("pulmonary_arterial_compliance", "RC_time_s", lambda factor: predict_hemodynamics(v7, flow, age, {"pulmonary_arterial_compliance": factor}).RC_time_s),
        ("flow_compliance_exponent", "RC_time_s", lambda factor: predict_hemodynamics(v7, flow, age, {"flow_compliance_exponent": factor}).RC_time_s),
        ("functional_capillary_volume", "capillary_residence_s", lambda factor: 0.0000859 * factor / 0.0001),
        ("cardiac_output", "capillary_residence_s", lambda factor: 0.0000859 / (0.0001 * factor)),
    ]
    result: list[SensitivityRecord] = []
    for parameter, output, evaluate in specifications:
        elasticities, converged = central_relative_sensitivity(evaluate, steps, tolerance)
        direction = "positive" if elasticities[-1] > 0 else "negative" if elasticities[-1] < 0 else "zero"
        result.append(SensitivityRecord(parameter, output, elasticities, converged, direction))
    return result


def plan_errors(document: dict[str, Any]) -> list[str]:
    result = schema_errors(document)
    if result:
        return result
    header = document["plan"]
    parent = header["parent_ingress"]
    if parent["path"] != INGRESS_RELATIVE_PATH:
        result.append("parent ingress path changed from PCQ-1.4")
    parent_path = ROOT / parent["path"]
    if parent["sha256"] != EXPECTED_PARENT_SHA256 or not parent_path.is_file() or sha256(parent_path) != parent["sha256"]:
        result.append("parent PCQ-1.4 ingress hash is missing or changed")
    if header["baseline_commit"] != EXPECTED_BASELINE:
        result.append("PCQ-1.5 baseline commit changed")

    classes = {item["id"] for item in document["uncertainty_classes"]}
    if classes != UNCERTAINTY_CLASSES:
        result.append("the six frozen uncertainty classes are not covered exactly once")
    endpoints = {item["endpoint_id"] for item in document["observational_models"]}
    if endpoints != ENDPOINTS:
        result.append("the nine frozen endpoints are not covered exactly once")

    seen_paths: set[str] = set()
    model_ids: set[str] = set()
    for asset in document["model_assets"]:
        path = ROOT / asset["path"]
        if asset["path"] in seen_paths:
            result.append(f"duplicate model asset path: {asset['path']}")
        seen_paths.add(asset["path"])
        if not path.is_file() or sha256(path) != asset["sha256"]:
            result.append(f"model asset is missing or changed: {asset['path']}")
            continue
        model = load_json(path)
        if model["component"]["model_id"] != asset["model_id"]:
            result.append(f"model identity mismatch: {asset['path']}")
        if asset["model_id"] in model_ids:
            result.append(f"duplicate model identity: {asset['model_id']}")
        model_ids.add(asset["model_id"])
    aliases = {item["model_id"] for item in document["model_assets"] if item["equivalence_group"] == "v4-aggregate"}
    expected_aliases = {
        "lung.pulmonary-0d.healthy-adult-rest-exercise-age-invasive.v4",
        "lung.pulmonary-0d.healthy-adult-lobar-parallel.v7",
    }
    if aliases != expected_aliases:
        result.append("the v4/v7 aggregate-equivalence group changed")

    implementation_paths = {item["path"] for item in document["implementation_assets"]}
    if implementation_paths != {
        "models/organ/src/pulmonary_zero_dimensional.cpp",
        "models/capillary/src/capillary_bed.cpp",
    }:
        result.append("the frozen implementation-source set changed")
    for asset in document["implementation_assets"]:
        path = ROOT / asset["path"]
        if not path.is_file() or sha256(path) != asset["sha256"]:
            result.append(f"implementation asset is missing or changed: {asset['path']}")

    grid = document["analysis_grid"]
    if grid["flow_L_min"] != [6.0, 10.0, 14.0] or grid["age_years"] != [30.0, 50.0, 70.0]:
        result.append("the outcome-blind structural flow-age grid changed")
    steps = grid["finite_difference_relative_steps"]
    if steps != sorted(steps, reverse=True):
        result.append("finite-difference steps must decrease from coarse to fine")

    parameter_ids = [item["id"] for item in document["parameter_registry"]]
    if len(parameter_ids) != len(set(parameter_ids)):
        result.append("parameter registry identifiers must be unique")
    registered_endpoints = {endpoint for item in document["parameter_registry"] for endpoint in item["endpoint_ids"]}
    if registered_endpoints != ENDPOINTS:
        result.append("parameter registry does not cover all frozen endpoints")

    matrices = identifiability_matrices(grid["flow_L_min"])
    analyses = {item["id"]: item for item in document["identifiability_analyses"]}
    if set(analyses) != set(matrices):
        result.append("identifiability analysis set changed")
    else:
        for identifier, matrix in matrices.items():
            actual_rank = matrix_rank(matrix)
            item = analyses[identifier]
            if actual_rank != item["expected_rank"]:
                result.append(f"{identifier}: declared rank does not match the executable design")
            if item["expected_rank"] > item["parameter_count"]:
                result.append(f"{identifier}: rank exceeds parameter count")

    boundary = document["decision_boundary"]
    if boundary["global_variance_analysis"] != "blocked-until-joint-distributions-and-correlations-are-evidence-backed":
        result.append("global variance analysis must remain evidence-blocked")
    if boundary["transit_endpoint"] != "blocked-observation-model":
        result.append("PCQ-C2 transit comparison must remain blocked")
    if document["next_step"]["increment"] != "PCQ-1.6":
        result.append("PCQ-1.6 must remain the next increment")
    full_text = json.dumps(document).lower()
    for phrase in ("no candidate participant-level outcome", "refit"):
        if phrase not in full_text:
            result.append(f"PCQ-1.5 safeguard is missing: {phrase}")
    return result


def analysis_errors(document: dict[str, Any]) -> list[str]:
    result: list[str] = []
    predictions = structural_predictions(document)
    by_key = {(item.model_id, item.age_years, item.flow_L_min): item for item in predictions}
    v4 = "lung.pulmonary-0d.healthy-adult-rest-exercise-age-invasive.v4"
    v7 = "lung.pulmonary-0d.healthy-adult-lobar-parallel.v7"
    for age in document["analysis_grid"]["age_years"]:
        for flow in document["analysis_grid"]["flow_L_min"]:
            left = by_key[(v4, age, flow)]
            right = by_key[(v7, age, flow)]
            for field in ("mPAP_mmHg", "PVR_WU", "compliance_mL_mmHg", "RC_time_s"):
                if not math.isclose(getattr(left, field), getattr(right, field), rel_tol=1.0e-12, abs_tol=1.0e-12):
                    result.append(f"v4/v7 aggregate equivalence failed for {field} at age={age}, flow={flow}")
    high_flow = [item.mPAP_mmHg for item in predictions if item.age_years == 30.0 and item.flow_L_min == 14.0]
    if max(high_flow) - min(high_flow) <= 0.5:
        result.append("structural ensemble does not expose meaningful high-flow spread")

    sensitivity = sensitivity_records(document)
    if not all(item.converged for item in sensitivity):
        result.append("one or more local sensitivities did not converge across perturbation sizes")
    directions = {(item.parameter, item.output): item.signed_direction for item in sensitivity}
    expected_directions = {
        ("pulmonary_vascular_resistance", "mPAP_mmHg"): "positive",
        ("pulmonary_vascular_resistance", "PVR_WU"): "positive",
        ("flow_resistance_exponent", "PVR_WU"): "negative",
        ("pulmonary_arterial_compliance", "RC_time_s"): "positive",
        ("flow_compliance_exponent", "RC_time_s"): "negative",
        ("functional_capillary_volume", "capillary_residence_s"): "positive",
        ("cardiac_output", "capillary_residence_s"): "negative",
    }
    if directions != expected_directions:
        result.append("signed local-sensitivity directions changed")

    lower, upper = pvr_standard_uncertainty_envelope(15.0, 8.0, 5.5, 1.0, 1.0, 0.01)
    if not (0 < lower < upper):
        result.append("PVR covariance envelope did not retain bounded uncertainty")
    if not math.isclose(log_standard_uncertainty_from_cv(0.08), 0.07987244183095335, rel_tol=1.0e-12):
        result.append("Vc log-scale uncertainty conversion changed")
    ratio_lower, ratio_upper = ratio_relative_uncertainty_envelope(0.08, 0.10)
    if not (math.isclose(ratio_lower, 0.02, abs_tol=1.0e-12) and math.isclose(ratio_upper, 0.18, abs_tol=1.0e-12)):
        result.append("joint Vc/Q covariance envelope changed")
    return result


def validate_plan(path: Path = PLAN) -> dict[str, Any]:
    document = load_json(path)
    found = plan_errors(document)
    if not found:
        found.extend(analysis_errors(document))
    if found:
        raise UncertaintyPlanError("\n".join(found))
    return document


def summarize(document: dict[str, Any]) -> AnalysisSummary:
    predictions = structural_predictions(document)
    sensitivity = sensitivity_records(document)
    identifiability = document["identifiability_analyses"]
    return AnalysisSummary(
        uncertainty_class_count=len(document["uncertainty_classes"]),
        endpoint_count=len(document["observational_models"]),
        structural_model_count=len({item.model_id for item in predictions}),
        structural_grid_case_count=len(predictions),
        sensitivity_count=len(sensitivity),
        sensitivity_converged_count=sum(item.converged for item in sensitivity),
        identifiability_analysis_count=len(identifiability),
        non_identifiable_or_blocked_count=sum(
            item["status"] in {"non-identifiable", "blocked-observation-model"}
            for item in identifiability
        ),
        global_variance_analysis=document["decision_boundary"]["global_variance_analysis"],
        transit_endpoint=document["decision_boundary"]["transit_endpoint"],
    )


def main() -> int:
    try:
        document = validate_plan()
        summary = summarize(document)
        print(
            "PCQ-1.5 uncertainty and identifiability: ok "
            f"({summary.uncertainty_class_count} classes, {summary.endpoint_count} endpoints, "
            f"{summary.structural_model_count} pulmonary structures, "
            f"{summary.sensitivity_converged_count}/{summary.sensitivity_count} local sensitivities converged, "
            "no measured outcomes or qualification decisions)"
        )
    except (OSError, json.JSONDecodeError, UncertaintyPlanError) as error:
        print(f"PCQ-1.5 uncertainty and identifiability: FAILED\n{error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
