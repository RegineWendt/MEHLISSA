# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Execute the locked Paper 1 P1-E3 CLI/Python/Workbench integrity check."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "python"))

from mehlissa import MehlissaClient, load_result  # noqa: E402
from mehlissa_workbench.server import RunWorkspace, ScenarioWorkspace  # noqa: E402


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", required=True, type=Path)
    parser.add_argument("--result", required=True, type=Path)
    parser.add_argument("--scenario", required=True, type=Path)
    parser.add_argument("--provenance", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    executable = args.executable.resolve()
    result_path = args.result.resolve()
    scenario_path = args.scenario.resolve()
    provenance_path = args.provenance.resolve()
    output = args.output.resolve()
    for required in (executable, result_path, scenario_path, provenance_path):
        if not required.is_file():
            raise SystemExit(f"Required file is absent: {required}")
    if ROOT not in output.parents:
        raise SystemExit("Parity output must stay inside the repository")
    if output.exists():
        raise SystemExit(f"Output already exists: {output}")

    before_hash = sha256(result_path)
    client = MehlissaClient(executable, ROOT)
    cli_first = client.summarize_result(result_path)
    cli_second = client.summarize_result(result_path)
    python_first = load_result(result_path)
    python_second = load_result(result_path)

    job_dir = output / "workbench-job"
    job_dir.mkdir(parents=True)
    copied_result = job_dir / "result.json"
    copied_scenario = job_dir / "scenario-input.json"
    copied_provenance = job_dir / "provenance.json"
    shutil.copy2(result_path, copied_result)
    shutil.copy2(scenario_path, copied_scenario)
    shutil.copy2(provenance_path, copied_provenance)

    workspace_scenarios = ScenarioWorkspace(client, ROOT / "tmp/paper1-parity-scenarios")
    workspace = RunWorkspace(client, workspace_scenarios, ROOT / "tmp/paper1-parity-runs")
    relative_job = job_dir.relative_to(ROOT).as_posix()
    job: dict[str, object] = {
        "id": "paper1-parity",
        "kind": "scenario",
        "title": "Paper 1 parity projection",
        "status": "completed",
        "completed_at": None,
        "directory": relative_job,
        "plan": {
            "candidate_sha256": sha256(copied_scenario),
            "master_seeds": [python_first.summary["seed"]],
        },
        "artifacts": [
            {"name": "input", "label": "Scenario input", "path": copied_scenario.relative_to(ROOT).as_posix()},
            {"name": "result", "label": "Authoritative result", "path": copied_result.relative_to(ROOT).as_posix()},
            {"name": "provenance", "label": "Provenance", "path": copied_provenance.relative_to(ROOT).as_posix()},
        ],
    }
    workspace._jobs["paper1-parity"] = job  # controlled in-process integrity fixture
    dashboard = workspace.dashboard("paper1-parity")
    audit = workspace.audit("paper1-parity")

    if cli_first != cli_second:
        raise SystemExit("CLI summary differs across repeated reads")
    if python_first.document != python_second.document:
        raise SystemExit("Python reader differs across repeated reads")
    if dashboard["reader"] != "mehlissa.load_result":
        raise SystemExit("Workbench did not declare the authoritative scenario reader")
    if dashboard["summary"] != python_first.summary:
        raise SystemExit("Workbench summary differs from the Python authoritative reader")
    if audit["integrity"]["status"] != "verified":
        raise SystemExit("Workbench audit did not verify the authoritative artifacts")
    if sha256(result_path) != before_hash:
        raise SystemExit("An access path mutated the authoritative result")

    tampered_dir = output / "tampered-workbench-job"
    tampered_dir.mkdir(parents=True)
    tampered_result = tampered_dir / "result.json"
    tampered_scenario = tampered_dir / "scenario-input.json"
    tampered_provenance = tampered_dir / "provenance.json"
    shutil.copy2(result_path, tampered_result)
    shutil.copy2(scenario_path, tampered_scenario)
    shutil.copy2(provenance_path, tampered_provenance)
    document = json.loads(tampered_result.read_text(encoding="utf-8"))
    document["run"]["collector_count"] += 1
    write_json(tampered_result, document)
    tampered_job = json.loads(json.dumps(job))
    tampered_job["id"] = "paper1-parity-tampered"
    tampered_job["directory"] = tampered_dir.relative_to(ROOT).as_posix()
    tampered_job["artifacts"] = [
        {"name": "input", "label": "Scenario input", "path": tampered_scenario.relative_to(ROOT).as_posix()},
        {"name": "result", "label": "Tampered result", "path": tampered_result.relative_to(ROOT).as_posix()},
        {"name": "provenance", "label": "Provenance", "path": tampered_provenance.relative_to(ROOT).as_posix()},
    ]
    workspace._jobs["paper1-parity-tampered"] = tampered_job
    tampered_audit = workspace.audit("paper1-parity-tampered")
    if tampered_audit["integrity"]["status"] != "attention":
        raise SystemExit("Workbench audit failed to flag the tampered result")

    report = {
        "schema_version": "1.0.0",
        "experiment_id": "P1-E3-ACCESS-PARITY",
        "source_result": {"path": result_path.relative_to(ROOT).as_posix(), "sha256": before_hash},
        "cli": {"repeated_read_identity": True, "summary": cli_first},
        "python": {
            "reader": "mehlissa.load_result",
            "repeated_read_identity": True,
            "summary": python_first.summary,
            "runtime_stages": python_first.runtime_stages,
            "analysis_cases": python_first.analysis_cases,
        },
        "workbench": {
            "reader": dashboard["reader"],
            "summary_parity": True,
            "source_integrity": audit["integrity"],
            "tampered_result_status": tampered_audit["integrity"],
        },
        "authoritative_result_mutated": False,
        "scientific_value_parity": True,
        "interpretation": "Research-access integrity evidence, not a usability or scientific-validity study.",
    }
    write_json(output / "access-parity-report.json", report)
    print(output / "access-parity-report.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
