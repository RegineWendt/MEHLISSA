<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Paper 1 platform/methods release candidate

Candidate: `paper1-platform-methods-rc1-20260903`

Suggested tag: `paper1-platform-methods-rc1`

Status: review candidate; no DOI and no final tag

This is the auditable handoff package for a technical MEHLISSA platform and
methods paper.  It binds one source commit to a locked experiment protocol, a
six-family evidence and validity baseline, complete raw measurements, readable
reports, analysis scripts, a claim registry, and cryptographic hashes.

Start with:

1. `HANDOFF.md` for scope, reproduction, and reviewer decisions;
2. `release-candidate.json` for the machine-readable package identity;
3. `claim-to-artifact-registry.json` before drafting any result claim;
4. `measurements/README.md` and
   `docs/publication/PAPER1_TECHNICAL_MEASUREMENTS.md` for results;
5. `SHA256SUMS.json` to verify retained bytes.

For cross-platform identity, text files are hashed in their canonical LF form;
ZIP and PDF files are hashed byte for byte. The checksum manifest records this
rule explicitly, so Windows and Linux checkouts verify the same candidate.

The package is not a manuscript, does not modify any separate
`MEHLISSA-Papers` repository, and does not assert biological or clinical
validation.
