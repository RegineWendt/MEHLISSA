<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Bounded Pulmonary 0D Flow Adaptation

## 1. Decision

M3.9 adds a second, explicitly versioned pulmonary 0D candidate rather than
changing the resting v1 model retrospectively. The executable definition is
`data/lung-models/healthy-adult-rest-exercise-0d-v2.json`; it validates against
`lung-model-definition/1.2.0`.

The v2 model retains all v1 resting quantities and introduces bounded,
flow-dependent multipliers for pulmonary vascular resistance (PVR) and
pulmonary arterial compliance. It is intended for healthy-adult mean-flow
rest-to-exercise research scenarios. It is not a pulsatile, patient-specific,
disease, diagnostic, or treatment model.

## 2. Calibration-source independence

The adaptation is calibrated from Claessen et al. 2015, not from the Bentley
et al. validation observations. Claessen et al. studied 14 healthy controls
using exercise cardiac magnetic resonance with simultaneous invasive mean
pulmonary arterial pressure during incremental supine cycling. The aggregate
values used here are:

| Quantity | Rest mean ± SD | Peak exercise mean ± SD |
|---|---:|---:|
| cardiac output | 6.2 ± 1.9 L/min | 16.2 ± 5.3 L/min |
| total pulmonary resistance | 1.7 ± 0.6 WU | 1.4 ± 0.6 WU |
| pulmonary arterial compliance | 9.7 ± 3.1 mL/mmHg | 5.1 ± 1.8 mL/mmHg |
| reported RC time | 0.89 ± 0.25 s | 0.41 ± 0.17 s |

The attractive Wright et al. 2016 data set was rejected for calibration.
Bentley et al. states that its healthy control data came from the earlier
Toronto exercise studies and cites Wright et al.; therefore those publications
do not provide cohort-independent calibration and validation sets.

Claessen et al. is source-, institution-, and cohort-disjoint from Bentley et
al. The DOI URLs are also checked automatically by the validation evaluator.

## 3. Executable rule

For incoming cardiac output `Q`, the dimensionless activation ratio is

```text
x = clamp(Q / Q_ref, 1, x_max)
```

and the effective parameters are

```text
R_eff = R_0 x^a_R
C_eff = C_0 x^a_C.
```

The checked-in values are:

| Parameter | Value | Derivation |
|---|---:|---|
| `Q_ref` | 6.0 L/min | locked MEHLISSA v1 resting reference |
| `a_R` | −0.2021485749 | `ln((1.4/1.7)) / ln((16.2/6.2))` |
| `a_C` | −0.6693501452 | `ln((5.1/9.7)) / ln((16.2/6.2))` |
| `x_max` | 2.6129032258 | Claessen peak/rest flow ratio `16.2/6.2` |

At `x = 1`, v2 is exactly v1. Below the reference flow, the multiplier remains
one. Above `x_max`, it remains at its calibrated upper-flow value. This avoids
unsupported low-flow and supra-peak extrapolation.

At every host step, the existing analytical Windkessel update uses the
effective resistance and compliance:

```text
P_eq = P_LA + R_eff Q
tau_eff = R_eff C_eff
P(t + dt) = P_eq + (P(t) - P_eq) exp(-dt / tau_eff).
```

The current flow ratio, effective PVR, effective compliance, and effective RC
time are visible in `PulmonaryZeroDimensionalState`.

## 4. Independent post-calibration test

The protocol and Bentley observations are duplicated as immutable v2
validation input in
`data/validation/pulmonary-zero-dimensional/healthy-adult-independent-v2.json`.
Bentley is not read during calibration. Its measured mean PAWP and cardiac
output are boundary conditions, exactly as in M3.8.

| Bentley exercise endpoint | Mean ± SD | v1 prediction / abs. z | v2 prediction / abs. z | Result |
|---|---:|---:|---:|---|
| mPAP | 25 ± 5 mmHg | 28.44 / 0.688 | 26.85 / 0.369 | required pass |
| compliance | 4.6 ± 1.7 mL/mmHg | 5.00 / 0.235 | 3.29 / 0.769 | required pass |
| RC time | 0.23 ± 0.007 s | 0.360 / 18.571 | 0.209 / 3.005 | diagnostic fail |

All six required rest/exercise endpoints still pass the locked one-standard-
deviation rule. The diagnostic exercise RC discrepancy is reduced by about
84% in z-score magnitude but does not pass. The result therefore supports the
direction and bounded magnitude of the adaptation without claiming that the
exercise dynamics are solved.

## 5. Limitations and next scientific refinement

- Claessen did not report wedge pressure in healthy controls. Its total
  pulmonary-resistance ratio is therefore used as a proxy for the PVR
  multiplier; this is the main calibration mismatch.
- The exponents are derived from cohort means. Source SDs are recorded, but
  covariance was unavailable and exponent uncertainty is not propagated.
- The Claessen sample was small, predominantly male, younger than Bentley, and
  supine rather than semiupright.
- The model changes lumped PVR and compliance but does not resolve recruitment,
  distension, lobar redistribution, pressure-dependent compliance, heart rate,
  pulse pressure, or ventricular coupling.
- PAWP remains either a fixed resting surrogate or an externally measured
  boundary. MEHLISSA does not yet predict its exercise response.
- Transit time and right/left flow split remain fixed.

The next physiological improvement should use a jointly measured, subject-
level multipoint data set with CO, PAWP, mPAP, systolic/diastolic PA pressure,
and stroke volume. That would permit uncertainty-aware calibration of
recruitment and pressure-dependent compliance while reserving a genuinely
disjoint cohort for final validation.

## 6. Sources and rights

1. Claessen G, La Gerche A, Dymarkowski S, et al. Pulmonary vascular and right
   ventricular reserve in patients with normalized resting hemodynamics after
   pulmonary endarterectomy. *Journal of the American Heart Association*.
   2015;4:e001602. <https://doi.org/10.1161/JAHA.114.001602>.
2. Bentley RF, Barker M, Esfandiari S, et al. Normal and Abnormal Relationships
   of Pulmonary Artery to Wedge Pressure During Exercise. *Journal of the
   American Heart Association*. 2020;9:e016339.
   <https://doi.org/10.1161/JAHA.120.016339>.

Claessen et al. is CC-BY-NC-4.0 and Bentley et al. is
CC-BY-NC-ND-4.0. The articles are not redistributed. Only small attributed
aggregate facts are transcribed. The MEHLISSA schema, derived parameterization,
metadata arrangement, and documentation are CC-BY-4.0.
