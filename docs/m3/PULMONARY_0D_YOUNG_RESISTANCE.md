<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary 0D Young-Adult Resistance Qualification

## Status

M3.13 investigates the four remaining v3 failures in the Wolsk 20–39-year
series and adds an immutable v4 candidate. The counted, calibration-disjoint
Wolsk comparison agrees at 15/15 stages. This is a population-summary result,
not participant-level or anatomical validation.

## Residual diagnosis

The v3 predictions for the young series exceed the reported mean mPAP at every
stage. The residual is already +2.11 mmHg at rest and rises above +5 mmHg at
the two highest flows. The source reports the same mean body surface area,
1.9 m2, for every age stratum, and the evaluator already converts cardiac
index with that value. A missing BSA conversion therefore cannot explain the
age-specific error.

The remaining candidates were a lower young-adult resistance level, a stronger
young-adult flow response, or a more detailed distensible-vessel model.

## Calibration evidence

Kovacs et al. (2012) reanalysed published individual right-heart-catheterization
measurements from 222 healthy subjects. For supine participants aged 24–50
years, resting PVR was 69 ± 28 dyn·s·cm−5. In subjects aged 50 years or
younger, an 85% increase in cardiac output during moderate exercise reduced
PVR by 12%.

Calibration source: Kovacs G, Olschewski A, Berghold A, Olschewski H,
*Pulmonary vascular resistances during exercise in normal subjects: a
systematic review*, European Respiratory Journal 2012;39:319–328,
[doi:10.1183/09031936.00008611](https://doi.org/10.1183/09031936.00008611).

The exercise observation implies

```text
young flow exponent = ln(0.88) / ln(1.85)
                    = -0.2078
```

which is nearly identical to the existing Claessen-calibrated exponent
−0.2021. Adding a separate young-adult flow law would therefore add complexity
without an evidence-supported material change. The evidence instead identifies
the resting resistance level as the important correction.

## Locked v4 derivation

One Wood unit equals 80 dyn·s·cm−5, so the source value is

```text
young invasive PVR = 69 / 80
                   = 0.8625 WU

young multiplier = 0.8625 / 1.2
                 = 0.71875
```

The v4 candidate applies this multiplier only to the existing young band. Its
supported lower age is narrowed from 18 to 24 years, matching the directly
applicable source stratum. The 40–under-60 reference multiplier and the
Kane-derived 60–85 multiplier remain unchanged. Flow adaptation, compliance,
PAWP handling, transit, and perfusion split are also unchanged.

## Calibration and validation separation

The source roles are deliberately explicit:

- Claessen (2015) calibrates the common bounded flow response;
- Kane (2016) calibrates the older resistance multiplier;
- Kovacs (2012) calibrates the invasive young resistance level; and
- Wolsk (2017) is the only counted v4 population validation cohort.

Kovacs (2012) reuses part of the historical invasive literature corpus studied
by Kovacs (2009). The three Kovacs (2009) stages are therefore omitted from the
v4 validation case instead of being mislabeled independent. The frozen v2 and
v3 comparisons remain available for historical comparison.

## No-refit result

No Wolsk value or acceptance interval was changed, and no v4 parameter was
selected from a Wolsk result.

| Wolsk series | v3 agreement | v4 agreement |
|---|---:|---:|
| age 20–39 | 1/5 | 5/5 |
| age 40–59 | 5/5 | 5/5 |
| age 60–80 | 5/5 | 5/5 |
| **Counted total** | **11/15** | **15/15** |

Across the 15 Wolsk stages, pressure RMSE decreases from 2.453 to
1.413 mmHg. In the young series it decreases from 3.663 to 1.165 mmHg. The
largest remaining young residual is +2.08 mmHg at the 50% exercise stage and
still lies inside the directly reported 95% confidence interval.

## Executable artifacts

- model definition:
  `data/lung-models/healthy-adult-rest-exercise-age-invasive-0d-v4.json`;
- validation case:
  `data/validation/pulmonary-zero-dimensional/healthy-population-multipoint-v3.json`;
- unchanged schemas: `lung-model-definition/1.3.0` and
  `pulmonary-zero-dimensional-population-multipoint-validation/1.1.0`.

## Interpretation limits

- The calibration is a systematic reanalysis of historical invasive records,
  not a new prospective cohort.
- The 24–50-year aggregate is applied only to a narrower 24–under-40 band;
  it does not resolve within-band variation.
- The source SD is recorded in this model card but is not propagated through
  the deterministic multiplier.
- Fifteen agreements with confidence intervals around population means do not
  constitute individual physiological tolerance or participant-level
  validation.
- The result does not validate pulsatile pressure, pressure-dependent
  compliance, anatomy, sex effects, disease, or regional perfusion.
- v4 is a research candidate and must not be used for diagnosis or treatment.

## Next scientific step

The young discrepancy no longer justifies further parameter tuning. The next
valuable pulmonary refinement is structural: implement a pressure-dependent
distensible-vessel or anatomical regional candidate and compare it against the
same frozen cohorts plus participant-level measurements when reuse terms are
available.
