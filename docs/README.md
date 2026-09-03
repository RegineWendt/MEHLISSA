<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Documentation

Repository licensing is described in [`LICENSE.md`](../LICENSE.md); the rules
for data are documented in [`DATA_LICENSING.md`](DATA_LICENSING.md).

## Project status and further development

- [Current-state analysis](IST_ANALYSE.md) – comparison of the literature, MEHLISSA 1.x, MEHLISSA 2.0, data sets, and application scenarios.
- [Project Status and Collaboration Brief](PROJECT_STATUS_AND_COLLABORATION_BRIEF.md) – maintainable English source for the achieved M0–M7 platform, present uses, scientific limits, UX program, and contributor opportunities; the [rendered PDF](../output/pdf/MEHLISSA_Next_Project_Status_and_Collaboration_Brief.pdf) is the shareable edition.
- [Roadmap for a new MEHLISSA generation](ROADMAP.md) – dissertation-aligned target architecture, development phases, quality gates, scenarios, and the long-term digital-twin path.
- [English User Guide](USER_GUIDE.md) – non-expert introduction, experiment decision aid, guided research workflows, installation, commands, models, and interpretation limits.
- [UX-6.1 product and technical foundation](ux/UX6_1_PRODUCT_AND_TECHNICAL_FOUNDATION.md) – workbench roles, prioritized workflows, wireframes, technology decision, threat/privacy/accessibility baseline, and local prototype acceptance.
- [UX-6.2 guided scenario workspace](ux/UX6_2_GUIDED_SCENARIO_WORKSPACE.md) – schema-derived guided fields, complete-source retention, bounded file access, non-overwriting save-as, and local acceptance.
- [UX-6.3 validation and corrective feedback](ux/UX6_3_VALIDATION_AND_CORRECTIVE_FEEDBACK.md) – authoritative live validation, stable located issues, repair guidance, warning/error separation, execution gating, and shareable summaries.
- [UX-6.4 run and campaign control](ux/UX6_4_RUN_AND_CAMPAIGN_CONTROL.md) – explicit run-plan confirmation, individual and six-run campaign execution, cancellation, bounded logs, and retained artifacts.
- [System requirements](requirements/SYSTEM_REQUIREMENTS.md) – 79 numbered functional, architectural, user-experience, and quality requirements with origin, priority, and verification method.
- [Traceability matrix](requirements/TRACEABILITY_MATRIX.md) – mapping of all requirements to the literature, current status, roadmap gate, and planned verification.
- [Fingerprinting reference scenario](requirements/FINGERPRINTING_SCENARIO.md) – domain baseline, reference values, acceptance levels, and tests for the first vertical demonstrator.
- [Architecture decisions](architecture/README.md) – new kernel, four-layer co-simulation, technology, scenario priority, and evidence rules.
- [M0 gate review](m0/M0_GATE_REVIEW.md) – passed acceptance review of the domain, technical, and licensing foundation decisions.
- [License and data inventory](m0/LICENSE_AND_DATA_INVENTORY.md) – local inventory, checksums, defects, third-party sources, and release rules.
- [Target users and workflows](m0/USERS_AND_WORKFLOWS.md) – roles, prioritized workflows, and release usability.
- [Data gaps and validation partners](m0/VALIDATION_AND_DATA_PARTNERS.md) – required evidence, public sources, and potential expert partners.
- [Development environment and build](DEVELOPMENT.md) – reproducible CMake/vcpkg build, test execution, and quality tools for MEHLISSA Next.
- [Software architecture and developer guide](architecture/SOFTWARE_ARCHITECTURE.md) – implemented system structure, public APIs, cross-layer contracts, module-extension workflow, and the current path and limits of personalization.
- [M1 – Trustworthy Kernel](m1/README.md) – implementation status, increments, and use of the versioned experiment manifest.
- [M2 gate review](m2/M2_GATE_REVIEW.md) – formal review of the validated body-layer milestone and its declared scientific limits.
- [M3 – Body–Organ Coupling](m3/README.md) – completed milestone evidence for versioned exchange contracts and the lung reference organ.
- [M4 – Capillary Communication](m4/README.md) – completed implementation evidence for capillary transport and molecular-channel comparisons.
- [M4 gate review](m4/M4_GATE_REVIEW.md) – passed technical review of capillary transit, balanced exchange, stable molecular channels, and shared multi-resolution cases.
- [M5 – Cell Response](m5/README.md) – completed technical milestone covering deterministic and stochastic binding, capillary signal hand-off, intracellular response, delivery, apoptosis feedback, and population scaling.
- [M5.1 receptor-ligand baseline](m5/RECEPTOR_LIGAND_BASELINE.md) – exact binding transient, threshold semantics, executable reference, and scientific limits.
- [M5.2 capillary-to-cell signal hand-off](m5/CAPILLARY_CELL_SIGNAL_HANDOFF.md) – neutral non-consuming tissue observation, adapter profile, executable reference path, and scientific limits.
- [M5.3 time-varying receptor binding](m5/TIME_VARYING_RECEPTOR_BINDING.md) – bounded RK4 trajectory model, constant-limit convergence, pulsed reference, and scientific limits.
- [M5.4 stochastic receptor binding](m5/STOCHASTIC_RECEPTOR_BINDING.md) – exact finite-receptor SSA, named cell streams, analytical population moments, and declared FP/FN experiment.
- [M5.5 intracellular response network](m5/INTRACELLULAR_RESPONSE_NETWORK.md) – shared two-stage ODE/SSA network, internal effector event, population comparison, and validity boundary.
- [M5.6 conservative drug delivery](m5/CONSERVATIVE_DRUG_DELIVERY.md) – threshold-derived nanodevice activation, analytical release/uptake chain, exact amount ownership, and validity boundary.
- [M5.7 apoptosis and higher-layer feedback](m5/APOPTOSIS_AND_HIGHER_LAYER_FEEDBACK.md) – synthetic bounded dose response, irreversible commitment state, neutral feedback event, and validity boundary.
- [M5.8 population scale and validity](m5/POPULATION_SCALE_AND_VALIDITY.md) – cohort-compressed one-trillion-cell reference, bounded output, parameter sensitivities, and model-selection scopes.
- [M5 evidence qualification](m5/M5_EVIDENCE_QUALIFICATION.md) – analytical software evidence, external comparison candidates, and retained biological gaps.
- [M5 gate review](m5/M5_GATE_REVIEW.md) – passed technical review of signal-to-response coupling, higher-layer feedback, analytical references, population validity, and User Guide impact.
- [M6 – Nano-IoT Communication](m6/README.md) – accepted communication milestone spanning versioned nanodevices, local and multihop transport, an active gateway, BAN/station control, an external-simulator boundary, and fail-closed resilience scenarios.
- [M6.1 nanodevice and local-message contract](m6/NANODEVICE_AND_LOCAL_MESSAGE_CONTRACT.md) – capability composition, payload and resource profiles, endpoint semantics, checked reference hand-off, and current non-claims.
- [M6.2 detection to one-hop communication](m6/DETECTION_TO_ONE_HOP_COMMUNICATION.md) – neutral M5 detection adapter, scheduled local link, explicit delivery/drop results, exact metrics, and interpretation boundary.
- [M6.3 cluster, relay, and bounded multi-hop communication](m6/CLUSTER_RELAY_AND_MULTI_HOP.md) – strict topology, deterministic route strategies, store-and-forward semantics, exact aggregate metrics, and non-claims.
- [M6.4 active gateway uplink and downlink](m6/ACTIVE_GATEWAY_UPLINK_AND_DOWNLINK.md) – resource-bounded gateway endpoint, neutral measurement publication, local command translation/routing, exact reference, and safety boundary.
- [M6.5 BAN and external control loop](m6/BAN_AND_EXTERNAL_STATION.md) – replaceable BAN transport, governed external station policy, traceable command return, and synthetic reference loop.
- [M6.6 external network-simulator adapter](m6/EXTERNAL_NETWORK_SIMULATOR_ADAPTER.md) – metadata-only request/response boundary, typed and JSON clients, identity checks, and external-model non-claims.
- [M6.7 resilience and boundary misuse](m6/M6_RESILIENCE_AND_BOUNDARY_MISUSE.md) – strict twelve-case catalog, failure accounting, fail-closed production boundaries, and explicit security non-claims.
- [M6 gate review](m6/M6_GATE_REVIEW.md) – passed technical review of the end-to-end Nano-IoT path, communication metrics, resilience scope, and User Guide impact.
- [M7 – Fingerprinting vertical slice](m7/README.md) – accepted research-software demonstrator spanning composition, binding, tiles, executed communication, uncertainty, and one holistic result.
- [M7.1 Level-A composition contract](m7/LEVEL_A_COMPOSITION_CONTRACT.md) – versioned FP9 scenario selection, causal stage order, schema-validated M2–M6 artifacts, typed composer, and current execution boundary.
- [M7.2-M7.4 runtime, result, and Level-B detection](m7/RUNTIME_RESULT_AND_LEVEL_B_DETECTION.md) – typed runtime probe, identity trace, SHA-256 result manifest, receptor binding, and negative control.
- [M7.5-M7.7 Levels C-E and holistic run](m7/LEVELS_C_TO_E_AND_HOLISTIC_RUN.md) – explicit tiles, executed locator-to-station path, sensitivity/misclassification analysis, and strict result 2.0.0.
- [M7 gate review](m7/M7_GATE_REVIEW.md) – passed software-demonstrator review with explicit historical, synthetic, physiological, and clinical limitations.
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
scenarios, or user experience must update the requirements, traceability
matrix, Roadmap, User Guide, and Project Status and Collaboration Brief as
applicable. Contributor-facing documents must expand milestone, priority,
realism-level, workflow, and domain abbreviations at first use.
