<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.3 Validation and Corrective Feedback

## 1. Outcome

UX-6.3 makes scenario validity visible before save or execution. Each complete
candidate is passed to the same accepted `mehlissa scenario validate` command
used at the command line. The workbench presents that authoritative decision,
adds a stable location and repair suggestion, distinguishes blocking errors
from non-blocking research warnings, and produces a plain-text summary that can
be copied into an issue, review, or experiment record.

Run control is deliberately absent until UX-6.4. The validation response
already exposes `run_allowed`; it is true only when the accepted command
accepts the exact candidate identified by the returned SHA-256 digest. The UI
keeps save unavailable while validation is pending or unsuccessful. The save
endpoint repeats authoritative validation to prevent a stale browser state or
direct request from bypassing the gate.

## 2. Validation layers

The workbench uses two complementary layers without creating a second
scientific validator:

1. Schema-derived field checks turn the type, minimum, maximum, minimum length,
   and pattern annotations already supplied to the editor into precise local
   messages. These identify the affected control immediately.
2. The Python host reconstructs the complete candidate from its approved source
   and allow-listed changes, writes a short-lived private candidate, and calls
   `MehlissaClient.validate_scenario()`. That command performs JSON Schema,
   semantic composition, artifact existence/schema, role-completeness, stage
   order, and target/baseline compatibility checks.

Only the second layer decides `valid` and `run_allowed`. A disagreement between
the schema-derived structural hints and an accepted CLI result fails closed as
a workbench error. Temporary candidates are removed and their paths are not
included in the shareable report.

## 3. Issue contract

Every issue has five fields:

| Field | Meaning |
|---|---|
| `severity` | `error` blocks save/run; `warning` records a research interpretation risk but does not invalidate the candidate |
| `code` | stable `WBV-*` workbench code or the existing `MEHLISSA-E####` command code |
| `path` | editable dotted field, document section such as `artifacts`, or `$` for a whole-document diagnostic |
| `message` | concise explanation of the observed problem |
| `guidance` | an actionable next correction, without claiming that software validity establishes scientific validity |

The initial workbench code families are:

| Code | Meaning |
|---|---|
| `WBV-1001` | field type mismatch |
| `WBV-1002` | non-finite numeric value |
| `WBV-1003` / `WBV-1004` | value below/above its schema limit |
| `WBV-1005` | required text is empty |
| `WBV-1006` | text does not match the schema pattern |
| `WBV-1900` | command failed without a recognized structured MEHLISSA code |
| `WBV-2001` | collector population lacks a published interpolation basis |
| `WBV-2002` | changed seed is one new realization, not an uncertainty estimate |

Native command failures retain their `MEHLISSA-E####` identifier. Known JSON
instance paths and semantic diagnostic phrases are mapped to a guided field or
complete-document section. A failure that cannot safely be narrowed is located
at `$`; it is never guessed into an unrelated field.

## 4. Private validation API

`POST /api/scenario/validate` accepts the same bounded `source_id` and
allow-listed `changes` object as save-as. It does not accept a document, path,
command, or artifact replacement from the browser. Its versioned response is:

```json
{
  "api_version": "1.0.0",
  "valid": true,
  "run_allowed": true,
  "authoritative": true,
  "validator": "mehlissa scenario validate",
  "candidate_sha256": "...",
  "error_count": 0,
  "warning_count": 0,
  "issues": [],
  "summary_text": "..."
}
```

The browser debounces edits and discards superseded responses, so a slower old
request cannot replace a newer decision. The endpoint retains all UX-6.1/6.2
loopback, Host-header, session capability, JSON body-size, and file-boundary
controls. Validation has no run or output side effect.

`POST /api/scenario/save` returns HTTP 422 with the same `validation` object if
the complete candidate is rejected. No destination is created. HTTP 400 remains
reserved for malformed or unsafe requests and HTTP 409 for an existing
destination.

## 5. Researcher workflow

1. Open a starter or saved scenario. Initial authoritative validation begins
   automatically.
2. Edit a guided value. The status changes to **Checking**, save is disabled,
   and the complete candidate is revalidated after a short pause.
3. Correct red errors at their highlighted field or document section. Keep the
   stable code when asking another contributor for help.
4. Review amber warnings. A warning permits saving but changes how the result
   may be interpreted.
5. Expand **Shareable validation summary** to inspect the decision, scenario
   identity, candidate SHA-256, counts, issues, repair guidance, and non-clinical
   boundary. Copy it when a review record is needed.
6. Save only after the status is **Valid**. Save-as performs the same validation
   again and never overwrites an existing file.

## 6. Acceptance evidence

The automated positive fixture is accepted by both the workbench endpoint and
the CLI. Negative fixtures cover a schema minimum, incompatible target/timer
identity, and a missing cross-file artifact. They retain native MEHLISSA codes,
resolve to `run.collector_count`, `target`, or `artifacts`, provide repair text,
and set both `valid` and `run_allowed` false. An HTTP-level test proves an
invalid candidate returns 422, creates no file, and has no run endpoint to
bypass the future gate. A valid changed seed demonstrates a non-blocking warning.

Desktop and narrow-screen browser acceptance additionally checks live status,
field highlighting, readable issue/report panels, keyboard-reachable controls,
copy behavior, save gating, and the absence of horizontal overflow. Full
supported CI remains pending until the repository owner requests a push.
