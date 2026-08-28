<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary 0D Pressure Distensibility

## Purpose

M3.14 introduces an immutable v5 structural candidate for healthy pulmonary
pressure–flow behaviour. It asks whether a pressure-distensible vascular bed
can replace the empirical flow-dependent resistance law in v4 while preserving
the already qualified resting reference and age bands.

This is a model-comparison increment, not a claim that v5 supersedes v4. The
frozen population result is retained even where v5 performs worse.

## Structural equation

The implementation follows the Linehan distensible-vessel relationship:

```text
mPAP = (((1 + alpha * LAP)^5 + 5 * alpha * R0 * Q)^(1/5) - 1) / alpha
```

where `Q` is cardiac output, `LAP` is left-atrial pressure, `R0` is the
zero-pressure resistance, and `alpha` is the fractional diameter change per
unit pressure. All executable quantities are stored in SI units. The v5 card
uses the healthy-population mean `alpha = 0.020 mmHg^-1`, equivalent to
`0.000150012315169131 Pa^-1`, reported by Reeves et al.

Linehan et al. provide the governing equation and interpretation of `R0` and
`alpha`. Reeves et al. tested the formulation in 59 healthy sea-level subjects
and reported pressure predictions close to measured mean pulmonary-arterial
pressure. These sources support the structure and the fixed healthy reference
coefficient; they are not reused as the Wolsk validation cohort.

## Reference-state normalization

The existing PVR value is a measured secant resistance at the model card's
reference state, not `R0`. At construction, MEHLISSA solves the Linehan
equation inversely for `R0` using the card's locked reference cardiac output,
reference LAP, age multiplier, and reference PVR. Consequently, activating
pressure distensibility does not move the qualified resting equilibrium.

The runtime exposes both the effective secant PVR and the derived
zero-pressure resistance as diagnostics. It uses the inverse equation for
outflow, so pressure and flow remain analytically consistent.

## Model-contract decision

Schema `lung-model-definition/1.4.0` adds an optional
`hemodynamics.pressure_distensibility` block containing:

- reference cardiac output;
- reference left-atrial pressure; and
- the pressure-distensibility coefficient.

Pressure distensibility and the v2–v4 empirical `flow_adaptation` block are
mutually exclusive in both JSON Schema and C++ validation. Combining them
would apply two alternative pressure–flow corrections and double-count
vascular recruitment/distension.

The v5 definition retains v4's independently calibrated age resistance bands,
but replaces its empirical flow exponent with the Linehan structure. Versions
v1 through v4 and their validation records remain immutable.

## Frozen independent comparison

`healthy-pressure-distensible-population-v1.json` repeats the same 15 Wolsk
age-stratified stages used for v4. The v5 parameter and acceptance protocol
were locked before repository evaluation, and parameter fitting is forbidden.

| Wolsk age stratum | Agreeing stages | Pressure RMSE |
|---|---:|---:|
| 20–39 years | 5/5 | 1.284 mmHg |
| 40–59 years | 5/5 | not used for fitting |
| 60–80 years | 1/5 | 5.411 mmHg |
| **Total** | **11/15** | — |

The result is scientifically useful but does not pass the same population
comparison that v4 passes at 15/15. A single all-age healthy coefficient is
adequate for the young and middle aggregate series but under-represents the
older pressure rise. Reeves et al. also reported a tendency toward lower
distensibility in older men. MEHLISSA does not infer or fit an age law from the
Wolsk validation failures.

## Interpretation and limitations

- v4 remains the current empirical population-reference candidate.
- v5 is the immutable fixed-distensibility baseline for investigating
  pressure-dependent pulmonary haemodynamics; v6 is its age-conditioned
  sensitivity successor.
- The fixed coefficient is a population mean, not a patient parameter.
- The model remains a mean-pressure 0D bed. It has no explicit arteries,
  veins, lobes, pulsatile wave propagation, recruitment threshold, ventilation,
  or gas exchange.
- Equilibrium pressure and diagnostic outflow use the nonlinear equation;
  transient relaxation still uses the existing effective-RC approximation.
- Age conditioning currently changes the resting resistance level, not
  `alpha`; the older-stratum result shows that these mechanisms are not
  interchangeable.
- Published aggregate confidence intervals are not individual physiological
  tolerance ranges.

M3.15 follows the smaller of these paths with an independently sourced older
coefficient; see
[Pulmonary 0D Age-Conditioned Distensibility](PULMONARY_0D_AGE_DISTENSIBILITY.md).
It improves older RMSE but leaves stage agreement unchanged, so parallel
anatomical pulmonary beds remain the stronger subsequent structural question.

## Executable evidence

- model definition: `data/lung-models/healthy-adult-pressure-distensible-age-0d-v5.json`;
- schema: `data/schemas/lung-model-definition/1.4.0.schema.json`;
- frozen comparison: `data/validation/pulmonary-zero-dimensional/healthy-pressure-distensible-population-v1.json`;
- quantity, loader, model-equation, inverse-flow, mutual-exclusion, and
  population-validation tests in the C++ test suite.

## Primary sources

- Linehan JH et al. *A simple distensible vessel model for interpreting
  pulmonary vascular pressure-flow curves.* Journal of Applied Physiology,
  1992. <https://doi.org/10.1152/jappl.1992.73.3.987>
- Reeves JT et al. *Pulmonary artery pressure-flow relationships during
  exercise in lowlanders and highlanders.* American Journal of Physiology –
  Lung Cellular and Molecular Physiology, 2005.
  <https://doi.org/10.1152/ajplung.00162.2004>
