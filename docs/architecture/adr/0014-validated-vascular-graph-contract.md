<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0014: Validated Vascular Graph Contract

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M2; `BODY-001`, `BODY-002`, `BODY-005`, `BODY-006`, `DATA-001`

## Context

The legacy circulation derives edges from exact coordinate equality and adds
diameter, velocity, and missing transitions implicitly in code. Data,
assumptions, and algorithms are therefore inseparable. Invalid or incomplete
networks also become visible only during simulation.

The new body layer needs an independent data contract that loads alternative
models without rebuilding and detects physical inconsistencies before
simulation. It belongs in `models/body`, not in the scenario-neutral kernel.

## Decision

1. Vascular models use JSON Schema `vascular-graph/1.0.0` and explicit string IDs that need not be contiguous.
2. Version 1.0.0 describes closed circulations only. Every graph must form a single strongly connected component and every segment must have at least one explicit successor.
3. Coordinates, length, diameter, cross-sectional area, volume, flow, and mean velocity are required fields with SI units fixed in their names.
4. In addition to the schema, the semantic validator checks:
   - unique segment, successor, and source IDs;
   - geometric continuity of every edge;
   - Euclidean length, circular cross section, and cylindrical volume;
   - `flow = cross-sectional area × mean velocity`;
   - flow conservation at every geometric branch and merge;
   - transitions normalized to one and agreement of their fractions with successor flows.
5. Model validity, source citation and license, evidence quality, and relative uncertainty are data fields rather than free-form accompanying notes.
6. Controlled domain-data errors use `MEHLISSA-E2005`.
7. The first reference graph is deliberately synthetic and CC BY 4.0. It contains four segments, a branch, a merge, and non-contiguous IDs, but makes no physiological claim.
8. The 1995 legacy data set is not copied into the Next data area because its chain of rights is unresolved. Its migration is also blocked on domain grounds until, in particular, the missing transition from vessel 9 and robust flow assumptions are documented.

## Consequences

Positive:

- A model can no longer gain an edge merely through accidentally matching coordinates.
- Unit and flow errors are rejected before an experiment begins.
- Sources and assumptions travel with the model and can later be included in provenance and model cards.
- Synthetic tests remain legally and scientifically distinct from historical data.

Negative and limitations:

- The strict contract requires parameters absent from the legacy data set; an apparently quick one-to-one conversion is therefore impossible.
- A circular cross section and straight cylindrical segment are M2 abstractions. Later 1D/CFD models need their own geometry and flow contracts.
- Exact mass conservation is appropriate for stationary reference parameter sets; time-dependent compliance and storage require a new model version.
- Relative uncertainty is initially documented but not yet propagated.

## Alternatives

- **CSV plus implicit rules:** rejected because schema, sources, and topology could not be validated together.
- **Continue deriving edges from coordinates:** rejected because rounding and unintended point equality can change topology.
- **Make missing flows optional:** rejected for the stationary M2 reference contract because transitions and mass conservation would then be unverifiable.
- **Fill legacy values with defaults:** rejected because invented physiology could appear to be measured or literature data.
