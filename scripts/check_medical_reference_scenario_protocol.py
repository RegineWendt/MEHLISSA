# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the prospective MRSQ-1.2 protocol without opening participant data."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "data/qualification/medical-reference-scenario-protocol-v1.json"
SCHEMA = ROOT / "data/schemas/medical-reference-scenario-protocol/1.0.0.schema.json"
PARENT = ROOT / "data/qualification/medical-reference-scenario-candidate-register-v1.json"

EXPECTED_HF_REVISION = "b6daf89015481bdc20a238866df730f90274157d"
EXPECTED_GITHUB_REVISION = "b8e04e56934a2a12d87bbaa2258a1c1f8e250d15"
EXPECTED_ASSETS = {
    "train/dataset_description.json": (1202, "fea2f716fac2b7ff55ae9c20f59e8f138db19146", False),
    "train/participants.json": (2137, "7ce6a4ef14e9e738e1550a6dcd5f677e272afed4", False),
    "train/derivatives/readouts/metadata.csv": (14682, "ccb10f855f5bf81c86463955a2087404e6c52f4c", True),
    "train/derivatives/readouts/input_functions.csv": (6517065, "4c9d833768c9bff036e6c1bfe8de2f93f533f664", True),
    "train/derivatives/readouts/tacs.csv": (239084285, "7cfeae7e85657fcfcda060408a2e49db6be1c94e", True),
    "train/derivatives/readouts/volumes.csv": (4078118, "c462868c114ce81e1aa62d0c28bfadba8286f548", True),
}
EXPECTED_ANALYSIS_SETS = {
    "MRSQ-AS-CLOSED": "primary",
    "MRSQ-AS-CONDITIONAL": "diagnostic-only",
    "MRSQ-AS-AUDIT": "integrity-only",
}
EXPECTED_REGIONS = {
    "MRSQ-ROI-LUNG": [
        "lung_upper_lobe_left",
        "lung_lower_lobe_left",
        "lung_upper_lobe_right",
        "lung_middle_lobe_right",
        "lung_lower_lobe_right",
    ],
    "MRSQ-ROI-LIVER": ["liver"],
    "MRSQ-ROI-KIDNEY": ["kidney_left", "kidney_right"],
    "MRSQ-ROI-BLADDER": ["urinary_bladder"],
}
EXPECTED_ENDPOINTS = {
    "MRSQ-P1": ("MRSQ-OM-IDIF", "prospectively frozen aortic image-derived input region", 0.25, 0.75, 1.25),
    "MRSQ-P2": ("MRSQ-OM-TAC", "MRSQ-ROI-LUNG", 0.30, 0.70, 1.30),
    "MRSQ-P3": ("MRSQ-OM-TAC", "MRSQ-ROI-LIVER", 0.30, 0.70, 1.30),
    "MRSQ-P4": ("MRSQ-OM-TAC", "MRSQ-ROI-KIDNEY", 0.35, 0.65, 1.35),
    "MRSQ-P5": ("MRSQ-OM-AMOUNT", "MRSQ-ROI-BLADDER", 0.40, 0.60, 1.40),
}
EXPECTED_UNCERTAINTY = {
    "numerical",
    "input-function",
    "pet-reconstruction",
    "segmentation-partial-volume",
    "parameter",
    "structural",
    "cohort",
}


class MedicalReferenceScenarioProtocolError(ValueError):
    """Raised when the frozen MRSQ-1.2 protocol is inconsistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _close(actual: object, expected: float, tolerance: float = 1.0e-12) -> bool:
    return isinstance(actual, (int, float)) and abs(float(actual) - expected) <= tolerance


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    result: list[str] = []
    schema = load_json(root / SCHEMA.relative_to(ROOT))
    validator = Draft202012Validator(schema, format_checker=FormatChecker())
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"schema: {location}: {error.message}")
    if result:
        return result

    parent = document["parent_selection"]
    parent_path = root / parent["path"]
    if parent_path != root / PARENT.relative_to(ROOT) or not parent_path.is_file():
        result.append("MRSQ-1.2 parent selection is missing or changed")
    elif parent["sha256"] != sha256(parent_path):
        result.append("MRSQ-1.2 parent selection hash mismatch")

    governance = document["governance"]
    if governance["participant_files_opened"]:
        result.append("MRSQ-1.2 must remain before participant-file access")
    if governance["validation_outcomes_inspected"]:
        result.append("MRSQ-1.2 must remain before validation-outcome inspection")
    if governance["ingress_authorized"]:
        result.append("MRSQ-1.2 cannot authorize participant-data ingress")
    governance_text = json.dumps(governance, ensure_ascii=True).lower()
    for phrase in ("outside git", "quarantine", "direct identifier", "retention", "institutional"):
        if phrase not in governance_text:
            result.append(f"governance boundary is missing: {phrase}")

    freeze = document["source_freeze"]
    repository = freeze["data_repository"]
    if repository["revision"] != EXPECTED_HF_REVISION:
        result.append("Hugging Face revision changed")
    if repository["licence"] != "CC-BY-4.0" or not repository["public"] or repository["gated"]:
        result.append("Hugging Face access or licence boundary changed")
    assets = {item["path"]: item for item in repository["minimum_assets"]}
    if set(assets) != set(EXPECTED_ASSETS):
        result.append("minimum-data asset set changed")
    for path, (size, blob_id, participant_level) in EXPECTED_ASSETS.items():
        asset = assets.get(path, {})
        if asset.get("size_bytes") != size or asset.get("git_blob_id") != blob_id:
            result.append(f"remote asset identity changed: {path}")
        if asset.get("participant_level") is not participant_level:
            result.append(f"participant-level classification changed: {path}")
        expected_state = "blocked-until-MRSQ-1.3" if participant_level else "metadata-only-download-permitted"
        if asset.get("ingress_state") != expected_state:
            result.append(f"ingress state changed: {path}")
        if asset.get("content_sha256") != "required-after-authorized-download-before-first-parse":
            result.append(f"content SHA-256 gate changed: {path}")
    exclusions = " ".join(repository["excluded_assets"]).lower()
    for phrase in ("nifti", "patlak_ki.csv", "unreleased 20", "data-explorer"):
        if phrase not in exclusions:
            result.append(f"minimum-data exclusion is missing: {phrase}")
    processing = freeze["processing_repository"]
    if processing["version"] != EXPECTED_GITHUB_REVISION:
        result.append("processing-repository revision changed")
    if "noassertion" not in processing["rights"].lower() or "no copied" not in processing["role"].lower():
        result.append("unlicensed processing-code safeguard changed")
    if freeze["full_image_fallback"]["version"] != "V1":
        result.append("PublicnEUro version changed")
    if "not authorized or needed" not in freeze["full_image_fallback"]["access"].lower():
        result.append("full-image access must remain outside MRSQ-1.2")
    context_locators = {item["locator"] for item in freeze["construction_context"]}
    if context_locators != {
        "doi:10.1007/s00259-020-05124-y",
        "doi:10.3389/fnume.2025.1556848",
        "doi:10.1038/s41597-025-05997-4",
        "doi:10.6028/jres.119.013",
    }:
        result.append("source-disjoint construction or uncertainty context changed")

    sets = {item["id"]: item["qualification_role"] for item in document["analysis_sets"]}
    if sets != EXPECTED_ANALYSIS_SETS:
        result.append("closed, conditional and audit analysis roles changed")
    conditional = next(item for item in document["analysis_sets"] if item["id"] == "MRSQ-AS-CONDITIONAL")
    if "cannot" not in conditional["outcome_rule"].lower() or "complete" not in conditional["outcome_rule"].lower():
        result.append("conditional-input track lost its non-qualification boundary")

    source_roles = {item["id"]: item for item in document["source_roles"]}
    expected_roles = {
        "HEDYPET-ADMIN-METADATA",
        "HEDYPET-IDIF",
        "HEDYPET-ORGAN-TAC",
        "SOURCE-DISJOINT-KINETICS",
        "SOURCE-DISJOINT-REPEATABILITY",
        "MEHLISSA-SYNTHETIC",
    }
    if set(source_roles) != expected_roles:
        result.append("source roles changed")
    roles_text = json.dumps(source_roles, ensure_ascii=True).lower()
    for phrase in ("parameter fitting", "validation observation", "source-disjoint", "synthetic", "clinical"):
        if phrase not in roles_text:
            result.append(f"source-role boundary is missing: {phrase}")

    models = {item["id"]: item for item in document["observation_models"]}
    if set(models) != {"MRSQ-OM-IDIF", "MRSQ-OM-TAC", "MRSQ-OM-AMOUNT", "MRSQ-OM-PATLAK"}:
        result.append("observation-model set changed")
    if models.get("MRSQ-OM-PATLAK", {}).get("status") != "secondary-nongating":
        result.append("Patlak must remain secondary and nongating")
    model_text = json.dumps(models, ensure_ascii=True).lower()
    for phrase in ("integrate", "frame duration", "decay", "no smoothing", "do not use ki to fit"):
        if phrase not in model_text:
            result.append(f"observation transformation is missing: {phrase}")

    regions = {item["id"]: item for item in document["regions"]}
    if set(regions) != set(EXPECTED_REGIONS):
        result.append("primary region set changed")
    for region_id, components in EXPECTED_REGIONS.items():
        if regions.get(region_id, {}).get("required_source_components") != components:
            result.append(f"region composition changed: {region_id}")
    if "four-lobe substitute" not in regions["MRSQ-ROI-LUNG"]["missing_rule"].lower():
        result.append("five-lobe completeness rule changed")
    if "unilateral substitution" not in regions["MRSQ-ROI-KIDNEY"]["missing_rule"].lower():
        result.append("bilateral kidney completeness rule changed")

    endpoints = {item["id"]: item for item in document["endpoint_rules"]}
    if set(endpoints) != set(EXPECTED_ENDPOINTS):
        result.append("five primary endpoints changed")
    for endpoint_id, expected in EXPECTED_ENDPOINTS.items():
        model, region, nrmse, lower, upper = expected
        endpoint = endpoints.get(endpoint_id, {})
        limits = endpoint.get("limits", {})
        if endpoint.get("analysis_set") != "MRSQ-AS-CLOSED":
            result.append(f"primary endpoint moved outside closed-loop track: {endpoint_id}")
        if endpoint.get("observation_model") != model or endpoint.get("region") != region:
            result.append(f"endpoint observation identity changed: {endpoint_id}")
        if endpoint.get("minimum_complete_participants") != 60:
            result.append(f"minimum cohort changed: {endpoint_id}")
        for key, value in (
            ("cohort_upper_90_ci_median_duration_weighted_nrmse", nrmse),
            ("cohort_90_ci_geometric_mean_auc_ratio_lower", lower),
            ("cohort_90_ci_geometric_mean_auc_ratio_upper", upper),
        ):
            if not _close(limits.get(key), value):
                result.append(f"numeric gate changed: {endpoint_id} {key}")

    statistics = document["statistics"]
    if not statistics["participant_first"] or statistics["bootstrap_replicates"] != 10000:
        result.append("participant-first bootstrap plan changed")
    if statistics["bootstrap_seed"] != 18042026 or statistics["cohort_interval"] != "two-sided-90-percent-percentile-bootstrap":
        result.append("frozen cohort interval or seed changed")
    stats_text = json.dumps(statistics, ensure_ascii=True).lower()
    for phrase in ("do not impute", "residual outliers", "descriptive only", "all five"):
        if phrase not in stats_text:
            result.append(f"statistical boundary is missing: {phrase}")

    uncertainties = {item["class"] for item in document["uncertainty_policy"]}
    if uncertainties != EXPECTED_UNCERTAINTY:
        result.append("seven uncertainty classes changed")
    uncertainty_text = json.dumps(document["uncertainty_policy"], ensure_ascii=True).lower()
    for phrase in ("one tenth", "early frames", "sub-017", "no residual-driven", "no invented", "no individual"):
        if phrase not in uncertainty_text:
            result.append(f"uncertainty boundary is missing: {phrase}")

    policy = document["failure_policy"]
    if "all five" not in policy["pass"].lower() or "at least 60" not in policy["pass"].lower():
        result.append("overall PASS gate changed")
    if "20 to 59" not in policy["partial"] or "fewer than 20" not in policy["blocked"]:
        result.append("partial or blocked cohort thresholds changed")
    if "immutable" not in policy["amendment"].lower() or "retroactively rescue" not in policy["amendment"].lower():
        result.append("post-outcome amendment policy changed")

    controls = document["negative_controls"]
    if len(controls) != 16 or len(set(controls)) != 16:
        result.append("MRSQ-1.2 must retain sixteen unique negative controls")
    controls_text = " ".join(controls).lower()
    for phrase in ("revision-or-blob", "participant-file", "double-radioactive", "outcome-as-calibration", "conditional-idif", "residual-based", "clinical-validity"):
        if phrase not in controls_text:
            result.append(f"negative control is missing: {phrase}")

    intended = json.dumps(document["intended_use"], ensure_ascii=True).lower()
    for phrase in ("healthy-adult", "70-minute", "cohort-level", "clinical decision"):
        if phrase not in intended:
            result.append(f"intended-use boundary is missing: {phrase}")
    claim = document["allowed_claim"].lower()
    if "before participant-file ingress" not in claim or "five primary cohort endpoints" not in claim:
        result.append("allowed claim exceeds or omits the prospective boundary")
    forbidden = " ".join(document["forbidden_claims"]).lower()
    for phrase in ("any primary endpoint passes", "conditional", "clinical", "source code"):
        if phrase not in forbidden:
            result.append(f"forbidden claim is missing: {phrase}")
    return result


def validate(path: Path = PROTOCOL, root: Path = ROOT) -> dict[str, Any]:
    document = load_json(path)
    failures = errors(document, root)
    if failures:
        raise MedicalReferenceScenarioProtocolError("\n".join(failures))
    return document


def main() -> int:
    try:
        document = validate()
    except (OSError, json.JSONDecodeError, MedicalReferenceScenarioProtocolError) as error:
        print(f"MRSQ-1.2 prospective protocol: FAILED\n{error}")
        return 1
    print(
        "MRSQ-1.2 prospective protocol: ok "
        f"({len(document['endpoint_rules'])} primary endpoints, "
        f"{len(document['uncertainty_policy'])} uncertainty classes, "
        "6 frozen minimum assets, no participant-file ingress, MRSQ-1.3 next)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
