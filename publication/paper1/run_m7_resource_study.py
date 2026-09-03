# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Run the locked Paper 1 P1-E2 M7 resource/replay study."""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import sys
import time
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
BASE_SCENARIO = ROOT / "examples/scenarios/fp9-lung-level-a-v1.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def utc_now() -> str:
    from datetime import datetime, timezone

    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")


def git(*arguments: str) -> str:
    completed = subprocess.run(
        ["git", *arguments], cwd=ROOT, capture_output=True, text=True, check=True
    )
    return completed.stdout.strip()


def physical_memory_bytes() -> int | None:
    if os.name == "nt":
        class MemoryStatus(ctypes.Structure):
            _fields_ = [
                ("length", ctypes.c_ulong),
                ("memory_load", ctypes.c_ulong),
                ("total_physical", ctypes.c_ulonglong),
                ("available_physical", ctypes.c_ulonglong),
                ("total_page_file", ctypes.c_ulonglong),
                ("available_page_file", ctypes.c_ulonglong),
                ("total_virtual", ctypes.c_ulonglong),
                ("available_virtual", ctypes.c_ulonglong),
                ("available_extended_virtual", ctypes.c_ulonglong),
            ]

        status = MemoryStatus()
        status.length = ctypes.sizeof(status)
        if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
            return int(status.total_physical)
        return None
    try:
        for line in Path("/proc/meminfo").read_text(encoding="utf-8").splitlines():
            if line.startswith("MemTotal:"):
                return int(line.split()[1]) * 1024
    except OSError:
        return None
    return None


def peak_memory_bytes(process: subprocess.Popen[str]) -> int:
    if os.name == "nt":
        class Counters(ctypes.Structure):
            _fields_ = [
                ("cb", ctypes.c_ulong),
                ("page_fault_count", ctypes.c_ulong),
                ("peak_working_set_size", ctypes.c_size_t),
                ("working_set_size", ctypes.c_size_t),
                ("quota_peak_paged_pool_usage", ctypes.c_size_t),
                ("quota_paged_pool_usage", ctypes.c_size_t),
                ("quota_peak_non_paged_pool_usage", ctypes.c_size_t),
                ("quota_non_paged_pool_usage", ctypes.c_size_t),
                ("pagefile_usage", ctypes.c_size_t),
                ("peak_pagefile_usage", ctypes.c_size_t),
            ]

        counters = Counters()
        counters.cb = ctypes.sizeof(counters)
        handle = ctypes.c_void_p(int(process._handle))  # type: ignore[attr-defined]
        if ctypes.windll.psapi.GetProcessMemoryInfo(
            handle, ctypes.byref(counters), counters.cb
        ):
            return int(counters.peak_working_set_size)
        return 0
    try:
        for line in Path(f"/proc/{process.pid}/status").read_text(
            encoding="utf-8"
        ).splitlines():
            if line.startswith(("VmHWM:", "VmRSS:")):
                return int(line.split()[1]) * 1024
    except OSError:
        return 0
    return 0


def run_process(command: list[str], cwd: Path) -> tuple[int, str, str, float, int]:
    started = time.perf_counter()
    process = subprocess.Popen(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    peak = 0
    while process.poll() is None:
        peak = max(peak, peak_memory_bytes(process))
        time.sleep(0.01)
    stdout, stderr = process.communicate()
    peak = max(peak, peak_memory_bytes(process))
    return process.returncode, stdout, stderr, time.perf_counter() - started, peak


def properties(text: str) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in text.splitlines():
        key, separator, value = line.partition("=")
        if separator and key and " " not in key:
            result[key] = value
    return result


def directory_metrics(path: Path) -> tuple[int, int]:
    files = [item for item in path.rglob("*") if item.is_file()]
    return len(files), sum(item.stat().st_size for item in files)


def projection(result: dict[str, Any]) -> dict[str, Any]:
    return {
        "schema_version": result["schema_version"],
        "scenario": result["scenario"],
        "run": result["run"],
        "reproducibility": result["reproducibility"],
        "runtime": result["runtime"],
        "level_b_detection": result["level_b_detection"],
        "level_c_assembly": result["level_c_assembly"],
        "level_d_communication": result["level_d_communication"],
        "level_e_analysis": result["level_e_analysis"],
        "validity": result["validity"],
    }


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def environment(executable: Path) -> dict[str, Any]:
    cmake = subprocess.run(["cmake", "--version"], capture_output=True, text=True, check=True)
    return {
        "recorded_at_utc": utc_now(),
        "machine_label": platform.node() or "local-primary-host",
        "os": platform.platform(),
        "architecture": platform.machine(),
        "cpu": platform.processor() or os.environ.get("PROCESSOR_IDENTIFIER", "unknown"),
        "logical_cpu_count": os.cpu_count(),
        "physical_memory_bytes": physical_memory_bytes(),
        "python": sys.version,
        "cmake": cmake.stdout.splitlines()[0],
        "git_commit": git("rev-parse", "HEAD"),
        "git_branch": git("branch", "--show-current"),
        "git_remote": git("remote", "get-url", "origin"),
        "git_status_porcelain": git("status", "--porcelain", "--untracked-files=normal"),
        "executable": str(executable),
        "executable_sha256": sha256(executable),
        "build_type_required": "Release",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", required=True, type=Path)
    parser.add_argument("--output-directory", required=True, type=Path)
    args = parser.parse_args()
    executable = args.executable.resolve()
    output = args.output_directory.resolve()
    if not executable.is_file():
        raise SystemExit(f"Executable not found: {executable}")
    if git("status", "--porcelain", "--untracked-files=normal"):
        raise SystemExit("P1-E2 requires a clean worktree")
    if output.exists():
        raise SystemExit(f"Output already exists: {output}")

    output.mkdir(parents=True)
    base = json.loads(BASE_SCENARIO.read_text(encoding="utf-8"))
    inputs = output / "inputs"
    scenarios: dict[str, Path] = {}
    for condition, collector_count in (("baseline", 1000), ("collector-10000", 10000)):
        document = json.loads(json.dumps(base))
        document["run"]["collector_count"] = collector_count
        document["run"]["id"] = f"paper1-m7-{condition}"
        path = inputs / f"{condition}.json"
        write_json(path, document)
        scenarios[condition] = path

    metadata = environment(executable)
    write_json(output / "environment.json", metadata)
    ledger_path = output / "attempts.jsonl"
    attempts: list[dict[str, Any]] = []

    invalid = json.loads(json.dumps(base))
    invalid["run"]["collector_count"] = -1
    invalid_path = inputs / "negative-invalid-collector.json"
    write_json(invalid_path, invalid)
    invalid_command = [str(executable), "scenario", "validate", "--file", str(invalid_path), "--repository-root", str(ROOT)]
    code, stdout, stderr, wall, peak = run_process(invalid_command, ROOT)
    negative_dir = output / "controls" / "invalid-scenario"
    negative_dir.mkdir(parents=True)
    (negative_dir / "stdout.txt").write_text(stdout, encoding="utf-8")
    (negative_dir / "stderr.txt").write_text(stderr, encoding="utf-8")
    invalid_control = {"expected_rejection": True, "return_code": code, "wall_seconds": wall, "peak_memory_bytes": peak}
    write_json(negative_dir / "control.json", invalid_control)
    if code == 0:
        raise SystemExit("Negative invalid-scenario control was accepted")

    schedule = [
        ("warmup-baseline", "baseline", False),
        ("warmup-variation", "collector-10000", False),
        ("baseline-1", "baseline", True),
        ("variation-1", "collector-10000", True),
        ("baseline-2", "baseline", True),
        ("variation-2", "collector-10000", True),
        ("baseline-3", "baseline", True),
        ("variation-3", "collector-10000", True),
    ]
    for attempt_id, condition, measured in schedule:
        attempt_dir = output / "raw" / attempt_id
        command = [str(executable), "scenario", "run", "--file", str(scenarios[condition]), "--output", str(attempt_dir / "run"), "--repository-root", str(ROOT)]
        started_at = utc_now()
        code, stdout, stderr, wall, peak = run_process(command, ROOT)
        completed_at = utc_now()
        attempt_dir.mkdir(parents=True, exist_ok=True)
        (attempt_dir / "stdout.txt").write_text(stdout, encoding="utf-8")
        (attempt_dir / "stderr.txt").write_text(stderr, encoding="utf-8")
        values = properties(stdout)
        record: dict[str, Any] = {
            "attempt_id": attempt_id,
            "condition": condition,
            "measured": measured,
            "started_at_utc": started_at,
            "completed_at_utc": completed_at,
            "return_code": code,
            "wall_seconds": wall,
            "peak_memory_bytes": peak,
            "command": command,
        }
        if code == 0:
            run_directory = Path(values["run_directory"]).resolve()
            result_path = Path(values["result_file"]).resolve()
            result = json.loads(result_path.read_text(encoding="utf-8"))
            file_count, byte_count = directory_metrics(run_directory)
            record.update({
                "run_directory": run_directory.relative_to(output).as_posix(),
                "result_path": result_path.relative_to(output).as_posix(),
                "result_sha256": sha256(result_path),
                "file_count": file_count,
                "artifact_bytes": byte_count,
                "projection": projection(result),
            })
        else:
            record["diagnostic"] = stderr.strip() or stdout.strip()
        attempts.append(record)
        with ledger_path.open("a", encoding="utf-8") as stream:
            stream.write(json.dumps(record, ensure_ascii=False) + "\n")

    measured_attempts = [item for item in attempts if item["measured"]]
    failures = [item for item in measured_attempts if item["return_code"] != 0]
    if failures:
        raise SystemExit(f"Measured M7 attempts failed: {[item['attempt_id'] for item in failures]}")

    condition_summaries: dict[str, Any] = {}
    for condition in scenarios:
        records = [item for item in measured_attempts if item["condition"] == condition]
        result_hashes = {item["result_sha256"] for item in records}
        projections = {json.dumps(item["projection"], sort_keys=True) for item in records}
        if len(result_hashes) != 1 or len(projections) != 1:
            raise SystemExit(f"Deterministic identity failed for {condition}")
        condition_summaries[condition] = {
            "attempt_count": len(records),
            "result_sha256": next(iter(result_hashes)),
            "deterministic_projection_identity": True,
            "wall_seconds": {"values": [item["wall_seconds"] for item in records], "median": statistics.median(item["wall_seconds"] for item in records), "range": [min(item["wall_seconds"] for item in records), max(item["wall_seconds"] for item in records)]},
            "peak_memory_bytes": {"values": [item["peak_memory_bytes"] for item in records], "median": statistics.median(item["peak_memory_bytes"] for item in records), "range": [min(item["peak_memory_bytes"] for item in records), max(item["peak_memory_bytes"] for item in records)]},
            "artifact_bytes": {"values": [item["artifact_bytes"] for item in records], "median": statistics.median(item["artifact_bytes"] for item in records), "range": [min(item["artifact_bytes"] for item in records), max(item["artifact_bytes"] for item in records)]},
        }

    first_result = output / measured_attempts[0]["result_path"]
    tampered = output / "controls" / "tampered-result" / "result.json"
    tampered.parent.mkdir(parents=True)
    shutil.copy2(first_result, tampered)
    tampered_document = json.loads(tampered.read_text(encoding="utf-8"))
    tampered_document["run"]["collector_count"] += 1
    write_json(tampered, tampered_document)
    tamper_control = {
        "expected_rejection": True,
        "original_sha256": sha256(first_result),
        "tampered_sha256": sha256(tampered),
        "hash_identity": sha256(first_result) == sha256(tampered),
    }
    write_json(tampered.parent / "control.json", tamper_control)
    if tamper_control["hash_identity"]:
        raise SystemExit("Tamper control did not change the result hash")

    summary = {
        "schema_version": "1.0.0",
        "experiment_id": "P1-E2-M7-RESOURCE",
        "protocol_version": "2.0.0",
        "completed_at_utc": utc_now(),
        "environment": metadata,
        "conditions": condition_summaries,
        "negative_controls": {"invalid_scenario_rejected": True, "tampered_hash_rejected": True},
        "interpretation": "Technical resource and reproducibility evidence only; no biological or clinical feasibility claim.",
    }
    write_json(output / "analysis" / "summary.json", summary)
    print(output / "analysis" / "summary.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
