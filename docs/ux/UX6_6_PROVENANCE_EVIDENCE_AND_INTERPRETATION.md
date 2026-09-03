<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.6 Provenance, Evidence, and Interpretation Boundaries

## Purpose

UX-6.6 makes the origin and permissible interpretation of a displayed result
part of the normal result workflow. A numerical outcome is never shown as if it
stood alone: its retained input, software/build identity, seeds, model and
schema identities, evidence, licences, maturity, limitations, and non-clinical
scope remain adjacent and exportable.

This package does not increase physiological or clinical validity. It makes the
current validity state harder to overlook or overstate.

## User-facing contract

Opening a completed scenario or campaign now adds an **Audit summary** below
the UX-6.5 dashboard. It contains:

- a permanent research-only, non-medical-device, non-patient-specific notice;
- the workbench version and audit-contract version that produced the projection;
- the run title, completion time, complete seed plan, and experiment maturity;
- software version, Git revision and dirty state, compiler, operating system,
  and architecture when recorded by the accepted provenance artifact;
- a table of expected and observed SHA-256 values for retained inputs,
  results, manifests, model definitions, and schemas;
- scenario sources with citation, role, declared licence/status, and URL or
  repository location;
- one expandable card per selected component with model/profile identity,
  version, evidence class, validity statement, integrity, sources, licences,
  and limitations;
- all scenario, campaign, and result interpretation limitations;
- each derived campaign run's seed, manifest/result integrity, software
  version, Git revision, and complete retained provenance; and
- a download of the complete audit response as UTF-8 JSON.

The header, result workspace, and export all repeat the binding boundary that
MEHLISSA Next is research software and not validated for clinical or
patient-specific decisions.

## Authority and data flow

The browser does not calculate hashes or assign evidence status. The
capability-protected private endpoint
`GET /api/run/audit?id=<workbench-job-id>` calls `RunWorkspace.audit()`.
The host resolves the job from server-owned state, reads only its registered
artifacts or repository-bounded paths declared in an accepted result, and emits
audit contract version `1.0.0`.

For one scenario, the endpoint returns the original `provenance.json` object
without dropping fields. It also reads the accepted result's reproducibility
artifact list and the exact retained scenario input. For a campaign, it reads
the accepted aggregate result, each declared derived manifest/result, and each
derived result's provenance artifact. This keeps the audit trace connected to
the same machine-readable evidence used by the simulator and UX-6.5 readers.

## Integrity semantics

Each check has one of three explicit states:

| State | Meaning |
|---|---|
| `verified` | The declared expected SHA-256 equals the bytes or canonical validated candidate currently read. |
| `altered` | A file exists, but its observed SHA-256 differs from the declared identity. |
| `missing` | The declared file cannot be resolved inside the allowed job/repository boundary. |

The aggregate state is `verified` only if every check is verified. Any altered
or missing item produces `attention`. A verified digest means that retained
bytes match a declared identity; it is not evidence that a scientific model is
accurate.

The scenario candidate check deliberately reserializes the retained JSON using
the same canonical formatting used by UX-6.3 validation. This distinguishes the
validation identity from Windows newline translation. The separate provenance
profile check verifies the physical retained file bytes.

## Evidence and maturity semantics

Component maturity is a plain-language projection of the declared
`validity.evidence_class`:

| Evidence class | Displayed meaning |
|---|---|
| `software_test_surrogate` | Software-test surrogate |
| `literature_parameterized` | Literature-parameterized research model |
| `externally_derived` | Externally derived research model |
| `hypothesis` | Research hypothesis |
| absent or unknown | Undeclared — attention required |

Every class is accompanied by **Not clinically validated**. Scenario acceptance
Level A is explained as integrated vertical-slice software maturity, not as an
“A run” or a clinical evidence grade.

Evidence is `complete` only when the scenario and every selected component have
at least one source and every source declares an identifier, citation, and
licence/status. Missing declarations remain visible and make the aggregate
status `incomplete`; the workbench never fills them from memory or inference.

The present FP9 composition therefore reports incomplete evidence despite
verified integrity: the historical timer baseline predates the uniform source
contract and contains citations and repository locations but no per-source
licence fields. This is an actionable metadata gap, not a failed simulation and
not permission to infer a licence.

## Security and export

- The endpoint requires the ephemeral workbench session capability.
- The caller supplies a job identifier, never a filesystem path.
- Retained campaign paths must remain inside that job's directory.
- Model and schema paths must remain inside the repository.
- Regular content is inserted with `textContent`; no audit field becomes HTML.
- Only HTTP(S) evidence targets become external links and open with
  `noopener noreferrer`; repository paths remain text.
- The download is created locally from the exact displayed API response. No
  remote service, telemetry, or upload is involved.

## Local acceptance

UX-6.6 is locally accepted when:

1. the reference scenario returns its complete original provenance document;
2. every unmodified scenario input, result, model definition, and schema hash
   verifies;
3. a deliberately altered retained input produces `attention` and at least one
   `altered` check;
4. the missing historical timer licence is visibly reported as incomplete;
5. the six-run campaign returns all six seeds and verifies its source manifest,
   derived manifests, derived results, and provenance-linked result hashes;
6. incomplete, failed, cancelled, and running jobs remain zero-observation
   non-results with the non-clinical boundary intact;
7. the JSON export contains the same complete audit and provenance data shown
   in the interface;
8. the complete Windows/MSVC suite passes; and
9. desktop and narrow-viewport browser review covers integrity, evidence,
   component detail, persistent boundary, export, keyboard focus, console
   state, and horizontal overflow.

UX-6.7 next adds sensitivity and uncertainty visualization plus reproducible
figure, table, and analysis-data export. It must preserve this audit boundary
beside every plot.
