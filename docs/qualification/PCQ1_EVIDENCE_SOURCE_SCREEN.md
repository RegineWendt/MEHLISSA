<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1.2 Evidence-Source Screen

## 1. Decision and status

PCQ-1.2 is complete as a metadata, methods, access, rights, and independence
screen. It identifies actionable candidates for all four PCQ-1 tracks without
claiming that any new physiological qualification has passed.

The machine-readable authority is
`data/qualification/pulmonary-capillary-evidence-candidate-register-v1.json`,
validated against
`data/schemas/pulmonary-capillary-evidence-candidate-register/1.0.0.schema.json`.
It records thirteen source candidates, their track-specific ranking, measurement
jointness, uncertainty, access route, licence boundary, source independence,
possible cohort reuse, public outcome exposure, rejection reasons, and next
action.

This source-selection record remains frozen. The subsequent
[PCQ-1.5a repository-first data audit](PCQ1_REPOSITORY_FIRST_DATA_AUDIT.md)
binds that exact register by SHA-256 and records the later availability search,
five non-equivalent repository alternatives, and current unsent contact order
without rewriting the rankings or numeric analysis plan.

No candidate participant-level data were downloaded, received, or inspected.
No external request was sent. Article access is not permission to redistribute
participant data, and no source data acquire the repository's CC BY 4.0 licence.

## 2. Screening method

The screen prioritized primary human studies and official data repositories.
Each candidate was reviewed against:

1. population, age, health state, posture, and challenge;
2. measured variables and anatomical definitions;
3. whether required values were measured in the same participant and stage;
4. sample size, missingness, repeatability, covariance, and other uncertainty;
5. article and participant-data access;
6. processing, publication, redistribution, and retention rights;
7. separation from MEHLISSA parameterization and existing validation sources;
8. possible reuse of one cohort across publications; and
9. whether public results had already been visible before numeric tolerance
   locking.

The screen was not a systematic review or meta-analysis. It is a reproducible
candidate-selection step for a bounded qualification protocol. Search terms,
reasons, and persistent identifiers are retained in the register so that a
later reviewer can reproduce or extend the decision.

## 3. Track-level result

| Track | Priority path | Backup or context | Present conclusion |
|---|---|---|---|
| PCQ-H, participant hemodynamics | University of Arizona healthy-control iCPET | Coffman 2018 upright catheter cohort; Pandey 2020 upright historical cohort | Arizona remains the only screened candidate matching the primary supine, repeated mPAP-PAWP-cardiac-output tuple. It has only five controls and a public group age of 43 plus or minus 16 years, so individual 20-to-40-year eligibility, access, and reuse terms remain pending. |
| PCQ-R, regional perfusion | Bailey 2019 five-lobe supine/erect V/Q SPECT | Wong 2014 four pulmonary venous territories across posture and exercise; Hall 2014 right-lung spatial perfusion | No new participant-level five-lobe data are yet available. Dynamic sources require an observation model rather than pretending that four venous territories or spatial zones are five lobes. |
| PCQ-C, capillary volume and transit | D'Souza 2025 for functional `Vc` plus flow and hemoglobin; Lassen 2023 for whole-pulmonary transit and repeatability | Coffman 2017 five-stage upright `Vc`-flow-Hb replication | The two priority sources are complementary, not interchangeable. Their data and rights must be requested separately. |
| PCQ-J, joint coherence | D'Souza 2025 supports partial same-stage flow-volume coherence | Lassen 2023 supports a separate transit observation model | No screened cohort contains the complete hemodynamic, five-lobe, functional-volume, anatomically matched transit, and hematocrit state. PCQ-J therefore remains partial. |

## 4. Main findings

### 4.1 Functional capillary volume: D'Souza is the first request

D'Souza et al. studied 15 healthy nonsmoking young adults, including seven
women, at a mean age of 24 years. Visit 2 included baseline and two randomized
60 W semirecumbent exercise conditions. The multiple-inspired-oxygen DLCO
method provided functional pulmonary capillary blood volume (`Vc`) and
diffusing membrane capacity (`DM`); values were adjusted for hemoglobin.
Impedance cardiography provided cardiac output in 14 participants. A usable
tricuspid regurgitant signal allowed echocardiographic right-ventricular
systolic pressure in six participants.

This is the best screened PCQ-C candidate because age, recumbent posture,
functional volume, flow, and hematological adjustment align closely with the
frozen claim. It is not a complete PCQ-H source: it has no PAWP, no directly
measured mPAP, and only a small RVSP subset. It also does not measure capillary
transit. `Vc/Q` may be predeclared as a derived functional residence proxy, but
must never be relabelled as directly measured anatomical capillary transit.

The article is CC BY 4.0 and states that data are available from the
corresponding author on reasonable request. That statement does not define raw
data redistribution rights. A minimal, send-ready draft request is therefore
provided in [D'Souza 2025 data request](DSOUZA_2025_DATA_REQUEST.md).

### 4.2 Transit: Lassen is useful but definitionally circular for closure

Lassen et al. measured pulmonary-trunk-to-left-atrium mean bolus transit time
with rubidium-82 PET/CT in 33 healthy young adults at rest and during adenosine
stress; 25 had successful test-retest measurements. The paper reports
method-specific repeatability for transit, heart rate, stroke volume, and
derived pulmonary blood volume.

This is valuable for PCQ-C2 because it provides a defined whole-pulmonary
transit endpoint and repeatability evidence in a recumbent primary-age cohort.
However:

- pulmonary-trunk-to-left-atrium transit includes more than the capillary bed;
- tracer retention may prolong the measured transit;
- the study's pulmonary blood volume is defined as cardiac output multiplied
  by transit; and
- that derived volume therefore cannot independently validate the same
  `volume = flow x transit` identity in MEHLISSA.

PCQ-1.3 must freeze the anatomical observation model before using this source.
It can support transit compatibility and uncertainty; it cannot turn an
algebraic identity into biological validation.

### 4.3 Larger upright replication: Coffman is strong but not primary-posture

Coffman et al. 2017 measured duplicate `Vc`, diffusing capacity, cardiac output,
and earlobe capillary hemoglobin at rest and four exercise intensities in 31
healthy adults. Sixteen were aged 22-30; fifteen were aged 60-76. This is a
valuable five-stage replication and age-generalization candidate. The seated
upright posture is outside the first PCQ-1 primary context, and `Vc` and cardiac
output share a rebreathing method, creating possible correlated measurement
error.

The same Mayo research program produced a 2018 study with seven-stage invasive
mPAP, wedge pressure, and direct-Fick flow in nine catheterized healthy adults.
That cohort is a useful upright PCQ-H backup, but the two Coffman publications
must be treated as one evidence family until participant overlap is resolved.

### 4.4 Regional perfusion: five-lobe and dynamic evidence do not yet coincide

The existing Bourhis cohort already provides an independent aggregate
normal-supine comparison for all five lobes. It remains a reproduction control,
not a new PCQ-R cohort.

Bailey et al. are the closest screened next source because they report normal
five-lobe contributions after supine and erect tracer administration. Public
metadata describe only a small cohort; exact sample size, participant-level
availability, covariance, and reuse rights require full-text or author
confirmation.

Wong et al. measured pulmonary arterial and venous flows in 24 healthy adults
at prone rest, supine rest, and supine exercise. The dynamic design is strong,
but pulmonary veins provide four territories, with a separate right-middle
vein folded into the right-lower measurement. A legitimate comparison must
combine the corresponding MEHLISSA predictions in a frozen observation model.
The Hall et al. proton-MRI study supplies posture-matched rest/exercise spatial
perfusion in six healthy adults, but only for the right lung and not by lobe.

### 4.5 PVDOMICS is not the healthy invasive fallback previously assumed

The primary PVDOMICS methods paper states that healthy controls receive
non-invasive CPET, whereas invasive CPET is performed for consented pulmonary-
hypertension participants at selected centers. The official dbGaP record
provides controlled access under study `phs002451.v1.p1`, but controlled access
does not change the protocol mismatch.

PVDOMICS is therefore rejected as the initial PCQ-H healthy invasive source
unless a controlled variable inventory demonstrates eligible healthy
stage-matched mPAP, PAWP, and cardiac-output records. It remains potentially
useful for non-invasive covariates, phenotype generalization, and later disease
work.

## 5. Access, rights, and outcome-exposure boundary

The screen deliberately distinguishes four things:

- an article licence governs the publication, not automatically its raw data;
- an availability statement identifies a request route, not granted use;
- de-identification does not create redistribution permission; and
- repository-controlled data remain outside the public Git repository unless
  the grant explicitly permits redistribution.

Published abstracts, methods, group tables, and figures were visible during
the screen. This means PCQ-1 is prospectively frozen at the candidate and
endpoint level, but the next tolerances cannot honestly be called wholly blind
to all published aggregate outcomes. The mitigation is explicit and binding:

1. do not inspect participant records before PCQ-1.3;
2. derive tolerances from measurement repeatability, uncertainty propagation,
   and an independently reviewed physiological rationale;
3. never tune a tolerance to include a published candidate mean or spread;
4. preserve this exposure record in the final report; and
5. require a new independent split if any limit changes after record access.

## 6. Ordered next actions

1. Complete and commit the
   [PCQ-1.3 pre-outcome amendment](PCQ1_PRE_OUTCOME_AMENDMENT.md) before
   receiving or opening any participant-level outcome file. This is now done
   locally; the amendment freezes observation models, sample and precision
   floors, missingness, hemoglobin handling, statistical rules, and numeric
   gates.
2. Build PCQ-1.4 rights-aware source manifests, quarantine boundary, strict
   measured-data schemas, and outcome-blind fixture adapters. This is now
   complete locally and documented in the
   [data-ingress guide](PCQ1_DATA_INGRESS.md).
3. Have an authorized representative review the existing
   [UA iCPET request](../m3/UA_ICPET_DATA_REQUEST.md) and the
   [D'Souza request](DSOUZA_2025_DATA_REQUEST.md), then decide whether to send
   them under an identified institutional data controller.
4. Ask only for feasibility and reuse terms for Lassen participant-level
   transit replicates and Bailey five-lobe posture data. Lassen analysis remains
   blocked until its extra-capillary observation model is independently fixed.
5. Before any data arrive, prepare the approved PCQ-1.4 manifest and an
   institution-controlled quarantine directory outside the repository. Do not
   inspect a file that fails the manifest-first release gate.

External contact is not performed by this repository change. The responsible
institution must decide who can accept data-use terms and whether TU Berlin,
the University of Luebeck, or another legal entity is the recipient.

## 7. Primary sources

- D'Souza AW, et al. *Am J Physiol Lung Cell Mol Physiol*. 2025;328:L631-L637.
  <https://doi.org/10.1152/ajplung.00358.2024>
- Lassen ML, et al. *J Nucl Cardiol*. 2023;30:2504-2513.
  <https://doi.org/10.1007/s12350-023-03308-1>
- Coffman KE, et al. *J Appl Physiol*. 2017;122:1425-1434.
  <https://doi.org/10.1152/japplphysiol.00694.2016>
- Coffman KE, et al. *Physiol Rep*. 2018;6:e13565.
  <https://doi.org/10.14814/phy2.13565>
- Elliott J, et al. *J Am Heart Assoc*. 2023;12:e029667.
  <https://doi.org/10.1161/JAHA.123.029667>
- Pandey A, et al. *JACC Heart Fail*. 2020;8:111-121.
  <https://doi.org/10.1016/j.jchf.2019.08.020>
- Bailey DL, et al. *Semin Nucl Med*. 2019;49:58-61.
  <https://doi.org/10.1053/j.semnuclmed.2018.10.008>
- Wong DTH, et al. *J Physiol Sci*. 2014;64:105-112.
  <https://doi.org/10.1007/s12576-013-0298-z>
- Hall ET, et al. *J Appl Physiol*. 2014;116:451-461.
  <https://doi.org/10.1152/japplphysiol.00659.2013>
- Tang WHW, et al. *Circ Heart Fail*. 2020.
  <https://doi.org/10.1161/CIRCHEARTFAILURE.119.006363>
- PVDOMICS official dbGaP record, accession `phs002451.v1.p1`.
  <https://www.ncbi.nlm.nih.gov/projects/gap/cgi-bin/study.cgi?study_id=phs002451.v1.p1>

This screen supports research planning only. It does not establish clinical
validity, patient-specific prediction, or permission to use any restricted
record.
