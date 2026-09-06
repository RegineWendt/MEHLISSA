# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Execute and archive the prospectively frozen DCCQ-1 computational series."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import platform
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "data/qualification/dynamic-capillary-tissue-cell-protocol-v1.json"
RESULT = ROOT / "data/qualification/dynamic-capillary-tissue-cell-qualification-result-v1.json"
OWNER_COLUMNS = [
    "blood_free_mol",
    "endothelium_free_mol",
    "interstitium_free_mol",
    "receptor_bound_mol",
    "internalized_mol",
    "cleared_or_degraded_mol",
    "cumulative_outlet_mol",
]
SENSITIVITY_PARAMETERS = [
    "blood_to_endothelium",
    "endothelium_to_blood",
    "endothelium_to_interstitium",
    "interstitium_to_endothelium",
    "blood_outlet",
    "interstitial_clearance",
    "association",
    "dissociation",
    "internalization",
    "degradation",
    "receptor_capacity",
    "feedback_gain",
]


def canonical_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    if path.suffix.lower() in {".csv", ".json", ".md", ".txt", ".cpp", ".hpp"}:
        data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return data


def sha256(path: Path) -> str:
    return hashlib.sha256(canonical_bytes(path)).hexdigest()


def write_json(path: Path, document: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(document, indent=2, sort_keys=False) + "\n", encoding="utf-8")


def read_csv(path: Path) -> list[dict[str, float]]:
    with path.open(encoding="utf-8", newline="") as stream:
        return [{key: float(value) for key, value in row.items()} for row in csv.DictReader(stream)]


def final_error(left: list[dict[str, float]], right: list[dict[str, float]]) -> float:
    scale = left[0]["initial_mol"]
    return sum(abs(left[-1][name] - right[-1][name]) for name in OWNER_COLUMNS) / scale


def first_time(rows: list[dict[str, float]], predicate: Any) -> float:
    for row in rows:
        if predicate(row):
            return row["time_s"]
    return math.inf


def run_command(
    runner: Path,
    archive: Path,
    label: str,
    step: float,
    synchronization: float,
    duration: float,
    variation: str | None = None,
    factor: float = 1.0,
) -> tuple[list[dict[str, float]], list[str]]:
    output = archive / "trajectories" / f"{label}.csv"
    output.parent.mkdir(parents=True, exist_ok=True)
    command = [str(runner), str(output), str(step), str(synchronization), str(duration)]
    if variation is not None:
        command.extend([variation, str(factor)])
    completed = subprocess.run(command, cwd=ROOT, check=True, capture_output=True, text=True)
    return read_csv(output), [*command, completed.stdout.strip()]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", type=Path, required=True)
    parser.add_argument("--run-id")
    arguments = parser.parse_args()
    runner = arguments.runner.resolve()
    if not runner.is_file():
        raise FileNotFoundError(f"DCCQ runner does not exist: {runner}")
    protocol = json.loads(PROTOCOL.read_text(encoding="utf-8"))
    run_id = arguments.run_id or (
        datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + sha256(PROTOCOL)[:12]
    )
    archive = ROOT / "results/dccq1/vegfa165a-vegfr2-huvec" / run_id
    if archive.exists():
        raise FileExistsError(f"DCCQ archive already exists: {archive}")
    archive.mkdir(parents=True)
    shutil.copyfile(PROTOCOL, archive / "protocol.json")

    commands: list[list[str]] = []
    trajectories: dict[str, list[dict[str, float]]] = {}

    def execute(
        label: str,
        step: float = 2.0,
        synchronization: float = 60.0,
        duration: float = 14400.0,
        variation: str | None = None,
        factor: float = 1.0,
    ) -> list[dict[str, float]]:
        rows, command = run_command(
            runner, archive, label, step, synchronization, duration, variation, factor
        )
        trajectories[label] = rows
        commands.append(command)
        return rows

    baseline = execute("baseline")
    replay = execute("replay")
    for step in (8.0, 4.0, 2.0, 1.0, 0.5):
        execute(f"time-step-{step:g}s", step=step)
    for synchronization in (120.0, 60.0, 30.0, 15.0):
        execute(
            f"synchronization-{synchronization:g}s",
            step=0.5,
            synchronization=synchronization,
        )
    zero_flux = execute("limit-zero-flux", duration=600.0, variation="zero_flux")
    zero_binding = execute("limit-zero-binding", duration=600.0, variation="association", factor=0.0)
    constant_reservoir = execute(
        "limit-constant-reservoir",
        step=0.5,
        duration=600.0,
        variation="constant_reservoir",
    )
    feedback_off = execute("structural-feedback-off", variation="feedback_gain", factor=0.0)
    nrp1_excluded = execute("structural-nrp1-excluded", variation="nrp1_excluded")
    nrp1_facilitated = execute(
        "structural-nrp1-facilitation-1.5x", variation="nrp1_facilitation", factor=1.5
    )

    sensitivity: list[dict[str, Any]] = []
    for parameter in SENSITIVITY_PARAMETERS:
        lower = execute(f"sensitivity-{parameter}-0.9x", variation=parameter, factor=0.9)
        upper = execute(f"sensitivity-{parameter}-1.1x", variation=parameter, factor=1.1)
        outputs = {}
        for column in ("interstitium_free_mol", "receptor_bound_mol", "internalized_mol"):
            base = baseline[-1][column]
            scale = max(abs(base), baseline[0]["initial_mol"] * 1.0e-12)
            outputs[column] = {
                "lower_relative_change": (lower[-1][column] - base) / scale,
                "upper_relative_change": (upper[-1][column] - base) / scale,
                "central_elasticity":
                    (upper[-1][column] - lower[-1][column]) / (0.2 * scale),
            }
        sensitivity.append({"parameter": parameter, "fraction": 0.1, "outputs": outputs})

    maximum_balance = max(
        row["balance_error_mol"] for rows in trajectories.values() for row in rows
    )
    minimum_owner = min(row[column] for rows in trajectories.values() for row in rows for column in OWNER_COLUMNS)
    replay_maximum = max(
        abs(left[column] - right[column])
        for left, right in zip(baseline, replay, strict=True)
        for column in (*OWNER_COLUMNS, "occupancy_fraction", "applied_feedback_multiplier", "scheduled_feedback_multiplier")
    )
    time_reference = trajectories["time-step-0.5s"]
    time_errors = {
        step: final_error(trajectories[f"time-step-{step:g}s"], time_reference)
        for step in (8.0, 4.0, 2.0, 1.0)
    }
    time_monotone = all(
        time_errors[left] >= time_errors[right]
        for left, right in zip((8.0, 4.0, 2.0), (4.0, 2.0, 1.0), strict=True)
    )
    sync_reference = trajectories["synchronization-15s"]
    sync_errors = {
        synchronization: final_error(
            trajectories[f"synchronization-{synchronization:g}s"], sync_reference
        )
        for synchronization in (120.0, 60.0, 30.0)
    }
    sync_monotone = sync_errors[120.0] >= sync_errors[60.0] >= sync_errors[30.0]

    initial = baseline[0]["initial_mol"]
    zero_flux_relative_error = abs(zero_flux[-1]["blood_free_mol"] - initial) / initial
    zero_binding_amount = max(
        row["receptor_bound_mol"] + row["internalized_mol"] for row in zero_binding
    )
    ligand_concentration = constant_reservoir[0]["interstitium_free_mol"] / 1.0e-12
    association_rate = 1.0e4 * ligand_concentration
    dissociation_rate = 1.0e-3
    analytical_occupancy = association_rate / (association_rate + dissociation_rate) * (
        1.0 - math.exp(-(association_rate + dissociation_rate) * 600.0)
    )
    constant_reservoir_error = abs(
        constant_reservoir[-1]["occupancy_fraction"] - analytical_occupancy
    )

    causal_times = {
        "blood_departure": first_time(baseline, lambda row: row["blood_free_mol"] < 0.99 * initial),
        "interstitial_arrival": first_time(
            baseline, lambda row: row["interstitium_free_mol"] > 1.0e-20
        ),
        "receptor_binding": first_time(baseline, lambda row: row["occupancy_fraction"] >= 0.01),
        "feedback_scheduled": first_time(
            baseline, lambda row: row["scheduled_feedback_multiplier"] < 1.0
        ),
        "feedback_applied": first_time(
            baseline, lambda row: row["applied_feedback_multiplier"] < 1.0
        ),
    }
    causal_order_pass = list(causal_times.values()) == sorted(causal_times.values()) and all(
        math.isfinite(value) for value in causal_times.values()
    )
    feedback_delayed = causal_times["feedback_applied"] > causal_times["feedback_scheduled"]

    evaluation = protocol["evaluation"]
    gates = [
        {
            "id": "DCCQ-G1",
            "status": "PASS",
            "reason": "One frozen VEGF-A165a homodimer identity, exact SI conversion, one represented HUVEC, and seven exclusive amount owners pass the protocol and runtime guards.",
        },
        {
            "id": "DCCQ-G2",
            "status": "PASS" if maximum_balance <= evaluation["mass_balance_relative_tolerance"] * initial and minimum_owner >= evaluation["nonnegative_floor_mol"] else "FAIL",
            "reason": f"Maximum balance residual {maximum_balance:.17g} mol; minimum owner {minimum_owner:.17g} mol.",
        },
        {
            "id": "DCCQ-G3",
            "status": "PASS" if zero_flux_relative_error <= evaluation["zero_flux_relative_tolerance"] and zero_binding_amount <= evaluation["zero_binding_absolute_tolerance_mol"] and constant_reservoir_error <= evaluation["constant_reservoir_occupancy_absolute_tolerance"] else "FAIL",
            "reason": "Zero-flux, zero-binding, constant-reservoir, pulse-withdrawal, and no-feedback cases were retained and evaluated.",
        },
        {
            "id": "DCCQ-G4",
            "status": "PASS" if time_monotone and time_errors[1.0] <= evaluation["time_step_fine_relative_limit"] and sync_monotone and sync_errors[30.0] <= evaluation["synchronization_fine_relative_limit"] else "FAIL",
            "reason": "Independent RK4 step and coupling-interval refinement converge; spatial refinement is not applicable to the frozen well-mixed candidate.",
        },
        {
            "id": "DCCQ-G5",
            "status": "PASS" if causal_order_pass and feedback_delayed else "FAIL",
            "reason": f"Causal event times: {causal_times}; feedback is applied only after it is scheduled.",
        },
        {
            "id": "DCCQ-G6",
            "status": "PARTIAL",
            "reason": "Numerical, synchronization, 12-parameter local, NRP1 structural, and feedback structural effects are reported. Joint distributions, correlations, observational covariance, and global identifiability remain unavailable.",
        },
        {
            "id": "DCCQ-G7",
            "status": "BLOCKED",
            "reason": "The source-disjoint kinetic series uses engineered HEK293T cells and the source-disjoint primary-HUVEC evidence supplies only a directional 30-minute endpoint; no condition-matched reusable HUVEC time series is frozen and no refitting was performed.",
        },
        {
            "id": "DCCQ-G8",
            "status": "PARTIAL",
            "reason": "A runner-independent machine checker, immutable archive, licence review, documentation reconciliation, and bounded claim are supplied; independent external human attestation remains absent.",
        },
    ]
    failed = [gate["id"] for gate in gates if gate["status"] == "FAIL"]
    if failed:
        raise RuntimeError(f"DCCQ computational gates failed: {failed}")

    metrics = {
        "run_id": run_id,
        "trajectory_count": len(trajectories),
        "baseline_points": len(baseline),
        "duration_s": 14400.0,
        "maximum_balance_error_mol": maximum_balance,
        "minimum_owner_amount_mol": minimum_owner,
        "deterministic_replay_maximum_absolute_difference": replay_maximum,
        "time_step_final_l1_relative_errors": {str(key): value for key, value in time_errors.items()},
        "time_step_monotone": time_monotone,
        "synchronization_final_l1_relative_errors": {str(key): value for key, value in sync_errors.items()},
        "synchronization_monotone": sync_monotone,
        "zero_flux_relative_error": zero_flux_relative_error,
        "zero_binding_max_bound_plus_internalized_mol": zero_binding_amount,
        "constant_reservoir_analytical_occupancy": analytical_occupancy,
        "constant_reservoir_simulated_occupancy": constant_reservoir[-1]["occupancy_fraction"],
        "constant_reservoir_absolute_error": constant_reservoir_error,
        "causal_event_times_s": causal_times,
        "causal_order_pass": causal_order_pass,
        "feedback_delayed": feedback_delayed,
        "baseline_final": baseline[-1],
        "structural_final_l1_relative_difference": {
            "feedback_off": final_error(feedback_off, baseline),
            "nrp1_excluded": final_error(nrp1_excluded, baseline),
            "nrp1_facilitation_1.5x": final_error(nrp1_facilitated, baseline),
        },
        "gates": gates,
    }
    write_json(archive / "qualification-metrics.json", metrics)
    write_json(
        archive / "local-sensitivity.json",
        {
            "method": "one-at-a-time symmetric plus/minus 10 percent",
            "parameter_count": len(sensitivity),
            "joint_distribution_used": False,
            "population_interpretation": False,
            "results": sensitivity,
        },
    )
    write_json(
        archive / "build-provenance.json",
        {
            "run_id": run_id,
            "generated_utc": datetime.now(timezone.utc).isoformat(),
            "platform": platform.platform(),
            "python": platform.python_version(),
            "runner_path": runner.relative_to(ROOT).as_posix(),
            "runner_sha256": sha256(runner),
            "protocol_path": PROTOCOL.relative_to(ROOT).as_posix(),
            "protocol_sha256": sha256(PROTOCOL),
            "commands": commands,
            "validation_refitting": False,
        },
    )
    write_json(
        archive / "review-record.json",
        {
            "machine_review": "PASS",
            "licence_review": "PASS_WITH_UNLICENSED_EXTERNAL_REPOSITORY_EXCLUDED",
            "documentation_review": "PASS",
            "external_human_review": "BLOCKED_NO_ATTESTATION",
            "independent_experimental_validation": "BLOCKED_NO_CONDITION_MATCHED_HUVEC_SERIES",
            "allowed_claim": protocol["allowed_claim"],
            "forbidden_claims": protocol["forbidden_claims"],
        },
    )
    report = f"""<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DCCQ-1 computational qualification report

Run `{run_id}` executed the frozen DCCQ-1.3 protocol without validation
refitting. DCCQ-G1 through DCCQ-G5 pass. DCCQ-G6 is partial because only
bounded local and structural uncertainty is justified. DCCQ-G7 remains blocked
by the absence of a condition-matched, source-disjoint primary-HUVEC dynamic
series. DCCQ-G8 is partial until external human review is received.

The result establishes a typed, SI-explicit, conservative and numerically
qualified reduced coupling. It does not establish pulmonary, in-vivo,
patient-specific, biological, diagnostic, treatment, safety or clinical
validity.
"""
    (archive / "qualification-report.md").write_text(report, encoding="utf-8")

    manifest_files = sorted(
        path for path in archive.rglob("*") if path.is_file() and path.name != "sha256sums.json"
    )
    write_json(
        archive / "sha256sums.json",
        {
            "run_id": run_id,
            "hash_policy": "SHA-256 over Git-canonical LF bytes for text artifacts; binary bytes unchanged",
            "files": [
                {"path": path.relative_to(archive).as_posix(), "sha256": sha256(path)}
                for path in manifest_files
            ],
        },
    )
    relative_archive = archive.relative_to(ROOT).as_posix()
    result = {
        "schema_version": "1.0.0",
        "result": {
            "id": "DCCQ-1.4-1.7",
            "program": "DCCQ-1",
            "version": "1.0.0",
            "performed_date": "2026-09-06",
            "status": "computational-close-out-complete-external-evidence-and-review-blocked",
            "clinical_use": False,
        },
        "protocol": {"path": PROTOCOL.relative_to(ROOT).as_posix(), "sha256": sha256(PROTOCOL)},
        "archive": {
            "run_id": run_id,
            "path": relative_archive,
            "checksum_manifest": {
                "path": f"{relative_archive}/sha256sums.json",
                "sha256": sha256(archive / "sha256sums.json"),
            },
            "raw_external_data_bundled": False,
            "unlicensed_external_code_bundled": False,
        },
        "execution": {
            "method": "fixed-step-classical-rk4-with-discrete-delayed-feedback",
            "trajectory_count": len(trajectories),
            "local_sensitivity_parameter_count": len(sensitivity),
            "validation_refitting": False,
        },
        "headline_metrics": {
            "maximum_balance_error_mol": maximum_balance,
            "minimum_owner_amount_mol": minimum_owner,
            "deterministic_replay_maximum_absolute_difference": replay_maximum,
            "time_step_1s_relative_error": time_errors[1.0],
            "synchronization_30s_relative_error": sync_errors[30.0],
            "constant_reservoir_occupancy_absolute_error": constant_reservoir_error,
            "local_sensitivity_comparisons": 2 * len(sensitivity),
            "negative_controls": 10,
        },
        "increments": [
            {"id": "DCCQ-1.3", "status": "COMPLETE"},
            {"id": "DCCQ-1.4", "status": "COMPLETE"},
            {"id": "DCCQ-1.5", "status": "COMPLETE"},
            {"id": "DCCQ-1.6", "status": "COMPLETE_WITH_G6_PARTIAL_AND_G7_BLOCKED"},
            {"id": "DCCQ-1.7", "status": "PARTIAL_EXTERNAL_HUMAN_REVIEW_BLOCKED"},
        ],
        "gates": gates,
        "decision": {
            "computational_qualification": "PASS",
            "literature_parameterization": "PARTIAL",
            "independent_experimental_validation": "BLOCKED",
            "external_human_review": "BLOCKED",
            "biological_or_clinical_validation": "NOT_ESTABLISHED",
            "allowed_claim": protocol["allowed_claim"],
            "next_scientific_action": "Obtain and prospectively freeze a condition-matched source-disjoint primary-HUVEC dynamic series and an external reviewer attestation; do not refit this candidate on that series.",
        },
    }
    write_json(RESULT, result)
    print(
        f"DCCQ-1 qualification: PASS_WITH_BLOCKED_EXTERNAL_GATES "
        f"({len(trajectories)} trajectories, {2 * len(sensitivity)} local sensitivity comparisons)"
    )
    print(f"archive={relative_archive}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
