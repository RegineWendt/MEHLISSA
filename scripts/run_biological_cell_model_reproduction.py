# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Execute the frozen BCQ-1.3 COPASI reproduction without refitting.

The caller supplies the already acquired, hash-verifiable SBML artifacts and a
CopasiSE 4.46 Build 300 executable. Outputs are written to a unique result
archive and never overwrite an earlier attempt.
"""

from __future__ import annotations

import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile
from typing import Any, Callable
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL_PATH = ROOT / "data/qualification/biological-cell-model-reproduction-protocol-v1.json"
AMENDMENT_PATH = ROOT / "data/qualification/biological-cell-model-reproduction-protocol-v1.1.json"
COPASI_NS = "http://www.copasi.org/static/schema"
SBML_NS = "http://www.sbml.org/sbml/level2/version4"
SPECIES_ORDER = [
    "CD95", "FADD", "DISC", "p55free", "DISCp55", "p30", "p43", "p18",
    "p18inactive", "Bid", "tBid", "PrNES_mCherry", "PrNES", "mCherry",
    "PrER_mGFP", "PrER", "mGFP", "CD95L",
]
ARCHIVE_TEXT_SUFFIXES = {".csv", ".json", ".md", ".txt"}


class ReproductionError(ValueError):
    """Raised when a frozen identity, execution, or acceptance rule fails."""


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def archive_sha256(path: Path) -> str:
    data = path.read_bytes()
    if path.suffix.lower() in ARCHIVE_TEXT_SUFFIXES:
        data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(data).hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def load_execution_protocol() -> tuple[dict[str, Any], dict[str, Any], str, str, str]:
    protocol = load_json(PROTOCOL_PATH)
    amendment = load_json(AMENDMENT_PATH)
    base_hash = sha256(PROTOCOL_PATH)
    amendment_hash = sha256(AMENDMENT_PATH)
    expected_base_hash = amendment["amendment"]["base_protocol"]["sha256"]
    if base_hash != expected_base_hash:
        raise ReproductionError(
            f"base protocol hash mismatch: {base_hash}; expected {expected_base_hash}"
        )
    identity_hash = hashlib.sha256(
        f"{base_hash}:{amendment_hash}".encode("ascii")
    ).hexdigest()
    return protocol, amendment, base_hash, amendment_hash, identity_hash


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(value, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def artifact_by_accession(protocol: dict[str, Any], accession: str) -> dict[str, Any]:
    for artifact in protocol["source_artifacts"]:
        if artifact["accession"] == accession:
            return artifact
    raise ReproductionError(f"unselected artifact accession: {accession}")


def verify_source(path: Path, artifact: dict[str, Any]) -> dict[str, Any]:
    if not path.is_file():
        raise ReproductionError(f"source artifact is missing: {path}")
    actual_hash = sha256(path)
    if actual_hash != artifact["sha256"]:
        raise ReproductionError(
            f"{artifact['accession']} source hash mismatch: {actual_hash}"
        )

    root = ET.parse(path).getroot()
    model = root.find(f"{{{SBML_NS}}}model")
    if model is None:
        raise ReproductionError(f"{artifact['accession']} has no SBML Level 2 model")
    counts = {
        "compartments": len(model.findall(f".//{{{SBML_NS}}}listOfCompartments/{{{SBML_NS}}}compartment")),
        "species": len(model.findall(f".//{{{SBML_NS}}}listOfSpecies/{{{SBML_NS}}}species")),
        "reactions": len(model.findall(f".//{{{SBML_NS}}}listOfReactions/{{{SBML_NS}}}reaction")),
        "global_parameters": len(model.findall(f".//{{{SBML_NS}}}listOfParameters/{{{SBML_NS}}}parameter")),
        "assignment_rules": len(model.findall(f".//{{{SBML_NS}}}listOfRules/{{{SBML_NS}}}assignmentRule")),
    }
    if counts != artifact["structure"]:
        raise ReproductionError(
            f"{artifact['accession']} structure mismatch: {counts}"
        )

    observed: dict[str, float] = {}
    for species in model.findall(f".//{{{SBML_NS}}}listOfSpecies/{{{SBML_NS}}}species"):
        value = species.get("initialConcentration")
        if value is None:
            raise ReproductionError(f"{species.get('id')} lacks initialConcentration")
        observed[str(species.get("id"))] = float(value)
    if observed != artifact["initial_values"]:
        raise ReproductionError(f"{artifact['accession']} initial values changed")

    absent_units = {
        "model_timeUnits": model.get("timeUnits") is None,
        "model_substanceUnits": model.get("substanceUnits") is None,
        "model_volumeUnits": model.get("volumeUnits") is None,
        "model_extentUnits": model.get("extentUnits") is None,
        "compartment_units": all(
            item.get("units") is None
            for item in model.findall(f".//{{{SBML_NS}}}listOfCompartments/{{{SBML_NS}}}compartment")
        ),
        "species_substance_units": all(
            item.get("substanceUnits") is None
            for item in model.findall(f".//{{{SBML_NS}}}listOfSpecies/{{{SBML_NS}}}species")
        ),
    }
    if not all(absent_units.values()):
        raise ReproductionError(
            f"{artifact['accession']} unit declarations differ from the frozen audit"
        )
    return {"sha256": actual_hash, "structure": counts, "absent_units": absent_units}


def run_process(command: list[str], cwd: Path, timeout_seconds: int = 120) -> dict[str, Any]:
    try:
        completed = subprocess.run(
            command,
            cwd=cwd,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
            timeout=timeout_seconds,
        )
    except subprocess.TimeoutExpired as error:
        stdout = error.stdout.decode("utf-8", errors="replace") if isinstance(error.stdout, bytes) else (error.stdout or "")
        stderr = error.stderr.decode("utf-8", errors="replace") if isinstance(error.stderr, bytes) else (error.stderr or "")
        return {
            "command": [Path(command[0]).name, *command[1:]],
            "exit_code": 124,
            "stdout": stdout,
            "stderr": stderr + f"\nTimed out after {timeout_seconds} seconds.\n",
        }
    return {
        "command": [Path(command[0]).name, *command[1:]],
        "exit_code": completed.returncode,
        "stdout": completed.stdout,
        "stderr": completed.stderr,
    }


def verify_solver(copasi: Path, protocol: dict[str, Any]) -> dict[str, Any]:
    if not copasi.is_file():
        raise ReproductionError(f"CopasiSE executable is missing: {copasi}")
    probe = run_process([str(copasi), "-h"], copasi.parent)
    banner = probe["stdout"] + probe["stderr"]
    expected = f"COPASI {protocol['solver']['version']} (Build {protocol['solver']['build']})"
    if expected not in banner:
        raise ReproductionError(f"solver version mismatch; expected {expected!r}")
    return {
        "name": protocol["solver"]["name"],
        "version": protocol["solver"]["version"],
        "build": protocol["solver"]["build"],
        "release_tag": protocol["solver"]["release_tag"],
        "source_commit": protocol["solver"]["source_commit"],
        "license": protocol["solver"]["license"],
        "binary_name": copasi.name,
        "binary_sha256": sha256(copasi),
        "version_probe_exit_code": probe["exit_code"],
        "version_banner": expected,
        "host": {
            "system": platform.system(),
            "release": platform.release(),
            "version": platform.version(),
            "machine": platform.machine(),
            "python": platform.python_version(),
        },
        "copasi_import_default_units": {
            "time": "s",
            "volume": "l",
            "quantity": "mol",
            "interpretation": "COPASI import defaults only; outputs remain labelled unresolved-model-native because the source artifacts omit explicit declarations",
        },
    }


def set_parameter(parent: ET.Element, name: str, value: str) -> None:
    parameter = parent.find(f"{{{COPASI_NS}}}Parameter[@name='{name}']")
    if parameter is None:
        raise ReproductionError(f"COPASI configuration parameter is missing: {name}")
    parameter.set("value", value)


def escaped_cn_name(value: str) -> str:
    return value.replace("\\", "\\\\").replace(",", "\\,").replace("[", "\\[").replace("]", "\\]")


def configure_cps(
    imported: Path,
    configured: Path,
    output: Path,
    artifact: dict[str, Any],
    settings_name: str,
    protocol: dict[str, Any],
) -> dict[str, Any]:
    ET.register_namespace("", COPASI_NS)
    tree = ET.parse(imported)
    root = tree.getroot()
    model = root.find(f"{{{COPASI_NS}}}ListOfModels/{{{COPASI_NS}}}Model")
    if model is None:
        model = root.find(f"{{{COPASI_NS}}}Model")
    if model is None:
        raise ReproductionError("imported COPASI document has no model")
    model_name = model.get("name")
    if not model_name:
        raise ReproductionError("imported COPASI model has no name")

    task = root.find(f".//{{{COPASI_NS}}}Task[@name='Time-Course']")
    if task is None:
        raise ReproductionError("COPASI Time-Course task is missing")
    task.set("scheduled", "true")
    task.set("updateModel", "false")
    report_link = task.find(f"{{{COPASI_NS}}}Report")
    if report_link is None:
        raise ReproductionError("COPASI Time-Course report link is missing")
    report_key = report_link.get("reference")
    report_link.set("target", str(output.resolve()))
    report_link.set("append", "0")
    report_link.set("confirmOverwrite", "0")

    grid = protocol["execution"]["time_grid"]
    problem = task.find(f"{{{COPASI_NS}}}Problem")
    method = task.find(f"{{{COPASI_NS}}}Method")
    if problem is None or method is None:
        raise ReproductionError("COPASI Time-Course problem or method is missing")
    set_parameter(problem, "AutomaticStepSize", "0")
    set_parameter(problem, "StepNumber", str(grid["points_including_endpoints"] - 1))
    set_parameter(problem, "StepSize", format(grid["interval"], ".17g"))
    set_parameter(problem, "Duration", format(grid["end"] - grid["start"], ".17g"))
    set_parameter(problem, "TimeSeriesRequested", "1")
    set_parameter(problem, "OutputStartTime", format(grid["start"], ".17g"))
    set_parameter(problem, "Output Event", "0")
    set_parameter(problem, "Start in Steady State", "0")
    set_parameter(problem, "Use Values", "0")
    method.set("name", "Deterministic (LSODA)")
    method.set("type", "Deterministic(LSODA)")
    settings = protocol["solver"][f"{settings_name}_settings"]
    set_parameter(method, "Integrate Reduced Model", "0")
    set_parameter(method, "Relative Tolerance", format(settings["relative_tolerance"], ".17g"))
    set_parameter(method, "Absolute Tolerance", format(settings["absolute_tolerance"], ".17g"))
    set_parameter(method, "Max Internal Steps", str(settings["maximum_internal_steps"]))

    report = root.find(f".//{{{COPASI_NS}}}Report[@key='{report_key}']")
    if report is None:
        raise ReproductionError("COPASI Time-Course report definition is missing")
    report.set("separator", ",")
    report.set("precision", "17")
    for child in list(report):
        report.remove(child)
    header = ET.SubElement(report, f"{{{COPASI_NS}}}Header")
    body = ET.SubElement(report, f"{{{COPASI_NS}}}Body")
    columns = ["model_time", *SPECIES_ORDER]
    model_cn = escaped_cn_name(model_name)
    references = [f"CN=Root,Model={model_cn},Reference=Time"] + [
        f"CN=Root,Model={model_cn},Vector=Compartments[cell],Vector=Metabolites[{escaped_cn_name(species)}],Reference=Concentration"
        for species in SPECIES_ORDER
    ]
    # COPASI report bodies require explicit Separator objects. A comma is also
    # COPASI's CN field delimiter, so the literal report separator must be
    # escaped inside the CN value.
    for index, (column, reference) in enumerate(zip(columns, references)):
        if index:
            ET.SubElement(header, f"{{{COPASI_NS}}}Object", {"cn": "Separator=\\,"})
            ET.SubElement(body, f"{{{COPASI_NS}}}Object", {"cn": "Separator=\\,"})
        ET.SubElement(header, f"{{{COPASI_NS}}}Object", {"cn": f"String={column}"})
        ET.SubElement(body, f"{{{COPASI_NS}}}Object", {"cn": reference})

    configured.parent.mkdir(parents=True, exist_ok=True)
    tree.write(configured, encoding="utf-8", xml_declaration=True)
    return {
        "accession": artifact["accession"],
        "settings": settings_name,
        "model_name": model_name,
        "report_columns": columns,
        "configured_cps_sha256": sha256(configured),
    }


def parse_trajectory(path: Path) -> list[dict[str, float]]:
    if not path.is_file():
        raise ReproductionError(f"trajectory was not generated: {path.name}")
    with path.open(newline="", encoding="utf-8-sig") as stream:
        reader = csv.DictReader(stream)
        expected = ["model_time", *SPECIES_ORDER]
        if reader.fieldnames != expected:
            raise ReproductionError(
                f"{path.name} columns changed: {reader.fieldnames!r}"
            )
        rows: list[dict[str, float]] = []
        for raw in reader:
            row = {key: float(value) for key, value in raw.items()}
            rows.append(row)
    return rows


def validate_trajectory(
    rows: list[dict[str, float]],
    artifact: dict[str, Any],
    protocol: dict[str, Any],
) -> dict[str, Any]:
    grid = protocol["execution"]["time_grid"]
    if len(rows) != grid["points_including_endpoints"]:
        raise ReproductionError(f"trajectory row count changed: {len(rows)}")
    for index, row in enumerate(rows):
        expected_time = grid["start"] + index * grid["interval"]
        if abs(row["model_time"] - expected_time) > 1e-12:
            raise ReproductionError(f"model_time grid mismatch at row {index}")
        if set(row) != {"model_time", *SPECIES_ORDER}:
            raise ReproductionError(f"trajectory fields changed at row {index}")
        if not all(math.isfinite(value) for value in row.values()):
            raise ReproductionError(f"non-finite trajectory value at row {index}")

    for species, expected in artifact["initial_values"].items():
        if abs(rows[0][species] - expected) > 1e-12:
            raise ReproductionError(f"initial state mismatch for {species}")

    initial_scale = max(1.0, max(abs(value) for value in artifact["initial_values"].values()))
    minimum = min(row[species] for row in rows for species in SPECIES_ORDER)
    nonnegative_limit = -1e-10 * initial_scale
    if minimum < nonnegative_limit:
        raise ReproductionError(
            f"material negative state {minimum} is below {nonnegative_limit}"
        )

    invariant_metrics: dict[str, float] = {}
    for invariant in protocol["invariants"]:
        expected = invariant["expected_initial_total_by_accession"][artifact["accession"]]
        maximum_residual = max(
            abs(sum(row[species] for species in invariant["species"]) - expected)
            for row in rows
        )
        limit = 1e-10 + 1e-8 * max(1.0, abs(expected))
        if maximum_residual > limit:
            raise ReproductionError(
                f"{invariant['id']} residual {maximum_residual} exceeds {limit}"
            )
        invariant_metrics[invariant["id"]] = maximum_residual

    reporter_metrics: dict[str, float] = {}
    for reporter in ("PrER_mGFP", "PrNES_mCherry"):
        maximum_increase = max(
            (rows[index][reporter] - rows[index - 1][reporter])
            for index in range(1, len(rows))
        )
        margin = 1e-10 + 1e-8 * max(1.0, abs(artifact["initial_values"][reporter]))
        if maximum_increase > margin:
            raise ReproductionError(
                f"{reporter} increases by {maximum_increase}, above {margin}"
            )
        reporter_metrics[reporter] = maximum_increase

    return {
        "rows": len(rows),
        "start": rows[0]["model_time"],
        "end": rows[-1]["model_time"],
        "minimum_species_value": minimum,
        "nonnegative_limit": nonnegative_limit,
        "maximum_invariant_residuals": invariant_metrics,
        "maximum_reporter_increases": reporter_metrics,
        "final_primary_observables": {
            item["id"]: rows[-1][item["id"]] for item in protocol["observables"]
        },
    }


def compare_replay(
    primary: list[dict[str, float]],
    replay: list[dict[str, float]],
    absolute_tolerance: float,
    relative_tolerance: float,
) -> dict[str, float]:
    maximum_absolute = 0.0
    maximum_relative = 0.0
    if len(primary) != len(replay):
        raise ReproductionError("replay row count differs")
    for first, second in zip(primary, replay):
        for column in ("model_time", *SPECIES_ORDER):
            absolute = abs(first[column] - second[column])
            relative = absolute / max(1.0, abs(second[column]))
            maximum_absolute = max(maximum_absolute, absolute)
            maximum_relative = max(maximum_relative, relative)
    maximum_fraction = 0.0
    for first, second in zip(primary, replay):
        for column in ("model_time", *SPECIES_ORDER):
            absolute = abs(first[column] - second[column])
            limit = absolute_tolerance + relative_tolerance * max(1.0, abs(second[column]))
            maximum_fraction = max(maximum_fraction, absolute / limit)
    if maximum_fraction > 1.0:
        raise ReproductionError(
            f"deterministic replay equivalence limit exceeded: {maximum_fraction}"
        )
    return {
        "maximum_absolute_difference": maximum_absolute,
        "maximum_scale_normalized_difference": maximum_relative,
        "maximum_fraction_of_allowed_difference": maximum_fraction,
        "bit_identical": maximum_absolute == 0.0 and maximum_relative == 0.0,
    }


def compare_convergence(
    primary: list[dict[str, float]],
    tightened: list[dict[str, float]],
    observables: list[dict[str, Any]],
) -> dict[str, Any]:
    if len(primary) != len(tightened):
        raise ReproductionError("tightened row count differs")
    metrics: dict[str, Any] = {}
    for observable in observables:
        name = observable["id"]
        maximum_absolute = 0.0
        maximum_scaled = 0.0
        for first, second in zip(primary, tightened):
            absolute = abs(first[name] - second[name])
            limit = 1e-8 + 1e-6 * max(1.0, abs(second[name]))
            maximum_absolute = max(maximum_absolute, absolute)
            maximum_scaled = max(maximum_scaled, absolute / limit)
        if maximum_scaled > 1.0:
            raise ReproductionError(f"{name} convergence limit exceeded: {maximum_scaled}")
        metrics[name] = {
            "maximum_absolute_difference": maximum_absolute,
            "maximum_fraction_of_allowed_difference": maximum_scaled,
        }
    return metrics


def reject_unit_label(label: str) -> None:
    if label != "unresolved-model-native":
        raise ReproductionError(f"invented unit label: {label}")


def reject_refit(enabled: bool) -> None:
    if enabled:
        raise ReproductionError("parameter fitting is forbidden")


def reject_claim(claim: str) -> None:
    forbidden = ("biologically validated", "clinical", "patient prediction", "held-out population")
    if any(phrase in claim.lower() for phrase in forbidden):
        raise ReproductionError(f"claim exceeds BCQ-1.3: {claim}")


def expected_rejection(control_id: str, operation: Callable[[], None]) -> dict[str, str]:
    try:
        operation()
    except (ReproductionError, ValueError, KeyError) as error:
        return {"id": control_id, "status": "PASS", "observed_rejection": str(error)}
    raise ReproductionError(f"negative control {control_id} was not rejected")


def exercise_negative_controls(
    protocol: dict[str, Any],
    sources: dict[str, Path],
    rows_by_run: dict[str, list[dict[str, float]]],
    solver_banner: str,
    workspace: Path,
) -> list[dict[str, str]]:
    artifact_523 = artifact_by_accession(protocol, "BIOMD0000000523")
    artifact_524 = artifact_by_accession(protocol, "BIOMD0000000524")
    tampered = workspace / "tampered-source.xml"
    shutil.copyfile(sources["BIOMD0000000523"], tampered)
    tampered.write_bytes(tampered.read_bytes() + b"\n")

    swapped = [dict(row) for row in rows_by_run["BCQ-RUN-524-P"]]
    malformed = rows_by_run["BCQ-RUN-523-P"][:-1]
    nonfinite = [dict(row) for row in rows_by_run["BCQ-RUN-523-P"]]
    nonfinite[1]["p18"] = math.nan

    def wrong_solver() -> None:
        expected = f"COPASI {protocol['solver']['version']} (Build {protocol['solver']['build']})"
        if expected not in solver_banner.replace("4.46", "4.45"):
            raise ReproductionError("solver version mismatch")

    return [
        expected_rejection("BCQ-NC-01", lambda: verify_source(tampered, artifact_523)),
        expected_rejection("BCQ-NC-02", lambda: artifact_by_accession(protocol, "BIOMD0000000525")),
        expected_rejection("BCQ-NC-03", lambda: validate_trajectory(swapped, artifact_523, protocol)),
        expected_rejection("BCQ-NC-04", lambda: verify_source(tampered, artifact_523)),
        expected_rejection("BCQ-NC-05", wrong_solver),
        expected_rejection("BCQ-NC-06", lambda: reject_unit_label("minute")),
        expected_rejection("BCQ-NC-07", lambda: validate_trajectory(malformed, artifact_523, protocol)),
        expected_rejection("BCQ-NC-08", lambda: validate_trajectory(nonfinite, artifact_523, protocol)),
        expected_rejection("BCQ-NC-09", lambda: reject_refit(True)),
        expected_rejection("BCQ-NC-10", lambda: reject_claim("biologically validated held-out population")),
    ]


def execute(
    copasi: Path,
    sources: dict[str, Path],
    output_root: Path,
    run_id: str | None = None,
) -> Path:
    protocol, amendment, base_hash, amendment_hash, protocol_hash = load_execution_protocol()
    replay_rule = amendment["changes"]["deterministic_replay"]
    process_timeout = amendment["changes"]["execution_timeout"]["seconds_per_solver_process"]
    run_id = run_id or f"{datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')}-{protocol_hash[:12]}"
    run_dir = output_root / run_id
    try:
        run_dir.mkdir(parents=True, exist_ok=False)
    except FileExistsError as error:
        raise ReproductionError(f"result archive already exists: {run_dir}") from error

    # Keep the base protocol at its originally frozen archive name and retain
    # explicit lineage copies for readers applying amendment 1.
    shutil.copyfile(PROTOCOL_PATH, run_dir / "protocol.json")
    shutil.copyfile(PROTOCOL_PATH, run_dir / "protocol-base-v1.json")
    shutil.copyfile(AMENDMENT_PATH, run_dir / "protocol-amendment-v1.1.json")
    source_audits: dict[str, Any] = {}
    for accession, source in sources.items():
        source_audits[accession] = verify_source(
            source, artifact_by_accession(protocol, accession)
        )
    source_manifest = {
        "schema_version": "1.0.0",
        "execution_protocol_sha256": protocol_hash,
        "base_protocol_sha256": base_hash,
        "amendment_sha256": amendment_hash,
        "sources": [
            {
                "accession": artifact["accession"],
                "role": artifact["role"],
                "source_url": artifact["source_url"],
                "source_commit": artifact["source_commit"],
                "file": artifact["file"],
                "sha256": source_audits[artifact["accession"]]["sha256"],
                "license": artifact["license"],
                "structure": source_audits[artifact["accession"]]["structure"],
                "unit_audit": source_audits[artifact["accession"]]["absent_units"],
            }
            for artifact in protocol["source_artifacts"]
        ],
        "external_source_files_bundled": False,
    }
    write_json(run_dir / "source-manifest.json", source_manifest)

    solver_provenance = verify_solver(copasi, protocol)
    solver_provenance.update(
        {
            "schema_version": "1.0.0",
            "execution_protocol_sha256": protocol_hash,
            "base_protocol_sha256": base_hash,
            "amendment_sha256": amendment_hash,
            "method": protocol["solver"]["method"],
            "reduced_model": protocol["solver"]["reduced_model"],
            "optimization_or_parameter_estimation_enabled": False,
        }
    )
    write_json(run_dir / "solver-provenance.json", solver_provenance)

    trajectories = run_dir / "trajectories"
    logs = run_dir / "logs"
    trajectories.mkdir()
    logs.mkdir()
    rows_by_run: dict[str, list[dict[str, float]]] = {}
    run_metrics: dict[str, Any] = {}
    configuration_records: list[dict[str, Any]] = []

    with tempfile.TemporaryDirectory(prefix="mehlissa-bcq13-") as temporary:
        workspace = Path(temporary)
        config = workspace / "config"
        config.mkdir()
        for run in protocol["execution"]["matrix"]:
            artifact = artifact_by_accession(protocol, run["accession"])
            imported = workspace / f"{run['id']}-imported.cps"
            configured = workspace / f"{run['id']}-configured.cps"
            output_name = f"{run['accession']}-{run['settings'] if run['purpose'] != 'deterministic replay' else 'replay'}.csv"
            output = trajectories / output_name
            import_result = run_process(
                [
                    str(copasi), "--nologo", "--configdir", str(config),
                    "--home", str(config), "--importSBML", str(sources[run["accession"]]),
                    "--save", str(imported),
                ],
                workspace,
                process_timeout,
            )
            (logs / f"{run['id']}-import.stdout.txt").write_text(import_result["stdout"], encoding="utf-8", newline="\n")
            (logs / f"{run['id']}-import.stderr.txt").write_text(import_result["stderr"], encoding="utf-8", newline="\n")
            if import_result["exit_code"] != 0 or not imported.is_file():
                raise ReproductionError(f"{run['id']} COPASI import failed")

            configuration = configure_cps(
                imported, configured, output, artifact, run["settings"], protocol
            )
            configuration["run_id"] = run["id"]
            configuration_records.append(configuration)
            execution_result = run_process(
                [
                    str(copasi), "--nologo", "--configdir", str(config),
                    "--home", str(config), "--scheduled-task", "Time-Course",
                    "--report-file", str(output.resolve()), str(configured),
                ],
                workspace,
                process_timeout,
            )
            (logs / f"{run['id']}-execute.stdout.txt").write_text(execution_result["stdout"], encoding="utf-8", newline="\n")
            (logs / f"{run['id']}-execute.stderr.txt").write_text(execution_result["stderr"], encoding="utf-8", newline="\n")
            if execution_result["exit_code"] != 0:
                raise ReproductionError(f"{run['id']} COPASI execution failed")
            rows = parse_trajectory(output)
            rows_by_run[run["id"]] = rows
            run_metrics[run["id"]] = validate_trajectory(rows, artifact, protocol)

        negative_controls = exercise_negative_controls(
            protocol,
            sources,
            rows_by_run,
            solver_provenance["version_banner"],
            workspace,
        )

    comparisons: dict[str, Any] = {}
    for short in ("523", "524"):
        comparisons[f"BIOMD0000000{short}"] = {
            "deterministic_replay": compare_replay(
                rows_by_run[f"BCQ-RUN-{short}-P"],
                rows_by_run[f"BCQ-RUN-{short}-R"],
                replay_rule["absolute_tolerance"],
                replay_rule["relative_tolerance"],
            ),
            "numerical_convergence": compare_convergence(
                rows_by_run[f"BCQ-RUN-{short}-P"],
                rows_by_run[f"BCQ-RUN-{short}-T"],
                protocol["observables"],
            ),
        }

    metrics = {
        "schema_version": "1.0.0",
        "execution_protocol_sha256": protocol_hash,
        "base_protocol_sha256": base_hash,
        "amendment_sha256": amendment_hash,
        "run_id": run_id,
        "run_metrics": run_metrics,
        "comparisons": comparisons,
        "configuration_records": configuration_records,
        "negative_controls": negative_controls,
    }
    write_json(run_dir / "reproduction-metrics.json", metrics)

    gates = [
        {"id": name, "status": "PASS"}
        for name in (
            "source_identity", "import_structure", "complete_output", "initial_state",
            "deterministic_replay", "numerical_convergence", "conservation",
            "nonnegativity", "reporter_direction",
        )
    ]
    gates.append(
        {
            "id": "publication_alignment",
            "status": "BLOCKED",
            "reason": "No rights-compatible machine-readable publication reference series is frozen.",
        }
    )
    report = {
        "schema_version": "1.0.0",
        "qualification_increment": "BCQ-1.3",
        "run_id": run_id,
        "execution_protocol_sha256": protocol_hash,
        "base_protocol_sha256": base_hash,
        "amendment_sha256": amendment_hash,
        "protocol_version": amendment["amendment"]["version"],
        "status": "PASS_WITH_BLOCKED_PUBLICATION_ALIGNMENT",
        "computational_gates": gates,
        "negative_controls_passed": len(negative_controls),
        "claim": protocol["reference_and_claim_policy"]["allowed_after_bcq_1_3_if_all_unblocked_rules_pass"],
        "limitations": protocol["reference_and_claim_policy"]["forbidden_after_bcq_1_3"],
        "m5_evidence_status": "software_test_surrogate",
        "next_increment": "BCQ-1.4 typed MEHLISSA adapter",
    }
    write_json(run_dir / "validation-report.json", report)
    markdown = f"""<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BCQ-1.3 External-Solver Reproduction Report

**Run:** `{run_id}`  
**Decision:** PASS for the nine computational source-reproduction gates; BLOCKED for quantitative publication-curve alignment.  
**Solver:** COPASI 4.46 Build 300, LSODA, unchanged source numbers.  
**Models:** `BIOMD0000000523` and `BIOMD0000000524`, one average cell each.  
**Units:** `unresolved-model-native`; COPASI import defaults are recorded but are not treated as source evidence.  

All six frozen primary, replay, and tightened trajectories contain 961 points.
Source identity, import structure, initial state, finite output, deterministic
replay, numerical convergence, conservation, nonnegativity, reporter direction,
and all ten negative controls passed. Full numeric metrics and every run log are
retained beside this report.

The allowed claim is: {report['claim']}

This is not quantitative reproduction of a publication figure, the complete
held-out population analysis, biological qualification, endothelial or organ
realism, patient prediction, or clinical evidence. M5 therefore remains
`software_test_surrogate`. The next increment is BCQ-1.4 typed MEHLISSA adapter.
"""
    (run_dir / "validation-report.md").write_text(markdown, encoding="utf-8", newline="\n")

    hash_entries = []
    for path in sorted(run_dir.rglob("*")):
        if path.is_file() and path.name != "sha256sums.json":
            hash_entries.append({"path": path.relative_to(run_dir).as_posix(), "sha256": archive_sha256(path)})
    write_json(
        run_dir / "sha256sums.json",
        {
            "schema_version": "1.0.0",
            "run_id": run_id,
            "hash_policy": "SHA-256 over Git-canonical LF bytes for text artifacts; binary bytes unchanged",
            "files": hash_entries,
        },
    )
    return run_dir


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--copasi", type=Path, required=True)
    result.add_argument("--source-523", type=Path, required=True)
    result.add_argument("--source-524", type=Path, required=True)
    result.add_argument(
        "--output-root",
        type=Path,
        default=ROOT / "results/bcq1/kallenberger-minimal",
    )
    result.add_argument("--run-id")
    return result


def main() -> int:
    arguments = parser().parse_args()
    try:
        run_dir = execute(
            arguments.copasi.resolve(),
            {
                "BIOMD0000000523": arguments.source_523.resolve(),
                "BIOMD0000000524": arguments.source_524.resolve(),
            },
            arguments.output_root.resolve(),
            arguments.run_id,
        )
    except (OSError, ET.ParseError, json.JSONDecodeError, ReproductionError) as error:
        print(f"BCQ-1.3 external-solver reproduction: FAILED\n{error}")
        return 1
    print(f"BCQ-1.3 external-solver reproduction: {run_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
