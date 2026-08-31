<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0037: Stochastic receptor binding and population classification

- Status: Accepted
- Date: 2026-08-31

## Context

M5.1 and M5.3 describe receptor occupancy as a continuous deterministic
fraction. They cannot represent reaction noise when a cell has finitely many
receptors, cell-to-cell outcome distributions, or threshold classification
errors. M5.4 needs these effects without weakening reproducibility or claiming
clinical diagnostic performance.

## Decision

Use the Gillespie direct stochastic simulation algorithm for the same reversible
one-to-one reaction as M5.1: binding propensity is `kon L (N-B)` and
dissociation propensity is `koff B`. `N` is a fixed integer receptor count,
`B` is the bound count, and `L` is a prescribed piecewise-constant external
reservoir. Every cell receives a caller-owned `RandomStream` whose name is
derived from master seed, experiment prefix, cohort, and cell index.

An upward threshold crossing is retained as the cell's detection result even if
later dissociation reduces occupancy. A population experiment contains equally
sized declared signal-positive and signal-negative cohorts. It reports bounded
distribution summaries and a complete confusion matrix. FP and FN describe
this synthetic threshold experiment only.

The checked population is compared with independent analytical finite-time
binomial means and variances. Its statistical tolerances and maximum FP/FN rates
are data, declared before execution. Exact replay is also tested.

## Consequences

- Integer bounds and receptor-count conservation hold after every reaction.
- Named per-cell streams make a cohort reproducible and independent of execution
  order, enabling future parallel evaluation.
- The model exposes reaction noise, population distributions, FP, and FN without
  inventing biological heterogeneity.
- Runtime scales with the number of reaction events; an explicit event budget
  and bounded trace retention prevent unbounded runs and output.
- The result is software verification, not biological or diagnostic validation.

## Alternatives considered

- Randomizing a deterministic occupancy was rejected because it would not model
  the reaction path or threshold-crossing time.
- Tau leaping was deferred because the small checked system does not need its
  approximation and the exact SSA gives a clearer reference.
- One shared population stream was rejected because results would depend on
  cell scheduling and obstruct safe parallelism.

## Affected requirements and gates

This advances `CELL-002` and provides an SSA/distribution implementation toward
`CELL-004`. It implements Roadmap increment M5.4 and establishes documented
synthetic single-cell and population scopes. Biological qualification and the
intracellular-network part of the M5 gate remain open.
