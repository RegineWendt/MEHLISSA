# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the PCQ-1.2 evidence-source candidate register."""

from __future__ import annotations

import json
from pathlib import Path

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
REGISTER = ROOT / "data/qualification/pulmonary-capillary-evidence-candidate-register-v1.json"
SCHEMA = ROOT / "data/schemas/pulmonary-capillary-evidence-candidate-register/1.0.0.schema.json"


class EvidenceCandidateRegisterError(ValueError):
    pass


def load(path: Path) -> dict:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def errors(document: dict) -> list[str]:
    validator = Draft202012Validator(load(SCHEMA), format_checker=FormatChecker())
    result = [error.message for error in validator.iter_errors(document)]

    summaries = document.get("track_summary", [])
    summary_tracks = [item.get("track") for item in summaries]
    expected_tracks = {"PCQ-H", "PCQ-R", "PCQ-C", "PCQ-J"}
    if set(summary_tracks) != expected_tracks or len(summary_tracks) != 4:
        result.append("track summaries must cover PCQ-H, PCQ-R, PCQ-C, and PCQ-J exactly once")

    candidates = document.get("candidates", [])
    candidate_ids = [item.get("id") for item in candidates]
    if len(candidate_ids) != len(set(candidate_ids)):
        result.append("candidate identifiers must be unique")
    by_id = {item.get("id"): item for item in candidates}

    ranked: dict[str, list[tuple[int, str]]] = {track: [] for track in expected_tracks}
    seen_ranks: set[tuple[str, int]] = set()
    for candidate in candidates:
        candidate_id = candidate.get("id", "<missing>")
        track_ids = set(candidate.get("track_ids", []))
        candidate_rank_tracks: set[str] = set()
        for ranking in candidate.get("rankings", []):
            track = ranking.get("track")
            rank = ranking.get("rank")
            if track not in track_ids:
                result.append(f"ranking track is absent from track_ids: {candidate_id} {track}")
            if track in candidate_rank_tracks:
                result.append(f"candidate has duplicate ranking for track: {candidate_id} {track}")
            candidate_rank_tracks.add(track)
            if (track, rank) in seen_ranks:
                result.append(f"rank must be unique within track: {track} {rank}")
            seen_ranks.add((track, rank))
            if track in ranked and isinstance(rank, int):
                ranked[track].append((rank, candidate_id))

        access = candidate.get("access", {})
        if candidate.get("decision") in {"priority-request", "backup-request"}:
            if candidate.get("data_license") != "not established":
                result.append(f"requested candidate must retain unresolved raw-data licence: {candidate_id}")
            if "request" not in access.get("participant_data", "").lower():
                result.append(f"requested candidate must document participant-data request: {candidate_id}")

    for summary in summaries:
        track = summary.get("track")
        listed = summary.get("ranked_candidate_ids", [])
        expected = [candidate_id for _, candidate_id in sorted(ranked.get(track, []))]
        if listed != expected:
            result.append(f"ranked candidate list does not match candidate rankings for {track}")
        for candidate_id in listed:
            if candidate_id not in by_id:
                result.append(f"track summary references unknown candidate: {candidate_id}")

    expected_candidates = {
        "PCQ-SRC-H-001",
        "PCQ-SRC-H-004",
        "PCQ-SRC-R-001",
        "PCQ-SRC-R-002",
        "PCQ-SRC-CJ-001",
        "PCQ-SRC-C-002",
        "PCQ-SRC-C-003",
        "PCQ-SRC-CM-001",
    }
    if not expected_candidates.issubset(by_id):
        result.append("register is missing one or more required screened candidate roles")

    pvd = by_id.get("PCQ-SRC-H-004", {})
    if pvd.get("decision") != "rejected-for-primary":
        result.append("PVDOMICS must remain rejected for primary PCQ-H under its published healthy-control protocol")
    if "non-invasive" not in pvd.get("jointness", ""):
        result.append("PVDOMICS rejection must record non-invasive healthy-control CPET")

    dsouza = by_id.get("PCQ-SRC-CJ-001", {})
    if dsouza.get("outcome_exposure", {}).get("status") != "public-aggregate-inspected":
        result.append("D'Souza public aggregate outcome exposure must remain explicit")
    if set(dsouza.get("track_ids", [])) != {"PCQ-C", "PCQ-J"}:
        result.append("D'Souza must remain a PCQ-C and partial PCQ-J candidate")

    lassen = by_id.get("PCQ-SRC-C-003", {})
    lassen_text = json.dumps(lassen).lower()
    if not all(term in lassen_text for term in ("pulmonary trunk", "left atrium", "algebra", "cannot independently")):
        result.append("Lassen candidate must preserve anatomical and circular-closure limitations")

    joint_summary = next((item for item in summaries if item.get("track") == "PCQ-J"), {})
    if joint_summary.get("status") != "partial-candidates-only":
        result.append("PCQ-J cannot advance beyond partial candidate status in PCQ-1.2")

    actions = document.get("next_actions", [])
    priorities = [item.get("priority") for item in actions]
    if priorities != list(range(1, len(actions) + 1)):
        result.append("next-action priorities must be contiguous and ordered from one")
    external = [item for item in actions if "request" in item.get("action", "").lower()]
    if any("not sent" not in item.get("authorization", "") for item in external):
        result.append("external request actions must state that PCQ-1.2 did not send them")

    full_text = json.dumps(document).lower()
    for required in (
        "no participant-level candidate data",
        "not tuned",
        "redistribution",
        "cohort",
        "observation model",
        "clinical_use",
    ):
        if required not in full_text:
            result.append(f"required screening safeguard is absent: {required}")
    return result


def validate(path: Path = REGISTER) -> None:
    document = load(path)
    found = errors(document)
    if found:
        raise EvidenceCandidateRegisterError("\n".join(found))


if __name__ == "__main__":
    try:
        validate()
    except (OSError, json.JSONDecodeError, EvidenceCandidateRegisterError) as error:
        print(f"PCQ-1.2 evidence candidate register: FAILED\n{error}")
        raise SystemExit(1)
    document = load(REGISTER)
    print(
        "PCQ-1.2 evidence candidate register: ok "
        f"({len(document['candidates'])} candidates, 4 tracks, no participant outcomes acquired)"
    )
