<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MRSQ-1.2 Prospective Healthy-Adult Dynamic FDG-PET Protocol

## 1. Decision and timing

MRSQ means **Medical Reference Scenario Qualification**. MRSQ-1.2 freezes the
first complete medical-reference-scenario evaluation before any HedyPET
participant file or candidate validation outcome is opened. It converts the
[MRSQ-1.1 data-first selection](MRSQ1_SCENARIO_SELECTION.md) into an exact,
machine-checked intended-use, data, analysis, uncertainty, and claim protocol.

> **Historical freeze notice:** This document intentionally preserves the state
> before implementation and outcome access. MRSQ-1.3 through MRSQ-1.7 have since
> completed the computational path without opening HedyPET participant outcomes.
> The [bounded result](MRSQ1_QUALIFICATION_RESULT.md) is the current status
> authority; measured cohort execution and external human review remain blocked.

At this point:

- no participant row, input-function value, time-activity value, volume, SUV,
  SUL, or Patlak value has been opened or analysed;
- no FDG transport, uptake, trapping, renal-excretion, radioactive-decay, or
  PET observation candidate exists;
- data ingress remains unauthorized; and
- no validation endpoint has a result.

The current permitted statement is therefore about prospective discipline, not
physiological success.

## 2. Intended use

The frozen question is:

> Can a frozen, source-disjoint MEHLISSA candidate reproduce cohort-level
> healthy-adult dynamic total-body [18F]FDG PET observations from a declared
> bolus injection through vascular delivery, lung, liver, kidney, and urinary-
> bladder behavior without validation refitting?

The target population is the released HedyPET healthy-adult cohort with the
source-defined 70-minute dynamic acquisition. The initial expected maximum is
80 released participants. The decision unit is a cohort endpoint: participant-
first calculations preserve variability, but MRSQ-1 does not classify or make a
prediction about an individual.

The result is limited to the frozen tracer, acquisition, scanner family,
population, regions, and transformations. It is not a normative clinical
reference range, medical device evaluation, disease model, diagnosis, treatment
recommendation, safety result, or validation of MEHLISSA's FP9 nanodevices,
VEGF mechanism, CD95 mechanism, or molecular communication.

## 3. Frozen evidence identities

### 3.1 Publication and data snapshot

The cohort and acquisition authority is Hinge et al.,
[doi:10.1038/s41597-026-08157-4](https://doi.org/10.1038/s41597-026-08157-4),
published 24 August 2026. The article describes 100 age- and sex-stratified
healthy adults, a 70-minute dynamic acquisition, and an initial release of 80
participants.

The minimum-data source is the public, ungated Hugging Face dataset
`DEPICT-RH/Multimodal-HC`, DOI
[10.57967/hf/9560](https://huggingface.co/datasets/DEPICT-RH/Multimodal-HC),
at exact repository revision:

```text
b6daf89015481bdc20a238866df730f90274157d
```

The public metadata API reported 9,463 repository objects and `cc-by-4.0` at the
time of the freeze. Repository revision, exact path, byte size, and Git blob
identity are bound in the machine protocol. A SHA-256 checksum is deliberately
not fabricated from metadata: MRSQ-1.3 must calculate and verify the content
SHA-256 after an authorized selective download and before the first parse.

### 3.2 Minimum data, not the 302 GB collection

Only six objects are in scope. The first two are dataset/schema metadata; the
remaining four contain participant-level information and remain blocked until
MRSQ-1.3.

| Frozen path | Bytes | Purpose | Current access state |
|---|---:|---|---|
| `train/dataset_description.json` | 1,202 | dataset/BIDS/licence metadata | metadata inspection permitted |
| `train/participants.json` | 2,137 | participant-field dictionary, not rows | metadata inspection permitted |
| `train/derivatives/readouts/metadata.csv` | 14,682 | acquisition, injection, normalization, and eligibility fields | blocked |
| `train/derivatives/readouts/input_functions.csv` | 6,517,065 | vascular input observations | blocked |
| `train/derivatives/readouts/tacs.csv` | 239,084,285 | organ and tissue dynamic observations | blocked |
| `train/derivatives/readouts/volumes.csv` | 4,078,118 | region composition and bladder amount transformation | blocked |

NIfTI, CT, MRI, listmode, and reconstruction objects are excluded. Static
`means.csv`, the 382,470,712-byte `patlak_ki.csv`, interactive explorer output,
and the unreleased 20-participant test set are also excluded from the primary
qualification. This reduces the potential acquisition from about 302 GB to
about 250 MB without weakening the primary dynamic question.

The full-image PublicnEUro record
[doi:10.70883/UYAG3430](https://datacatalog.publicneuro.eu/dataset/PN000015%20multimodal%20total-body%20dynamic%2018F-FDG%20PET-CT-MRI/V1)
requires an identified user and Data User Agreement. It is neither needed nor
authorized for MRSQ-1. The method repository is frozen at Git commit
`b8e04e56934a2a12d87bbaa2258a1c1f8e250d15`; because no repository licence was
found, its code may be inspected for method provenance but not copied, executed,
or redistributed as part of MEHLISSA.

## 4. Source separation

HedyPET has three deliberately separate roles:

1. administration and acquisition metadata may become locked candidate inputs;
2. the image-derived input function is an external observation in the primary
   closed-loop track and an unchanged forcing input in a separately labelled
   diagnostic track; and
3. organ time-activity curves are validation outcomes only.

No HedyPET input function, organ curve, region value, SUV/SUL value, Patlak
value, or interactive display may select equations, tune parameters, choose
regions, relax limits, or exclude a participant. Model equations, parameters,
and any calibration must come from source-disjoint literature, analytical
constraints, or a separately identified cohort and are frozen later in
MRSQ-1.5.

The predeclared construction and uncertainty context includes:

- Liu et al. 2021, [doi:10.1007/s00259-020-05124-y](https://doi.org/10.1007/s00259-020-05124-y),
  for source-disjoint healthy-organ two-tissue FDG kinetic context;
- van Sluis et al. 2025,
  [doi:10.3389/fnume.2025.1556848](https://doi.org/10.3389/fnume.2025.1556848),
  for long-axial-field-of-view image-derived-input-function uncertainty;
- Gutschmayer et al. 2025,
  [doi:10.1038/s41597-025-05997-4](https://doi.org/10.1038/s41597-025-05997-4),
  for source-disjoint healthy-organ test/retest and segmentation variability;
  and
- NIST's fluorine-18 review,
  [doi:10.6028/jres.119.013](https://doi.org/10.6028/jres.119.013), for the
  physical decay identity.

These sources justify model families and uncertainty categories. They do not
silently supply HedyPET-specific uncertainty or authorize unreported parameter
choices.

## 5. Three analysis tracks

| Track | Input | Role | Claim boundary |
|---|---|---|---|
| `MRSQ-AS-CLOSED` | recorded administration, acquisition, body-size, and frame metadata only | primary injection-to-PET evaluation | required for complete MRSQ qualification |
| `MRSQ-AS-CONDITIONAL` | unchanged observed image-derived input function after candidate freeze | diagnostic isolation of organ kinetics | cannot validate injection-to-vascular delivery or the complete path |
| `MRSQ-AS-AUDIT` | frame, unit, decay, region, volume, and optional derived-readout metadata | transformation and integrity checks | no physiological qualification |

The conditional track is scientifically useful because it separates an error in
vascular delivery from an error in an organ model. It is not a rescue mechanism:
a conditional pass cannot compensate for a failed closed-loop endpoint.

## 6. Frozen observation models

### 6.1 Vascular input function

The closed candidate predicts a continuous whole-blood-equivalent arterial
activity concentration. The comparison uses one aortic image-derived input
series selected by source semantics before values are parsed. The prediction is
integrated over each source frame and divided by its duration. The adapter must
match the source decay-reference time and plasma/whole-blood convention; a
second decay correction is forbidden.

### 6.2 Organ time-activity curves

The predicted PET-visible region concentration includes the prospectively
frozen vascular, reversible, and trapped contributions. It is volume-weighted
across source components and frame-averaged on the original source intervals.
No point sampling, result-driven smoothing, interpolation, erosion, frame
selection, or region selection is allowed.

### 6.3 Urinary-bladder activity amount

The source bladder concentration is multiplied by its frozen source volume to
form PET-visible activity amount in Bq. This makes an excretion sink observable
without pretending that radioactive activity directly identifies chemical mass.

### 6.4 Patlak, SUV, and SUL

Patlak Ki and static SUV/SUL are secondary, nongating audit quantities. A
Patlak comparison becomes permissible only when the exact input-function
choice, frame subset, unit, weighting, and transformation can be independently
reproduced. It must never be used to fit the dynamic candidate.

## 7. Frozen regions

Primary regions are fixed before outcome access:

- **lung:** volume-weighted five-lobe composite of left upper, left lower,
  right upper, right middle, and right lower lobes;
- **liver:** one non-eroded whole-liver region;
- **kidney:** volume-weighted left-plus-right kidney composite; and
- **urinary bladder:** one non-eroded bladder region with a valid volume.

A missing pulmonary lobe or kidney side makes that participant incomplete for
the corresponding endpoint. Four-lobe, unilateral, eroded, or residual-selected
substitutes are not allowed. MRSQ-1.3 must verify the exact source label mapping
against the metadata dictionary before it is allowed to parse outcome values.

## 8. Five primary endpoints and limits

Every primary decision uses the closed-loop track and at least 60 complete
released participants. The limits are conservative **engineering qualification
gates chosen before HedyPET outcomes**, not claims about HedyPET measurement
error. They are deliberately wider for renal and bladder behavior, where
excretion, segmentation, and physiological variability are stronger.

| Endpoint | Observation | Upper 90% CI of cohort median duration-weighted NRMSE | 90% CI of geometric-mean AUC ratio |
|---|---|---:|---:|
| `MRSQ-P1` | aortic input function | <= 0.25 | 0.75 to 1.25 |
| `MRSQ-P2` | five-lobe lung TAC | <= 0.30 | 0.70 to 1.30 |
| `MRSQ-P3` | liver TAC | <= 0.30 | 0.70 to 1.30 |
| `MRSQ-P4` | bilateral-kidney TAC | <= 0.35 | 0.65 to 1.35 |
| `MRSQ-P5` | urinary-bladder activity amount | <= 0.40 | 0.60 to 1.40 |

MRSQ-P1 also requires the upper 90% confidence limit of the cohort median
absolute peak-frame-midpoint error not to exceed the larger of ten seconds and
one source peak-frame duration. Organ peak timing is descriptive because source
frame durations change and liver/kidney/bladder structures introduce additional
timing ambiguity.

For each participant, duration-weighted NRMSE is the duration-weighted root-
mean-square residual divided by the largest absolute observed value. A zero or
nonfinite denominator blocks that participant-endpoint. AUC uses trapezoidal
integration over identical accepted frame midpoints; positive participant AUC
ratios are aggregated on the natural-log scale.

## 9. Cohort statistics and missingness

Participant metrics are calculated first. Cohort medians and geometric means
use deterministic two-sided 90% percentile-bootstrap intervals with 10,000
replicates and seed `18042026`.

Primary values are not imputed. Every endpoint reports:

- total released records encountered;
- eligibility count and each exclusion reason;
- complete-case count;
- every retained participant metric through an opaque run identifier;
- cohort estimate and interval; and
- every passed, failed, partial, or blocked gate.

Age, source-declared sex, and body-size variables are descriptive only. There
is no subgroup qualification, threshold adjustment, causal demographic claim,
or individual classification. All five endpoints and both required metrics must
pass; an endpoint cannot compensate for another endpoint, and there is no post-
outcome reprioritization.

## 10. Uncertainty plan

Seven uncertainty classes are mandatory:

1. **Numerical:** independent solver-step and synchronization/frame-integration
   convergence; numerical error may consume at most one tenth of the narrowest
   endpoint tolerance.
2. **Input function:** aortic-region, early spillover/partial volume, plasma/
   whole-blood, and late bias; a published late IDIF bias below ten percent is
   not assumed for the early peak.
3. **PET reconstruction:** exact reconstruction, frame schedule, unit, and decay
   reference; the documented `sub-017` reconstruction deviation is retained as
   a flag/sensitivity stratum rather than silently excluded.
4. **Segmentation and partial volume:** the non-eroded primary composition is
   frozen; erosion or partial-volume variants are structural sensitivities only.
5. **Parameter:** every parameter needs source-disjoint provenance, sensitivity,
   and any genuinely supported joint distribution.
6. **Structural:** irreversible/reversible trapping, liver dual input, renal
   excretion, and vascular-fraction alternatives are reported against one
   prospectively frozen reference.
7. **Cohort:** fixed participant-first estimates, intervals, missingness, and
   descriptive demographic views without individual inference.

Unknown correlations remain unknown. They are not replaced with invented
independence assumptions merely to enable a probabilistic analysis.

## 11. Decision and amendment rules

`PASS` requires at least 60 complete participants for every primary endpoint,
all NRMSE and AUC gates, the MRSQ-P1 timing gate, critical controls, and a frozen
MRSQ-1.5 candidate run without validation refitting.

`FAIL` records an evaluable primary numeric gate outside its limit.
`PARTIAL_EVIDENCE` applies to 20-59 complete participants or diagnostic/
secondary evidence without the full primary set. `BLOCKED` applies below 20
complete participants, or when rights, governance, identity, units, frames,
regions, candidate freeze, or a critical control are unresolved.

After any participant outcome is opened, this protocol is immutable. A necessary
change creates a new retained version and declares prior outcome exposure. It
cannot retroactively rescue MRSQ-1.

## 12. Governance and data boundary

The Hugging Face metadata says the selected snapshot is public and ungated,
but technical availability does not by itself answer the local institutional
governance question. Before a participant-level CSV is opened, MRSQ-1.3 must:

- record the local decision for reuse of openly advertised, deidentified
  participant-level research data;
- confirm licence, revision, paths, byte sizes, blob identities, and then content
  SHA-256 values;
- approve an explicit outside-repository quarantine root, responsible person,
  access list, purpose, review date, retention, and deletion trigger;
- reject direct identifiers and unexpected identifying columns;
- map source pseudonyms to run-scoped opaque identifiers;
- ensure no participant row or raw observation reaches terminal, CI, application
  logs, Git, or committed reports; and
- demonstrate all failure paths using arbitrary synthetic fixtures first.

The protocol does not assert that formal ethics approval is required or not
required. It requires the responsible institution to make and record that
determination before ingress.

## 13. Controls

Sixteen machine-checked negative controls cover parent and remote identity,
premature ingress, identifier/log leakage, unknown frame/unit/decay semantics,
point sampling, double decay correction, validation-as-calibration, misuse of
the conditional track, post-outcome mutation, residual-based exclusions,
result-driven region choice, incomplete lobe/kidney substitution, static or
Patlak replacement of the dynamic endpoints, invented uncertainty, and clinical
overclaiming.

## 14. Next increment

MRSQ-1.3 is a **rights-aware selective FDG-PET data-ingress boundary**. It will
initially be implemented and tested against synthetic fixtures only. It may
inspect the two small metadata/schema objects, but participant CSV access stays
disabled until the manifest and local governance decision satisfy this protocol.

The machine authority is
`data/qualification/medical-reference-scenario-protocol-v1.json`, validated by:

```powershell
python scripts/check_medical_reference_scenario_protocol.py
python -m unittest tests/test_medical_reference_scenario_protocol.py
```

Passing these checks proves prospective integrity. It does not prove that data
ingress is authorized, an FDG model exists, or any physiological endpoint passes.
