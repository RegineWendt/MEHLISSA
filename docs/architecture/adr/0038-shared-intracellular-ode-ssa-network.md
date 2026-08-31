<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0038: Shared intracellular ODE/SSA network

- Status: Accepted
- Date: 2026-08-31

## Context

M5.1–M5.4 stop at receptor occupancy and threshold detection. M5 needs an
intracellular state change, while its gate also requires a reaction network to
be checked against an analytical or external reference. A solver comparison is
meaningful only if both implementations share one explicit topology, rates,
inputs, initial state, and output definition.

## Decision

Introduce a two-stage, well-mixed activation/deactivation network:

```text
receptor occupancy -> inactive/active messenger -> inactive/active effector
```

Receptor occupancy multiplies messenger activation; active-messenger fraction
multiplies effector activation. Each stage has an independent deactivation
reaction and a conserved total pool. Implement the same network as bounded RK4
fractions and as exact Gillespie SSA molecule counts. A piecewise-constant
receptor-occupancy trajectory is their common input, and first upward effector
threshold crossing is their common response event.

Use one named random stream per SSA cell. Compare a predeclared population mean
with the deterministic result and check the ODE against fixed reference values.
All topology and parameters remain a synthetic software-test surrogate.

## Consequences

- Receptor state now drives executable intracellular messenger and effector
  states instead of ending at detection.
- ODE and SSA differences isolate numerical and finite-count effects because
  their reaction semantics are shared.
- Both pools are bounded and conserved; traces and work have explicit limits.
- The response event can later drive M5.6 activation/release without coupling
  release logic into the network solver.
- The comparison is not biological validation and does not qualify a pathway.

## Alternatives considered

- A single activation state was rejected because it would not form a useful
  intracellular cascade or exercise propagated dynamics.
- Separate ODE and SSA profiles were rejected because silent topology drift
  would make their comparison ambiguous.
- A named biological pathway was deferred until kinetics, population, state,
  and assay endpoints can be qualified from suitable evidence.

## Affected requirements and gates

This advances `CELL-002` and `CELL-004`, implements Roadmap increment M5.5, and
provides the intracellular-network part of the third M5 gate statement at the
synthetic software-verification level. Biological/external validation remains
open.
