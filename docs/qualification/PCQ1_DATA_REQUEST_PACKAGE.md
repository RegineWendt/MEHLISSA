<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1 Data-Request Package and Waiting-Period Plan

## Purpose and boundary

This package turns the PCQ-1.5a repository-first audit into a consistent set of
external request drafts. It does not show that a request was sent, that access
was granted, or that participant-level outcomes were inspected. Publication
contact addresses are convenience pointers and must be verified immediately
before use.

All requests preserve the PCQ-1.3 no-refit protocol, the PCQ-1.4 manifest-first
ingress boundary, and the PCQ-1.5 uncertainty and identifiability rules. Raw or
pseudonymous records stay outside Git in institution-controlled storage. Only
an authorized institutional representative may send a request or accept data-
use terms.

## Request queue

| Order | Candidate and scientific role | Draft | Publicly listed contact | Request state |
|---|---|---|---|---|
| 1 | D'Souza 2025 (`PCQ-SRC-CJ-001`): same-stage functional capillary volume and flow | [D'Souza request](DSOUZA_2025_DATA_REQUEST.md) | Professor Michael K. Stickland, `michael.stickland@ualberta.ca` | send-ready after institutional review |
| 2 | Arizona iCPET (`PCQ-SRC-H-001`): supine multipoint invasive pressure-flow trajectory | [Arizona request](../m3/UA_ICPET_DATA_REQUEST.md) | Professor Rebecca R. Vanderpool, `rebecca.vanderpool@osumc.edu` | send-ready after institutional review |
| 3 | Bailey 2019 (`PCQ-SRC-R-002`): participant-level five-lobe perfusion fractions and posture | [Bailey request](BAILEY_2019_DATA_REQUEST.md) | Professor Dale L. Bailey, `Dale.Bailey@sydney.edu.au` | send-ready after institutional review |
| 4 | Lassen 2023 (`PCQ-SRC-C-003`): possible future whole-pulmonary transit evidence | [Lassen feasibility inquiry](LASSEN_2023_FEASIBILITY_REQUEST.md) | Dr Martin Lyngby Lassen, `martin.lyngby.lassen@regionh.dk` | feasibility only; do not request records yet |

The first three messages request the smallest useful de-identified extract and
the associated reuse terms. The fourth deliberately asks only whether future
access would be possible. A positive Lassen reply cannot activate PCQ-C2 until
an independently fixed whole-pulmonary observation model exists.

## One institutional cover sentence

If a university data office requires a short description before the full
message, use:

> MEHLISSA is an open academic research simulator, and this request seeks the
> smallest de-identified extract needed for a prospectively locked, no-refit
> model qualification; participant records will remain in institution-
> controlled storage outside the public repository.

## Administrative preparation before sending

1. Name the legal recipient institution and the person authorized to sign a
   data-use agreement.
2. Have an independent reviewer sign off PCQ-1.3, PCQ-1.4, and the request that
   will be sent.
3. Create an institution-controlled quarantine directory outside the
   repository and nominate its access administrator.
4. Agree a secure transfer route, retention period, deletion procedure, and
   incident contact before inviting file delivery.
5. Replace sender placeholders and verify every recipient address against a
   current institutional or publisher source.
6. Keep dates, responses, grants, and retention deadlines in a restricted
   administrative log. Do not add correspondence or participant data to Git.

## Work that can proceed while replies are pending

### Stream A - finish the outcome-blind PCQ-1.6 execution shell

Implement and test the locked evaluator only with arbitrary synthetic fixtures.
The shell should consume PCQ-1.4 normalized records, calculate only the frozen
PCQ-1.3 endpoints and uncertainty rules, and emit the declared states
`qualified`, `not-qualified`, `inconclusive-insufficient-precision`,
`blocked-access-or-rights`, `blocked-observation-model`, or
`out-of-scope-stress-test`. Tests must exercise complete, partial, missing,
blocked, and failed cases without encoding a real study result.

### Stream B - prepare source-neutral reporting and provenance

Prepare a machine-readable result schema, a human report template, checksum
and software-version capture, and a decision log that retains every eligible,
partial, negative, and blocked result. It must be possible to generate a
legitimate `blocked-access-or-rights` report before data arrive. Do not add a
source-specific converter until a provider supplies its actual data dictionary.

### Stream C - complete data-governance dry runs

Exercise the PCQ-1.4 manifest, quarantine-path, privacy, rights, overlap, and
secure-deletion workflow using synthetic files only. Decide who may approve a
manifest and who may run the evaluation; these should not silently collapse
into an unchecked single-person action.

### Stream D - resolve the independent transit observation model

Use sources independent of Lassen's participant outcomes to define or reject
the mapping between pulmonary-trunk-to-left-atrium transit and MEHLISSA's
capillary-only residence time. Pre-capillary, post-capillary, mixing, and
tracer-retention terms must be fixed or bounded prospectively. If they cannot
be defended, PCQ-C2 remains `blocked-observation-model`; a positive data-access
reply does not override that state.

### Stream E - prepare response triage without changing the protocol

| Reply | Action |
|---|---|
| positive, with compatible rights | complete the exact manifest and DUA, obtain independent approval, receive via the secure route, verify the checksum, then normalize and execute without refitting |
| positive, but fields or sample are incomplete | ingest only authorized compatible fields and retain the predeclared pilot, inconclusive, partial, or blocked state |
| records unavailable or rights incompatible | record the access outcome, keep the source blocked, and activate a backup only through a prospectively reviewed successor amendment |
| no reply | send one concise follow-up after an institutionally chosen interval, then record the unresolved access state; do not relax endpoints or numeric gates |
| unexpected new data source offered | inspect metadata and rights first; create and independently review a successor amendment before opening participant outcomes |

## Recommended next implementation increment

The best use of the waiting period is the source-neutral PCQ-1.6 execution and
reporting shell in Streams A and B. It advances the project even if no request
succeeds, because a reproducible blocked or inconclusive outcome is itself part
of the predeclared scientific workflow. The real source adapters, evaluation,
and physiological conclusions remain deferred until authorized records and
their actual data dictionaries exist.

## Contact-source verification

- D'Souza corresponding-author address:
  <https://doi.org/10.1152/ajplung.00358.2024>
- Arizona corresponding-author address and data-availability statement:
  <https://pmc.ncbi.nlm.nih.gov/articles/PMC10757516/>
- Bailey electronic address:
  <https://pubmed.ncbi.nlm.nih.gov/30545518/>
- Lassen reprint address:
  <https://doi.org/10.1007/s12350-023-03308-1>

These links identify the publication contacts; they do not establish current
employment, permission, ethics approval, data availability, or reuse rights.
