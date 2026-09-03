# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the locked Paper 1 protocol v2 and its pre-measurement semantics."""

from __future__ import annotations

import json
from pathlib import Path

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "publication/paper1/technical-experiment-protocol-v2.0.0.json"
SCHEMA = ROOT / "data/schemas/paper1-technical-protocol/2.0.0.schema.json"


class ProtocolError(ValueError):
    pass


def load(path: Path) -> dict:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def errors(document: dict) -> list[str]:
    schema = load(SCHEMA)
    validator = Draft202012Validator(schema, format_checker=FormatChecker())
    result = [error.message for error in validator.iter_errors(document)]
    question_ids = [item["id"] for item in document.get("evaluation_questions", [])]
    if set(question_ids) != {"RQ-T1", "RQ-T2", "RQ-T3", "RQ-T4"}:
        result.append("technical evaluation questions must cover RQ-T1 through RQ-T4 exactly")
    experiment_ids = [item["id"] for item in document.get("experiments", [])]
    if set(experiment_ids) != {
        "P1-E1-BODY-OBSERVATION",
        "P1-E2-M7-RESOURCE",
        "P1-E3-ACCESS-PARITY",
    }:
        result.append("protocol must cover all three locked experiments exactly")
    if document.get("amendment", {}).get("predecessor_modified") is not False:
        result.append("frozen predecessor must remain unmodified")
    if document.get("protocol", {}).get("manuscript_text") is not False:
        result.append("protocol must not be manuscript text")
    text = json.dumps(document).lower()
    for required in (
        "dirty worktree",
        "partial_run",
        "failed_run",
        "resource_limited_run",
        "sha-256",
        "peak",
        "workbench",
        "clinical",
    ):
        if required not in text:
            result.append(f"required protocol concept is absent: {required}")
    return result


def validate(path: Path = PROTOCOL) -> None:
    found = errors(load(path))
    if found:
        raise ProtocolError("\n".join(found))


if __name__ == "__main__":
    try:
        validate()
    except (OSError, json.JSONDecodeError, ProtocolError) as error:
        print(f"Paper 1 protocol v2: FAILED\n{error}")
        raise SystemExit(1)
    print("Paper 1 protocol v2: ok (4 technical questions, 3 locked experiments)")
