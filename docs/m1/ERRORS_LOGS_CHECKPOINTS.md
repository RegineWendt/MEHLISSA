<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Error, Log, and Checkpoint Contract

## Error catalog 1.0

| Identifier | Enum | Meaning | CLI status |
|---|---|---|---:|
| `MEHLISSA-E1001` | `command_line_invalid` | invalid CLI invocation | 2 |
| `MEHLISSA-E2001` | `input_unreadable` | input is missing or unreadable | 3 |
| `MEHLISSA-E2002` | `json_invalid` | invalid JSON or JSONL | 3 |
| `MEHLISSA-E2003` | `schema_invalid` | validation schema itself is invalid | 3 |
| `MEHLISSA-E2004` | `manifest_invalid` | experiment violates its contract | 3 |
| `MEHLISSA-E2005` | `data_invalid` | input data violate their domain contract | 3 |
| `MEHLISSA-E3001` | `output_unwritable` | output cannot be written | 4 |
| `MEHLISSA-E3002` | `provenance_invalid` | provenance document violates its contract | 4 |
| `MEHLISSA-E4001` | `lifecycle_invalid` | invalid component or run state | 5 |
| `MEHLISSA-E4002` | `invariant_violated` | kernel or model invariant violated | 5 |
| `MEHLISSA-E4003` | `numeric_overflow` | numeric counter or time range exceeded | 5 |
| `MEHLISSA-E5001` | `checkpoint_invalid` | checkpoint or referenced state is invalid | 6 |
| `MEHLISSA-E5002` | `checkpoint_incompatible` | valid but incompatible checkpoint | 6 |
| `MEHLISSA-E9001` | `internal_failure` | unclassified internal error | 1 |

Assigned identifiers are public machine contracts. Diagnostic text may gain
more context, but the meaning of an identifier must not change.

## `run.log.jsonl`

Every line conforms to `data/schemas/log-record/1.0.0.schema.json`. Sequences
start at zero, contain no gaps, and represent write order. Simulation time is
stored in integer nanoseconds; the UTC timestamp is observational metadata only.

The runner emits at least:

1. `run_started` at simulation time zero;
2. `run_completed` after all outputs succeed; or
3. best-effort `run_failed` with an error identifier.

## `checkpoint-000000.json`

The manifest conforms to `data/schemas/checkpoint/1.0.0.schema.json`. It does not
contain large model states itself, but references them using relative paths,
their own schema versions, and SHA-256 checksums. Components can thus use
independent formats without bypassing the shared envelope.

M1 produces a final checkpoint for the still component-free minimal experiment.
M2 adds stateful component snapshots and the resume command. Until then, the
round-trip test already verifies the schema, path boundary, name uniqueness,
and tamper detection.
