<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M5.8 Population Scale and Validity

## Purpose

M5.8 closes the population-scaling gap without obscuring the difference between
an explicit stochastic cell and a compressed deterministic population. It adds
a cohort-weighted variant of the M5.7 apoptosis rule, predeclared sensitivity
checks, bounded output, and explicit model-selection guidance.

## Contract

The population request contains:

- stable request and observation-time identity;
- a finite list of uniquely identified amount cohorts;
- one positive 64-bit cell count and one nonnegative intracellular amount per
  cohort; and
- a positive maximum number of cohort details that may be retained.

The model evaluates the numerically stable M5.7 Hill function once per cohort.
It returns total, viable, and apoptosis-committed cell counts, committed
fraction, cell-weighted mean effect, evaluated and omitted cohort counts, and a
bounded prefix of cohort details. Count overflow and ambiguous or nonphysical
cohorts are rejected before evaluation.

The model is computationally scalable because work is proportional to input
cohort count, not represented cell count. It is scientifically scalable only
when cohort homogeneity is a defensible experiment assumption.

## Checked reference

The strict profile is:

```text
examples/cell-models/synthetic-apoptosis-population-v1.json
data/schemas/apoptosis-population-profile/1.0.0.schema.json
```

Four cohorts represent one trillion synthetic cells:

| Cohort | Cells | Intracellular amount | Hill effect | State |
|---|---:|---:|---:|---|
| unexposed | `200,000,000,000` | `0 nmol` | `0` | viable |
| low | `300,000,000,000` | `25 nmol` | `0.2` | viable |
| half-max | `250,000,000,000` | `50 nmol` | `0.5` | viable |
| high | `250,000,000,000` | `100 nmol` | `0.8` | apoptosis committed |

At the declared `0.65` threshold, the exact aggregate is 750 billion viable
and 250 billion committed cells: committed fraction `0.25` and weighted mean
effect `0.385`. Only two cohort details are retained, while all four cohorts
contribute to the aggregates. A second test scales the same weights down to
1,000 cells and obtains identical fractions and mean effect.

## Predeclared sensitivity

The profile changes one response assumption at a time while preserving the
four prescribed cohorts:

| Case | Half-max amount | Threshold | Committed fraction | Mean effect |
|---|---:|---:|---:|---:|
| baseline | `50 nmol` | `0.65` | `0.25` | `0.385` |
| lower half-max | `25 nmol` | `0.65` | `0.50` | `0.5852941176470589` |
| higher half-max | `75 nmol` | `0.65` | `0.00` | `0.26692307692307693` |
| lower threshold | `50 nmol` | `0.50` | `0.50` | `0.385` |

These exact synthetic results show that the committed fraction is sensitive to
both the response scale and the discrete decision threshold, while the mean
continuous effect is independent of the threshold. They are not biological
uncertainty intervals or treatment-response estimates.

## Which population model to use

| Question | Appropriate M5 model | Validity scope |
|---|---|---|
| exact homogeneous single-cell binding | M5.1 analytical | constant non-depleting ligand reservoir |
| time-varying homogeneous single-cell binding | M5.3 RK4 | prescribed piecewise-constant reservoir |
| receptor-count noise and classification distribution | M5.4 explicit-cell SSA | finite receptors, named stream per simulated cell; cost grows with simulated cells |
| intracellular ODE/SSA variability | M5.5 network | generic conserved synthetic topology; explicit SSA population currently 1,000 cells |
| one final intracellular inventory and state | M5.7 apoptosis response | one deterministic homogeneous synthetic cell |
| very large weighted deterministic population | M5.8 compressed population | homogeneous cells within each prescribed amount cohort; cost grows with cohort count |

M5.8 must not be selected merely because a large number is desired. Use M5.4
or M5.5 when individual stochastic trajectories matter. Use M5.8 when the
research question is about exact aggregation over an already specified set of
homogeneous strata. Neither variant currently models a biological cell
population.

## Verification boundary

Automated tests cover strict schema and semantic validation, the analytical
four-cohort result, a one-trillion-to-one-thousand scaling invariant, all
predeclared sensitivities, bounded detail retention, duplicate IDs, invalid
limits, nonphysical inputs, reference tampering, and 64-bit count overflow.

The tests do not qualify drug, cell, pathway, dose response, threshold,
population distribution, or clinical outcome. See
[M5 Evidence Qualification](M5_EVIDENCE_QUALIFICATION.md) and
[M5 Gate Review](M5_GATE_REVIEW.md).
