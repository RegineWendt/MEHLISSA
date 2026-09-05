<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1.3 Pre-Outcome Amendment

## 1. Decision and status

PCQ-1.3 is the locked analysis amendment for the first pulmonary and capillary
qualification cycle. Its machine-readable authority is
`data/qualification/pulmonary-capillary-preoutcome-amendment-v1.json`, validated
against schema
`data/schemas/pulmonary-capillary-preoutcome-amendment/1.0.0.schema.json`.

The amendment was written before any candidate participant-level outcome file
was downloaded, received, or inspected. It binds the PCQ-1 design and PCQ-1.2
candidate register by SHA-256, selects guarded source roles, freezes eight
observation models, six primary numeric gates, sample-size floors, statistical
methods, and failure states, and names PCQ-1.4 as the next implementation step.

Published aggregate means, spreads, figures, and repeatability results were
already visible during source screening. PCQ-1.3 is therefore accurately called
**pre-participant-outcome**, not wholly outcome-blind. The numeric limits were
not selected to include any published candidate mean or spread. A later change
to an endpoint, observation equation, eligibility rule, uncertainty floor,
numeric limit, or analysis method requires a new protocol version and an
independent validation split.

This amendment does not mean that any physiological track has passed. Source
access, data rights, participant eligibility, adapters, uncertainty execution,
and no-refit evaluation remain pending. Independent pulmonary-method review is
recommended before locked execution.

## 2. Guarded source selection

| Track | Selected source and role | Activation boundary |
|---|---|---|
| PCQ-H | University of Arizona iCPET controls: primary pilot and confirmatory source only if the sample is sufficient | access and rights; individual health, age, posture, stage timing, pressure reference, cardiac-output method, and at least three complete mPAP-PAWP-output stages |
| PCQ-R | Bailey 2019: provisional primary five-lobe source | feasibility, participant-level five-lobe values, normalization, segmentation, covariance or replicates, and rights |
| PCQ-C1 / partial PCQ-J | D'Souza 2025: primary functional-volume and same-stage flow-volume source | same-stage `Vc` and flow, measured hemoglobin and correction, baseline posture, condition order, missingness, and rights |
| PCQ-C2 | Lassen 2023: provisional whole-pulmonary transit source | remains blocked until extra-capillary and mixing-delay terms are fixed independently of Lassen outcomes; participant records and rights are also pending |

The selection is not a claim that the sources are interchangeable. D'Souza
provides functional capillary volume and flow but no measured capillary transit.
Lassen measures transit from pulmonary trunk to left atrium, not through the
capillary segment alone. Bailey's five-lobe availability is not yet confirmed.
No screened source contains the complete PCQ-J state.

## 3. Population and precision floors

The first claim remains a healthy-adult recumbent reference context, primarily
ages 20 through 40. The following floors prevent a small available cohort from
being misreported as a definitive qualification:

| Track | Pilot floor | Track-decision floor | Required observations | Consequence below floor |
|---|---:|---:|---|---|
| PCQ-H | 3 participants | 10 participants | at least three ordered same-visit stages per participant, spanning at least 2 L/min cardiac output for the slope endpoint | 3-9 is pilot compatibility only; fewer than 3 is insufficient |
| PCQ-R | 5 participants | 10 participants | five same-state anatomical lobe fractions | aggregate-only or fewer than 10 remains supportive/pilot evidence |
| PCQ-C1 | 8 participants | 12 participants | positive baseline functional `Vc`, same-stage context, and hemoglobin handling | wider or underpowered intervals are inconclusive |
| PCQ-C2 | not activated | 20 participants if unblocked | eligible resting whole-pulmonary transit plus an independently frozen observation model | blocked, regardless of available published mean |
| PCQ-J | none | none | a complete joint cohort does not currently exist | remains partial |

The Arizona publication reports only five controls, with a public group age of
43 plus or minus 16 years and 60% female. Individual ages are unknown. Even if
all five records are granted, this source alone can therefore produce only a
bounded pilot under PCQ-1.3; a second healthy recumbent cohort is needed for a
full PCQ-H decision. This limitation is intentional and cannot be relaxed after
seeing trajectories.

The floors are feasibility safeguards, not claims of formal prospective power.
Equivalence requires the complete predeclared confidence interval to lie inside
the margin. If the interval is too wide, the correct result is
`inconclusive-insufficient-precision`, not a pass and not a model failure.

## 4. Frozen observation models

### 4.1 Participant hemodynamics

For every eligible stage, observed cardiac output and PAWP are boundary
conditions rather than calibration targets. MEHLISSA predicts mPAP without
refitting. The primary residual is:

```text
predicted mPAP - observed invasive mPAP
```

Observed PVR is derived only from a joint stage tuple:

```text
(observed mPAP - observed PAWP) / observed cardiac output
```

The primary pressure-flow slope is ordinary least squares over at least three
ordered stages. A peak-minus-rest two-point ratio is diagnostic only. RVSP,
systolic PAP, echo-derived mPAP, or population reference pressure cannot replace
invasive stage mPAP.

Compliance and resistance-compliance time are secondary and require a complete
systolic pressure, diastolic pressure, heart-rate, and output tuple. Missing
inputs are not imputed.

### 4.2 Five-lobe perfusion

The primary observation is a five-part anatomical composition normalized to
one. MEHLISSA supplies the corresponding five frozen lobe flow fractions.
Errors are evaluated per lobe, over all five lobes, and for the right-lung sum.

Four pulmonary-vein territories from Wong, right-lung spatial slices from
Hall, and gravitational zones are not silently converted to five lobes. They
require separate, frozen observation models and remain extensions rather than
the PCQ-R1 primary comparison.

### 4.3 Functional capillary volume

The model quantity is the frozen functional perfused volume, not morphometric
lumen capacity. The measured quantity is hemoglobin-adjusted `Vc` from the
multiple-inspired-oxygen DLCO regression. The comparison uses the positive
model-to-observed ratio on a log scale.

The source-supplied hemoglobin-adjusted value and its measured hemoglobin
metadata are retained. MEHLISSA does not apply a second undocumented correction.
Systemic hemoglobin correction is not relabelled as a direct measurement of
pulmonary capillary hematocrit; that missing covariate remains an explicit
limitation.

### 4.4 Whole-pulmonary transit

Lassen's rubidium-82 endpoint begins at the pulmonary trunk and ends at the left
atrium. The frozen capillary card represents only equivalent capillary
residence. A valid future model would require:

```text
predicted whole-pulmonary transit
  = independently fixed precapillary delay
  + frozen capillary residence
  + independently fixed postcapillary delay
  + independently fixed method-specific mixing delay
```

Those extra terms do not yet exist with adequate independent evidence.
PCQ-C2 is therefore locked as `blocked-observation-model`. The 0.859-second
capillary residence must not be compared directly with Lassen's whole-pulmonary
measurement. Lassen's pulmonary blood volume is defined as output multiplied
by transit and cannot independently validate the same volume-flow-transit
identity in MEHLISSA.

### 4.5 Partial joint diagnostic

D'Souza permits a same-stage `Vc / cardiac output` functional residence proxy.
The matching MEHLISSA quantity is computed with the same equation. This is a
useful coherence diagnostic but neither side is a measured anatomical
capillary-transit endpoint. It cannot complete PCQ-J.

## 5. Numeric gates fixed before record access

| Endpoint | Locked primary rule |
|---|---|
| PCQ-H1 mPAP | standard uncertainty floor 2 mmHg; use a larger supplied value; absolute stage error at most 5 mmHg for at least 80% of stages; every participant RMSE at most 5 mmHg; two-sided 90% cohort mean-bias interval wholly within plus or minus 5 mmHg |
| PCQ-H2 PVR | propagate at least 2 mmHg mPAP, 3 mmHg PAWP, and 10% flow standard uncertainty; a stage passes the larger of 0.5 WU or 25% relative error; at least 80% of stages and the complete 90% cohort relative-bias interval within plus or minus 25% |
| PCQ-H3 pressure-flow slope | at least three stages spanning 2 L/min; absolute participant slope difference at most 0.75 WU for at least 80% of participants; cohort median absolute difference at most 0.75 WU |
| PCQ-R1 five-lobe fractions | participant maximum lobe error 5 percentage points, five-lobe RMSE 3 points, and right-lung error 4 points; at least 80% pass all three and the cohort compositional center passes the same limits |
| PCQ-C1 functional `Vc` | method CV 8%, regression `r²` at least 0.95, at least 12 complete records, and the complete two-sided 90% bootstrap interval for the model/observed geometric-mean ratio inside 0.80-1.25 |
| PCQ-C2 whole-pulmonary transit | not activated; if independently unblocked before outcome access: at least 20 resting records and complete 90% ratio interval inside 0.75-1.3333, with 17.2% resting repeatability and a one-sided 25% tracer-retention bias sensitivity |

The mPAP, PVR, slope, and participant-lobe margins are conservative protocol
choices for a low-order research model, not clinical diagnostic cutoffs. If a
provider supplies larger method uncertainty than the floor, the larger value is
used; uncertainty may not be reduced after outcomes are known.

The existing aggregate PCQ-R reproduction retains its original stricter limits:
3 percentage points per lobe, 2 points five-lobe RMSE, and 3 points right-lung
error. Passing that existing case does not substitute for new participant data.

The 20% `Vc` equivalence zone is wider than the method protocol's reported 8%
between-trial coefficient of variation and explicitly allows measurement plus
reference-model discrepancy. It is still demanding because the complete 90%
interval, not merely its point estimate, must remain inside the zone.

The transit ratio zone is wider because Lassen reports a 17.2% resting
repeatability coefficient and identifies method-specific tracer-retention bias.
It remains dormant until the anatomical observation model is independently
fixed.

## 6. Statistical execution rules

- Equivalence decisions use two-sided 90% confidence intervals.
- Resampling uses a participant-cluster bootstrap with 10,000 resamples and
  fixed seed `20260905`; all stages belonging to a sampled participant remain
  together.
- Compatible proportions are accompanied by two-sided 95% Wilson intervals so
  that small-sample uncertainty stays visible.
- Primary values are never imputed. Counts for eligibility, completeness, and
  missingness are reported by source, participant, stage, and endpoint.
- No outcome-based outlier deletion is permitted. A source quality flag remains
  in the report; exclusion is allowed only under a rule fixed before access.
- All six primary endpoints are retained. There is no global pass, endpoint
  substitution, or subgroup selected because it succeeds.

Every endpoint terminates in one of six explicit states:

1. `qualified`;
2. `not-qualified`;
3. `inconclusive-insufficient-precision`;
4. `blocked-access-or-rights`;
5. `blocked-observation-model`; or
6. `out-of-scope-stress-test`.

For PCQ-J, numerical volume-flow-transit relative closure remains a hard
software invariant at `1e-12`. It is never promoted to physiological evidence.

## 7. Why these decisions are scientifically conservative

The University of Arizona method supplies the correct joint invasive variables
and supine stages, but the five-person control cohort cannot support a full
track decision under this amendment. The published iCPET workflow also shows
that local variability and the number of included stages can influence fitted
distensibility, supporting the minimum-stage and flow-span rules.

The multiple-inspired-oxygen `Vc` protocol reports satisfactory between-trial
coefficients of variation of 7% for DLCO, 8% for `Vc`, and 15% for membrane
capacity, requires hemoglobin correction, and uses a minimum regression `r²` of
0.95. PCQ-C1 consumes the 8% and `r²` values as method evidence, not the
candidate cohort's physiological outcome.

Lassen reports resting and stress transit repeatability coefficients of 17.2%
and 17.9%. PCQ-C2 preserves those values but also preserves the wider anatomical
definition, tracer behavior, and algebraic dependence of derived pulmonary
blood volume.

None of these design choices grants participant-data rights or validates a
patient-specific digital twin. The complete amendment is a research-software
qualification plan with a non-clinical claim boundary.

## 8. Next increment

PCQ-1.4 should implement strict, rights-aware data adapters without requiring
or embedding restricted records. It should provide:

- one schema per accepted measurement family;
- a provenance and rights manifest with source checksum, permitted uses,
  retention, and redistribution state;
- a quarantine boundary keeping restricted records outside Git;
- synthetic outcome-blind fixtures for adapter and negative tests;
- unit, posture, age, stage, missingness, and source-overlap rejection; and
- report states matching this amendment exactly.

External requests remain separate institutional actions. PCQ-1.3 sends no
email and accepts no data-use agreement.

## 9. Primary method sources

1. Elliott J, Menakuru N, Martin KJ, et al. iCPET Calculator: A Web-Based
   Application to Standardize the Calculation of Alpha Distensibility in
   Patients With Pulmonary Arterial Hypertension. *Journal of the American
   Heart Association*. 2023;12:e029667.
   <https://doi.org/10.1161/JAHA.123.029667>
2. Tedjasaputra V, van Diepen S, Collins SE, et al. Assessment of Pulmonary
   Capillary Blood Volume, Membrane Diffusing Capacity, and Intrapulmonary
   Arteriovenous Anastomoses During Exercise. *Journal of Visualized
   Experiments*. 2017;(120):54949.
   <https://doi.org/10.3791/54949>
3. D'Souza AW, Brotto AR, Hicks B, et al. Pulmonary capillary blood volume and
   diffusing membrane capacity during exercise in humans: role of pulmonary
   artery pressure. *American Journal of Physiology-Lung Cellular and Molecular
   Physiology*. 2025;328:L631-L637.
   <https://doi.org/10.1152/ajplung.00358.2024>
4. Lassen ML, Byrne C, Hartmann JP, et al. Pulmonary blood volume assessment
   from a standard cardiac rubidium-82 imaging protocol: impact of
   adenosine-induced hyperemia. *Journal of Nuclear Cardiology*.
   2023;30:2504-2513.
   <https://doi.org/10.1007/s12350-023-03308-1>
5. Bailey DL, Farrow CE, Lau EM. V/Q SPECT-Normal Values for Lobar Function
   and Comparison With CT Volumes. *Seminars in Nuclear Medicine*.
   2019;49:58-61.
   <https://doi.org/10.1053/j.semnuclmed.2018.10.008>

The complete thirteen-source selection history remains in the
[PCQ-1.2 screen](PCQ1_EVIDENCE_SOURCE_SCREEN.md). This amendment supports
research planning and reproducible evaluation only. It does not authorize
diagnosis, treatment, or other clinical use.
