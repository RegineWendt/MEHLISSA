<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BCQ-1.2-A1 Prospective Replay Amendment

## Decision

Protocol version 1.0 remains immutable and its completed run
`20260906T070728Z-7ea0d2273f31` remains a failed run. The model
`BIOMD0000000523` differed between two independent COPASI process launches by
at most `2.637534635141492e-11` in absolute value and
`2.0191528122055834e-13` after normalization by `max(1, abs(replay))`.
`BIOMD0000000524` was bit-identical. Because version 1.0 required exact zero,
the deterministic-replay gate failed even though the observed difference was
far below the solver tolerance.

The machine-readable authority for the prospective amendment is
`data/qualification/biological-cell-model-reproduction-protocol-v1.1.json`.
It references the immutable version 1.0 protocol by path and SHA-256. This
amendment must be committed before any version 1.1 result is generated.

## Amended replay rule

For every parsed time and state value:

```text
abs(primary - replay) <= 1e-12 + 1e-12 * max(1, abs(replay))
```

The relative term is one thousand times tighter than the frozen primary LSODA
relative tolerance of `1e-9`. The result records the maximum absolute
difference, maximum scale-normalized difference, and maximum fraction of the
allowed difference. A result may be called *bit-identical* only when both
reported differences are exactly zero; otherwise the permitted term is
*numerically equivalent*.

## Execution safeguard

Each COPASI process receives a 120-second wall-clock timeout. A timeout fails
the attempt and leaves its partial archive intact. This responds to one stalled
setup attempt and does not change the equations or numerical method.

## Unchanged decisions

The amendment changes no source, case, hash, model number, initial value,
parameter, unit interpretation, solver or binary identity, LSODA setting,
time grid, observable, invariant, convergence rule, negative control, no-refit
rule, claim boundary, or publication-alignment status. Publication alignment
remains blocked and M5 remains `software_test_surrogate`.

## Interpretation

This is a disclosed protocol amendment after a failed version 1.0 outcome, not
a retroactive pass or an outcome-hidden threshold change. A new version 1.1
run is required. If it passes, the supported statement is limited to
numerically equivalent external-solver reproduction of the two unchanged
average-cell artifacts; it is not publication-curve agreement or biological,
organ-level, patient-level, or clinical validation.
