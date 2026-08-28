<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Published-Population Pulmonary Multipoint Validation

## 1. Result

M3.11 removes controlled subject-data access as a blocker for the next
independent physiological check. The immutable flow-adaptive v2 model is
evaluated against four published healthy-population pressure-flow series from
two calibration-disjoint sources. No model parameter is fitted to either
source.

This is a **qualified partial population-validation result**:

| Independent series | Participants | Stages agreeing with the locked rule | Pressure RMSE |
|---|---:|---:|---:|
| Kovacs et al., pooled upright rest to maximal exercise | 193 | 3/3 | 5.557 mmHg |
| Wolsk et al., age 20–39, supine | 20 | 0/5 | 4.627 mmHg |
| Wolsk et al., age 40–59, supine | 22 | 5/5 | 1.935 mmHg |
| Wolsk et al., age 60–80, supine | 20 | 2/5 | 1.849 mmHg |
| **Total** | **255** | **10/18** | not pooled across protocols |

The result is not a subject-level, patient-level, or clinical validation. It
does, however, test complete published rest/challenge/exercise series rather
than isolated endpoints and reveals a specific missing model dimension: age.

## 2. Evidence

The executable evidence case is
`data/validation/pulmonary-zero-dimensional/healthy-population-multipoint-v1.json`.
Its schema is
`data/schemas/pulmonary-zero-dimensional-population-multipoint-validation/1.0.0.schema.json`.

Kovacs et al. report weighted mean and SD for upright resting, slight-exercise,
and maximal-exercise hemodynamics in 193 healthy volunteers. The contributing
exercise publications predate the 2015 Claessen calibration cohort. The table
therefore provides an immediately usable, attributed population trajectory
that is independent of the v2 calibration source.

Wolsk et al. prospectively measured 62 healthy volunteers using supine right-
heart catheterization. The repository stores the three non-overlapping age
strata—20–39 years (n=20), 40–59 years (n=22), and 60–80 years (n=20)—and does
not duplicate them through the overlapping all-participant summary. Each
series contains rest, passive leg raise, and three exercise stages. Cardiac
output was measured by thermodilution and published as cardiac index.

Only attributed numerical facts are transcribed. Source articles remain under
their publisher terms and are not redistributed or relicensed. The MEHLISSA
schema, transcription, and documentation are CC BY 4.0.

## 3. Locked protocol

The protocol was chosen before repository evaluation:

- the v2 definition is immutable;
- measured mean PAWP and mean flow are boundary conditions at each stage;
- PVR, compliance, flow exponent, and flow cap are not fitted;
- each series must have at least three distinct mean flows;
- calibration-source URL reuse is rejected;
- every source and every series must be explicitly cohort-disjoint;
- the model reports every stage, residual, interval decision, and series fit;
- no overall result can hide a failed subgroup.

For a mean-plus-SD series, agreement means that the prediction lies within
mean ± 2 SD. The multiplier follows the upper-limit convention described by
Kovacs et al. For a mean-plus-95%-CI series, agreement means that the prediction
lies inside the directly reported CI for the population mean.

The latter rule is intentionally strict. A 95% CI describes uncertainty around
the estimated group mean; it is not a 95% range for individuals. Consequently,
a CI failure demonstrates disagreement with the reported population mean, not
that a prediction is physiologically impossible for an individual.

## 4. Unit and derivation policy

Published values remain in their reported units in the evidence JSON so that
table transcription can be audited without reverse conversion. The loader
normalizes pressures from mmHg to Pa and flows from L/min to m3/s immediately.

Wolsk et al. report cardiac index rather than absolute cardiac output. For each
age stratum, the evaluator derives mean absolute flow as

\[
Q_{mean}=CI_{mean}\,BSA_{mean}
\]

using the reported stratum mean BSA of 1.9 m2. This is an aggregate mean-flow
approximation. It must not be interpreted as reconstruction of any participant's
cardiac output.

## 5. Interpretation

The pooled upright Kovacs series passes all three relatively broad mean ± 2 SD
limits. This supports the broad rest-to-maximal-exercise pressure response of
the current candidate, despite protocol heterogeneity.

The age-stratified Wolsk result is more discriminating:

- age 40–59 agrees at all five stages;
- age 20–39 agrees at no stage; the model is already 2.612 mmHg high at rest
  and 6.589 mmHg high at the 75% stage;
- age 60–80 agrees at the 25% and 75% stages but falls below the reported mean
  interval at rest, passive leg raise, and 50% exercise; representative
  residuals are −1.072, −2.184, and −3.035 mmHg respectively.

This pattern is coherent with the current model contract: v2 represents one
composite healthy adult and has no age input. The independent evidence therefore
does not justify changing acceptance thresholds. It justifies a future,
separately calibrated age-conditioned candidate while retaining Wolsk as an
untouched validation source.

## 6. What is and is not closed

Closed by M3.11:

- immediate access to independent, published multipoint population evidence;
- a separate schema for population summaries rather than pseudonymous subjects;
- exact statistical semantics for mean/SD and mean/95% CI;
- cardiac-index-to-flow derivation with explicit BSA provenance;
- executable source-disjointness and no-refit checks; and
- a reproducible partial result with subgroup failures exposed.

Still open:

- independent participant-level trajectories and within-person uncertainty;
- a separately calibrated and independently validated age-conditioned model;
- anatomical, pulsatile, and regional pulmonary refinement;
- posture/protocol harmonization across sources; and
- clinical validation, which is outside the present model's intended use.

The Arizona request remains worthwhile as a later, higher-resolution evidence
tier, but it no longer blocks progress with published population validation.

## 7. References

1. Kovacs G, Berghold A, Scheidl S, Olschewski H. Pulmonary arterial pressure
   during rest and exercise in healthy subjects: a systematic review.
   *European Respiratory Journal*. 2009;34:888–894.
   <https://doi.org/10.1183/09031936.00145608>
2. Wolsk E, et al. Rest and Exercise Hemodynamics in Relation to Age: A
   Prospective Study of Normal Human Physiology. *JACC: Heart Failure*.
   2017;5(5):337–346. <https://doi.org/10.1016/j.jchf.2016.10.012>
