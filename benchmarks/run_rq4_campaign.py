# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Orchestrate the frozen Paper 1 RQ4 benchmark campaign without dependencies."""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import math
import os
import random
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


SUPPORTED_PLAN_VERSION = "1.0.0"
SUPPORTED_REPORT_VERSION = "1.0.0"
REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
MANIFEST_SCHEMA = REPOSITORY_ROOT / "data/schemas/body-transport-benchmark/1.0.0.schema.json"
RESULT_SCHEMA = REPOSITORY_ROOT / "data/schemas/body-transport-benchmark-report/1.0.0.schema.json"
OBSERVATION_SCHEMA = REPOSITORY_ROOT / "data/schemas/transport-observation-report/1.0.0.schema.json"
PLAN_SCHEMA = REPOSITORY_ROOT / "data/schemas/rq4-benchmark-campaign/1.0.0.schema.json"
CAMPAIGN_REPORT_SCHEMA = REPOSITORY_ROOT / "data/schemas/rq4-benchmark-campaign-report/1.0.0.schema.json"


class CampaignError(RuntimeError):
    """Raised when the frozen campaign contract cannot be honored."""


@dataclass(frozen=True)
class Condition:
    population: int
    policy_id: str
    duration_ns: int | None = None
    anchor: bool = False

    @property
    def key(self) -> str:
        prefix = "anchor-" if self.anchor else ""
        return f"{prefix}n{self.population}-{self.policy_id.lower()}"


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as source:
            document = json.load(source)
    except (OSError, json.JSONDecodeError) as error:
        raise CampaignError(f"Cannot read JSON document '{path}': {error}") from error
    if not isinstance(document, dict):
        raise CampaignError(f"Expected a JSON object in '{path}'")
    return document


def atomic_write_json(path: Path, document: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", encoding="utf-8", newline="\n") as output:
        json.dump(document, output, indent=2, sort_keys=True)
        output.write("\n")
    os.replace(temporary, path)


def require_exact_keys(document: dict[str, Any], expected: set[str], role: str) -> None:
    actual = set(document)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        raise CampaignError(f"{role} keys differ; missing={missing}, extra={extra}")


def validate_plan(plan: dict[str, Any], *, executing: bool, pilot: bool) -> None:
    require_exact_keys(
        plan,
        {
            "schema_version",
            "campaign_id",
            "machine_label",
            "ordering_seed",
            "measured_blocks",
            "execution_requirements",
            "limits",
            "model",
            "state_profile",
            "scenario",
            "long_duration_anchor",
            "population_matrix",
            "o3_pilot_status",
            "o3_pilot_evidence_path",
            "policies",
        },
        "campaign plan",
    )
    if plan["schema_version"] != SUPPORTED_PLAN_VERSION:
        raise CampaignError("Unsupported campaign-plan schema version")
    if plan["measured_blocks"] != 7:
        raise CampaignError("The frozen RQ4 campaign requires exactly seven measured blocks")
    if plan["scenario"] != {
        "master_seed": 2018,
        "duration_ns": 420_000_000_000,
        "injection_time_ns": 0,
        "injection_segment_id": "bvs95-001",
    }:
        raise CampaignError("The campaign scenario differs from the frozen RQ4 protocol")
    if plan["long_duration_anchor"] != {
        "population": 6_359,
        "duration_ns": 7_200_000_000_000,
        "policy_id": "O0",
    }:
        raise CampaignError("The long-duration anchor differs from the frozen RQ4 protocol")
    matrix = plan["population_matrix"]
    if matrix["mandatory"] != [1_000, 10_000, 100_000] or matrix["conditional"] != 1_000_000:
        raise CampaignError("The campaign population matrix differs from the frozen RQ4 protocol")
    if plan["limits"]["timeout_seconds"] != 1800 or not math.isclose(
        plan["limits"]["maximum_physical_memory_fraction"], 0.8
    ):
        raise CampaignError("The campaign resource limits differ from the frozen RQ4 protocol")
    policies = plan["policies"]
    if [policy["policy_id"] for policy in policies] != ["O0", "O1", "O2", "O3"]:
        raise CampaignError("Policies must occur exactly once in O0, O1, O2, O3 order")
    if executing and not pilot and plan["machine_label"].startswith("replace-"):
        raise CampaignError("Freeze a real machine_label before executing measured runs")
    if executing and not pilot and plan["o3_pilot_status"] != "frozen":
        raise CampaignError("O3 limits must be piloted and marked frozen before measured runs")
    if plan["execution_requirements"] != {"build_type": "Release", "git_dirty": False}:
        raise CampaignError("Measured RQ4 runs require a clean Release build")


def validate_frozen_pilot(plan: dict[str, Any], plan_path: Path) -> None:
    if plan["o3_pilot_status"] != "frozen":
        return
    configured = plan["o3_pilot_evidence_path"]
    if not isinstance(configured, str) or not configured:
        raise CampaignError("A frozen O3 pilot requires an evidence path")
    evidence_path = resolve_plan_path(plan_path, configured)
    evidence = load_json(evidence_path)
    o3_policy = policy_map(plan)["O3"]
    if evidence.get("reported_measurement") is not False:
        raise CampaignError("O3 pilot evidence must be marked as unreported")
    if any(evidence["observed"]["truncation"].values()):
        raise CampaignError("The frozen O3 pilot evidence contains truncation")
    if evidence["frozen_limits"] != {
        "maximum_trajectory_records": o3_policy["maximum_trajectory_records"],
        "maximum_measurement_records": o3_policy["maximum_measurement_records"],
        "maximum_aggregate_records": o3_policy["maximum_aggregate_records"],
    }:
        raise CampaignError("O3 plan limits differ from the frozen pilot evidence")
    if evidence["observed"]["trajectory_records"] >= o3_policy["maximum_trajectory_records"]:
        raise CampaignError("Frozen O3 trajectory limit has no headroom")
    if evidence["observed"]["measurement_records"] >= o3_policy["maximum_measurement_records"]:
        raise CampaignError("Frozen O3 measurement limit has no headroom")


def resolve_plan_path(plan_path: Path, configured: str) -> Path:
    path = Path(configured)
    if not path.is_absolute():
        path = plan_path.parent / path
    return path.resolve()


def policy_map(plan: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {policy["policy_id"]: policy for policy in plan["policies"]}


def mandatory_conditions(plan: dict[str, Any]) -> list[Condition]:
    result = [
        Condition(population, policy)
        for population in plan["population_matrix"]["mandatory"]
        for policy in ("O0", "O1", "O2")
    ]
    result.append(Condition(1_000, "O3"))
    anchor = plan["long_duration_anchor"]
    result.append(
        Condition(
            anchor["population"], anchor["policy_id"], anchor["duration_ns"], anchor=True
        )
    )
    return result


def conditional_conditions(plan: dict[str, Any]) -> list[Condition]:
    population = plan["population_matrix"]["conditional"]
    return [Condition(population, policy) for policy in ("O0", "O1", "O2")]


def randomized_blocks(
    conditions: list[Condition], block_count: int, ordering_seed: int, phase_offset: int = 0
) -> list[list[Condition]]:
    randomizer = random.Random(ordering_seed + phase_offset)
    blocks: list[list[Condition]] = []
    for _ in range(block_count):
        block = conditions.copy()
        randomizer.shuffle(block)
        blocks.append(block)
    return blocks


def build_schedule(plan: dict[str, Any], conditions: list[Condition], phase: str) -> list[dict[str, Any]]:
    schedule: list[dict[str, Any]] = []
    for condition in conditions:
        schedule.append(
            {"phase": phase, "kind": "warmup", "block": None, "condition": condition.key}
        )
    offset = 0 if phase == "mandatory" else 1_000_000
    for block_index, block in enumerate(
        randomized_blocks(
            conditions, plan["measured_blocks"], plan["ordering_seed"], phase_offset=offset
        ),
        start=1,
    ):
        for condition in block:
            schedule.append(
                {
                    "phase": phase,
                    "kind": "measured",
                    "block": block_index,
                    "condition": condition.key,
                }
            )
    return schedule


def condition_lookup(conditions: list[Condition]) -> dict[str, Condition]:
    return {condition.key: condition for condition in conditions}


def create_manifest(
    plan: dict[str, Any], plan_path: Path, condition: Condition, run_id: str
) -> dict[str, Any]:
    model = plan["model"]
    state = plan["state_profile"]
    policy = {key: value for key, value in policy_map(plan)[condition.policy_id].items() if key != "required_truncation"}
    return {
        "schema_version": "1.0.0",
        "benchmark_id": f"{plan['campaign_id']}-{condition.key}-{run_id}",
        "machine_label": plan["machine_label"],
        "model": {
            **model,
            "path": str(resolve_plan_path(plan_path, model["path"])),
            "schema_path": str(resolve_plan_path(plan_path, model["schema_path"])),
        },
        "state_profile": {
            **state,
            "path": str(resolve_plan_path(plan_path, state["path"])),
            "schema_path": str(resolve_plan_path(plan_path, state["schema_path"])),
        },
        "master_seed": plan["scenario"]["master_seed"],
        "duration_ns": condition.duration_ns or plan["scenario"]["duration_ns"],
        "injection": {
            "time_ns": plan["scenario"]["injection_time_ns"],
            "segment_id": plan["scenario"]["injection_segment_id"],
            "particle_count": condition.population,
        },
        "observation": policy,
    }


def physical_memory_bytes() -> int:
    if sys.platform == "win32":
        class MemoryStatus(ctypes.Structure):
            _fields_ = [
                ("dwLength", ctypes.c_ulong),
                ("dwMemoryLoad", ctypes.c_ulong),
                ("ullTotalPhys", ctypes.c_ulonglong),
                ("ullAvailPhys", ctypes.c_ulonglong),
                ("ullTotalPageFile", ctypes.c_ulonglong),
                ("ullAvailPageFile", ctypes.c_ulonglong),
                ("ullTotalVirtual", ctypes.c_ulonglong),
                ("ullAvailVirtual", ctypes.c_ulonglong),
                ("ullAvailExtendedVirtual", ctypes.c_ulonglong),
            ]

        status = MemoryStatus()
        status.dwLength = ctypes.sizeof(status)
        if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
            return int(status.ullTotalPhys)
    if hasattr(os, "sysconf"):
        try:
            return int(os.sysconf("SC_PHYS_PAGES")) * int(os.sysconf("SC_PAGE_SIZE"))
        except (ValueError, OSError):
            pass
    raise CampaignError("Cannot determine physical memory for the resource guard")


def process_rss_bytes(process_id: int) -> int | None:
    if sys.platform == "win32":
        class ProcessMemoryCounters(ctypes.Structure):
            _fields_ = [
                ("cb", ctypes.c_ulong),
                ("PageFaultCount", ctypes.c_ulong),
                ("PeakWorkingSetSize", ctypes.c_size_t),
                ("WorkingSetSize", ctypes.c_size_t),
                ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                ("PagefileUsage", ctypes.c_size_t),
                ("PeakPagefileUsage", ctypes.c_size_t),
            ]

        kernel32 = ctypes.windll.kernel32
        kernel32.OpenProcess.restype = ctypes.c_void_p
        kernel32.OpenProcess.argtypes = [ctypes.c_ulong, ctypes.c_int, ctypes.c_ulong]
        kernel32.CloseHandle.argtypes = [ctypes.c_void_p]
        handle = kernel32.OpenProcess(0x0400 | 0x0010, False, process_id)
        if not handle:
            return None
        try:
            counters = ProcessMemoryCounters()
            counters.cb = ctypes.sizeof(counters)
            if ctypes.windll.psapi.GetProcessMemoryInfo(
                handle, ctypes.byref(counters), counters.cb
            ):
                return int(counters.WorkingSetSize)
        finally:
            kernel32.CloseHandle(handle)
        return None
    status_path = Path(f"/proc/{process_id}/status")
    try:
        for line in status_path.read_text(encoding="utf-8").splitlines():
            if line.startswith("VmRSS:"):
                return int(line.split()[1]) * 1024
    except (OSError, ValueError, IndexError):
        return None
    return None


def run_process_guarded(
    command: list[str], stdout_path: Path, stderr_path: Path, timeout_seconds: int,
    memory_limit_bytes: int, poll_interval_seconds: float
) -> tuple[str, int | None, int, float]:
    started = time.monotonic()
    sampled_peak = 0
    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        process = subprocess.Popen(command, stdout=stdout, stderr=stderr)
        reason = "completed"
        while process.poll() is None:
            elapsed = time.monotonic() - started
            rss = process_rss_bytes(process.pid)
            if rss is not None:
                sampled_peak = max(sampled_peak, rss)
                if rss > memory_limit_bytes:
                    reason = "memory_limit"
                    process.kill()
                    break
            if elapsed > timeout_seconds:
                reason = "timeout"
                process.kill()
                break
            time.sleep(poll_interval_seconds)
        return_code = process.wait()
    return reason, return_code, sampled_peak, time.monotonic() - started


def run_paths(output_dir: Path, entry: dict[str, Any], sequence: int) -> dict[str, Path]:
    block = "warmup" if entry["block"] is None else f"block-{entry['block']:02d}"
    stem = f"{sequence:03d}-{entry['phase']}-{block}-{entry['condition']}"
    return {
        "manifest": output_dir / "manifests" / f"{stem}.manifest.json",
        "result": output_dir / "results" / f"{stem}.result.json",
        "observation": output_dir / "observations" / f"{stem}.observation.json",
        "stdout": output_dir / "logs" / f"{stem}.stdout.log",
        "stderr": output_dir / "logs" / f"{stem}.stderr.log",
    }


def execute_entry(
    plan: dict[str, Any], plan_path: Path, driver: Path, output_dir: Path,
    entry: dict[str, Any], condition: Condition, sequence: int, memory_limit: int,
    *, pilot: bool
) -> dict[str, Any]:
    started_at = utc_now()
    paths = run_paths(output_dir, entry, sequence)
    for path in paths.values():
        path.parent.mkdir(parents=True, exist_ok=True)
    run_id = "pilot" if pilot else ("warmup" if entry["kind"] == "warmup" else f"b{entry['block']:02d}")
    manifest = create_manifest(plan, plan_path, condition, run_id)
    atomic_write_json(paths["manifest"], manifest)
    command = [
        str(driver),
        "--manifest", str(paths["manifest"]),
        "--manifest-schema", str(MANIFEST_SCHEMA),
        "--result-schema", str(RESULT_SCHEMA),
        "--observation-schema", str(OBSERVATION_SCHEMA),
        "--output", str(paths["result"]),
        "--observation-output", str(paths["observation"]),
    ]
    reason, return_code, sampled_peak, elapsed = run_process_guarded(
        command,
        paths["stdout"],
        paths["stderr"],
        plan["limits"]["timeout_seconds"],
        memory_limit,
        plan["limits"]["poll_interval_ms"] / 1000.0,
    )
    record: dict[str, Any] = {
        **entry,
        "sequence": sequence,
        "started_at_utc": started_at,
        "elapsed_seconds": elapsed,
        "return_code": return_code,
        "sampled_peak_rss_bytes": sampled_peak,
        "paths": {name: str(path) for name, path in paths.items()},
        "status": reason,
        "diagnostic": None,
    }
    if reason != "completed":
        record["diagnostic"] = f"Child process exceeded the {reason.replace('_', ' ')} guard"
        return record
    if return_code != 0:
        record["status"] = "driver_failed"
        record["diagnostic"] = f"Benchmark driver returned {return_code}"
        return record
    try:
        result = load_json(paths["result"])
    except CampaignError as error:
        record["status"] = "invalid_result"
        record["diagnostic"] = str(error)
        return record
    if result.get("benchmark", {}).get("id") != manifest["benchmark_id"]:
        record["status"] = "invalid_result"
        record["diagnostic"] = "Result benchmark ID does not match the generated manifest"
        return record
    if result["platform"]["peak_resident_set_bytes"] > memory_limit:
        record["status"] = "memory_limit"
        record["diagnostic"] = "Reported peak resident set exceeded the campaign guard"
        return record
    if not pilot:
        requirements = plan["execution_requirements"]
        if result["software"]["build_type"] != requirements["build_type"]:
            record["status"] = "invalid_result"
            record["diagnostic"] = "Benchmark was not produced by the required Release build"
            return record
        if result["software"]["git_dirty"] != requirements["git_dirty"]:
            record["status"] = "invalid_result"
            record["diagnostic"] = "Benchmark was not produced from the required clean tree"
            return record
    required = policy_map(plan)[condition.policy_id]["required_truncation"]
    actual = result["observation"]["truncation"]
    mismatches = [key for key, value in required.items() if value is not None and actual[key] != value]
    if mismatches:
        record["status"] = "invalid_result"
        record["diagnostic"] = f"Unexpected truncation state for {', '.join(mismatches)}"
        return record
    record["status"] = "completed"
    return record


def completed_results(records: list[dict[str, Any]], *, measured_only: bool = True) -> list[tuple[dict[str, Any], dict[str, Any]]]:
    result: list[tuple[dict[str, Any], dict[str, Any]]] = []
    for record in records:
        if record["status"] != "completed" or (measured_only and record["kind"] != "measured"):
            continue
        result.append((record, load_json(Path(record["paths"]["result"]))))
    return result


def quartile_summary(values: list[float | int]) -> dict[str, float | int]:
    if len(values) != 7:
        raise CampaignError(f"Expected seven measurements, received {len(values)}")
    quartiles = statistics.quantiles(values, n=4, method="inclusive")
    return {
        "median": statistics.median(values),
        "q1": quartiles[0],
        "q3": quartiles[2],
        "iqr": quartiles[2] - quartiles[0],
        "minimum": min(values),
        "maximum": max(values),
        "all": values,
    }


def summarize(records: list[dict[str, Any]]) -> dict[str, Any]:
    grouped: dict[str, list[dict[str, Any]]] = {}
    for record, result in completed_results(records):
        grouped.setdefault(record["condition"], []).append(result)
    summaries: dict[str, Any] = {}
    for condition, results in grouped.items():
        results.sort(key=lambda item: item["benchmark"]["id"])
        if len(results) != 7:
            summaries[condition] = {"complete": False, "completed_measurements": len(results)}
            continue
        summaries[condition] = {
            "complete": True,
            "simulation_ns": quartile_summary([item["timing"]["simulation_ns"] for item in results]),
            "peak_resident_set_bytes": quartile_summary(
                [item["platform"]["peak_resident_set_bytes"] for item in results]
            ),
            "transition_throughput_per_second": quartile_summary(
                [item["summary"]["transition_throughput_per_second"] for item in results]
            ),
            "observation_output_bytes": quartile_summary(
                [item["observation"]["output"]["bytes"] for item in results]
            ),
        }
    return summaries


def correctness_key(result: dict[str, Any]) -> dict[str, Any]:
    summary = result["summary"]
    return {
        "transition_count": summary["transition_count"],
        "injected_particle_count": summary["injected_particle_count"],
        "active_particle_count": summary["active_particle_count"],
        "extracted_particle_count": summary["extracted_particle_count"],
        "final_population_hash": summary["final_population_hash"],
        "random_streams": summary["random_streams"],
    }


def verify_correctness(records: list[dict[str, Any]]) -> list[dict[str, Any]]:
    by_population_block: dict[tuple[int, int], list[tuple[dict[str, Any], dict[str, Any]]]] = {}
    for record, result in completed_results(records):
        population = result["configuration"]["injection"]["particle_count"]
        by_population_block.setdefault((population, record["block"]), []).append((record, result))
    violations: list[dict[str, Any]] = []
    for (population, block), entries in sorted(by_population_block.items()):
        by_policy = {result["configuration"]["observation_policy"]["policy_id"]: (record, result) for record, result in entries}
        anchor_group = any(record["condition"].startswith("anchor-") for record, _ in entries)
        expected_policies = (
            {"O0"}
            if anchor_group
            else ({"O0", "O1", "O2", "O3"} if population == 1_000 else {"O0", "O1", "O2"})
        )
        missing_policies = sorted(expected_policies - set(by_policy))
        if missing_policies:
            violations.append(
                {
                    "population": population,
                    "block": block,
                    "kind": "missing_policies",
                    "policies": missing_policies,
                }
            )
        if "O0" not in by_policy:
            continue
        reference = correctness_key(by_policy["O0"][1])
        for policy, (_, result) in sorted(by_policy.items()):
            if correctness_key(result) != reference:
                violations.append(
                    {"population": population, "block": block, "policy": policy, "kind": "core_invariant_mismatch"}
                )
        detailed = {policy: pair for policy, pair in by_policy.items() if policy in {"O1", "O2", "O3"}}
        if len(detailed) > 1:
            reference_policy = sorted(detailed)[0]
            reference_record, reference_result = detailed[reference_policy]
            reference_observation = load_json(Path(reference_record["paths"]["observation"]))
            for policy, (record, result) in sorted(detailed.items()):
                observation = load_json(Path(record["paths"]["observation"]))
                if result["observation"]["measurement_counts"] != reference_result["observation"]["measurement_counts"]:
                    violations.append(
                        {"population": population, "block": block, "policy": policy, "kind": "measurement_total_mismatch"}
                    )
                if observation["population_snapshots"] != reference_observation["population_snapshots"]:
                    violations.append(
                        {"population": population, "block": block, "policy": policy, "kind": "population_snapshot_mismatch"}
                    )
    return violations


def conditional_eligibility(plan: dict[str, Any], records: list[dict[str, Any]]) -> dict[str, Any]:
    selected = [
        result
        for record, result in completed_results(records)
        if record["condition"] == "n100000-o0" and record["phase"] == "mandatory"
    ]
    if len(selected) != 7:
        return {"eligible": False, "reason": "seven completed 100,000-entity O0 runs are required"}
    runtime_seconds = statistics.median(item["timing"]["simulation_ns"] for item in selected) / 1e9
    peak_rss = statistics.median(item["platform"]["peak_resident_set_bytes"] for item in selected)
    physical_memory = statistics.median(item["platform"]["physical_memory_bytes"] for item in selected)
    rss_fraction = peak_rss / physical_memory if physical_memory else math.inf
    thresholds = plan["population_matrix"]["eligibility"]
    eligible = (
        runtime_seconds < thresholds["o0_median_simulation_seconds_below"]
        and rss_fraction < thresholds["o0_median_peak_rss_fraction_below"]
    )
    return {
        "eligible": eligible,
        "reason": "both thresholds passed" if eligible else "one or both thresholds failed",
        "median_simulation_seconds": runtime_seconds,
        "median_peak_rss_bytes": peak_rss,
        "median_peak_rss_fraction": rss_fraction,
        "thresholds": thresholds,
    }


def report_document(
    plan: dict[str, Any], plan_path: Path, driver: Path | None, status: str,
    schedule: list[dict[str, Any]], records: list[dict[str, Any]], started_at: str,
    eligibility: dict[str, Any] | None = None
) -> dict[str, Any]:
    failures = [record for record in records if record["status"] != "completed"]
    violations = verify_correctness(records) if records else []
    return {
        "schema_version": SUPPORTED_REPORT_VERSION,
        "campaign_id": plan["campaign_id"],
        "status": status,
        "started_at_utc": started_at,
        "updated_at_utc": utc_now(),
        "plan": {"path": str(plan_path), "sha256": sha256_file(plan_path)},
        "driver": None if driver is None else {"path": str(driver), "sha256": sha256_file(driver)},
        "schemas": {
            "campaign_plan": {"path": str(PLAN_SCHEMA), "sha256": sha256_file(PLAN_SCHEMA)},
            "campaign_report": {
                "path": str(CAMPAIGN_REPORT_SCHEMA),
                "sha256": sha256_file(CAMPAIGN_REPORT_SCHEMA),
            },
            "benchmark_manifest": {
                "path": str(MANIFEST_SCHEMA),
                "sha256": sha256_file(MANIFEST_SCHEMA),
            },
            "benchmark_report": {
                "path": str(RESULT_SCHEMA),
                "sha256": sha256_file(RESULT_SCHEMA),
            },
            "observation_report": {
                "path": str(OBSERVATION_SCHEMA),
                "sha256": sha256_file(OBSERVATION_SCHEMA),
            },
        },
        "ordering_seed": plan["ordering_seed"],
        "schedule": schedule,
        "runs": records,
        "summaries": summarize(records),
        "correctness": {"passed": not violations, "violations": violations},
        "conditional_population": eligibility,
        "suitable_for_analysis": status == "complete" and not failures and not violations,
    }


def execute_schedule(
    plan: dict[str, Any], plan_path: Path, driver: Path, output_dir: Path,
    schedule: list[dict[str, Any]], conditions: list[Condition], records: list[dict[str, Any]],
    started_at: str, report_path: Path, memory_limit: int,
    ledger_schedule: list[dict[str, Any]] | None = None,
) -> None:
    recorded_schedule = schedule if ledger_schedule is None else ledger_schedule
    lookup = condition_lookup(conditions)
    for entry in schedule:
        sequence = len(records) + 1
        record = execute_entry(
            plan, plan_path, driver, output_dir, entry, lookup[entry["condition"]], sequence,
            memory_limit, pilot=False
        )
        records.append(record)
        atomic_write_json(
            report_path,
            report_document(
                plan, plan_path, driver, "running", recorded_schedule, records, started_at
            ),
        )


def run_pilot(plan: dict[str, Any], plan_path: Path, driver: Path, output_dir: Path) -> int:
    condition = Condition(1_000, "O3")
    entry = {"phase": "pilot", "kind": "warmup", "block": None, "condition": condition.key}
    started_at = utc_now()
    memory_limit = int(
        physical_memory_bytes() * plan["limits"]["maximum_physical_memory_fraction"]
    )
    record = execute_entry(
        plan, plan_path, driver, output_dir, entry, condition, 1, memory_limit, pilot=True
    )
    schedule = [entry]
    report = report_document(
        plan, plan_path, driver,
        "pilot_complete" if record["status"] == "completed" else "pilot_failed",
        schedule, [record], started_at
    )
    if record["status"] == "completed":
        result = load_json(Path(record["paths"]["result"]))
        report["pilot_recommendation"] = {
            "observed_trajectory_records": result["observation"]["record_counts"]["trajectory_records"],
            "observed_measurement_records": result["observation"]["record_counts"]["measurement_records"],
            "configured_trajectory_limit": policy_map(plan)["O3"]["maximum_trajectory_records"],
            "configured_measurement_limit": policy_map(plan)["O3"]["maximum_measurement_records"],
            "can_freeze": not any(result["observation"]["truncation"].values()),
        }
    atomic_write_json(output_dir / "campaign-report.json", report)
    return 0 if record["status"] == "completed" else 3


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--plan", type=Path, required=True)
    parser.add_argument("--driver", type=Path)
    parser.add_argument("--output-directory", type=Path, required=True)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true", help="Generate and record the mandatory schedule only")
    mode.add_argument("--pilot-o3", action="store_true", help="Execute one unreported O3 pilot")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    plan_path = arguments.plan.resolve()
    output_dir = arguments.output_directory.resolve()
    plan = load_json(plan_path)
    executing = not arguments.dry_run
    validate_plan(plan, executing=executing, pilot=arguments.pilot_o3)
    validate_frozen_pilot(plan, plan_path)
    if output_dir.exists() and any(output_dir.iterdir()):
        raise CampaignError(f"Output directory must be absent or empty: {output_dir}")
    output_dir.mkdir(parents=True, exist_ok=True)

    driver = None if arguments.driver is None else arguments.driver.resolve()
    if executing and (driver is None or not driver.is_file()):
        raise CampaignError("--driver must identify an existing benchmark executable")
    for configured in (plan["model"], plan["state_profile"]):
        for field in ("path", "schema_path"):
            resolved = resolve_plan_path(plan_path, configured[field])
            if not resolved.is_file():
                raise CampaignError(f"Campaign input does not exist: {resolved}")

    if arguments.pilot_o3:
        assert driver is not None
        return run_pilot(plan, plan_path, driver, output_dir)

    conditions = mandatory_conditions(plan)
    schedule = build_schedule(plan, conditions, "mandatory")
    started_at = utc_now()
    report_path = output_dir / "campaign-report.json"
    if arguments.dry_run:
        atomic_write_json(
            report_path,
            report_document(plan, plan_path, None, "planned", schedule, [], started_at),
        )
        print(f"campaign_id={plan['campaign_id']} mandatory_runs={len(schedule)} status=planned")
        return 0

    assert driver is not None
    records: list[dict[str, Any]] = []
    memory_limit = int(
        physical_memory_bytes() * plan["limits"]["maximum_physical_memory_fraction"]
    )
    execute_schedule(
        plan, plan_path, driver, output_dir, schedule, conditions, records, started_at,
        report_path, memory_limit
    )
    eligibility = conditional_eligibility(plan, records)
    if eligibility["eligible"]:
        optional_conditions = conditional_conditions(plan)
        optional_schedule = build_schedule(plan, optional_conditions, "conditional")
        schedule.extend(optional_schedule)
        execute_schedule(
            plan, plan_path, driver, output_dir, optional_schedule, optional_conditions, records,
            started_at, report_path, memory_limit, ledger_schedule=schedule
        )
    final_status = "complete" if all(record["status"] == "completed" for record in records) else "complete_with_failures"
    report = report_document(
        plan, plan_path, driver, final_status, schedule, records, started_at, eligibility
    )
    if not report["correctness"]["passed"]:
        final_status = "complete_with_failures"
        report["status"] = final_status
        report["suitable_for_analysis"] = False
    atomic_write_json(report_path, report)
    print(
        f"campaign_id={plan['campaign_id']} runs={len(records)} status={final_status} "
        f"analysis_ready={str(report['suitable_for_analysis']).lower()}"
    )
    return 0 if report["suitable_for_analysis"] else 3


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except CampaignError as error:
        print(f"[MEHLISSA-RQ4] {error}", file=sys.stderr)
        raise SystemExit(3) from error
