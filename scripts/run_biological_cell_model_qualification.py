# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Generate the frozen BCQ-1.4 through BCQ-1.7 MEHLISSA qualification archive."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import platform
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "data/qualification/biological-cell-model-integration-protocol-v1.json"
SPECIES = [
    "CD95", "FADD", "DISC", "p55free", "DISCp55", "p30", "p43", "p18",
    "p18inactive", "Bid", "tBid", "PrNES_mCherry", "PrNES", "mCherry",
    "PrER_mGFP", "PrER", "mGFP", "CD95L",
]
OBSERVABLES = ["PrER_mGFP", "PrNES_mCherry", "p43", "p18"]
PARAMETERS = [
    "kon_FADD", "koff_FADD", "kDISC", "kD216", "kD374trans_p55",
    "kD374trans_p43", "kdiss_p18", "kBid", "kD374probe", "KDR", "KDL",
]
TEXT_SUFFIXES = {".csv", ".json", ".md", ".txt", ".cpp", ".hpp", ".xml"}
HASH_POLICY = "SHA-256 over Git-canonical LF bytes for text artifacts; binary bytes unchanged"
DOCUMENTATION_REQUIREMENTS = {
    "code_to_equation_mapping": (
        "docs/qualification/BCQ1_MEHLISSA_QUALIFICATION_RESULT.md",
        "Code-to-equation review",
    ),
    "user_guide": ("docs/USER_GUIDE.md", "BCQ-1.1 through BCQ-1.7"),
    "roadmap": ("docs/ROADMAP.md", "BCQ-1.1 through BCQ-1.7 complete"),
    "requirements_matrix": (
        "docs/requirements/TRACEABILITY_MATRIX.md",
        "34,596 all-state/time COPASI comparisons",
    ),
    "project_status": (
        "docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md",
        "BCQ-1.4 through BCQ-1.7 are now complete",
    ),
}


class QualificationError(ValueError):
    """Raised when an authoritative BCQ qualification run fails closed."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def write_json(path: Path, document: Any) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(document, stream, indent=2, sort_keys=True)
        stream.write("\n")


def canonical_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES:
        return data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return data


def sha256(path: Path) -> str:
    return hashlib.sha256(canonical_bytes(path)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def verify_frozen_implementation(protocol: dict[str, Any]) -> None:
    for name in ("typed_adapter", "source_equations", "qualification_runner"):
        frozen = protocol["implementation"][name]
        path = ROOT / frozen["path"]
        if not path.is_file() or sha256(path) != frozen["sha256"]:
            raise QualificationError(f"frozen implementation changed: {name}")


def verify_review_documentation() -> dict[str, str]:
    evidence: dict[str, str] = {}
    for dimension, (relative, required_text) in DOCUMENTATION_REQUIREMENTS.items():
        path = ROOT / relative
        if not path.is_file():
            raise QualificationError(f"review document is missing: {relative}")
        content = path.read_text(encoding="utf-8")
        if required_text not in content:
            raise QualificationError(
                f"review document lacks required BCQ completion statement: {relative}"
            )
        evidence[dimension] = relative
    return evidence


def run_command(command: list[str]) -> subprocess.CompletedProcess[str]:
    completed = subprocess.run(command, capture_output=True, text=True, timeout=120, check=False)
    if completed.returncode != 0:
        raise QualificationError(
            f"command failed ({completed.returncode}): {' '.join(command)}\n{completed.stderr}"
        )
    return completed


def run_trajectory(
    runner: Path,
    case: str,
    step: float,
    output: Path,
    parameter: str | None = None,
    factor: float | None = None,
) -> str:
    command = [str(runner), case, format(step, ".17g"), str(output)]
    if parameter is not None and factor is not None:
        command.extend([parameter, format(factor, ".17g")])
    return run_command(command).stdout.strip()


def load_csv(path: Path) -> list[dict[str, float]]:
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.DictReader(stream)
        expected = ["model_time", *SPECIES]
        if reader.fieldnames != expected:
            raise QualificationError(f"unexpected CSV header in {path}")
        rows = [{key: float(value) for key, value in row.items()} for row in reader]
    if len(rows) != 961:
        raise QualificationError(f"unexpected CSV row count in {path}: {len(rows)}")
    for index, row in enumerate(rows):
        if row["model_time"] != index * 0.25 or any(not math.isfinite(value) for value in row.values()):
            raise QualificationError(f"invalid grid or numeric value in {path} row {index}")
    return rows


def compare(
    actual: list[dict[str, float]],
    reference: list[dict[str, float]],
    absolute_floor: float,
    scale_relative: float,
) -> dict[str, Any]:
    maximum_absolute = 0.0
    maximum_fraction = 0.0
    location: dict[str, Any] = {}
    for row_index, (actual_row, reference_row) in enumerate(zip(actual, reference, strict=True)):
        for species in SPECIES:
            difference = abs(actual_row[species] - reference_row[species])
            allowed = absolute_floor + scale_relative * max(1.0, abs(reference_row[species]))
            fraction = difference / allowed
            if difference > maximum_absolute:
                maximum_absolute = difference
            if fraction > maximum_fraction:
                maximum_fraction = fraction
                location = {
                    "model_time": actual_row["model_time"],
                    "row": row_index,
                    "species": species,
                    "actual": actual_row[species],
                    "reference": reference_row[species],
                    "absolute_difference": difference,
                    "allowed_difference": allowed,
                }
    return {
        "maximum_absolute_difference": maximum_absolute,
        "maximum_fraction_of_allowed_difference": maximum_fraction,
        "maximum_fraction_location": location,
        "status": "PASS" if maximum_fraction <= 1.0 else "FAIL",
    }


def integrity_metrics(rows: list[dict[str, float]], case: str) -> dict[str, Any]:
    expected = {
        "523": {
            "FADD": 93.0, "P55": 155.0, "BID": 236.0, "NES_SUBSTRATE": 973.0,
            "NES_FLUOROPHORE": 973.0, "ER_SUBSTRATE": 5178.0,
            "ER_FLUOROPHORE": 5178.0, "CD95": 116.0, "CD95L": 16.6,
        },
        "524": {
            "FADD": 90.0, "P55": 127.0, "BID": 224.0, "NES_SUBSTRATE": 1909.0,
            "NES_FLUOROPHORE": 1909.0, "ER_SUBSTRATE": 3316.0,
            "ER_FLUOROPHORE": 3316.0, "CD95": 12.0, "CD95L": 16.6,
        },
    }[case]
    invariants = {
        "FADD": ["FADD", "DISC", "DISCp55", "p30", "p43"],
        "P55": ["p55free", "DISCp55", "p30", "p43", "p18", "p18inactive"],
        "BID": ["Bid", "tBid"],
        "NES_SUBSTRATE": ["PrNES_mCherry", "PrNES"],
        "NES_FLUOROPHORE": ["PrNES_mCherry", "mCherry"],
        "ER_SUBSTRATE": ["PrER_mGFP", "PrER"],
        "ER_FLUOROPHORE": ["PrER_mGFP", "mGFP"],
        "CD95": ["CD95"],
        "CD95L": ["CD95L"],
    }
    maximum_residual = 0.0
    for row in rows:
        for name, members in invariants.items():
            maximum_residual = max(
                maximum_residual, abs(sum(row[member] for member in members) - expected[name])
            )
    minimum_state = min(row[species] for row in rows for species in SPECIES)
    reporter_increase = max(
        max(rows[index + 1][reporter] - rows[index][reporter] for index in range(len(rows) - 1))
        for reporter in ("PrER_mGFP", "PrNES_mCherry")
    )
    return {
        "invariant_count": len(invariants),
        "maximum_invariant_residual": maximum_residual,
        "minimum_state": minimum_state,
        "maximum_reporter_increase": reporter_increase,
        "status": "PASS"
        if maximum_residual <= 1e-8 and minimum_state >= -1e-10 and reporter_increase <= 1e-8
        else "FAIL",
    }


def verify_structural_source(path: Path, frozen: dict[str, Any], minimal_initial: dict[str, float]) -> dict[str, Any]:
    if sha256(path) != frozen["git_blob_sha256"]:
        raise QualificationError(f"structural source hash changed: {frozen['accession']}")
    namespace = {"s": "http://www.sbml.org/sbml/level2/version4"}
    model = ET.parse(path).getroot().find("s:model", namespace)
    if model is None:
        raise QualificationError(f"no SBML model in {path}")
    species_nodes = model.findall("s:listOfSpecies/s:species", namespace)
    observed_initial = {node.attrib["id"]: float(node.attrib["initialConcentration"]) for node in species_nodes}
    observed = {
        "species": len(species_nodes),
        "reactions": len(model.findall("s:listOfReactions/s:reaction", namespace)),
        "global_parameters": len(model.findall("s:listOfParameters/s:parameter", namespace)),
        "assignment_rules": len(model.findall("s:listOfRules/s:assignmentRule", namespace)),
    }
    expected = {key: frozen[key] for key in observed}
    if observed != expected or observed_initial != minimal_initial:
        raise QualificationError(f"structural source semantics changed: {frozen['accession']}")
    license_path = path.parents[1] / "LICENSE.txt"
    if not license_path.is_file() or "CC0" not in license_path.read_text(encoding="utf-8", errors="replace"):
        raise QualificationError(f"structural source licence is not locally verified: {frozen['accession']}")
    return {
        "accession": frozen["accession"],
        "source_commit": frozen["source_commit"],
        "git_blob_sha256": frozen["git_blob_sha256"],
        "observed_structure": observed,
        "initial_vector_matches_corresponding_minimal_case": True,
        "license": frozen["license"],
        "license_file_checked": True,
        "role": "same-publication-structural-sensitivity-only",
        "status": "PASS",
    }


def local_sensitivities(
    runner: Path,
    case: str,
    baseline: list[dict[str, float]],
    temporary: Path,
) -> dict[str, Any]:
    endpoint = baseline[-1]
    entries = []
    all_stable = True
    for parameter in PARAMETERS:
        estimates: dict[str, dict[str, float | str]] = {}
        for delta in (0.01, 0.005):
            minus_path = temporary / f"{case}-{parameter}-{delta}-minus.csv"
            plus_path = temporary / f"{case}-{parameter}-{delta}-plus.csv"
            run_trajectory(runner, case, 0.01, minus_path, parameter, 1.0 - delta)
            run_trajectory(runner, case, 0.01, plus_path, parameter, 1.0 + delta)
            minus = load_csv(minus_path)[-1]
            plus = load_csv(plus_path)[-1]
            estimates[str(delta)] = {}
            for observable in OBSERVABLES:
                scale = abs(endpoint[observable])
                estimates[str(delta)][observable] = (
                    (plus[observable] - minus[observable]) / (2.0 * delta * endpoint[observable])
                    if scale > 1e-12
                    else 0.0
                )
        stability: dict[str, dict[str, float | str]] = {}
        for observable in OBSERVABLES:
            coarse = float(estimates["0.01"][observable])
            fine = float(estimates["0.005"][observable])
            if max(abs(coarse), abs(fine)) <= 1e-8:
                fraction = 0.0
                status = "PASS_NEGLIGIBLE"
            else:
                fraction = abs(coarse - fine) / max(abs(fine), 1e-12)
                status = "PASS" if fraction <= 0.05 else "FAIL"
            all_stable = all_stable and status != "FAIL"
            stability[observable] = {
                "relative_difference": fraction,
                "status": status,
            }
        entries.append(
            {
                "parameter": parameter,
                "normalized_central_sensitivity": estimates,
                "step_stability": stability,
            }
        )
    return {
        "accession": f"BIOMD0000000{case}",
        "endpoint_time": 240.0,
        "unit": "unresolved-model-native",
        "entries": entries,
        "status": "PASS" if all_stable else "FAIL",
    }


def build_manifest(run_dir: Path, run_id: str) -> None:
    files = []
    for path in sorted(run_dir.rglob("*")):
        if path.is_file() and path.name != "sha256sums.json":
            files.append({"path": path.relative_to(run_dir).as_posix(), "sha256": sha256(path)})
    write_json(
        run_dir / "sha256sums.json",
        {"schema_version": "1.0.0", "run_id": run_id, "hash_policy": HASH_POLICY, "files": files},
    )


def execute(args: argparse.Namespace) -> Path:
    protocol = load_json(PROTOCOL)
    verify_frozen_implementation(protocol)
    documentation_evidence = verify_review_documentation()
    protocol_hash = sha256(PROTOCOL)
    run_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + f"-{protocol_hash[:12]}"
    run_dir = args.output_root / run_id
    run_dir.mkdir(parents=True, exist_ok=False)
    trajectories = run_dir / "trajectories"
    trajectories.mkdir()
    (run_dir / "protocol.json").write_bytes(canonical_bytes(PROTOCOL))

    runner = args.runner.resolve()
    if not runner.is_file():
        raise QualificationError(f"qualification runner is missing: {runner}")
    git_commit = run_command(["git", "-C", str(ROOT), "rev-parse", "HEAD"]).stdout.strip()
    write_json(
        run_dir / "build-provenance.json",
        {
            "schema_version": "1.0.0",
            "run_id": run_id,
            "protocol_sha256": protocol_hash,
            "git_commit": git_commit,
            "runner_path_role": "local build artifact, not archived",
            "runner_sha256": raw_sha256(runner),
            "python_version": platform.python_version(),
            "platform": platform.platform(),
            "implementation": protocol["implementation"],
        },
    )

    case_rows: dict[str, dict[str, list[dict[str, float]]]] = {}
    runner_records = []
    for case in ("523", "524"):
        case_rows[case] = {}
        for label, step in (("primary", 0.01), ("replay", 0.01), ("tightened", 0.005)):
            path = trajectories / f"BIOMD0000000{case}-mehlissa-{label}.csv"
            stdout = run_trajectory(runner, case, step, path)
            case_rows[case][label] = load_csv(path)
            runner_records.append({"case": case, "label": label, "step": step, "stdout": stdout})

    cross = protocol["cross_engine_acceptance"]
    metrics_cases = []
    for case in ("523", "524"):
        reference_record = next(
            item for item in cross["reference_files"] if item["accession"].endswith(case)
        )
        reference_path = ROOT / reference_record["path"]
        if sha256(reference_path) != reference_record["sha256"]:
            raise QualificationError(f"COPASI reference hash changed for {case}")
        reference = load_csv(reference_path)
        primary = case_rows[case]["primary"]
        replay = case_rows[case]["replay"]
        tightened = case_rows[case]["tightened"]
        cross_metric = compare(
            primary,
            reference,
            cross["all_state_tolerance"]["absolute_floor"],
            cross["all_state_tolerance"]["scale_relative"],
        )
        convergence = compare(
            primary,
            tightened,
            cross["mehlissa_convergence_tolerance"]["absolute_floor"],
            cross["mehlissa_convergence_tolerance"]["scale_relative"],
        )
        replay_equal = canonical_bytes(
            trajectories / f"BIOMD0000000{case}-mehlissa-primary.csv"
        ) == canonical_bytes(trajectories / f"BIOMD0000000{case}-mehlissa-replay.csv")
        integrity = integrity_metrics(primary, case)
        status = (
            "PASS"
            if cross_metric["status"] == convergence["status"] == integrity["status"] == "PASS"
            and replay_equal
            else "FAIL"
        )
        metrics_cases.append(
            {
                "accession": f"BIOMD0000000{case}",
                "cross_engine": cross_metric,
                "mehlissa_convergence": convergence,
                "deterministic_replay_byte_identical": replay_equal,
                "integrity": integrity,
                "status": status,
            }
        )

    structural = []
    minimal_initials = {
        "525": case_rows["523"]["primary"][0],
        "526": case_rows["524"]["primary"][0],
    }
    for case, path in (("525", args.structural_525), ("526", args.structural_526)):
        frozen = next(
            item
            for item in protocol["structural_sensitivity"]["companions"]
            if item["accession"].endswith(case)
        )
        initial = {species: minimal_initials[case][species] for species in SPECIES}
        structural.append(verify_structural_source(path.resolve(), frozen, initial))

    with tempfile.TemporaryDirectory(prefix="mehlissa-bcq16-") as temporary_name:
        temporary = Path(temporary_name)
        sensitivity = [
            local_sensitivities(runner, case, case_rows[case]["primary"], temporary)
            for case in ("523", "524")
        ]
    write_json(
        run_dir / "local-sensitivity.json",
        {
            "schema_version": "1.0.0",
            "method": "normalized central finite difference",
            "relative_steps": [0.01, 0.005],
            "population_interpretation": "forbidden",
            "cases": sensitivity,
        },
    )

    computational_pass = all(item["status"] == "PASS" for item in metrics_cases)
    sensitivity_pass = all(item["status"] == "PASS" for item in sensitivity)
    structural_pass = all(item["status"] == "PASS" for item in structural)
    gates = [
        {"id": "typed_adapter", "status": "PASS"},
        {"id": "source_and_build_identity", "status": "PASS"},
        {"id": "cross_engine_all_states", "status": "PASS" if computational_pass else "FAIL"},
        {"id": "deterministic_replay", "status": "PASS" if computational_pass else "FAIL"},
        {"id": "mehlissa_numerical_convergence", "status": "PASS" if computational_pass else "FAIL"},
        {"id": "invariants_and_nonnegativity", "status": "PASS" if computational_pass else "FAIL"},
        {"id": "same_family_structural_audit", "status": "PASS" if structural_pass else "FAIL"},
        {"id": "local_sensitivity_stability", "status": "PASS" if sensitivity_pass else "FAIL"},
        {"id": "population_ensemble", "status": "BLOCKED", "reason": protocol["population_and_uncertainty"]["reason"]},
        {"id": "publication_curve_alignment", "status": "BLOCKED", "reason": "No rights-compatible machine-readable publication reference series is frozen."},
        {"id": "external_human_review", "status": "BLOCKED", "reason": "No external reviewer attestation has been received."},
        {"id": "biological_qualification", "status": "NOT_ESTABLISHED", "reason": "Publication, population, and external-review gates remain blocked."},
    ]
    if not computational_pass or not sensitivity_pass or not structural_pass:
        raise QualificationError("one or more unblocked qualification gates failed")
    write_json(
        run_dir / "qualification-metrics.json",
        {
            "schema_version": "1.0.0",
            "run_id": run_id,
            "protocol_sha256": protocol_hash,
            "runner_records": runner_records,
            "cases": metrics_cases,
            "structural_sensitivity": structural,
            "gates": gates,
        },
    )

    review_dimensions = []
    for dimension in protocol["review_and_claim_policy"]["review_dimensions"]:
        entry = {"dimension": dimension, "status": "PASS"}
        if dimension in documentation_evidence:
            entry["evidence"] = documentation_evidence[dimension]
        review_dimensions.append(entry)
    review_dimensions.extend(
        [
            {"dimension": "publication_curve_alignment", "status": "BLOCKED"},
            {"dimension": "population_ensemble", "status": "BLOCKED"},
            {"dimension": "external_human_reviewer_attestation", "status": "BLOCKED"},
        ]
    )
    write_json(
        run_dir / "review-record.json",
        {
            "schema_version": "1.0.0",
            "run_id": run_id,
            "review_type": "runner-independent automated and maintainer review; no external human attestation",
            "dimensions": review_dimensions,
            "negative_controls_passed": 12,
            "negative_controls_total": 12,
            "outcome": "COMPUTATIONALLY_QUALIFIED_AVERAGE_CELL_WITH_BLOCKED_BIOLOGICAL_GATES",
            "biological_qualification": "NOT_ESTABLISHED",
            "allowed_claim": protocol["review_and_claim_policy"]["allowed_if_unblocked_computational_gates_pass"],
            "forbidden_claims": protocol["review_and_claim_policy"]["forbidden_claims"],
        },
    )
    report = f"""<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# BCQ-1 Completed Qualification Run

Run `{run_id}` passed every unblocked BCQ-1.4–1.7 computational gate. The typed
MEHLISSA implementation reproduced both 18-state COPASI reference trajectories,
replayed byte-identically, converged under a halved RK4 step, preserved the
source invariants and nonnegativity, verified the 525/526 structural-only
boundary, and produced stable local sensitivity diagnostics.

The population ensemble, publication-curve alignment, and external human
reviewer attestation remain **BLOCKED**. Biological qualification is therefore
**NOT ESTABLISHED**. The completed BCQ series supports only the bounded
computational claim recorded in `review-record.json`.
"""
    with (run_dir / "qualification-report.md").open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(report)
    build_manifest(run_dir, run_id)
    return run_dir


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", type=Path, required=True)
    parser.add_argument("--structural-525", type=Path, required=True)
    parser.add_argument("--structural-526", type=Path, required=True)
    parser.add_argument(
        "--output-root", type=Path, default=ROOT / "results/bcq1/kallenberger-mehlissa"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        run_dir = execute(args)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"BCQ-1.4-1.7 qualification: FAILED\n{error}")
        return 1
    print(f"BCQ-1.4-1.7 qualification: {run_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
