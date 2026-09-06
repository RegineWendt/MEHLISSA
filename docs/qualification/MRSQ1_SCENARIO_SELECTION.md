<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MRSQ-1.1 Medical Reference Scenario Selection

## Decision

MRSQ means **Medical Reference Scenario Qualification**. MRSQ-1.1 selects a
healthy-adult dynamic total-body [18F]fluorodeoxyglucose ([18F]FDG) PET scenario
as the first prospective external end-to-end qualification target. The intended
path is:

```text
intravenous 18F-FDG administration
  -> vascular input and whole-body transport
  -> organ and tissue exchange
  -> intracellular FDG trapping and physical radioactive decay
  -> frame-integrated total-body PET observation
  -> cohort-level comparison and bounded research report
```

The external evidence candidate is the HedyPET/Multimodal-HC data family. Its
2026 Scientific Data descriptor reports 100 healthy adults stratified by age
and sex, a 70-minute dynamic total-body PET acquisition, dynamic reconstructions,
organ and tissue segmentations, image-derived input functions, pre-extracted
time-activity curves, SUV/SUL readouts, and Patlak Ki values. Eighty participants
are reported as immediately released; the remaining 20 are scheduled for 2027.

MRSQ-1.1 is a **prospective selection**, not a data analysis or validation
result. No participant file or candidate validation outcome table was opened,
downloaded, parsed, or summarized. Public descriptive metadata and an
illustrative repository README output were not used as endpoint evidence.

## Why this scenario was selected

Five candidates were scored from zero to three on nine pre-outcome dimensions.
Eligibility is applied before score: an endpoint-only or non-injection dataset
cannot outrank a viable injection-to-measurement candidate merely because it is
easy to download. Scores then make the trade-offs within that boundary visible.

| Rank | Candidate | Score / 27 | Decision |
|---:|---|---:|---|
| 1 | HedyPET healthy-adult dynamic total-body [18F]FDG PET | 25 | selected for the MRSQ-1.2 prospective protocol |
| 2 | First-pass lung DCE-MRI in healthy adults and idiopathic pulmonary fibrosis | 20 | pulmonary fallback; reusable participant curves not located |
| 3 | PhysioNet thermodilution cardiac output plus arterial pressure | 21 | useful endpoint comparator, but no complete dilution curve or organ/tissue path |
| 4 | PhysioNet HeartCycle synchronized ICG/ECHO/ECG/PPG | 21 | strong measurement resource, but no injection |
| 5 | MEHLISSA M7 FP9 fingerprinting vertical slice | 13 | integration regression only; it cannot externally validate itself |

HedyPET is the only screened candidate that currently combines:

- a declared intravenous input and an external time-resolved observation;
- human whole-body, organ, and tissue coverage in one study;
- advertised numeric input functions and time-activity curves;
- persistent publication and dataset identities;
- an explicit CC-BY-4.0 data declaration;
- variation across a healthy adult cohort; and
- a bounded route through existing MEHLISSA body, lung, capillary, experiment,
  provenance, result, campaign, and Workbench interfaces.

The selection is not pulmonary exclusivity. The lung remains a represented and
observable organ, while the total-body dataset makes conservation, organ
distribution, uptake, excretion, and measurement transformations testable in a
single scenario. This is closer to the dissertation's system-level vision than
validating another isolated pulmonary scalar.

## Exact evidence and access state

| Artifact | Stable identity | Observed access/right state | MRSQ role |
|---|---|---|---|
| Hinge et al. data descriptor | [doi:10.1038/s41597-026-08157-4](https://doi.org/10.1038/s41597-026-08157-4), published 24 August 2026 | open-access article, CC-BY-4.0 unless separately credited | cohort, acquisition, artifact and limitation authority; not model calibration |
| Hugging Face dataset | [doi:10.57967/hf/9560](https://huggingface.co/datasets/DEPICT-RH/Multimodal-HC) | dataset card declares CC-BY-4.0 and approximately 302 GB total; exact revision and selective readout paths not yet frozen | candidate validation observations; no file accessed in MRSQ-1.1 |
| PublicnEUro dynamic dataset V1 | [doi:10.70883/UYAG3430](https://datacatalog.publicneuro.eu/dataset/PN000015%20multimodal%20total-body%20dynamic%2018F-FDG%20PET-CT-MRI/V1) | catalog declares CC-BY-4.0; access route and any data-use agreement must be reconciled | persistent image-series identity and fallback route |
| Processing repository | [DEPICT-RH/Multimodal-HC](https://github.com/DEPICT-RH/Multimodal-HC) | publicly inspectable; source-code licence not established in this screen | method inspection and future hash binding only; no code copying |

The descriptions of full-image access are not yet fully consistent across the
public routes. That is not hidden by calling the data simply “open.” MRSQ-1.2
must bind one exact revision, enumerate the minimum required files, determine
whether derived readouts are directly retrievable, record all applicable terms,
and decide whether institutional review is required before ingress.

## What can be reused and what must be added

The following MEHLISSA capabilities are directly reusable:

- the BVS95 vascular graph and typed transport contracts;
- healthy-adult body-state and pulmonary variants;
- capillary ownership, exchange, and observation contracts;
- experiment manifests, provenance, deterministic execution, campaigns,
  reports, Python readers, notebooks, and Workbench; and
- the existing qualification discipline: frozen candidates, negative controls,
  uncertainty, sensitivity, retained failures, and independent checking.

The following scientific models are **not** reusable as FDG physiology:

- the DCCQ VEGF-A165a/VEGFR2/HUVEC model is ligand- and receptor-specific;
- the BCQ CD95 apoptosis mechanism has unresolved source-native units and is
  not a glucose-tracer uptake model;
- the FP9 nanodevice chain is not a PET scanner or image observation model; and
- current organ models do not represent FDG phosphorylation and trapping,
  dephosphorylation, renal excretion, fluorine-18 decay, PET frame integration,
  reconstruction effects, or partial-volume behavior.

MRSQ therefore adds interchangeable FDG transport/exchange/kinetic and PET
observation modules. It must not add tracer-specific branches to the simulation
kernel.

## Source-role and governance boundary

The released HedyPET observations are reserved for no-refit validation. Model
equations and calibration parameters must come from source-disjoint physiology
and FDG kinetic literature, analytical constraints, or an explicitly separate
calibration family. HedyPET outcomes may not be used to tune the candidate and
then be described as independent validation.

MRSQ-1 is initially cohort-level and non-personalized. The MRSQ-1.2 protocol has
now frozen the following conditions, which MRSQ-1.3 must enforce before any
participant file is opened:

- confirm the exact licence and access route;
- acquire only the minimum required metadata, input functions, and curves;
- keep participant-level source files outside Git;
- reject direct identifiers and avoid raw observations in terminal logs;
- specify retention, deletion, redistribution, and derived-result rules; and
- record the institutional governance decision.

No diagnostic, treatment, individual-prediction, patient-specific, medical-
device, or clinical decision-support use is authorized.

## MRSQ-1 work packages

| Increment | Plain-language purpose | Exit condition |
|---|---|---|
| **MRSQ-1.1 — scenario selection** | Choose the externally observable case before reading its outcomes. | **Complete:** five scenarios, eight sources, access and licence boundaries, reuse and missing models are frozen. |
| **MRSQ-1.2 — prospective protocol** | Decide exactly what will be claimed, read, modelled, compared, and accepted. | **Complete:** exact revision and minimum remote-object identities, intended use, source roles, access boundary, cohort aggregation, observation models, endpoints, tolerances, uncertainty, missingness, amendment, and failure rules are frozen; content checksums remain an explicit pre-parse duty of MRSQ-1.3. |
| **MRSQ-1.3 — selective data ingress** | Read only authorized FDG-PET inputs safely and reproducibly. | **Synthetic PASS; measured BLOCKED:** manifest-first logic and twelve failure controls pass, while governance, content hashes and authoritative frame durations still prohibit participant-file access. |
| **MRSQ-1.4 — executable scenario** | Implement FDG transport, exchange, trapping, decay, and PET observation without changing the kernel. | **Complete:** typed source-disjoint modules pass numerical, replay, decay, bounds and negative tests. |
| **MRSQ-1.5 — candidate freeze** | Complete model construction before seeing validation outcomes. | **Complete:** parameters, assumptions, source/code identities, seven uncertainty classes and the eight-frame no-refit software reference are immutable. |
| **MRSQ-1.6 — external evaluation** | Compare the locked cohort candidate with HedyPET observations. | **Software complete; external execution BLOCKED:** participant-first metrics, bootstrap and decision states pass arbitrary tests; no HedyPET outcome was opened. |
| **MRSQ-1.7 — independent close-out** | Reconcile the science, software, data rights, archive, claims, and documentation. | **Machine complete; human BLOCKED:** runner-independent checks pass, but no external human attestation exists. |

## Permitted statement at MRSQ-1.1

> MEHLISSA has prospectively selected and licence-screened a healthy-adult
> dynamic total-body [18F]FDG PET evidence family for its first medical
> injection-to-measurement qualification programme.

MRSQ-1.1 does **not** implement, calibrate, validate, or clinically qualify an
FDG transport, uptake, or PET observation model.

Even after a successful MRSQ-1.6, the maximum intended statement is that a
frozen healthy-adult cohort reference scenario was evaluated without validation
refitting against identified external dynamic PET observations, with reported
uncertainty and limitations. Disease, individual, diagnostic, treatment,
biological-sensing, nanodevice, and clinical validity require separate evidence.

## Machine authority and reproduction

The machine-readable authority is
`data/qualification/medical-reference-scenario-candidate-register-v1.json`,
validated by its JSON Schema and a semantic fail-closed checker:

```powershell
python scripts/check_medical_reference_scenario_candidates.py
python -m unittest tests/test_medical_reference_scenario_candidates.py
```

The negative tests reject candidate/rank drift, premature participant-data
access, validation-outcome inspection, loss of source identity, conflated data
and code rights, ineligible alternatives, circular M7 evidence, and skipping
the prospective MRSQ-1.2 protocol.

Subsequent status: MRSQ-1.2 through MRSQ-1.7 have closed the computational path
at its evidence boundary. The immutable pre-outcome protocol remains documented
in [MRSQ-1.2 Prospective Healthy-Adult Dynamic FDG-PET
Protocol](MRSQ1_PROSPECTIVE_PROTOCOL.md); current implementation, archive,
blocked measured ingress and non-claim are documented in the [bounded MRSQ-1
result](MRSQ1_QUALIFICATION_RESULT.md).
