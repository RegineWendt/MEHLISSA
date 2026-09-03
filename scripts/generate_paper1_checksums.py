# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Generate the deterministic SHA-256 inventory for a Paper 1 candidate."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CANDIDATE = (
    ROOT
    / "publication/paper1/release-candidates/paper1-platform-methods-rc1-20260903"
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate-directory", type=Path, default=DEFAULT_CANDIDATE)
    args = parser.parse_args()
    candidate = args.candidate_directory.resolve()
    if ROOT not in candidate.parents:
        raise SystemExit("Candidate directory must be inside the repository")
    manifest = json.loads((candidate / "release-candidate.json").read_text(encoding="utf-8"))
    files = []
    for path in sorted(candidate.rglob("*")):
        if not path.is_file() or path.name in {"SHA256SUMS.json", "SHA256SUMS.json.license"}:
            continue
        files.append(
            {
                "path": path.relative_to(candidate).as_posix(),
                "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                "bytes": path.stat().st_size,
            }
        )
    result = {
        "schema_version": "1.0.0",
        "candidate_id": manifest["candidate_id"],
        "algorithm": "SHA-256",
        "files": files,
    }
    (candidate / "SHA256SUMS.json").write_text(
        json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n"
    )
    print(f"wrote {len(files)} checksums")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
