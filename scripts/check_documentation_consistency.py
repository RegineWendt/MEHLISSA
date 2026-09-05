# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Check shared MEHLISSA documentation facts and requirement traceability."""

from __future__ import annotations

from collections import Counter
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
STATE_PATH = ROOT / "docs" / "PROJECT_STATE.json"
SYSTEM_REQUIREMENTS = ROOT / "docs" / "requirements" / "SYSTEM_REQUIREMENTS.md"
TRACEABILITY = ROOT / "docs" / "requirements" / "TRACEABILITY_MATRIX.md"

IMPLEMENTATION_STATUSES = {"DONE", "PART", "LEGACY", "SPEC"}
EVIDENCE_STATUSES = {"VERIFIED", "PART", "UNVERIFIED", "RESEARCH"}


def read(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8")


def requirement_rows(markdown: str) -> dict[str, list[str]]:
    rows: dict[str, list[str]] = {}
    for line in markdown.splitlines():
        if re.match(r"^\| [A-Z]+-\d{3} \|", line):
            cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
            rows[cells[0]] = cells
    return rows


def local_links(relative_path: str, markdown: str) -> list[Path]:
    source = ROOT / relative_path
    targets: list[Path] = []
    for match in re.finditer(r"\[[^\]]+\]\((<?)([^)>]+)(>?)\)", markdown):
        target = unquote(match.group(2).strip())
        if target.startswith(("http://", "https://", "mailto:", "#")):
            continue
        path_text = target.split("#", 1)[0]
        if not path_text:
            continue
        targets.append((source.parent / path_text).resolve())
    return targets


def main() -> int:
    state = json.loads(STATE_PATH.read_text(encoding="utf-8"))
    errors: list[str] = []

    research_use = state["research_use"]
    shared_checks = {
        "docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md": [
            state["status_date_display"],
            state["development_branch"],
            state["current_focus"],
            research_use["workbench_version"],
            research_use["release_commit"],
            research_use["verified_ci_run"],
        ],
        "docs/ROADMAP.md": [
            research_use["release_commit"],
            research_use["verified_ci_run"],
            "Scientific qualification and realism",
        ],
        "docs/USER_GUIDE.md": [
            research_use["workbench_version"],
            research_use["release_commit"],
            research_use["verified_ci_run"],
        ],
        "docs/DEVELOPMENT.md": [
            research_use["workbench_version"],
            str(research_use["windows_msvc_ctest_count"]),
        ],
        "README.md": [research_use["workbench_version"]],
    }
    canonical_markdown: dict[str, str] = {}
    for relative_path, expected_values in shared_checks.items():
        markdown = read(relative_path)
        canonical_markdown[relative_path] = markdown
        normalized_markdown = re.sub(r"\s+", " ", markdown)
        for value in expected_values:
            if re.sub(r"\s+", " ", value) not in normalized_markdown:
                errors.append(f"{relative_path}: missing project-state value {value!r}")

    roadmap = canonical_markdown["docs/ROADMAP.md"]
    for package in state["next_scientific_packages"]:
        if package not in roadmap:
            errors.append(f"docs/ROADMAP.md: missing next scientific package {package!r}")

    system_text = SYSTEM_REQUIREMENTS.read_text(encoding="utf-8")
    trace_text = TRACEABILITY.read_text(encoding="utf-8")
    system_ids = re.findall(r"^\| ([A-Z]+-\d{3}) \|", system_text, re.MULTILINE)
    trace_ids = re.findall(r"^\| ([A-Z]+-\d{3}) \|", trace_text, re.MULTILINE)
    for label, values in (("system requirements", system_ids), ("traceability", trace_ids)):
        duplicates = sorted(key for key, count in Counter(values).items() if count != 1)
        if duplicates:
            errors.append(f"{label}: requirement IDs must occur once: {duplicates}")
    if set(system_ids) != set(trace_ids):
        errors.append(
            "system requirements and traceability IDs differ: "
            f"only-system={sorted(set(system_ids) - set(trace_ids))}, "
            f"only-trace={sorted(set(trace_ids) - set(system_ids))}"
        )

    trace_rows = requirement_rows(trace_text)
    for requirement_id, row in trace_rows.items():
        if len(row) != 6:
            errors.append(
                f"traceability: {requirement_id} must have six columns for "
                "implementation and evidence status"
            )
            continue
        implementation_status, evidence_status = row[2], row[3]
        if implementation_status not in IMPLEMENTATION_STATUSES:
            errors.append(
                f"traceability: {requirement_id} has unknown implementation status "
                f"{implementation_status!r}"
            )
        if evidence_status not in EVIDENCE_STATUSES:
            errors.append(
                f"traceability: {requirement_id} has unknown evidence status "
                f"{evidence_status!r}"
            )
        if evidence_status == "VERIFIED" and implementation_status != "DONE":
            errors.append(
                f"traceability: {requirement_id} cannot be evidence-VERIFIED while "
                f"implementation is {implementation_status}"
            )
        if evidence_status == "PART" and implementation_status not in {"DONE", "PART"}:
            errors.append(
                f"traceability: {requirement_id} cannot have partial Next evidence "
                f"while implementation is {implementation_status}"
            )

    for requirement_id, expected in state["traceability_expectations"].items():
        row = trace_rows.get(requirement_id)
        if row is None:
            errors.append(f"traceability: missing expected row {requirement_id}")
            continue
        actual = {
            "implementation": row[2] if len(row) >= 3 else "<missing>",
            "evidence": row[3] if len(row) >= 4 else "<missing>",
        }
        if actual != expected:
            errors.append(
                f"traceability: {requirement_id} has statuses {actual}, expected {expected}"
            )

    section_checks = {
        "DATA-002": f"RM {state['roadmap_sections']['experiment_and_result_format']}",
        "UX-004": f"RM {state['roadmap_sections']['visualization_and_user_tools']}",
        "QUA-005": f"RM {state['roadmap_sections']['documentation']}",
    }
    system_rows = requirement_rows(system_text)
    for requirement_id, expected_reference in section_checks.items():
        for label, rows in (("system requirements", system_rows), ("traceability", trace_rows)):
            row = rows.get(requirement_id, [])
            if not any(expected_reference in cell for cell in row):
                errors.append(
                    f"{label}: {requirement_id} must reference {expected_reference}"
                )

    link_sources = [
        "README.md",
        "docs/README.md",
        "docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md",
        "docs/ROADMAP.md",
        "docs/USER_GUIDE.md",
        "docs/DEVELOPMENT.md",
        "docs/architecture/SOFTWARE_ARCHITECTURE.md",
    ]
    for relative_path in link_sources:
        markdown = canonical_markdown.get(relative_path) or read(relative_path)
        for target in local_links(relative_path, markdown):
            if not target.exists():
                errors.append(f"{relative_path}: broken local link to {target}")

    forbidden = {
        "docs/ux/README.md": ["post-M7 user-experience program"],
        "docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md": [
            "and integrated visualization."
        ],
        "docs/architecture/SOFTWARE_ARCHITECTURE.md": [
            "planned for the M7/M8 path"
        ],
    }
    for relative_path, phrases in forbidden.items():
        markdown = read(relative_path)
        for phrase in phrases:
            if phrase in markdown:
                errors.append(f"{relative_path}: obsolete phrase remains: {phrase!r}")

    if errors:
        for error in errors:
            print(f"documentation consistency error: {error}", file=sys.stderr)
        return 1

    implementation_counts = Counter(row[2] for row in trace_rows.values())
    evidence_counts = Counter(row[3] for row in trace_rows.values())
    implementation_summary = ", ".join(
        f"{status}={implementation_counts[status]}" for status in sorted(IMPLEMENTATION_STATUSES)
    )
    evidence_summary = ", ".join(
        f"{status}={evidence_counts[status]}" for status in sorted(EVIDENCE_STATUSES)
    )
    print(
        "documentation consistency: ok "
        f"({len(system_ids)} requirements; implementation: {implementation_summary}; "
        f"evidence: {evidence_summary}; {len(link_sources)} canonical link sources)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
