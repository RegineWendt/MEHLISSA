<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0049: Scenario-Owned Fingerprinting Composition

- Status: Accepted
- Date: 2026-09-01
- Decision owners: MEHLISSA Next maintainers

## Context

M2-M6 provide independently tested model libraries and explicit couplers, but
no single declaration selects the complete fingerprinting stack. Adding FP9
branches to the kernel or individual layers would violate layer independence.
Putting unversioned file choices directly in a runner would make archived runs
ambiguous. Conversely, calling a collection of loaded profiles an end-to-end
simulation would overstate what is executed.

## Decision

The first vertical slice will live in an independent
`MEHLISSA::fingerprinting_scenario` package above all M2-M6 libraries. A strict,
versioned scenario profile owns scenario/run/target identity, master seed,
collector cohort, artifact definition/schema references, causal stage order,
sources, limitations, and acceptance flags.

M7.1 composition validates every selected artifact against its own schema and
checks the FP9 timer through its typed API. It returns a typed immutable plan.
It does not instantiate or advance the non-timer models; that responsibility
belongs to the M7.2 scenario coordinator. Model libraries remain reusable and
contain no FP9-specific branch.

## Consequences

- Every layer selection is explicit, versioned, reviewable, and reproducible.
- Existing model schemas can validate a complete candidate stack before
  runtime wiring begins.
- Scenario-specific topology remains data under a general M6 schema.
- M7.2 gains one stable identity and stage-order contract.
- The scenario package currently requires all M2-M6 build options.
- Schema validity alone does not prove cross-layer runtime compatibility or
  biological validity; those claims remain open and documented.

## Alternatives considered

- **Add fingerprint logic to the kernel:** rejected because it would couple a
  neutral runtime to one medical scenario.
- **Put orchestration into one model layer:** rejected because no biological or
  communication layer owns the complete workflow.
- **Use only the generic M1 experiment manifest:** deferred because its model
  strings do not resolve typed M2-M6 artifacts or the fingerprint stage
  contract.
- **Instantiate every model in M7.1:** rejected as too large a change to review
  before the selection and identity contract exists.

## Affected requirements and gates

- `ARC-001`, `ARC-002`, `ARC-007`
- `SCN-001`
- `QUA-003`, `QUA-005`
- Gate M7
