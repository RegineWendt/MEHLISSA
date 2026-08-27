<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0012: Structured Errors, Run Logs, and Checkpoints

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M1; `SYS-002`, `SYS-007`, `DATA-001`, `DATA-003`, `QUA-005`

## Context

Free-form error messages and changing exit codes cannot be evaluated reliably
by automated experiments, CI, and later Python tools. Long simulation runs must
also document their progress in machine-readable form and be able to leave a
defined resume point.

A checkpoint is robust only when it is unambiguously bound to the experiment,
time, seed, random state, and versioned component states. A raw memory image
would be platform-dependent and could silently treat changes as compatible.

## Decision

1. Controlled errors derive from `MehlissaError` and carry a stable numeric `ErrorCode` and an identifier of the form `MEHLISSA-Edddd`.
2. Assigned identifiers are not reinterpreted. New causes receive a new identifier; free-form diagnostics may be refined.
3. The CLI writes the identifier to `stderr` and maps error groups to stable exit-status values.
4. Every run writes `run.log.jsonl`. Each line is an independent JSON document conforming to versioned schema `1.0.0` and contains sequence, UTC time, simulation time, level, source, event, and message.
5. Error records additionally contain the numeric code and identifier. If writing an error message itself fails, the original error remains authoritative.
6. Checkpoints use a versioned JSON manifest instead of a raw memory image. Version `1.0.0` binds the experiment hash, software version, sequence, simulation time, master seed, and draw counts of named random streams.
7. Component states reside in separate files addressed relative to the checkpoint directory. Each reference states the component name, state schema version, and SHA-256 checksum.
8. Writing and loading validate the manifest against JSON Schema, reject escaping component paths, check unique names, and verify referenced checksums.
9. After a successful run, the minimal experiment writes a final `checkpoint-000000.json`. Actual resumption follows when stateful M2 components implement a snapshot contract.

## Consequences

Positive:

- Scripts can evaluate cause and exit status independently of wording.
- JSONL remains processable line by line even for large logs and is flushed after every record.
- Checkpoints are self-describing, versionable, and protected against swapped or subsequently modified component states.
- RNG draw counts enable algorithmically defined reconstruction without serializing platform-specific memory layout.

Negative:

- UTC timestamps differ between otherwise identical runs; deterministic comparisons must exclude observational fields.
- Flushing every log line costs I/O performance. A buffered variant may be added later only with explicit loss tolerance.
- M1 specifies and verifies the checkpoint but does not yet perform full resumption of domain components.
- Reconstructing highly advanced RNG streams solely from a counter can be expensive; a later portable engine-state format needs its own version and comparison tests.

## Alternatives

- **Text messages only:** rejected because identifiers and fields would not be stably machine-readable.
- **One large JSON log:** rejected because interruption could leave the entire document invalid and would impede streaming.
- **Binary memory image:** rejected because layout, endianness, and software compatibility would remain implicit.
- **Component state directly in the manifest:** rejected because large or binary states would bloat the control manifest and could not be versioned independently.
- **Absolute snapshot paths:** rejected because checkpoints could not then be moved or archived portably.
