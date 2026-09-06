# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Runner-independent MRSQ-1 archive, claim, and evidence-boundary review."""

from __future__ import annotations
import csv
import hashlib
import json
import math
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
FREEZE = ROOT / "data/qualification/medical-reference-scenario-candidate-freeze-v1.json"
CLOSEOUT = ROOT / "data/qualification/medical-reference-scenario-closeout-v1.json"


def load(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream: return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""): digest.update(chunk)
    return digest.hexdigest()


def errors(root: Path = ROOT) -> list[str]:
    result: list[str] = []; freeze = load(root / FREEZE.relative_to(ROOT)); closeout = load(root / CLOSEOUT.relative_to(ROOT))
    for group in (freeze["parents"], freeze["implementation"]):
        for item in group.values():
            if not isinstance(item, dict) or "path" not in item: continue
            path = root / item["path"]
            if not path.is_file() or sha256(path) != item["sha256"]: result.append(f"frozen hash mismatch: {item['path']}")
    archive = freeze["archive"]; archive_path = root / archive["path"]
    if not archive_path.is_file() or sha256(archive_path) != archive["sha256"] or archive_path.stat().st_size != archive["size_bytes"]:
        result.append("software-reference archive identity mismatch")
    else:
        with archive_path.open(newline="", encoding="utf-8") as stream: rows = list(csv.DictReader(stream))
        if len(rows) != archive["rows"]: result.append("software-reference row count mismatch")
        for row in rows:
            if any(not math.isfinite(float(value)) or float(value) < 0 for value in row.values()): result.append("software-reference value is invalid"); break
    if freeze["candidate"]["participant_files_opened"] or freeze["candidate"]["validation_outcomes_inspected"]:
        result.append("candidate was not frozen outcome-blind")
    increments = {item["id"]: item for item in closeout["increments"]}
    if set(increments) != {f"MRSQ-1.{i}" for i in range(1, 8)}: result.append("close-out does not cover all seven increments")
    if closeout["gates"]["measured_ingress"] != "BLOCKED" or closeout["gates"]["five_primary_cohort_endpoints"] != "BLOCKED" or closeout["gates"]["external_human_review"] != "BLOCKED":
        result.append("external evidence blockers were overstated")
    for asset in closeout["software_evidence"]["assets"]:
        path = root / asset["path"]
        if not path.is_file() or sha256(path) != asset["sha256"]:
            result.append(f"software-evidence hash mismatch: {asset['path']}")
    protocol = load(root / "data/qualification/medical-reference-scenario-protocol-v1.json")
    expected = {item["id"]: item["limits"] for item in protocol["endpoint_rules"]}
    actual = {item["id"]: item for item in closeout["software_evidence"]["endpoint_limits"]}
    if set(actual) != set(expected):
        result.append("evaluation endpoint set differs from protocol")
    for endpoint_id, limits in expected.items():
        item = actual.get(endpoint_id, {})
        if (item.get("nrmse") != limits["cohort_upper_90_ci_median_duration_weighted_nrmse"] or
                item.get("auc_lower") != limits["cohort_90_ci_geometric_mean_auc_ratio_lower"] or
                item.get("auc_upper") != limits["cohort_90_ci_geometric_mean_auc_ratio_upper"]):
            result.append(f"evaluation limits differ from protocol: {endpoint_id}")
    contract = closeout["software_evidence"]["cohort_contract"]
    if contract["replicates"] != protocol["statistics"]["bootstrap_replicates"] or contract["seed"] != protocol["statistics"]["bootstrap_seed"]:
        result.append("evaluation bootstrap differs from protocol")
    text = json.dumps(closeout).lower()
    for phrase in ("frame-duration", "institutional", "at least 60", "external human", "no physiological"):
        if phrase not in text: result.append(f"close-out boundary missing: {phrase}")
    if len(freeze["uncertainty_plan"]) != 7 or len(freeze["declared_assumptions"]) < 4: result.append("freeze lacks uncertainty or assumptions")
    return result


def main() -> int:
    found = errors()
    if found: print("MRSQ-1.7 close-out: FAILED\n" + "\n".join(found)); return 1
    print("MRSQ-1.7 close-out: computational path PASS; measured cohort and external human review BLOCKED")
    return 0


if __name__ == "__main__": raise SystemExit(main())
