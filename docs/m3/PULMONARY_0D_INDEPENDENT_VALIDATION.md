<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Independent Validation of the Pulmonary 0D Candidate

## 1. Question and decision

This validation asks whether the locked healthy-adult pulmonary 0D candidate
predicts independent aggregate observations that were not used to select its
parameters. It is separate from software verification and from the ESC/ERS,
Wong, Swift, and dissertation evidence used by the model card.

The result is a **qualified aggregate validation pass** for mean pulmonary
pressure, resting RC behavior, and pulmonary arterial compliance. The
out-of-scope exercise challenge also exposes a clear limitation: the fixed
0.36 s RC time constant does not reproduce the independently observed exercise
value of 0.23 ± 0.007 s. This does not invalidate the declared resting model;
it is direct evidence that a future exercise variant needs state-dependent
vascular recruitment, resistance, or compliance.

This is not patient-level or clinical validation.

## 2. Independence and protocol lock

The executable validation case is
`data/validation/pulmonary-zero-dimensional/healthy-adult-independent-v1.json`.
Its source URLs are checked automatically against every source URL in the
model definition. Evaluation fails if any source is reused.

The protocol was fixed in the repository before the repository validator was
first executed. It was not prospectively registered. The acceptance rule is:

> Every required endpoint prediction must lie within one reported cohort
> standard deviation of the independent mean (`|z| <= 1`).

Diagnostic endpoints expose limitations but do not determine the qualified
pass. One standard deviation is an engineering compatibility criterion, not a
statistical equivalence test. Inputs and endpoints are stored in SI units; the
clinical units below are display units only.

## 3. Independent evidence

### 3.1 Scope-matched supine rest

Forton et al. studied 26 healthy adult volunteers in supine, semirecumbent, and
upright positions. In the scope-matched supine resting condition they reported
cardiac output 6.0 ± 1.0 L/min and mPAP 15.5 ± 0.8 mmHg. Cardiac output and
mPAP were derived by Doppler echocardiography. The model remains fully locked
for this comparison and receives only the independent mean cardiac output.

### 3.2 Invasive near-scope and stress crosschecks

Bentley et al. reported right-heart-catheter and thermodilution measurements
from 36 healthy controls at semiupright rest and steady-state submaximal
exercise. Measured PAWP is treated as an observed downstream boundary
condition; this is not fitting. The model's PVR and compliance remain locked.
The resting condition is a near-scope invasive crosscheck. Exercise is labeled
explicitly as an out-of-scope stress test.

## 4. Reproducible results

| Condition | Endpoint | Independent mean ± SD | Prediction | abs. z | Role | Result |
|---|---|---:|---:|---:|---|---|
| Forton supine rest | mPAP | 15.5 ± 0.8 mmHg | 15.20 mmHg | 0.375 | required | pass |
| Bentley semiupright rest | mPAP | 17 ± 3 mmHg | 16.76 mmHg | 0.080 | required | pass |
| Bentley semiupright rest | compliance | 5.6 ± 1.6 mL/mmHg | 5.00 mL/mmHg | 0.375 | required | pass |
| Bentley semiupright rest | RC time | 0.39 ± 0.10 s | 0.36 s | 0.300 | required | pass |
| Bentley semiupright exercise | mPAP | 25 ± 5 mmHg | 28.44 mmHg | 0.688 | required | pass |
| Bentley semiupright exercise | compliance | 4.6 ± 1.7 mL/mmHg | 5.00 mL/mmHg | 0.235 | required | pass |
| Bentley semiupright exercise | RC time | 0.23 ± 0.007 s | 0.36 s | 18.571 | diagnostic | fail |

All 6 required endpoints pass. One diagnostic endpoint fails as intended. The
validator obtains predictions from the executable `PulmonaryZeroDimensionalModel`,
not from a separately reimplemented pressure equation.

For measured-wedge conditions, the evaluated mean pressure is

```text
predicted mPAP = measured PAWP + locked PVR * measured cardiac output.
```

The locked PVR is 1.2 WU and the locked compliance is 5.0 mL/mmHg.

## 5. Sources and data rights

1. Forton K, Motoji Y, Deboeck G, Faoro V, Naeije R. Effects of body position
   on exercise capacity and pulmonary vascular pressure-flow relationships.
   *Journal of Applied Physiology*. 2016;121:1145-1150.
   <https://doi.org/10.1152/japplphysiol.00372.2016>.
2. Bentley RF, Barker M, Esfandiari S, et al. Normal and Abnormal Relationships
   of Pulmonary Artery to Wedge Pressure During Exercise. *Journal of the
   American Heart Association*. 2020;9:e016339.
   <https://doi.org/10.1161/JAHA.120.016339>.

The MEHLISSA schema, factual transcription, metadata arrangement, and this
report are CC-BY-4.0. The source articles are not redistributed and retain
their own terms. Bentley et al. is CC-BY-NC-ND-4.0; only small reported
aggregate facts are transcribed with attribution. Forton et al. remains under
publisher terms.

## 6. Interpretation and remaining work

This increment establishes independent aggregate evidence without hiding the
following limitations:

- no individual records or covariance matrices were available;
- one source uses echocardiographic estimates rather than catheter values;
- the invasive source is semiupright rather than supine;
- PAWP is a surrogate downstream boundary for left-atrial pressure;
- the one-SD rule demonstrates cohort compatibility, not equivalence;
- no patient, disease, lobar, pulsatile, gas-exchange, or clinical claim is
  supported; and
- exercise adaptation is demonstrably absent from the fixed RC model.

The next scientific model increment should add an exercise-state rule for PVR
and compliance, calibrated on a data set separate from Bentley et al., then
retain Bentley et al. as the independent stress-test set. A later stronger
validation should use subject-level, jointly measured supine CO, PAWP, mPAP,
and compliance with a prespecified analysis plan.
