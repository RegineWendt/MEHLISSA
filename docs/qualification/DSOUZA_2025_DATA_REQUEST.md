<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Draft Data Request - D'Souza 2025 Pulmonary Capillary Cohort

## Status

This is a send-ready draft, not a record of a submitted request. An authorized
project representative must review the institutional, ethical, and data-use
wording before sending it to the corresponding author named in the article.
PCQ-1.3 is committed. No participant-level data should be opened unless the
PCQ-1.4 rights/provenance manifest is approved and the file resolves outside
Git inside the explicitly declared institutional quarantine root.

## Suggested subject

Request for minimal de-identified stage-level pulmonary capillary and flow data
from D'Souza et al. 2025

## Suggested message

Dear Professor Stickland,

We are developing MEHLISSA, an open research simulator for multiscale medical
transport and physiological scenarios. We are preparing a prospective,
no-refit qualification of a frozen equivalent pulmonary capillary model.

Your 2025 article, "Pulmonary capillary blood volume and diffusing membrane
capacity during exercise in humans: role of pulmonary artery pressure,"
reports measurements that are particularly relevant because pulmonary
capillary blood volume, cardiac output, and hemoglobin adjustment were obtained
in the same young healthy participants and experimental states.

Would it be possible to receive a minimal de-identified stage-level extract for
non-commercial academic validation? We request only the following fields when
available:

- a study-specific pseudonym unrelated to clinical identifiers;
- sex, age in whole years or an approved age band, height, body mass, and body
  surface area if already derived;
- condition and order: baseline, 60 W control exercise, or 60 W cuff exercise;
- posture and workload;
- cardiac output, heart rate, and stroke volume;
- DLCO, functional pulmonary capillary blood volume (`Vc`), and diffusing
  membrane capacity (`DM`);
- measured hemoglobin concentration, the units, sampling method, and the exact
  hemoglobin correction applied to `Vc` and `DM`;
- the three inspired-oxygen measurements or fitted slope/intercept, fit
  quality, and replicate values used to derive `Vc` and `DM`, if these can be
  shared;
- right-ventricular systolic pressure and its inputs for the six participants
  with an adequate tricuspid regurgitant signal, clearly marked missing for
  others;
- per-field units, missingness codes, quality flags, and any study-supplied
  repeatability or measurement-error estimates; and
- a data dictionary sufficient to distinguish measured, adjusted, and derived
  values.

We do not request names, initials, dates of birth, examination dates, contact
information, locations, free text, imaging files, clinical identifiers, or a
key capable of reconnecting pseudonyms to individuals. A less granular extract
is acceptable if required by your ethics approval.

The intended primary analysis compares the frozen model's functional
pulmonary capillary volume with the observed `Vc` distribution and examines
same-stage flow-volume compatibility. Any `Vc/cardiac output` quantity will be
reported only as a derived functional residence proxy, not as directly measured
anatomical capillary transit. RVSP will remain an exploratory surrogate and
will not be presented as invasive mPAP. We will not refit the frozen model to
these records.

Could you also confirm:

- whether this cohort overlaps participants in prior University of Alberta
  publications on exercise `Vc`, `DM`, or intrapulmonary arteriovenous
  anastomoses;
- whether every shared variable was obtained in the same visit and state;
- whether the baseline measurements were made in the same semirecumbent
  posture as the exercise conditions;
- the permitted processing, retention period, and security requirements;
- whether publication of derived aggregate statistics and residual plots is
  allowed;
- whether redistribution of the de-identified extract is prohibited or can be
  permitted under stated terms; and
- the required citation, acknowledgment, data-use agreement, and institutional
  signatory.

If raw redistribution is not permitted, we will keep the source records outside
the public repository and publish only the schema, provenance/checksum record,
analysis code, and permitted non-identifying aggregate results.

Thank you for considering this request.

Kind regards,

[Name]
[Role and institution]
[Institutional address]
[Email]
[Project URL]

## Before sending

- replace all bracketed fields;
- select the legal recipient institution;
- confirm who may accept a data-use agreement;
- independently review the committed PCQ-1.3 amendment and PCQ-1.4 ingress
  contract before data delivery or outcome access;
- prepare the exact rights/provenance manifest and outside-repository
  quarantine directory before accepting a file;
- request secure delivery rather than ordinary email attachment if records are
  granted; and
- record the sent date, response, grant, and retention deadline in a non-public
  administrative register.

## Source

D'Souza AW, Brotto AR, Hicks B, et al. Pulmonary capillary blood volume and
diffusing membrane capacity during exercise in humans: role of pulmonary
artery pressure. *American Journal of Physiology-Lung Cellular and Molecular
Physiology*. 2025;328:L631-L637.
<https://doi.org/10.1152/ajplung.00358.2024>

The article states that data are available from the corresponding author on
reasonable request and is licensed CC BY 4.0. Those article terms do not by
themselves grant participant-data redistribution rights.
