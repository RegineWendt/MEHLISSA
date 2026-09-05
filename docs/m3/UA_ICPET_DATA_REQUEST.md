<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Draft Data Request – UA iCPET Healthy Controls

## Status

This is a send-ready draft, not a record of an already submitted request. An
authorized project representative must review the institutional, ethical, and
data-use wording before sending it to the corresponding author listed in
Elliott et al. 2023.

PCQ-1.3 is committed. No participant-level data should be opened unless the
PCQ-1.4 rights/provenance manifest is approved and the file resolves outside
Git inside the explicitly declared institutional quarantine root.

## Suggested subject

Request for de-identified stage-level hemodynamics from the healthy-control
iCPET cohort (JAHA 2023, e029667)

## Suggested message

Dear Professor Vanderpool,

We are developing MEHLISSA, an open research simulator for multiscale medical
transport and physiological scenarios. We are currently validating a locked,
zero-dimensional pulmonary circulation model against an independent
subject-level exercise data set.

Your 2023 JAHA article on the iCPET calculator reports five healthy controls
with multipoint supine exercise hemodynamics. We would like to ask whether a
minimal, de-identified stage-level extract for those controls could be made
available for non-commercial academic validation.

For each control, we need only:

- a study-specific pseudonym unrelated to clinical identifiers;
- confirmation of healthy-control status;
- body position and cardiac-output method;
- stage order and workload in watts;
- cardiac output, mean pulmonary arterial pressure, and pulmonary arterial
  wedge pressure for each jointly measured stage; and
- if available, heart rate plus systolic and diastolic pulmonary arterial
  pressure.

We would also appreciate confirmation that these controls were not drawn from
the Toronto cohorts reported by Wright/Bentley or the Leuven cohort reported by
Claessen, which MEHLISSA already uses for other evidence roles.

We do not request names, dates, contact information, clinical identifiers,
free text, or any key capable of reconnecting the pseudonyms to individuals.
The model parameters and analysis protocol will be locked before we inspect the
measurements. The records will be used only for validation, not calibration.

Could you also specify the applicable data-use terms, whether redistribution of
the de-identified extract or only derived aggregate results is permitted, and
the required citation/acknowledgment? We are happy to sign an appropriate data
use agreement and to keep the source records outside the public repository if
redistribution is not permitted.

The intended analysis compares each observed mPAP–cardiac-output trajectory
with the pre-specified model prediction while applying measured PAWP as a
boundary condition. It reports per-stage residuals, pressure-flow slopes, PVR,
and, when pulsatile pressures and heart rate are available, compliance and RC
time. No patient-specific or clinical claims will be made.

Thank you for considering this request.

Kind regards,

[Name]
[Role and institution]
[Institutional address]
[Email]
[Project URL]

## Before sending

- replace all bracketed fields;
- confirm who is authorized to accept a data-use agreement;
- decide whether TU Berlin, University of Lübeck, or another institution is the
  formal recipient;
- attach or link the locked analysis protocol only after project review;
- review the PCQ-1.4 ingress contract, prepare its exact manifest, and create
  the institution-controlled outside-repository quarantine directory before
  accepting any file; and
- record the sent date and response in a non-public project register, not in a
  subject-data file.

## Source

Elliott J, Menakuru N, Martin KJ, et al. *Journal of the American Heart
Association*. 2023;12:e029667.
<https://doi.org/10.1161/JAHA.123.029667>
