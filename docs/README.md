<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Documentation

Repository licensing is described in [`LICENSE.md`](../LICENSE.md); the rules
for data are documented in [`DATA_LICENSING.md`](DATA_LICENSING.md).

## Project status and further development

- [Current-state analysis](IST_ANALYSE.md) – comparison of the literature, MEHLISSA 1.x, MEHLISSA 2.0, data sets, and application scenarios.
- [Roadmap for a new MEHLISSA generation](ROADMAP.md) – dissertation-aligned target architecture, development phases, quality gates, scenarios, and the long-term digital-twin path.
- [English User Guide](USER_GUIDE.md) – non-expert introduction, experiment decision aid, guided research workflows, installation, commands, models, and interpretation limits.
- [System requirements](requirements/SYSTEM_REQUIREMENTS.md) – 72 numbered functional, architectural, and quality requirements with origin, priority, and verification method.
- [Traceability matrix](requirements/TRACEABILITY_MATRIX.md) – mapping of all requirements to the literature, current status, roadmap gate, and planned verification.
- [Fingerprinting reference scenario](requirements/FINGERPRINTING_SCENARIO.md) – domain baseline, reference values, acceptance levels, and tests for the first vertical demonstrator.
- [Architecture decisions](architecture/README.md) – new kernel, four-layer co-simulation, technology, scenario priority, and evidence rules.
- [M0 gate review](m0/M0_GATE_REVIEW.md) – passed acceptance review of the domain, technical, and licensing foundation decisions.
- [License and data inventory](m0/LICENSE_AND_DATA_INVENTORY.md) – local inventory, checksums, defects, third-party sources, and release rules.
- [Target users and workflows](m0/USERS_AND_WORKFLOWS.md) – roles, prioritized workflows, and release usability.
- [Data gaps and validation partners](m0/VALIDATION_AND_DATA_PARTNERS.md) – required evidence, public sources, and potential expert partners.
- [Development environment and build](DEVELOPMENT.md) – reproducible CMake/vcpkg build, test execution, and quality tools for MEHLISSA Next.
- [M1 – Trustworthy Kernel](m1/README.md) – implementation status, increments, and use of the versioned experiment manifest.
- [M2 gate review](m2/M2_GATE_REVIEW.md) – formal review of the validated body-layer milestone and its declared scientific limits.
- [M3 – Body–Organ Coupling](m3/README.md) – completed milestone evidence for versioned exchange contracts and the lung reference organ.
- [M4 – Capillary Communication](m4/README.md) – completed implementation evidence for capillary transport and molecular-channel comparisons.
- [M4 gate review](m4/M4_GATE_REVIEW.md) – passed technical review of capillary transit, balanced exchange, stable molecular channels, and shared multi-resolution cases.
- [M5 – Cell Response](m5/README.md) – open milestone covering deterministic and stochastic binding, capillary signal hand-off, and population classification.
- [M5.1 receptor-ligand baseline](m5/RECEPTOR_LIGAND_BASELINE.md) – exact binding transient, threshold semantics, executable reference, and scientific limits.
- [M5.2 capillary-to-cell signal hand-off](m5/CAPILLARY_CELL_SIGNAL_HANDOFF.md) – neutral non-consuming tissue observation, adapter profile, executable reference path, and scientific limits.
- [M5.3 time-varying receptor binding](m5/TIME_VARYING_RECEPTOR_BINDING.md) – bounded RK4 trajectory model, constant-limit convergence, pulsed reference, and scientific limits.
- [M5.4 stochastic receptor binding](m5/STOCHASTIC_RECEPTOR_BINDING.md) – exact finite-receptor SSA, named cell streams, analytical population moments, and declared FP/FN experiment.
- [Pulmonary 0D independent validation](m3/PULMONARY_0D_INDEPENDENT_VALIDATION.md) – source-disjoint healthy-cohort comparison, acceptance results, and the exercise RC limitation.
- [Pulmonary 0D flow adaptation](m3/PULMONARY_0D_FLOW_ADAPTATION.md) – independently calibrated bounded rest-to-exercise PVR/compliance response and post-calibration stress test.
- [Pulmonary 0D subject-level multipoint validation](m3/PULMONARY_0D_SUBJECT_MULTIPOINT_VALIDATION.md) – pseudonymous stage-data contract, immutable-model trajectory analysis, evidence safeguards, and data-access status.
- [Pulmonary 0D published-population multipoint validation](m3/PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md) – independent Kovacs/Wolsk pressure-flow series, locked statistical decisions, results, and age-dependent limitations.
- [Pulmonary 0D age conditioning](m3/PULMONARY_0D_AGE_CONDITIONING.md) – separate Kane age calibration, executable v3 contract, no-refit population result, and remaining young-stratum limitation.
- [Pulmonary 0D young-adult resistance qualification](m3/PULMONARY_0D_YOUNG_RESISTANCE.md) – invasive young-PVR calibration, v4 evidence separation, and 15/15 disjoint Wolsk population result.
- [Pulmonary 0D pressure distensibility](m3/PULMONARY_0D_PRESSURE_DISTENSIBILITY.md) – Linehan v5 structural candidate, reference-state normalization, and frozen 11/15 Wolsk diagnostic.
- [Pulmonary 0D age-conditioned distensibility](m3/PULMONARY_0D_AGE_DISTENSIBILITY.md) – independently calibrated v6 older coefficient, improved older RMSE, and unchanged 11/15 stage result.

The historical analysis refers to legacy revision `4f4fc5a` (tag
`legacy-baseline-2026-08-26`). The development documents describe branch
`mehlissa-next-generation`. Material changes to the architecture, data sets,
or scenarios must update the requirements, traceability matrix, and roadmap
together.
