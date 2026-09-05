<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Draft Data Request - Bailey 2019 Five-Lobe V/Q SPECT Cohort

## Status

This is a send-ready draft, not a record of a submitted request or granted
access. An authorized project representative must review the institutional,
ethical, and data-use wording before sending it. No participant-level file may
be opened until a PCQ-1.4 rights/provenance manifest is approved and the file
resolves outside Git inside the declared institution-controlled quarantine
root.

## Suggested recipient

Professor Dale L. Bailey. PubMed lists `Dale.Bailey@sydney.edu.au` as the
electronic address for the article. Verify that address on an institutional or
publisher page immediately before sending.

## Suggested subject

Academic request for de-identified participant-level five-lobe V/Q SPECT data
from Bailey et al. 2019

## Suggested message

Dear Professor Bailey,

We are developing MEHLISSA, an open research simulator for multiscale medical
transport and physiological scenarios. We have prospectively frozen a
no-refit qualification protocol for a five-lobe pulmonary perfusion model.

Your 2019 article, "V/Q SPECT-Normal Values for Lobar Function and Comparison
With CT Volumes," is especially relevant because it reports functional
contributions for all five lung lobes in normal subjects and distinguishes
supine from erect radiotracer administration while retaining the supine SPECT/
CT acquisition position.

Would it be possible to receive a minimal de-identified participant-level
extract for non-commercial academic validation? For each normal subject and
available condition, we request only:

- a study-specific pseudonym unrelated to clinical identifiers;
- age in whole years or an approved age band, sex, height, body mass, and body
  surface area or body mass index when already available;
- radiotracer administration posture and the separate scan-acquisition
  posture;
- the relative perfusion contribution of the right upper, right middle, right
  lower, left upper, and left lower lobes;
- the corresponding five-lobe ventilation contribution and CT anatomical
  volume fractions, if available;
- the denominator and normalization rule used for every fraction;
- segmentation, reconstruction, and quality-control identifiers needed to
  interpret the values;
- repeat measurements, uncertainty estimates, or quality flags when
  available; and
- per-field units, missingness codes, and a concise data dictionary.

We do not request names, dates of birth, examination dates, contact
information, locations, clinical identifiers, imaging files, free text, or a
key capable of reconnecting the pseudonyms to individuals. A less granular
extract is acceptable if required by the original ethics approval.

The primary analysis will compare the frozen model with participant-level
five-lobe perfusion fractions in the eligible healthy, resting, supine
condition. Erect administration and ventilation or CT-derived fractions will
be retained as explicitly labelled secondary or stress-test evidence. We will
not refit the model to these observations, select lobes after seeing results,
or present the study as clinical validation.

Could you also confirm:

- the number of participants for whom all five perfusion fractions and posture
  metadata remain available;
- whether ventilation, perfusion, CT volume, and both posture conditions can be
  linked within the same pseudonymous participant and visit;
- whether the normal cohort or source images overlap any other published
  cohort that we should treat as the same evidence family;
- the permitted processing, retention period, and security requirements;
- whether publication of non-identifying aggregate statistics, uncertainty
  summaries, and residual plots is allowed;
- whether redistribution of the extract is prohibited or permitted under
  stated terms; and
- the required citation, acknowledgment, data-use agreement, and institutional
  signatory.

If redistribution is not permitted, we will keep the source records outside
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
- select the legal recipient institution and its authorized DUA signatory;
- verify the recipient address rather than relying only on the publication;
- independently review PCQ-1.3 and the PCQ-1.4 ingress contract;
- prepare the exact rights/provenance manifest and outside-repository
  quarantine directory before accepting a file;
- request institution-approved secure delivery rather than an ordinary email
  attachment; and
- record only administrative request metadata, the response, the grant, and
  any retention deadline in a restricted project register.

## Source

Bailey DL, Farrow CE, Lau EM. V/Q SPECT-Normal Values for Lobar Function and
Comparison With CT Volumes. *Seminars in Nuclear Medicine*. 2019;49:58-61.
<https://doi.org/10.1053/j.semnuclmed.2018.10.008>

The publication and its indexed contact address do not by themselves grant
participant-data access or redistribution rights.
