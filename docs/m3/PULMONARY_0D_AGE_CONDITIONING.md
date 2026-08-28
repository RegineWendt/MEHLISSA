<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary 0D Age Conditioning

## Status

M3.12 adds an age-conditioned successor to the bounded flow-adaptive pulmonary
0D model. The implementation and population-level evaluation are complete.
The result is a qualified improvement, not a complete validation pass.

## Scientific separation

The evidence roles are deliberately disjoint:

- Claessen et al. (2015) remains the source for bounded flow adaptation.
- Kane et al. (2016) supplies the new age calibration.
- Kovacs et al. (2009) and Wolsk et al. (2017) remain validation-only and are
  not used to choose any model parameter.

Kane et al. report 467 adults aged 18–85 without cardiopulmonary disease and
with normal exercise capacity in the abstract and Table 1. The results text
states 469, and the five Table 2 stratum counts also sum to 469; the derivation
below therefore uses the published stratum counts and records this source
inconsistency. The study reports non-invasive Doppler pulmonary
artery systolic pressure (PASP), estimated cardiac output (CO), and PASP/CO by
age group. It post-dates the literature-search window reported by Kovacs and is
separate from the Wolsk cohort. The source is therefore suitable for a first
independent age calibration.

Primary source: Kane GC et al., *Impact of age on pulmonary artery systolic
pressures at rest and with exercise*, Echo Research and Practice 2016;
3:53–61, [doi:10.1530/ERP-16-0006](https://doi.org/10.1530/ERP-16-0006),
CC-BY-NC-4.0.

## Locked derivation

The source's resting PASP/CO ratios are used only as relative total-load
proxies. The 40–59-year strata form the reference:

```text
reference = (93 × 4.0 + 127 × 4.2) / (93 + 127)
          = 4.115454545

young multiplier = 3.8 / reference
                 = 0.923348796

older mean = (127 × 4.5 + 84 × 4.8) / (127 + 84)
           = 4.619431280

older multiplier = older mean / reference
                 = 1.122459555
```

The executable bands are:

| Represented age | PVR multiplier |
|---|---:|
| 18 to under 40 years | 0.923348796 |
| 40 to under 60 years | 1.0 |
| 60 to 85 years | 1.122459555 |

The default age is 55 years, the midpoint of the 50–59 reference band. A
scenario or validation series may replace it with its represented age. Values
below 18 or above 85 are rejected instead of extrapolated.

The multiplier is applied after the existing flow-dependent PVR multiplier:

```text
effective PVR = baseline PVR × flow multiplier × age multiplier
```

Compliance, PAWP/left-atrial pressure, transit time, and right/left perfusion
are unchanged. Kane does not provide invasive evidence that would justify age
rules for those quantities.

## Executable artifacts

- schema: `data/schemas/lung-model-definition/1.3.0.schema.json`;
- model definition:
  `data/lung-models/healthy-adult-rest-exercise-age-0d-v3.json`;
- independent population case:
  `data/validation/pulmonary-zero-dimensional/healthy-population-multipoint-v2.json`;
- validation schema:
  `data/schemas/pulmonary-zero-dimensional-population-multipoint-validation/1.1.0.schema.json`.

The population schema now permits an optional `representative_age_years` for a
series. The Wolsk strata use predeclared midpoints of 29.5, 49.5, and 70 years.
The pooled Kovacs series has no age stratum and therefore uses the v3 reference
age. The evaluation report records both the represented age and the applied
multiplier.

## No-refit result

The same M3.11 observations and acceptance rules were evaluated without
changing a validation value or fitting to a validation result:

| Independent series | v2 agreement | v3 agreement |
|---|---:|---:|
| Kovacs pooled upright | 3/3 | 3/3 |
| Wolsk age 20–39 | 0/5 | 1/5 |
| Wolsk age 40–59 | 5/5 | 5/5 |
| Wolsk age 60–80 | 2/5 | 5/5 |
| **Total** | **10/18** | **14/18** |

The independently derived age factor resolves the older-stratum direction of
bias and leaves the reference stratum unchanged. It is too small to explain
most of the young-stratum discrepancy. That failure remains visible and is an
important scientific result: age-dependent resistance alone is not a complete
description of the observed pressure-flow differences.

## Interpretation limits

- Kane uses Doppler PASP and estimated CO, not invasive mPAP, PAWP, or PVR.
- PASP/CO is a proxy for a relative age effect, not a direct PVR measurement.
- Three stepwise bands do not represent continuous ageing or within-band
  variation.
- Source uncertainty and covariance are not propagated into the multipliers.
- The publication's 467-versus-469 participant-count inconsistency cannot be
  resolved from the article; weighting follows the counts printed in Table 2.
- The model does not add age-dependent compliance, ventricular, valve,
  pulsatile, regional, sex-specific, or patient-specific physiology.
- Four remaining validation-stage failures prohibit describing v3 as fully
  validated.

## Next scientific step

The next age-related refinement must explain the young high-flow discrepancy
using evidence not present in Kane and not fitted to Wolsk. Candidate mechanisms
include age-dependent flow adaptation, posture/protocol interaction, body-size
conditioning beyond a single stratum mean BSA, and a more explicit pressure–
flow/compliance representation. Any successor must preserve v2 and v3 as frozen
comparators and repeat the same validation case.
