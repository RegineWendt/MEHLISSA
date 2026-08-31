<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Stochastic Receptor Binding and Population Classification (M5.4)

## What this increment adds

M5.4 represents each receptor as free or bound and simulates individual binding
and dissociation events with the exact Gillespie direct SSA. It uses the same
mass-action chemistry as M5.1/M5.3, accepts the same kind of prescribed
piecewise-constant ligand history as M5.3, and adds:

- finite receptor counts and integer count conservation;
- reproducible, explicitly named random streams owned by the caller;
- bounded single-cell event traces, final/peak occupancy, and first detection;
- independently named streams for every cell in two population cohorts;
- mean, population variance, extrema, 5th/50th/95th quantiles, events, and
  random-draw totals; and
- TP, FN, FP, TN, sensitivity, specificity, FNR, and FPR.

## Checked synthetic experiment

The strict profile is
`examples/cell-models/synthetic-stochastic-receptor-ligand-v1.json`; its schema
is `data/schemas/stochastic-receptor-ligand-profile/1.0.0.schema.json`.

Each of 2,000 cells per cohort starts with zero of 40 receptors bound and is
observed for ten seconds. Both cohorts use `kon = 1000 m3/(mol s)`,
`koff = 0.1 s-1`, and a 0.5 ever-crossed detection threshold. The positive
reservoir is `0.0003 mol/m3`; the negative reservoir is `0.00005 mol/m3`.

For independent identical receptors, the exact finite-time bound probability is
the M5.1 occupancy solution and the bound fraction has binomial variance
`p(1-p)/40`. The declared references are:

| Cohort | exact mean | exact fraction variance | seeded SSA mean | seeded SSA variance |
|---|---:|---:|---:|---:|
| positive | 0.7362632708 | 0.0048544917 | 0.734963 | 0.00483544 |
| negative | 0.2589566133 | 0.0047974521 | 0.258312 | 0.00446934 |

The fixed seed and names produce 0 false negatives and 4 false positives, so
the observed FNR is 0 and FPR is 0.002. The predeclared gates allow absolute
mean error 0.01, variance error 0.001, and at most 0.02 for either error rate.
These gates verify implementation behavior; they are not inferred clinical
performance targets.

## Reproducibility and boundaries

Each stream name is `m5.4.synthetic.<cohort>.<cell-index>`. Replaying the same
seed and name reproduces the exact trajectory and population report. Changing a
name creates a separate deterministic stream. A concentration-boundary waiting
time is discarded and resampled under the new propensity; no reaction choice is
drawn for a non-occurring event.

The ligand remains a homogeneous non-depleting reservoir. All cells are
otherwise identical: there is no receptor-expression distribution, biological
population structure, intracellular network, spatial transport, competition,
turnover, calibrated threshold, or diagnostic interpretation. M5.5 must turn a
detection into an executable intracellular state rather than extending this
surface-binding model beyond its scope.

## Verification

The focused tests cover strict loading, exact replay, count conservation,
zero-signal behavior, analytical population moments, FP/FN accounting,
malformed inputs, event-budget exhaustion, and duplicate provenance:

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure `
  -R "stochastic|SSA|population|classification"
```

See [ADR-0037](../architecture/adr/0037-stochastic-receptor-binding-and-population-classification.md)
for the binding decision and [M5 implementation evidence](README.md) for the
remaining gate work.
