<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Subject-Level Pulmonary Multipoint Validation

## 1. Status

M3.10 provides the executable and schema-validated analysis path for
subject-level pulmonary pressure-flow trajectories. It does **not** claim that
subject-level validation has passed. The required measured stage data are not
publicly downloadable and have not been received under a data-use grant.

The implementation deliberately keeps three statements separate:

1. the multipoint software is verified with a synthetic test fixture;
2. a suitable independent measured cohort has been identified and a minimum
   data request has been prepared; and
3. physiological validation remains pending until licensed, pseudonymized
   measurements are received and evaluated without parameter refitting.

M3.11 now provides a separate, executable
[published-population multipoint validation](PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md)
from Kovacs and Wolsk. It allows physiological development to continue without
waiting for the Arizona request, but it does not turn aggregate group means
into participant-level trajectories. This document therefore remains the
higher-resolution evidence plan rather than the immediate M3 blocker. The
[PCQ-1.2 evidence-source screen](../qualification/PCQ1_EVIDENCE_SOURCE_SCREEN.md)
has since rechecked the candidate hierarchy against primary protocols and
official repositories. The subsequent
[PCQ-1.3 pre-outcome amendment](../qualification/PCQ1_PRE_OUTCOME_AMENDMENT.md)
supersedes the older open-ended acceptance wording for new PCQ-H evidence: it
locks the observation model, numeric gates, uncertainty floors, and a minimum
of ten eligible participants for a full track decision.

## 2. Preferred first data set

The first-choice cohort is the five-person healthy control group reported by
Elliott et al., *iCPET Calculator: A Web-Based Application to Standardize the
Calculation of Alpha Distensibility in Patients With Pulmonary Arterial
Hypertension*, JAHA 2023, DOI
[10.1161/JAHA.123.029667](https://doi.org/10.1161/JAHA.123.029667).

It is preferred because the study reports:

- an institution and investigator group distinct from the Toronto/Bentley and
  Leuven/Claessen cohorts already used by MEHLISSA;
- supine incremental exercise;
- an incremental multipoint protocol, with the exact number of complete stages
  per control to be confirmed before acceptance;
- stage-matched mPAP, PAWP, and cardiac output;
- direct-Fick cardiac output during exercise; and
- a healthy-control phenotype explicitly used to verify a multipoint analysis
  method.

The article and calculator software are public, but the clinical stage data are
available only from the corresponding author on reasonable request. Public
software examples are not substitutes for the measured control records.

The prepared request is
[UA iCPET Control Data Request](UA_ICPET_DATA_REQUEST.md).

## 3. Candidate hierarchy

| Priority | Candidate | Strength | Access or scope limitation |
|---|---|---|---|
| 1 | UA iCPET healthy controls, Elliott et al. 2023 | five healthy controls; supine multipoint protocol; joint mPAP/PAWP/CO acquisition | clinical records require author approval and explicit reuse terms; at least three complete stages per included control must be confirmed |
| 2 | Coffman et al. 2018 | seven-stage exercise protocol; invasive mPAP and wedge pressure plus direct-Fick flow in nine catheterized healthy adults | upright rather than primary recumbent context; small cohort; possible participant overlap with Coffman et al. 2017 must be resolved |
| 3 | Pandey et al. 2020 | 104 rigorously screened healthy adults spanning ages 20-76 with invasive upright exercise hemodynamics | historical source assembled from incompatible formats; exact availability of the complete PCQ-H endpoint tuple must be confirmed before selection |
| rejected for initial PCQ-H | PVDOMICS | standardized deep phenotyping and controlled dbGaP access | the published protocol assigns healthy controls to non-invasive CPET and invasive CPET to consented pulmonary-hypertension participants; only a controlled variable inventory proving an eligible healthy invasive trajectory could overturn this decision |
| later stress test | PEX-NET | large international registry; one to thirteen exercise measurements, median four | clinically referred rather than a clean healthy cohort; heterogeneous protocols; individual data are not public |

The open PhysioNet SCG-RHC collection is not accepted for this task because it
contains heart-failure catheterizations with pharmacological vasodilation, not
healthy incremental exercise. Toronto exercise cohorts associated with Wright,
Buchan, and Bentley are not accepted as an independent post-calibration set
because cohort reuse cannot be excluded.

## 4. Data minimization contract

MEHLISSA requests and stores only a study pseudonym and stage-level physiology.
Names, initials, dates of birth, examination dates, free-text notes, location,
contact details, and source-system identifiers are forbidden.

Each subject requires:

- study pseudonym;
- confirmation of healthy-control inclusion;
- body position and cardiac-output method;
- at least three stages with distinct cardiac outputs;
- stage order and workload;
- cardiac output, mPAP, and PAWP measured in the same stage; and
- optionally heart rate, systolic PAP, and diastolic PAP as an all-or-none set.

The machine-readable contract is
`data/schemas/pulmonary-zero-dimensional-multipoint-validation/1.0.0.schema.json`.
It restricts the phenotype to `healthy_control`, requires three stages, and
permits only pseudonymous identifiers.

## 5. Locked analysis

The v2 model definition remains immutable. For every measured stage, cardiac
output and PAWP are applied as observed boundary conditions; no PVR,
compliance, exponent, or flow limit is fitted.

For each stage, the evaluator reports:

\[
PVR_{obs} = \frac{mPAP - PAWP}{Q}
\]

If systolic PAP, diastolic PAP, and heart rate are all available, it also
reports:

\[
C_{obs} = \frac{60Q/HR}{sPAP-dPAP}, \qquad RC_{obs}=PVR_{obs}C_{obs}.
\]

For each subject, ordinary least-squares fits are calculated separately for
observed mPAP versus flow, predicted mPAP versus flow, and observed PAWP versus
flow. The report includes slope, intercept, coefficient of determination,
stage residuals, mean pressure error, and pressure RMSE.

No numerical pass threshold is inferred from the unavailable observations.
Acceptance limits must be locked from measurement repeatability or a reviewed
protocol before the measured values are exposed to model developers.

## 6. Evidence safeguards

- Calibration-source URLs are rejected if reused by a validation source.
- Every measured source must explicitly record a confirmed cohort-disjointness
  assessment; a distinct citation alone is insufficient.
- Stage ordinals must be contiguous from zero and workload must not decrease,
  preventing unordered records from masquerading as trajectories.
- A synthetic fixture is marked `synthetic_test_only` and rejected by the
  default evidence loader.
- Enabling synthetic input requires an explicit test-only load option, and the
  resulting report states `measured_evidence = false`.
- A scientific report can state measured evidence only for a schema-valid case
  marked `measured_validation` and supported by explicit data rights.
- No subject-level clinical file is committed until the grant permits public
  redistribution. If redistribution is prohibited, the repository will store
  only the schema, checksum/provenance manifest, analysis protocol, and
  non-identifying aggregate report.

## 7. Completion criteria

Subject-level validation becomes complete only when all of the following are
true:

1. access and reuse terms are documented;
2. the cohort is confirmed disjoint from calibration and prior validation;
3. at least three eligible healthy participants have three jointly measured
   stages for a bounded pilot, and at least ten meet the same rule for a full
   PCQ-H track decision;
4. acceptance criteria are locked before the values are inspected;
5. the immutable v2 model is evaluated without fitting;
6. subject and cohort reports expose all failures; and
7. the resulting permitted artifacts pass the full MEHLISSA CI matrix.

## 8. References

1. Elliott J, Menakuru N, Martin KJ, et al. iCPET Calculator: A Web-Based
   Application to Standardize the Calculation of Alpha Distensibility in
   Patients With Pulmonary Arterial Hypertension. *Journal of the American
   Heart Association*. 2023;12:e029667.
   <https://doi.org/10.1161/JAHA.123.029667>
2. Tang WHW, Wilcox JD, Jacob MS, et al. Comprehensive Diagnostic Evaluation of
   Cardiovascular Physiology in Patients With Pulmonary Vascular Disease:
   Insights From the PVDOMICS Program. *Circulation: Heart Failure*. 2020.
   <https://doi.org/10.1161/CIRCHEARTFAILURE.119.006363>
3. Coffman KE, et al. The effect of pulmonary vascular resistance on
   cardiopulmonary exercise response in older adults. *Physiological Reports*.
   2018;6:e13565.
   <https://doi.org/10.14814/phy2.13565>
4. Pandey A, et al. Determinants and prognostic implications of pulmonary
   arterial pressure response to exercise. *JACC: Heart Failure*.
   2020;8:111-121.
   <https://doi.org/10.1016/j.jchf.2019.08.020>
5. Kovacs G, et al. Pulmonary hemodynamics during exercise and prognosis in the
   PEX-NET registry. *European Respiratory Journal*. 2024.
   <https://doi.org/10.1183/13993003.00698-2024>
