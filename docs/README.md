<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Documentation

Use this page to select the document that matches the question you are trying
to answer. Detailed milestone evidence remains available through the milestone
indexes instead of being mixed into one flat list.

Repository licensing is described in [`LICENSE.md`](../LICENSE.md); data reuse
rules are documented in [`DATA_LICENSING.md`](DATA_LICENSING.md).

## Choose an entry point

| I want to... | Start with | What it provides |
|---|---|---|
| understand the current project or invite a collaborator | [Project Status and Collaboration Brief](PROJECT_STATUS_AND_COLLABORATION_BRIEF.md) or its [shareable PDF](../output/pdf/MEHLISSA_Next_Project_Status_and_Collaboration_Brief.pdf) | achieved M0-M7 platform, Workbench 1.0, present uses, evidence maturity, limitations, and collaboration opportunities |
| understand MEHLISSA and run an experiment | [User Guide](USER_GUIDE.md) | non-expert introduction, experiment decision aid, installation, Workbench/CLI/Python workflows, interpretation, and troubleshooting |
| build or contribute to the software | [Development Guide](DEVELOPMENT.md) | compilers, CMake, vcpkg, quality commands, CI, and documentation checks |
| understand APIs or add a model such as kidney | [Software Architecture and Developer Guide](architecture/SOFTWARE_ARCHITECTURE.md) | implemented structure, public APIs, coupling contracts, extension workflow, kidney outline, and personalization |
| understand what comes next | [Roadmap](ROADMAP.md) | integrated platform history, scientific qualification program, further scenarios and organs, scaling, and the digital-twin path |
| assess evidence for Paper 1 | [Evidence and Validity Baseline](publication/EVIDENCE_AND_VALIDITY_BASELINE.md) | schema-validated evidence roles, calibration/validation separation, source audit, claim boundaries, and bibliography export |
| reproduce Paper 1 technical measurements | [Technical Experiment Protocol v2](publication/PAPER1_TECHNICAL_EXPERIMENT_PROTOCOL_V2.md) | locked conditions, three experiments, metrics, controls, exclusions, failure handling, and archive structure |
| assess requirement coverage | [System Requirements](requirements/SYSTEM_REQUIREMENTS.md) and [Traceability Matrix](requirements/TRACEABILITY_MATRIX.md) | 82 numbered requirements, sources, implementation status, and verification evidence |
| understand the historical starting point | [Legacy-baseline analysis](IST_ANALYSE.md) | frozen analysis of legacy commit `4f4fc5a`; not the current Next status |

## Current project truth and maintenance

- [`PROJECT_STATE.json`](PROJECT_STATE.json) contains shared release facts,
  Roadmap section mappings, selected traceability expectations, and the ordered
  scientific qualification packages used by automated consistency checks.
- [Roadmap documentation rules](ROADMAP.md#67-documentation) define which
  living documents must be reviewed after scientific or user-visible changes.
- `python scripts/check_documentation_consistency.py` checks shared facts,
  requirement coverage, selected statuses and references, canonical local
  links, and obsolete terminology. CI runs the same check.
- `python scripts/check_evidence_validity_matrix.py` validates the Paper 1
  evidence baseline, its six-family coverage, cited-source roles and referenced
  artifacts. The associated negative tests run in CI.
- [Data licensing](DATA_LICENSING.md), the
  [licence and data inventory](m0/LICENSE_AND_DATA_INVENTORY.md), and
  [ADR-0007](architecture/adr/0007-repository-license.md) define release and
  reuse boundaries.

## Architecture, requirements, and decisions

- [Architecture Decision Record index](architecture/README.md) lists all
  accepted architectural decisions from the new kernel through Workbench 1.0.
- [Fingerprinting reference scenario](requirements/FINGERPRINTING_SCENARIO.md)
  defines the first complete vertical demonstrator and its realism levels.
- [Target users and workflows](m0/USERS_AND_WORKFLOWS.md) records the original
  research roles and prioritized workflows.
- [Data gaps and validation partners](m0/VALIDATION_AND_DATA_PARTNERS.md)
  identifies evidence and collaboration needs.

## Scientific and technical milestone evidence

| Milestone | Evidence index | Gate decision |
|---|---|---|
| M0 - charter, architecture, licensing, users, and evidence planning | [M0 evidence](m0/M0_GATE_REVIEW.md) | passed |
| M1 - trustworthy kernel, units, lifecycle, errors, determinism, and manifests | [M1 evidence](m1/README.md) | passed |
| M2 - body transport, validated vascular data, observations, and physiological profiles | [M2 evidence](m2/README.md) | [passed](m2/M2_GATE_REVIEW.md) |
| M3 - body-organ coupling and lung model family | [M3 evidence](m3/README.md) | [passed](m3/M3_GATE_REVIEW.md) |
| M4 - capillary transit, exchange, and molecular channels | [M4 evidence](m4/README.md) | [passed](m4/M4_GATE_REVIEW.md) |
| M5 - receptor, intracellular, delivery, response, and population models | [M5 evidence](m5/README.md) | [passed](m5/M5_GATE_REVIEW.md) |
| M6 - Nano-IoT, gateway, body-area network, external station, and resilience | [M6 evidence](m6/README.md) | [passed](m6/M6_GATE_REVIEW.md) |
| M7 - complete FP9/lung fingerprinting software demonstrator | [M7 evidence](m7/README.md) | [passed](m7/M7_GATE_REVIEW.md) |

The milestone indexes link to model cards, validation studies, schemas,
reference cases, and accepted scientific limitations. Pulmonary qualification
evidence is collected under [M3](m3/README.md); capillary qualification under
[M4](m4/README.md); and biological evidence boundaries under
[M5](m5/M5_EVIDENCE_QUALIFICATION.md).

## Integrated research-use layer

- [Research-use delivery index](ux/README.md) links the eight UX-6 increments
  that produced MEHLISSA Research Workbench 1.0.
- [Workbench 1.0 release acceptance](ux/UX6_8_RELEASE_ACCEPTANCE.md) records
  packaging, representative tasks, accessibility, recovery, clean installation,
  regression, and exact-commit publication evidence.
- [Example Workbench workspace](../examples/workbench/README.md) gives a short
  novice scenario and expert campaign route.

The historical labels M0-M7 and UX-1 through UX-6 describe delivery history.
The implemented result is one platform: scientific simulation models,
research-use interfaces, and assurance mechanisms for reproducibility,
evidence, verification, and validation.
