<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1 Pulmonary and Capillary Qualification Protocol

## 1. Decision and status

PCQ-1 is the first scientific qualification package after the M0-M7 software
demonstrator and Workbench 1.0. The machine-readable design is
`data/qualification/pulmonary-capillary-qualification-protocol-v1.json`,
validated against schema
`data/schemas/pulmonary-capillary-qualification-protocol/1.0.0.schema.json`.

**Current status:** design v0.1.0, PCQ-1.2 evidence-source screen,
[PCQ-1.3 pre-outcome amendment](PCQ1_PRE_OUTCOME_AMENDMENT.md) v0.2.0, and
[PCQ-1.4 rights-aware data ingress](PCQ1_DATA_INGRESS.md) completed.
The design was recorded before inspection of any new validation outcomes and
freezes the entering model artifacts, bounded candidate claim, qualification
tracks, endpoint hierarchy, analysis rules, uncertainty classes, negative
controls, amendment policy, and exit criteria. The source screen then reviewed
thirteen candidates and transparently recorded the aggregate outcomes visible
in their publications; no participant-level candidate data were acquired or
inspected. This progress does not claim that pulmonary or capillary
qualification has passed.

The amendment now freezes the selected-source roles, observation models,
sample-size and precision floors, uncertainty handling, analysis version, and
numeric decision limits before primary participant-level outcomes are exposed
to model developers. It deliberately leaves whole-pulmonary transit blocked
until independently sourced extra-capillary and mixing delays are fixed. Any
change after outcome access requires a new protocol version and an independent
validation split.

## 2. Bounded candidate claim

The claim to be tested is:

> For a declared healthy-adult recumbent reference context, frozen MEHLISSA
> pulmonary and capillary candidates reproduce selected pressure-flow,
> regional-perfusion, functional-capillary-volume, and transit observables
> within predeclared uncertainty-aware tolerances.

The first joint target is healthy adults aged 20-40 at rest in a recumbent or
supine position unless the accepted source-disjoint data justify a narrower
range. Supine exercise may be evaluated as an explicitly separate extension or
stress test. The represented resolution is an aggregate zero-dimensional
pulmonary circulation, five named parallel lobe beds, and an equivalent
pulmonary capillary bed.

PCQ-1 excludes disease prediction, gas exchange, pulsatile wave propagation,
patient-specific prediction, diagnostic or treatment decisions, and clinical
validity.

## 3. Frozen candidates

The design records SHA-256 hashes rather than relying on filenames alone:

| Candidate | Role | SHA-256 |
|---|---|---|
| `data/lung-models/healthy-adult-lobar-parallel-0d-v7.json` | aggregate pulmonary and five-lobe candidate | `1374bdd7eef2bc849b4fb5a06726adeecfec2ba2e74beb6d166db919b7a1c3c6` |
| `examples/capillary-models/pulmonary-healthy-adult-rest-supine-v1.json` | equivalent pulmonary capillary candidate | `b36bbacf118e062ccbb0e71b3d7794241ebdb9e461c7140242d4476509ea545b` |

The checker fails when either artifact changes without an explicit amendment.
A scientifically motivated model change creates a successor candidate; it does
not rewrite the result of the frozen candidate.

## 4. Four qualification tracks

| Track | Question | Present evidence | Evidence needed to exit |
|---|---|---|---|
| **PCQ-H - participant-level hemodynamics** | Does the frozen model reproduce stage-matched pressure-flow behavior within persons? | strict schema and evaluator verified with a synthetic fixture; aggregate and population comparisons available | source-disjoint healthy records with at least three joint cardiac-output, mean-pulmonary-pressure, and wedge-pressure stages; no refit; all observations reported |
| **PCQ-R - regional perfusion** | Are five-lobe fractions compatible with participant-level and changing physiological states? | two reconstructions from one independent aggregate normal-supine cohort pass existing compatibility limits | source-disjoint lobe-resolved participant data with posture, state, acquisition method, and uncertainty; dynamic data where available |
| **PCQ-C - capillary volume and transit** | Do functional volume and transit agree with independent physiology? | literature-bounded volume, morphometry, diameter, and exact numerical closure from different small historical cohorts | preferably joint flow and functional-volume observations, matched transit where possible, and an explicit hematocrit measurement or uncertainty model |
| **PCQ-J - joint coherence** | Can one compatible state explain hemodynamics, regional perfusion, volume, and transit together? | software composition and conservation only | a joint cohort, or an explicitly partial cross-cohort bridge that propagates age, posture, protocol, method, and cohort uncertainty |

A successful result in one track cannot silently advance another. In particular,
exact `volume = flow x transit` closure is numerical verification, not evidence
that the three physiological quantities are correct.

## 5. Endpoint hierarchy

The six primary endpoints are fixed at design stage. Numeric tolerances for new
data are deliberately not invented from unavailable outcomes.

| ID | Endpoint | Role | Main metrics | Tolerance state |
|---|---|---|---|---|
| PCQ-H1 | mean pulmonary arterial pressure at each stage | primary | signed residual, participant RMSE, cohort bias and interval, prediction-interval coverage | must be locked from repeatability and protocol uncertainty before outcome access |
| PCQ-H2 | stage-matched pulmonary vascular resistance | primary | signed/relative error, participant RMSE, cohort bias and interval | must propagate pressure, wedge-pressure, and flow uncertainty |
| PCQ-H3 | within-participant pressure-flow slope | primary | observed/predicted slope, difference and interval coverage; coefficient of determination is diagnostic | must use within-person repeated-stage uncertainty |
| PCQ-H4 | compliance and resistance-compliance time | secondary | signed/relative residual and participant/cohort summaries | evaluated only for complete stage-matched systolic pressure, diastolic pressure, and heart-rate tuples |
| PCQ-R1 | five normalized lobe perfusion fractions | primary | per-lobe error, maximum error, five-lobe RMSE, right-lung error | current aggregate reproduction remains locked at 3, 2, and 3 percentage points respectively; new participant/dynamic limits require a separate lock |
| PCQ-C1 | functional pulmonary capillary blood volume | primary | signed/relative error, cohort bias and interval, coverage | method-specific tolerance must include measurement and hematocrit uncertainty |
| PCQ-C2 | anatomically matched capillary or pulmonary transit | primary | signed/relative error, distributional distance, cohort bias and interval | measurement endpoints must match the simulation or use a predeclared observation model |
| PCQ-C3 | capillary hematocrit | covariate | value/source, endpoint sensitivity, uncertainty contribution | must be measured, evidence-backed, or propagated as uncertainty |
| PCQ-J1 | joint flow-volume-transit consistency | diagnostic | balance residual and variance-source contribution | numerical closure is a hard invariant; physiological compatibility has a separate data-derived tolerance |

The earlier aggregate one-standard-deviation pressure rule and population-mean
confidence intervals remain reproducible evidence for their original cases.
They are not recycled as participant-level physiological tolerance limits.

## 6. Data acquisition and eligibility

### 6.1 Candidate hierarchy already identified

| Need | Candidate or present source | Current use | Blocking decision |
|---|---|---|---|
| participant-level multipoint hemodynamics | University of Arizona iCPET healthy controls | preferred PCQ-H candidate | author approval, reuse terms, complete-stage count, and outcome-blind threshold basis |
| upright invasive hemodynamics | Coffman et al. 2018 | PCQ-H backup and posture/age generalization | nine catheterized healthy participants; possible overlap with Coffman 2017 must be resolved |
| broader deeply phenotyped controls | Pulmonary Vascular Disease Phenomics Program (PVDOMICS) | rejected for initial healthy invasive PCQ-H under the published protocol; possible later non-invasive or disease work | healthy controls receive non-invasive CPET, while invasive CPET is specified for pulmonary-hypertension participants; a controlled variable inventory would be needed to overturn this screen |
| heterogeneous exercise registry | Pulmonary Hemodynamics during Exercise Network (PEX-NET) | external stress-test or generalization evidence | protocol heterogeneity and non-healthy referral population prevent use as the initial narrow claim without a separate design |
| regional normal-supine perfusion | current Bourhis ventilation/perfusion single-photon emission computed tomography cohort | reproduce current PCQ-R aggregate result | a distinct participant-level or dynamic cohort is still required |
| functional capillary volume | current Lewis recumbent values | historical parameter anchor | small historical cohort, indirect method, hematocrit assumption, and no independent joint validation |
| morphometric capacity and surface | current Muehlfeld human lung morphometry | structural context and capacity anchor | destructive ex-vivo morphometry is not a functional-volume or transit validation |
| joint flow and functional capillary volume | D'Souza et al. 2025 | priority PCQ-C request and partial PCQ-J bridge | raw-data rights, posture details, covariance, and participant-level extract remain pending; `Vc/Q` is a derived functional residence proxy, not measured capillary transit |
| pulmonary transit | Lassen et al. 2023 | PCQ-C2 observation-model candidate | pulmonary-trunk-to-left-atrium transit is broader than capillary transit and its pulmonary blood volume is algebraically derived from flow times transit, so it cannot independently validate closure |
| dynamic or participant-level regional perfusion | Bailey et al. 2019 and Wong et al. 2014 | priority five-lobe request and four-territory dynamic backup | access and reuse terms remain pending; Wong requires a frozen observation model that combines right-middle and right-lower predictions |

The completed [PCQ-1.2 screen](PCQ1_EVIDENCE_SOURCE_SCREEN.md) and its
machine-readable candidate register capture population, posture, state,
measured variables, jointness, sample size, acquisition method, uncertainty,
access, licence, calibration overlap, cohort reuse, outcome exposure, and
acceptance or rejection reason. Published aggregates support source selection
and method context but cannot be relabelled as participant-level evidence.

### 6.2 Minimum eligibility rules

- The population and state must match a declared track or be labelled a stress
  test before evaluation.
- Measurements used together must come from the same participant and stage.
- Anatomical start and end points of transit measurements must be compatible
  with the simulated quantity or connected by a frozen observation model.
- Calibration and validation sources are disjoint by role; author, institution,
  cohort, recruitment period, protocol, and upstream reuse are reviewed.
- Access, processing, publication, redistribution, and retention rights are
  recorded before ingest.
- Restricted records remain outside the public repository. Only the schema,
  provenance/checksum record, analysis, and permitted aggregates are published.

## 7. Statistical and uncertainty plan

The validation unit is the participant when participant-level data are
available. Every eligible participant and stage is retained. Primary values are
not imputed; missingness and complete-case counts are reported per endpoint.
Age, sex, posture, acquisition method, and exercise state are descriptive
subgroups unless separately powered and predeclared.

The next amendment must state the sample-size or precision rationale. It must
also define whether each decision is based on an equivalence margin, prediction
interval coverage, measurement-error compatibility, or another reviewed rule.
A population confidence interval is never treated as an individual range.

PCQ-1 separates six uncertainty classes:

1. observational uncertainty and covariance;
2. parameter distributions and correlations;
3. structural spread among accepted pulmonary variants;
4. numerical error and convergence;
5. global sensitivity of influential parameters; and
6. identifiability of every adjustable parameter combination.

Calibration is permitted only on a dedicated calibration set. The frozen
validation candidates run without refitting. Non-identifiable parameters are
fixed by independent evidence or reported as non-identifiable; an optimizer's
single answer is not treated as unique physiology.

## 8. Controls and decision policy

Continuous integration will test at least the following negative cases:

- calibration-source reuse as validation;
- synthetic evidence submitted through the measured-evidence path;
- changed frozen assets without a protocol amendment;
- unordered or decreasing-workload trajectories;
- invalid or mutated units;
- out-of-scope population, posture, disease, or transit definitions presented
  as primary evidence; and
- deliberately broken volume-flow-transit closure.

Each track advances only when its primary endpoints have pre-outcome numeric
tolerances, eligible source-disjoint evidence, no-refit execution, uncertainty
analysis, and retained controls. Secondary failures narrow the interpretation.
Partial and negative findings remain versioned results and may motivate a new
candidate, but cannot be removed from the evaluated version.

## 9. Work packages

| Increment | Deliverable | Completion condition |
|---|---|---|
| ~~PCQ-1.1 design foundation~~ | **Completed locally:** human and machine-readable protocol, schema, frozen hashes, semantic checker, negative tests, and CI wiring | design validates and cannot claim success before outcomes |
| ~~PCQ-1.2 evidence-source screen~~ | **Completed locally:** reusable search log, thirteen-candidate machine register, corrected PVDOMICS eligibility, ranked track paths, and send-ready D'Souza request | rights, variables, jointness, methods, uncertainty, independence, public outcome exposure, and rejection reasons reviewed; no participant records acquired and no request sent |
| ~~PCQ-1.3 pre-outcome amendment~~ | **Completed locally:** selected-source roles, eight observation models, precision floors, six primary numeric gates, statistics, explicit blocked states, and analysis version | machine and human records pass their semantic and negative tests before participant-level outcome access |
| ~~PCQ-1.4 data adapters~~ | **Completed locally:** manifest-first rights/provenance gate, outside-Git quarantine boundary, four strict normalized schemas and adapters, metadata-only output, outcome-blind fixtures, and negative tests | invalid rights, paths, units, ages/states, stage tuples/order, source overlap, checksums, lobe sums, direct identifiers, and transit activation fail closed before evaluation |
| PCQ-1.5 uncertainty and identifiability | parameter distributions, structural ensemble, measurement-error propagation, convergence, sensitivity, and identifiability report | all six uncertainty classes are represented or explicitly unavailable with consequences |
| PCQ-1.6 locked execution | no-refit track reports with raw permissible artifacts and retained failures | every eligible observation, control, exclusion, and result is reproducible |
| PCQ-1.7 qualification review | bounded claims and statuses updated across evidence matrix, traceability, documentation, and shareable report | independent review agrees that wording does not exceed the evidence |

## 10. Immediate next action

PCQ-1.5 is next. Quantify the observational, parameter, structural, and
numerical uncertainty already declared by PCQ-1.1/1.3; run convergence and
sensitivity analyses; and state which parameters or claims are identifiable
from each observation family. This work begins with frozen model and synthetic
uncertainty inputs while participant-level access remains pending. It must not
weaken the PCQ-1.4 manifest-first quarantine boundary or activate the blocked
whole-pulmonary transit comparison.

## 11. Existing evidence retained

PCQ-1 builds on, but does not rewrite:

- [subject-level pulmonary multipoint validation plan](../m3/PULMONARY_0D_SUBJECT_MULTIPOINT_VALIDATION.md);
- [published-population multipoint validation](../m3/PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md);
- [independent aggregate pulmonary validation](../m3/PULMONARY_0D_INDEPENDENT_VALIDATION.md);
- [five-lobe perfusion validation](../m3/PULMONARY_LOBAR_PERFUSION_VALIDATION.md);
- [pulmonary capillary reference candidate](../m4/PULMONARY_CAPILLARY_QUALIFICATION.md); and
- [evidence and validity baseline](../publication/EVIDENCE_AND_VALIDITY_BASELINE.md).

The protocol is a research-software qualification plan. It is not prospective
clinical-trial registration and does not authorize clinical use.
