<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.2 Guided Scenario Workspace

## 1. Outcome and boundary

UX-6.2 turns the UX-6.1 discovery page into a bounded scenario workspace. A
researcher can open the accepted complete FP9/lung fingerprinting demonstrator,
change scalar fields described by its authoritative JSON Schema, save a new
scenario, reopen it, and inspect the complete retained JSON without editing the
source file directly.

The workbench does **not** start a simulation in this increment. It also does
not provide the field-level corrective validation planned for UX-6.3. A save is
nevertheless accepted only when the existing `mehlissa scenario validate`
command accepts the complete candidate. The C++ application and the versioned
schema therefore remain authoritative.

## 2. Researcher workflow

1. Start the workbench from the repository root.
2. Select the implemented complete FP9/lung model and a curated starter or a
   previously saved workbench scenario.
3. Read the description, unit, default, evidence note, and limitation shown
   beside every guided parameter.
4. Change scenario identity, version, title, run identity, master seed,
   collector population, or target metadata.
5. Inspect the complete, live source JSON projection. The 13 artifact bindings,
   acceptance sequence, sources, limitations, and any unsupported fields remain
   visible and are not reconstructed by the browser.
6. Choose **Save as**, provide a new `.json` filename, and let the accepted
   MEHLISSA validator check the complete candidate.
7. Continue from the newly saved scenario, which appears in the source chooser
   and can be reopened without losing values.

Switching sources, resetting a changed document, or leaving the page with
unsaved edits produces an explicit discard warning. A failed save leaves the
current document and its edits available.

## 3. Schema-derived field contract

The workbench recursively projects scalar fields from
`data/schemas/fingerprinting-scenario-profile/1.0.0.schema.json`. The schema now
uses ordinary JSON Schema annotations plus MEHLISSA presentation annotations:

| Annotation | Meaning in the workbench |
|---|---|
| `title` | concise field label |
| `description` | plain-language parameter meaning |
| `default` | documented starting value; never applied silently |
| `type`, `minimum`, `maximum`, `pattern`, `const` | native control and constraint hints |
| `x-unit` | explicit unit or metadata kind |
| `x-evidence` | origin or contract basis for the value |
| `x-limitation` | boundary on interpretation or substitution |

`const` fields are displayed but disabled. Arrays and composite structures are
source-only in UX-6.2. Adding another guided scalar field therefore starts with
the authoritative schema; no matching browser-side parameter registry exists.

The initial guided set contains the scenario and run identities, scenario
version and title, master seed, collector count, fingerprint identity, tissue,
and target region. The schema version, Level-A acceptance class, deterministic
replay requirement, and clinical non-claim are visible immutable fields.

## 4. Round-trip design

The browser submits only a source identity, a new filename, and changes to the
server-provided allow-list of editable paths. The host reloads the complete
original document, applies only those changes, writes a temporary candidate,
and calls `MehlissaClient.validate_scenario()`. It creates the destination only
after validation succeeds and uses exclusive creation so a race cannot turn
save-as into overwrite.

This design has three useful properties:

- hidden, array, future, and unsupported fields remain in the original object;
- browser code cannot replace artifact bindings or acceptance constraints
  through an undeclared patch; and
- schema or semantic failure cannot create an apparently usable destination.

An unknown field is reported in the source view rather than discarded. Because
the current schema rejects unknown properties, such a forward-incompatible
document cannot be saved until its schema/version support is implemented. This
is deliberate fail-closed behavior.

## 5. File and security boundary

The default destination is `workbench-scenarios/` under the repository. It is
ignored by Git because it contains researcher-authored local inputs. A caller
may use `--workspace <path>`, but the resolved directory must remain inside the
repository. Only safe basename-style JSON filenames are accepted; arbitrary
paths, URL paths, repository files, and source identifiers cannot be opened.

The UX-6.1 loopback, Host-header, session-capability, static allow-list,
no-telemetry, no-remote-assets, content-security, and safe-text-insertion
controls remain. UX-6.2 adds:

- a one-megabyte request-body limit and required JSON media type;
- capability protection on all scenario reads and writes;
- an explicit single POST operation rather than generic file access;
- server-side editable-path checking;
- authoritative validation before creation; and
- non-overwriting exclusive output creation.

The interface remains inappropriate for patient-identifiable data. The local
boundary reduces exposure; it is not a data-protection or clinical-governance
system.

## 6. Private workbench API

| Operation | Purpose |
|---|---|
| `GET /api/catalog` | read the validated model and example projection |
| `GET /api/scenarios` | list the curated template and safe local saved scenarios |
| `GET /api/scenario?id=<source-id>` | return the complete document, schema-derived fields, editable paths, evidence, limitations, and unknown-field paths |
| `POST /api/scenario/save` | validate and exclusively create a derived scenario from allow-listed changes |

These routes are private implementation contracts between the embedded page
and local Python host, not a supported remote REST API. Incompatible changes
must increment their response version and update browser and server tests
together.

## 7. Acceptance evidence

Automated tests cover discovery-format rejection, protected APIs, Host-header
and traversal rejection, loopback-only binding, security headers, schema field
projection, a validate/save/reopen round trip, exact retention of all artifact
and evidence arrays, exclusive-file conflict, workspace escape rejection, and
visible fail-closed treatment of an unknown future field.

The browser acceptance exercise covers:

- loading five model families, ten examples, and 13 guided/displayed fields;
- changing identity, title, run identity, and collector count;
- validated save-as and immediate reopen of the 10,000-collector scenario;
- a visible non-overwrite error for the same filename;
- desktop layout and 390-pixel responsive reflow without horizontal overflow;
- semantic names, native labelled controls, live status text, and permanent
  evidence/non-clinical context; and
- absence of remote content or browser-script HTML insertion.

Supported cross-platform CI remains pending until the local commits are pushed
at the project owner's request.
