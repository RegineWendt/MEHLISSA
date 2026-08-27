<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# System Requirements for MEHLISSA Next

**Status:** Phase 0 baseline

**As of:** 26 August 2026

**Purpose:** Translate the MEHLISSA vision described in the dissertation into
verifiable requirements for a new, scientifically robust simulation platform.

## 1. Scope

MEHLISSA Next is a research platform for simulating medical nano- and molecular
communication systems in the human body. It connects models at body, organ,
capillary, and cell level with nanodevices, gateways, body-area networks, and
external analysis or control components.

The system is initially **not a clinical medical device**, and its results are
not automatically clinically valid. Every model variant must disclose its
validity scope, data basis, and uncertainty.

The requirements describe the domain target state. Incremental implementation
is defined in the [roadmap](../ROADMAP.md). The
[traceability matrix](TRACEABILITY_MATRIX.md) maps them to sources, existing
code, milestones, and verification.

## 2. Sources and reading key

### 2.1 Primary sources

| Abbreviation | Source |
|---|---|
| DISS | dissertation, especially Chapters 4–6; page references use printed page numbers |
| MEH20 | *MEHLISSA: A Medical Holistic Simulation Architecture for Nanonetworks in Humans* |
| BVS18 | *BloodVoyagerS – Simulation of the Work Environment of Medical Nanobots* |
| VIS20 | *BVS-Vis: A Web-based Visualizer for BloodVoyagerS* |
| FP23 | *Proteome Fingerprinting as a Localization Scheme for Nanobots* |
| MEH25 | *MEHLISSA 2.0: Accelerating Full-body Molecular Communication Simulations* |
| RM | [roadmap for a new MEHLISSA generation](../ROADMAP.md) |

### 2.2 Origin

| Code | Meaning |
|---|---|
| `V` | directly from the domain vision or an explicit literature requirement |
| `B` | described as behavior or a result in an existing MEHLISSA/BVS revision |
| `A` | derived from the vision and roadmap for a robust implementation |
| `N` | deliberate new addition to the original vision |

Multiple codes are possible. `B` does not automatically mean that the existing
implementation is correct or sufficiently validated.

### 2.3 Priority and verification

| Code | Meaning |
|---|---|
| `P0` | foundation; required before domain extension |
| `P1` | core of the dissertation vision and first vertical demonstrator |
| `P2` | extension into a versatile research platform |
| `P3` | long-term personalization, scaling, or near-clinical vision |
| `T` | automated software, component, or regression test |
| `A` | analytical/numerical comparison or invariant check |
| `R` | reproduction of a published reference run |
| `E` | comparison with independent experimental or physiological data |
| `I` | inspection of schema, manifest, documentation, or user interface |

## 3. Cross-cutting system requirements

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| SYS-001 | The system shall represent simulation time monotonically, with an explicit unit and sufficient subsecond resolution. | A | P0 | T/A | RM 3.1, M1 |
| SYS-002 | A run shall be deterministically reproducible with identical software, configuration, input, and seeds; statistical models shall additionally support replicates. | A/N | P0 | T/R | RM 2.4, M1 |
| SYS-003 | Random processes shall use named, mutually decoupled random streams. | A/N | P0 | T | RM M1 |
| SYS-004 | Physical quantities shall have explicit, verifiable units; incompatible units shall not be combined silently. | A/N | P0 | T/I | RM 3.2, 6.1 |
| SYS-005 | Kernel, models, scenarios, data adapters, and evaluation shall have separate responsibilities. | A | P0 | I/T | RM 2.1–2.2 |
| SYS-006 | Scenario-specific logic shall not modify the general simulation kernel. | A | P0 | I | RM 2.2, M0 |
| SYS-007 | The system shall respond in a controlled manner to invalid configurations, numerical errors, and violated model invariants. | A/N | P0 | T | RM 3.1, M1 |
| SYS-008 | Models shall be interchangeable from coarse surrogates to detailed variants without changing a scenario's domain meaning. | V/A | P1 | T/R | DISS pp. 95–97, 133; RM 2.3 |

## 4. Multilayer and co-simulation architecture

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| ARC-001 | The system shall represent the four connected but independent layers: body, organ, capillary, and cell. | V | P1 | I/T | DISS pp. 94–96; MEH20 pp. 1–2 |
| ARC-002 | Each layer shall have its own spatial and temporal resolution, state model, and documented validity scope. | V/A | P1 | I/T | DISS p. 95; RM 2.1 |
| ARC-003 | Layers shall exchange entities, populations, substance flows, physiological states, and events through versioned contracts. | V/A | P1 | T/A | DISS pp. 95–97; RM 3.2 |
| ARC-004 | Exchanges between layers shall preserve temporal order and relevant conservation laws and identities. | A/N | P1 | T/A | RM 3.3, M3–M5 |
| ARC-005 | Coupling shall be bidirectional: lower layers can return events and aggregate effects to higher layers. | V/A | P1 | T | DISS pp. 99–100; MEH20 pp. 2–5 |
| ARC-006 | The orchestrator shall coordinate different time steps through defined synchronization points or events. | A/N | P1 | T/A | RM 3.3 |
| ARC-007 | External simulators shall be integrated through adapters or derived surrogates without coupling the kernel or layers to a specific external product. | V/A | P2 | T/I | DISS pp. 96–97, 129–133, 154; MEH20 pp. 2–5 |

## 5. Body layer

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| BODY-001 | The body layer shall model a complete closed circulation with large vessels, organ/regional transitions, and appropriate abstractions of small vessels. | V/B | P1 | T/R | DISS p. 100; BVS18 pp. 3–6 |
| BODY-002 | Every vessel shall have at least a stable ID, type, start/end geometry, length, diameter or cross section, connections, and data provenance. | V/A | P1 | T/I | DISS pp. 100, 113–117 |
| BODY-003 | The spatial model shall represent all relevant body regions, including extremities, in a documented 3D coordinate system. | V/B | P1 | T/R | DISS pp. 101–104, 115–117; BVS18 pp. 3–5 |
| BODY-004 | New or personalized body models shall be loadable and schema-validatable without changing simulation code. | V/B/A | P1 | T/I | DISS pp. 117, 140–143 |
| BODY-005 | Nanodevices, rare cells, and other mobile entities shall support injection, transport, tracking, and extraction. | V/B | P1 | T/R | DISS pp. 99–104, 113–115 |
| BODY-006 | Movement and branching shall be determined by configurable blood-flow/perfusion values and probabilities; branch shares shall sum to one. | V/B/A | P1 | T/A/R | DISS pp. 100–101, 118–122 |
| BODY-007 | The body layer shall support at least a compartment/graph model and optionally virtual laminar flows or imported streamlines. | V/B/A | P2 | T/R | DISS pp. 115–117; BVS18 p. 4 |
| BODY-008 | Physiological states such as rest, exercise, heart rate, and posture shall act through interchangeable parameter sets or coupled models. | V/A | P2 | T/E | DISS pp. 120–122; BVS18 p. 6 |
| BODY-009 | Blood components and their chemical, mechanical, or communication effects shall be representable in graduated model variants. | V | P2 | T/E | DISS p. 101; BVS18 pp. 3, 6 |
| BODY-010 | Distributions shall be verifiable through mass conservation, stationary distribution, and published BVS reference runs. | B/A | P1 | A/R | DISS pp. 104–108, 134–137; BVS18 pp. 4–6 |

## 6. Organ layer

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| ORG-001 | Organs shall support independent models with geometry, vascular structure, tissue classes, perfusion, and activity state. | V | P1 | T/E | DISS pp. 118–126; MEH20 p. 3 |
| ORG-002 | Organ perfusion shall be derivable from literature-based targets or coupled models and changeable by state. | V/B | P1 | A/R/E | DISS pp. 118–122 |
| ORG-003 | Entities and substance flows shall be exchanged between body and organ layers through defined entry and exit points. | V/A | P1 | T/A | DISS pp. 95, 153–154; RM M3 |
| ORG-004 | Detections and measurements shall be assignable to an organ or tissue, including method and localization uncertainty. | V | P1 | T/R | DISS pp. 122–123, Ch. 6 |
| ORG-005 | A coarse organ compartment and a more detailed organ/vascular model shall be interchangeable behind the same interface. | V/A | P2 | T/R | DISS pp. 123–126 |
| ORG-006 | Import and conversion steps for BodyParts3D, SimVascular, or patient data shall traceably preserve units, axes, provenance, and license. | V/A | P3 | T/I | DISS pp. 124–126; RM M3/M8 |

## 7. Capillary layer

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| CAP-001 | The capillary layer shall represent arterioles, a capillary bed, and venules in at least one parameterizable abstraction. | V | P1 | T/A | DISS pp. 126–129; MEH20 pp. 3–4 |
| CAP-002 | Capillary density, diameter, transit time, blood velocity, and branching shall be configurable per organ. | V/A | P1 | T/E | DISS pp. 126–129 |
| CAP-003 | Activity-dependent perfusion and precapillary sphincters shall change the perfused fraction of a capillary bed. | V | P2 | T/E | DISS pp. 127–128 |
| CAP-004 | Exchange among blood, endothelium, interstitium, and cell shall be modeled with explicit amounts/concentrations and conservation checks. | V/A | P1 | T/A/E | DISS p. 127; MEH20 pp. 3–4 |
| CAP-005 | Local position, residence duration, retention, and later adhesion/extravasation models shall be supported or reported as uncertainty. | V/A | P2 | T/E | DISS pp. 128–129 |
| CAP-006 | Molecular, electromagnetic, or other channel models shall be connectable as interchangeable detailed models or validated runtime/success distributions. | V/A | P1 | T/R | DISS p. 129, 154; MEH20 pp. 3–5 |
| CAP-007 | Cluster, relay, and multi-hop communication in the capillary bed shall be investigable and provide abstracted reachability to higher layers. | V | P2 | T/R | DISS p. 129; MEH20 pp. 3–4 |

## 8. Cell layer

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| CELL-001 | The cell layer shall model molecule/biomarker release, local concentrations, and detection by nanodevices. | V | P1 | T/A/E | DISS pp. 129–132 |
| CELL-002 | Receptor/ligand binding, detection thresholds, and stochastic misclassification shall be available as parameterizable models. | V/A | P1 | T/A/E | DISS pp. 130–132; FP23 pp. 2–6 |
| CELL-003 | Nanodevices shall release signals or active substances; diffusion, binding, and uptake shall be coupleable. | V | P1 | T/A/R | DISS pp. 130–132, 153–154; MEH20 pp. 4–5 |
| CELL-004 | Intracellular reactions shall be representable through reaction-time distributions, ODE/SSA models, or external simulators. | V/A | P2 | T/A/E | DISS pp. 131–133, 154 |
| CELL-005 | At least apoptosis and a generic measurable cell-state event shall be returnable to higher layers. | V | P2 | T/R/E | DISS pp. 153–154; MEH20 pp. 4–5 |

## 9. Nanodevices, nano-IoT, and gateways

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| IOT-001 | Nanodevices shall have type, capabilities, payload, internal state, target, and lifecycle; scenarios shall be able to add specialized types. | V/B/A | P1 | T | DISS pp. 113–115, 186–188 |
| IOT-002 | The system shall compose nano-IoT components from an in-body network, gateway, BAN device, and analysis/control station. | V | P1 | T/I | DISS pp. 96–97; MEH20 p. 2 |
| IOT-003 | Gateways shall be modeled as spatial measurement and communication sites with nano and macro sides. | V | P1 | T/R | DISS pp. 117–118, 187–190 |
| IOT-004 | Uplink measurements and downlink commands shall report latency, loss, error rate, and optionally energy separately from biological results. | V/A | P2 | T/R | DISS pp. 96–97, 117–118; RM M6 |
| IOT-005 | Communication models shall optionally run through ns-3 or other simulators without physiological models depending on them. | V/A | P2 | T/I | DISS pp. 96–100; MEH25 pp. 1–2 |

## 10. Data, experiments, evidence, and output

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| DATA-001 | All input data shall have a versioned schema, units, coordinate system, source, license, and checksum. | A/N | P0 | T/I | RM 6.1 |
| DATA-002 | An experiment manifest shall completely describe model variants, parameters, seeds, duration, injections, observations, and termination conditions. | A/N | P0 | T/I | RM 6.4 |
| DATA-003 | Every run shall produce a provenance manifest containing software/data versions, build, platform, and runtime. | A/N | P0 | T/I | RM 2.4, 6.4 |
| DATA-004 | Output shall distinguish events, aggregates, samples, and optional trajectories and support volume limits. | B/A | P1 | T/R | VIS20 pp. 1–2; MEH25 pp. 1–2; RM M2 |
| DATA-005 | Model assumptions shall be classified as published/observed, calibrated, validated, derived, or hypothetical. | A/N | P0 | I | RM 2.5, 6.2 |
| DATA-006 | Calibration and validation data shall be separated; fitting to target values shall not simultaneously count as independent validation. | A/N | P0 | I/E | RM 6.2 |
| DATA-007 | Medical results shall report uncertainty, sensitivity, and validity limitations together with the estimate. | A/N | P1 | T/I/E | RM 6.3 |
| DATA-008 | Regression runs shall use numerical/statistical tolerances rather than unreviewed exact equality when comparing stochastic models. | A/N | P1 | T/R | RM 5/M2, 6.2 |

## 11. Visualization and operation

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| UX-001 | The simulation shall run without visualization in local batch mode and later in HPC operation. | A/N | P1 | T | RM Phase 0/10 |
| UX-002 | A decoupled visualization shall read the body model, entities/populations, time series, and density/heatmap data. | B/A | P2 | T/I | VIS20 pp. 1–2; DISS pp. 109–113 |
| UX-003 | Users shall navigate time, rotate/pan/zoom views, and compare layers or runs. | B/A | P2 | I | VIS20 pp. 1–2; RM 6.5 |
| UX-004 | Visualizations and reports shall be reproducibly generated from stored results and shall not modify simulation state. | A/N | P1 | T/I | RM 6.5 |

## 12. Medical scenarios

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| SCN-001 | As the first vertical demonstrator, fingerprinting shall represent injection, transport, tissue detection, message formation, collection, and external readout. | V/B/A | P1 | R | DISS Ch. 6, especially pp. 185–190; FP23 pp. 5–6 |
| SCN-002 | Continuous monitoring shall model time-dependent biomarkers, personal baselines/thresholds, and alert paths. | V | P2 | T/E | DISS pp. 143–152 |
| SCN-003 | In-vivo liquid biopsy shall model release, transport, degradation, rare ctDNA detection, and comparison with blood samples. | V/B | P2 | R/E | DISS pp. 155–160 |
| SCN-004 | Metastasis prevention shall connect cell detachment, transport, detection, drug release, binding, and cell response across all layers. | V | P2 | T/R/E | DISS pp. 153–154; MEH20 pp. 4–5 |
| SCN-005 | CAR-T shall combine cell populations and local agent models and use published interaction models as interchangeable model components. | B/A | P2 | R/E | MEH25 pp. 1–2 |
| SCN-006 | A research digital twin shall incrementally support anatomical, physiological, and biochemical personalization and continuous data updates. | V/A | P3 | I/E | DISS pp. 140–143; RM M8 |

## 13. Non-functional quality objectives

| ID | Requirement | Origin | Priority | Verification | Source |
|---|---|---:|---:|---:|---|
| QUA-001 | The C++ kernel shall be built and tested automatically on Windows and Linux using at least MSVC, GCC, and Clang. | N | P0 | T | RM M1 |
| QUA-002 | Critical kernel code shall be checked with warnings as errors, static analysis, and appropriate sanitizers. | N | P0 | T | RM M1 |
| QUA-003 | Performance shall be assessed with versioned benchmarks; optimizations shall change reference results only within specified tolerances. | A/N | P1 | T/R | MEH25 pp. 1–2; RM Phase 10 |
| QUA-004 | Realistic scales shall become scalable through agent, population, compartment, field, and surrogate models. | A | P1 | T/R | MEH25 pp. 1–2; RM 2.3 |
| QUA-005 | Public interfaces, models, data schemas, and scenarios shall be versioned and documented for researchers. | A/N | P1 | I | RM 6.6 |
| QUA-006 | Patient data shall be processed only after a documented data-protection, consent, and pseudonymization concept exists. | N | P3 | I | RM M8 |

## 14. Acceptance rules

A requirement is implemented only when:

1. a responsible implementation or versioned data artifact exists;
2. the verification named in the table is automated or traceably documented;
3. validity scope and known limitations are documented;
4. the traceability matrix references a commit, test, data set, or report;
5. any change to published reference behavior is explained and scientifically assessed.

The baseline may be changed through Architecture Decision Records. A deviation
from the dissertation is permissible, but must be documented as a deliberate
decision with benefits, costs, and scientific consequences.
