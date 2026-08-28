<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary 0D Age-Conditioned Distensibility

## Purpose

M3.15 tests the smallest evidence-supported response to the older-stratum
failure exposed by v5: retain the Linehan pressure–flow structure and all
qualified resistance levels, but use a lower pulmonary vascular
distensibility coefficient for adults aged 60 years and older.

The Wolsk validation data were not used to select the boundary or either
coefficient. The v5 definition and result remain immutable.

## Independent calibration

Reeves et al. reanalysed invasive multipoint pressure, wedge-pressure, and
flow measurements from healthy subjects with the Linehan model. In the
Karolinska data summarized in their Table 2:

- two young groups, both with a mean age of 22 years, had `alpha` values of
  `0.021 ± 0.002` and `0.020 ± 0.002 mmHg^-1`; and
- 14 older subjects aged 61–83 years, with mean age 71 years, had
  `alpha = 0.015 ± 0.001 mmHg^-1`.

The reported uncertainty is standard error. The older value differed from the
younger Karolinska subjects at `p < 0.05`. The article describes the aging
signal specifically in older men, so sex-general use is an explicit
extrapolation.

V6 therefore applies:

```text
age < 60 years: alpha = 0.020 mmHg^-1
age >= 60 years: alpha = 0.015 mmHg^-1
```

The younger value preserves v5. The 60-year boundary is pre-existing in the
age-conditioning contract and is consistent with the source's older group
starting at 61 years. No continuous slope is inferred from two aggregate age
levels.

## Executable contract

Schema `lung-model-definition/1.5.0` adds an optional
`pressure_distensibility.older_coefficient`. It also adds `standard_error` to
the uncertainty vocabulary so the Reeves statistics are not mislabeled as
reference intervals.

An older coefficient is valid only when an age-conditioning block supplies
the older-age boundary. Runtime and model-card evidence must agree on its
presence and SI value. Empirical flow adaptation remains mutually exclusive
with pressure distensibility.

For either age band, MEHLISSA derives a matching zero-pressure resistance `R0`
from the selected `alpha`, reference PVR, reference flow, and reference LAP.
This preserves the age-specific resting equilibrium instead of allowing the
new coefficient to alter an already qualified resting reference.

## Frozen comparison

`healthy-pressure-distensible-population-v2.json` copies the 15 Wolsk stages
and acceptance protocol used for v5 and changes only the model binding to v6.
Parameter fitting remains forbidden.

| Wolsk age stratum | v5 stages | v6 stages | v5 RMSE | v6 RMSE |
|---|---:|---:|---:|---:|
| 20–39 years | 5/5 | 5/5 | 1.284 mmHg | unchanged |
| 40–59 years | 5/5 | 5/5 | — | unchanged |
| 60–80 years | 1/5 | 1/5 | 5.411 mmHg | 4.603 mmHg |
| **Total** | **11/15** | **11/15** | — | — |

The independently calibrated change moves older predictions in the expected
direction and reduces older-stratum RMSE by about 15%, but it does not bring
the four failing stages into their population-mean confidence intervals.
Consequently, v6 is an informative sensitivity-qualified structural candidate,
not a replacement for the empirical v4 reference.

## Interpretation

The result rejects the simple hypothesis that the v5 failure is explained by
an all-age `alpha` alone. Plausible remaining mechanisms include:

- a continuous interaction between age, resistance level, and
  distensibility rather than independent step functions;
- sex-specific distensibility, which the current scenario does not represent;
- regional recruitment and parallel vascular beds;
- activity-dependent left-atrial or closing-pressure behaviour not captured by
  a single mean-pressure bed; and
- differences between aggregate cohorts, posture, or measurement methods.

None of these mechanisms should be fitted to Wolsk. A successor requires an
independent calibration source and must be evaluated against the same frozen
records.

## Limitations

- The older calibration is directly supported in older men only.
- The rule is discontinuous and contains no training or fitness dimension.
- V6 remains a mean-pressure 0D model with an effective-RC transient
  approximation.
- It has no lobar anatomy, pulsatile waves, ventilation, gas exchange,
  disease, or patient-specific parameters.
- A population-mean confidence interval is not an individual tolerance range.

## Executable evidence

- model definition: `data/lung-models/healthy-adult-pressure-distensible-age-0d-v6.json`;
- schema: `data/schemas/lung-model-definition/1.5.0.schema.json`;
- frozen comparison: `data/validation/pulmonary-zero-dimensional/healthy-pressure-distensible-population-v2.json`;
- loader, evidence-binding, age-boundary, equilibrium, inverse-flow, validation,
  and backward-compatibility tests in the C++ suite.

## Primary source

- Reeves JT, Linehan JH, Stenmark KR. *Distensibility of the normal human lung
  circulation during exercise.* American Journal of Physiology – Lung Cellular
  and Molecular Physiology, 2005.
  <https://doi.org/10.1152/ajplung.00162.2004>
