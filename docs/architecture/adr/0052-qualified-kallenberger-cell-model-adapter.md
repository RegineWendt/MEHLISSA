<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# ADR-0052: Qualified Kallenberger cell-model adapter

- **Status:** Accepted
- **Date:** 2026-09-06
- **Applies to:** BCQ-1.4–1.7; `CELL-002`, `CELL-004`, and `CELL-005`

## Context

M5 previously provided only configurable synthetic receptor, intracellular,
delivery, and apoptosis fixtures. BCQ-1.1–1.3 selected and independently
reproduced a compact public CD95L–CD95–caspase-8 family, but importing SBML or
COPASI into the runtime would add a large optional dependency and could obscure
unresolved source units and no-refit boundaries.

## Decision

Implement the selected 13-reaction mechanism directly in the cell library.
Keep `KallenbergerMinimalMechanism`, which owns the source equations, separate
from `QualifiedCd95ApoptosisAdapter`, which owns typed M5 mapping and locks the
two source identities, stimulus, state order, observables, and
`unresolved-model-native` semantics. Use bounded, deterministically subdivided
classical RK4. Keep parameter variation behind the explicit qualification
runner and reject it at the public no-refit adapter.

Qualify all 18 states against the previously frozen COPASI archive. Treat the
larger 525/526 models as same-publication structure sensitivity only. Do not
invent a population distribution when joint initial-value data are unavailable.

## Consequences

- MEHLISSA has one published-mechanism average-cell variant with independent
  cross-engine numerical evidence and no runtime COPASI dependency.
- Unit ambiguity remains visible in the type and output metadata.
- The source equations can be reviewed independently from adapter semantics.
- The result is computational qualification only; publication, population,
  external-review, biological, endothelial, patient, and clinical claims stay
  blocked.
- A future SBML engine or population variant requires a new versioned protocol
  and cannot silently change this adapter.

## Alternatives considered

- **Bundle and execute SBML at runtime:** rejected for the first variant because
  it adds dependency and version complexity without improving the frozen claim.
- **Interpolate the COPASI CSV:** rejected because that would test a reference
  table rather than independently implement the equations.
- **Reuse the synthetic M5 network:** rejected because its two-stage topology is
  not the selected 13-reaction mechanism.
- **Invent independent protein distributions:** rejected because it would create
  unsupported population evidence.
