<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0041: Cohort-compressed apoptosis population

- Status: Accepted
- Date: 2026-08-31

## Context

M5.4 evaluates every stochastic cell explicitly. This is the correct contract
when receptor-count noise and a distribution of individual outcomes are the
question, but its work grows with represented cell count. M5.7 adds a
deterministic apoptosis response for one homogeneous intracellular inventory.
Gate M5 also requires a documented population variant and validity boundary.
Repeating the deterministic calculation for billions of identical cells would
add cost without information.

## Decision

Add a second, explicitly different population contract. A request contains a
bounded list of cohorts. Each cohort declares one intracellular amount and the
number of cells represented by that amount. The M5.7 Hill rule is evaluated
once per cohort; cell counts weight the mean effect and the viable/committed
totals. Cohort IDs must be unique, counts must be positive, and total-count
overflow is rejected.

The response always reports exact aggregates over every input cohort but
retains no more than the caller-declared number of cohort details. Therefore
model work is `O(number of cohorts)` and retained output is explicitly bounded;
neither depends on represented cell count. The strict profile includes a
one-trillion-cell/four-cohort reference and predeclared response-parameter
sensitivity cases.

Keep the M5.4 explicit-cell SSA model. The compressed model is an additional
resolution, not a replacement. It is exact only for cells that genuinely share
the same prescribed amount and response parameters. A continuous distribution
must be discretized deliberately, and the resulting approximation error belongs
to the experiment.

## Consequences

- Large deterministic populations can be represented without allocating or
  simulating one object per cell.
- Aggregate counts, fractions, and mean effect remain inspectable even when
  detailed cohort output is truncated.
- M5.4 remains the appropriate choice for intrinsic receptor noise and
  cell-level distributions; M5.8 is appropriate for weighted deterministic
  strata.
- The model does not generate heterogeneity, correlations, lineage, spatial
  structure, turnover, or interactions.
- Population size does not increase biological evidence: the entire reference
  remains a synthetic software-test surrogate.

## Alternatives considered

- Repeating M5.7 for every represented cell was rejected because it scales with
  cell count while returning identical values within a homogeneous stratum.
- Replacing M5.4 was rejected because cohort compression cannot reproduce
  intrinsic SSA noise or individual event histories.
- Accepting an arbitrary continuous distribution was deferred because it would
  require a quadrature/sampling contract and an explicit approximation budget.
- Emitting one higher-layer event per committed cell was rejected because it
  would defeat bounded output; a later scenario may consume the aggregate or
  deliberately expand a small selected cohort.

## Affected requirements and gates

This implements M5.8 population scaling and the population half of the fourth
Gate M5 statement. Together with the M5.4 explicit-cell population, the M5.7
single-cell response, the evidence qualification, and the User Guide review,
it supports formal closure of Gate M5 at the synthetic software-contract level.
It advances `CELL-005` but does not provide biological apoptosis validation.
