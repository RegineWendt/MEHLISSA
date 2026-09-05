<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1.5a Repository-First Data Audit

## 1. Decision in one paragraph

A repository-first search was completed before any newly located participant
file was downloaded or opened. Public and controlled repositories do contain
useful human hemodynamic, exercise, posture, and lung-imaging data, but no
located object is a drop-in replacement for the participant-level evidence
required by the frozen PCQ-1.3 endpoints. The primary access plan therefore
remains: D'Souza and the University of Arizona first, Bailey next, and Lassen
only after the whole-pulmonary transit observation model is independently
resolved. Five repository-backed alternatives are retained for supplementary,
method, generalization, or later stress-test protocols; none was promoted into
the current primary qualification.

The machine-readable authority is
`data/qualification/pulmonary-capillary-repository-audit-v1.json`, checked
against
`data/schemas/pulmonary-capillary-repository-audit/1.0.0.schema.json`.
It binds the unchanged PCQ-1.2 candidate register by SHA-256 instead of editing
the already frozen source selection.

## 2. Why this audit is separate

PCQ-1.2 ranked sources, PCQ-1.3 locked observation models and numeric rules,
PCQ-1.4 established rights-aware ingress, and PCQ-1.5 established uncertainty
and identifiability handling. Rewriting the PCQ-1.2 register after those steps
would break the prospective chain. PCQ-1.5a is therefore an addendum that asks
a narrower operational question:

> Can an existing public or controlled repository provide the required data
> without a direct author request, and can useful non-equivalent data be
> identified without inspecting outcomes?

The answer is **no** for an equivalent primary source and **yes** for useful
non-equivalent complements.

## 3. Search and outcome boundary

The 5 September 2026 audit used exact DOI/title, author/cohort, endpoint, and
population searches across Zenodo, Dryad, Figshare, OSF, PhysioNet, NCBI dbGaP,
BioLINCC, Harvard Dataverse, DataCite metadata, publisher supplements, and
institutional repositories. Search terms covered:

- healthy invasive exercise hemodynamics with stage-matched mean pulmonary
  arterial pressure, pulmonary arterial wedge pressure, and cardiac output;
- five-lobe or dynamic regional pulmonary perfusion;
- functional pulmonary capillary blood volume and hematological covariates;
- anatomically defined pulmonary or capillary transit; and
- healthy posture, exercise, cardiac-output, lung-water, and multimodal
  alternatives.

Permitted inspection was limited to repository metadata, persistent IDs,
access controls, licence fields, file names and types, checksums, publication
methods and data-availability statements, and public aggregate summaries. No
newly located participant file was downloaded, opened, parsed, or evaluated.
No participant-level outcome was inspected and no request was sent. Landing
pages sometimes displayed aggregate results; that exposure is recorded and
was not used to alter any frozen limit.

## 4. Audit of the intended sources

| Frozen candidate | Repository finding | Consequence |
|---|---|---|
| University of Arizona iCPET, DOI `10.1161/JAHA.123.029667` | [Zenodo record 7789925](https://zenodo.org/records/7789925) and the [GitHub repository](https://github.com/vanderpoolrr/iCPET_calculator) contain calculator software, not healthy-control trajectories. The [article](https://pmc.ncbi.nlm.nih.gov/articles/PMC10757516/) routes clinical data to reasonable request. | The prepared Arizona request remains necessary for primary participant-level hemodynamics. |
| Bailey 2019, DOI `10.1053/j.semnuclmed.2018.10.008` | The [publication record](https://pubmed.ncbi.nlm.nih.gov/30545518/) was located, but no public participant dataset or reusable numerical supplement was found. | Bailey remains the preferred five-lobe access inquiry. |
| D'Souza 2025, DOI `10.1152/ajplung.00358.2024` | The [official article](https://journals.physiology.org/doi/full/10.1152/ajplung.00358.2024) states that data are available from the corresponding author on reasonable request; no linked public participant dataset was located. | Direct contact remains necessary and has highest priority because this source can connect functional capillary volume with stage-matched flow. |
| Coffman 2017, DOI `10.1152/japplphysiol.00694.2016` | [White Rose Research Online](https://eprints.whiterose.ac.uk/id/eprint/114363/) provides an accepted manuscript, not participant data. | Retain as a backup after the primary joint-volume request. |
| Lassen 2023, DOI `10.1007/s12350-023-03308-1` | The [Springer article](https://link.springer.com/article/10.1007/s12350-023-03308-1) links two supplements. They contain aggregate figures and presentation material, not a table or participant records. | Access still needs author contact, and access alone cannot remove the frozen whole-pulmonary-versus-capillary transit blocker. |

DataCite related-identifier searches did not identify a target dataset for
these studies. Two apparent Coffman hits were unrelated deposits that cited
the article rather than its data. Similarly, literature-index records that
label a PubMed Central article as a “dataset” were not counted as participant
data.

## 5. Repository-backed alternatives

| Source | What is genuinely available | Appropriate role | Why it is not a replacement |
|---|---|---|---|
| [Posture matters](https://zenodo.org/records/19330868), Zenodo `10.5281/zenodo.19330868` | One openly downloadable spreadsheet for 12 healthy adults across upright, semi-recumbent, and supine exercise, with cardiac output by thoracic bioimpedance, stroke volume, heart rate, gas exchange, and workloads. No licence was displayed at the audit cutoff, so reuse remains blocked pending clarification. | Strong supplementary posture and cardiac-output generalization candidate after a new pre-outcome activation decision. | No invasive mean pulmonary arterial or wedge pressure, so the primary PCQ-H tuple cannot be computed. |
| [HeartCycle](https://physionet.org/content/heartcycle/1.0.0/), PhysioNet `10.13026/z865-eb23` | Open ODC-By-1.0 multimodal ICG, echocardiography, ECG, PPG, heart-sound, and derived hemodynamic records from 17 healthy volunteers. | Cardiac-output observation-method and healthy-variability evidence. | No invasive pulmonary arterial or wedge pressure tuple and no established match to the frozen exercise protocol. |
| [SCG-RHC](https://physionet.org/content/scg-rhc-wearable-database/1.0.0/), PhysioNet `10.13026/133d-pk11` | Open ODC-By-1.0 invasive pulmonary arterial pressure, wedge pressure, Fick/thermodilution cardiac output, ECG, and seismocardiography for 73 heart-failure referrals at rest and under vasodilators. | Later disease/method stress test for the pressure-flow adapter and uncertainty model. | Heart-failure referral population and pharmacological perturbation are outside the healthy exercise claim. |
| [Exercise lung-water study](https://www.ncbi.nlm.nih.gov/projects/gap/cgi-bin/study.cgi?study_id=phs003346.v1.p1), dbGaP `phs003346.v1.p1` | Controlled MRI exercise-lung-water study with 12 healthy volunteers and two heart-failure participants. | Future lung-water, gas-exchange, or imaging extension. | Extravascular lung water is not a frozen PCQ-1 endpoint. |
| [PVDOMICS](https://www.ncbi.nlm.nih.gov/projects/gap/cgi-bin/study.cgi?study_id=phs002451.v1.p1), dbGaP `phs002451.v1.p1` | Official controlled deep-phenotyping study with healthy controls and disease groups. | Later non-invasive or disease generalization after variable and cohort-overlap review. | The published protocol specifies non-invasive CPET for healthy controls and does not establish the required healthy invasive trajectory. |

These records prove an important distinction: a dataset can be technically
available yet physiologically ineligible, or physiologically attractive yet
unavailable without a data-use agreement. “In a repository” is therefore not
an evidence grade.

## 6. Frozen decision and contact order

No source role, endpoint, observation model, sample floor, covariance rule, or
numeric tolerance changes through this audit. The contact queue is:

1. D'Souza for joint functional capillary volume, flow, hematological
   covariates, covariance, and reuse terms;
2. University of Arizona for healthy supine invasive multipoint hemodynamics;
3. Bailey for participant-level five-lobe perfusion; and
4. Lassen only for feasibility after the anatomical transit observation model
   has independent support.

An authorized institutional representative must review and send each request.
This repository change sends none.

## 7. Rule for any alternative dataset

Do not download or inspect an alternative participant file for PCQ evaluation
merely because it is open. Before activation:

1. create a successor pre-outcome amendment naming its source role, endpoint,
   population, posture, observation model, licence, uncertainty/covariance
   treatment, overlap decision, and numeric rule;
2. obtain independent review of that amendment;
3. pass the PCQ-1.4 rights/provenance manifest and outside-repository ingress
   boundary; and
4. execute without refitting, retaining all exclusions, failures, missingness,
   and partial results.

PCQ-1.6 remains the next scientific increment. It stays
`blocked-access-or-rights` until an eligible selected source is authorized and
passes ingress. The new repository alternatives can shorten future work, but
they do not manufacture the missing primary evidence.
