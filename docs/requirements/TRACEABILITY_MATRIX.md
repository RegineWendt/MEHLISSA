<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Traceability Matrix – MEHLISSA Next

**As of:** 27 August 2026

**Reference document:** [system requirements](SYSTEM_REQUIREMENTS.md)

## 1. Purpose

This matrix connects every system requirement with its domain source, current
implementation status, planned roadmap gate, and a concrete verification
artifact. It is the operational checklist for architecture reviews and releases.

Status codes:

- `DONE`: present in the Next bootstrap and verified automatically;
- `PART`: partly present but not yet fully verified against the requirement;
- `LEGACY`: present only in a historical implementation or publication;
- `SPEC`: specified but not yet implemented;
- `RESEARCH`: additionally requires domain data, calibration, or experimental evidence.

`DONE` requires successful verification. Merely naming a class is insufficient.

## 2. Foundation and architecture

| ID | Source | Status 2026-08-27 | Target | Planned verification |
|---|---|---|---|---|
| SYS-001 | RM M1 | DONE | M1 | `simulation_clock_tests`, cross-platform CTest |
| SYS-002 | RM 2.4 | PART | M1/M7 | byte-identical M1 kernel reference run on MSVC/GCC/Clang; replicate planning and domain-model verification follow |
| SYS-003 | RM M1 | DONE | M1 | `random_stream_tests`, including stream names |
| SYS-004 | RM 3.2 | DONE | M1 | dimension-safe types for time, length, area, volume, speed, amount, and concentration; compile/unit tests |
| SYS-005 | RM 2.1–2.2 | PART | M0/M1 | architecture review of target structure and dependency rules |
| SYS-006 | RM M0 | PART | M1 | CI rule and review: `core/` does not import `scenarios/` |
| SYS-007 | RM M1 | DONE | M1 | stable error codes/CLI statuses and negative tests for configuration, overflow, lifecycle, log, and checkpoint invariants |
| SYS-008 | DISS pp. 95–97, 133 | SPEC | M3–M5 | same scenario with surrogate and detailed model |
| ARC-001 | DISS pp. 94–96 | PART | M3–M5 | generic `ModelComponent` boundary and first organ implementation; body/capillary/cell implementations follow |
| ARC-002 | DISS p. 95 | PART | M3–M5 | lung-compartment model card records scale, evidence, and validity; all further models require equivalent cards |
| ARC-003 | DISS pp. 95–97 | PART | M3 | versioned entity, population, substance-amount, and volume-flow contracts exist; physiological-state and event contracts follow |
| ARC-004 | RM 3.3 | PART | M3 | entity round trip and exact transfer-ID/payload balance tests pass; quantity endpoint integration follows |
| ARC-005 | DISS pp. 99–100 | SPEC | M3–M5 | return of a detection/cell event to a higher layer |
| ARC-006 | RM 3.3 | PART | M3 | fixed-transit lung result agrees for compatible 0.5 s and 1 s steps; orchestrated multirate synchronization follows |
| ARC-007 | DISS pp. 96–97, 133 | SPEC | M4/M5 | reference adapter plus equivalent surrogate |

## 3. Body and organ layers

| ID | Source | Status 2026-08-27 | Target | Planned verification |
|---|---|---|---|---|
| BODY-001 | DISS p. 100; BVS18 | DONE | M2 | complete, strongly connected, schema-validated 95-segment graph; converter and graph-invariant tests |
| BODY-002 | DISS pp. 100, 113–117 | DONE | M2 | SI schema and validator for ID, type, geometry, length, diameter, cross section, volume, flow, sources, and uncertainty; canonical M2.2 data set |
| BODY-003 | DISS pp. 101–104 | LEGACY | M2 | analytical 3D geometry tests and model report |
| BODY-004 | DISS pp. 117, 140–143 | DONE | M2 | the same validated loader and CLI load the synthetic four-segment and canonical 95-segment graphs; state profiles are also applied without rebuilding |
| BODY-005 | DISS pp. 99–104 | DONE | M2 | scheduled injection and partial/complete extraction, identity-preserving transport, stable extraction selection, and conservation of active plus extracted equals injected |
| BODY-006 | DISS pp. 100–101, 118–122 | DONE | M2 | 23 dissertation transitions, supported vessel-9 split, stationary flow conservation, reproducible branching, perfusion regression, and flow-conserving state recomputation; independent physiological validation remains separate |
| BODY-007 | DISS pp. 115–117 | PART | M2/M4 | deterministic transit compartment implemented; laminar/streamline variant and reference comparison follow |
| BODY-008 | DISS pp. 120–122 | PART | M2/M3 | machine-readable rest, 1.9× exercise, and 70° head-up-tilt profiles with sources, validity, and CLI exist; regional exercise redistribution, vertebral drainage, pressure, compliance, and dynamic state transitions follow in M3/M4 |
| BODY-009 | DISS p. 101 | RESEARCH | M4/M5 | documented blood-model variants and sensitivity |
| BODY-010 | BVS18 pp. 4–6 | DONE | M2 | deterministic 6,359/63,590-particle regression, equilibrium at minute 7, injection-site comparison, exact population conservation, and schema-validated golden reference |
| ORG-001 | DISS pp. 118–126 | PART | M3 | coarse pulmonary-circulation surrogate has a model card and contract tests; anatomy, perfusion, and activity-state variants follow |
| ORG-002 | DISS pp. 118–122 | PART | M3 | M2.4 checks 23 target/actual perfusions in the whole-body graph; organ-specific lung model and independent physiological validation follow in M3/M2.6 |
| ORG-003 | DISS pp. 95, 153–154 | DONE | M3 | tested body → lung → body ownership round trip with named ports, stable identity, synchronization time, and explicit outside-body ledger |
| ORG-004 | DISS pp. 122–123, Ch. 6 | LEGACY | M3/M7 | localization event with tissue and uncertainty |
| ORG-005 | DISS pp. 123–126 | PART | M3 | coarse and three-region pulmonary implementations pass the same entity contract; anatomical 0D/1D refinement and scenario-level parameter switching follow |
| ORG-006 | DISS pp. 124–126 | RESEARCH | M3/M8 | reproducible conversion with geometry verification |

## 4. Capillary and cell layers

| ID | Source | Status 2026-08-27 | Target | Planned verification |
|---|---|---|---|---|
| CAP-001 | DISS pp. 126–129 | SPEC | M4 | parameterized arteriole–capillary–venule reference case |
| CAP-002 | DISS pp. 126–129 | RESEARCH | M4 | organ-specific parameter cards and literature comparison |
| CAP-003 | DISS pp. 127–128 | SPEC | M4 | sphincter/activity scenario test |
| CAP-004 | DISS p. 127 | SPEC | M4 | substance balance across blood/interstitium/cell |
| CAP-005 | DISS pp. 128–129 | RESEARCH | M4/M5 | transit/retention distributions with validity scope |
| CAP-006 | DISS pp. 129, 154 | SPEC | M4 | analytical channel and one external/surrogate adapter |
| CAP-007 | DISS p. 129 | RESEARCH | M4/M6 | reachability and multi-hop comparison |
| CELL-001 | DISS pp. 129–132 | SPEC | M5 | biomarker field with analytical reference case |
| CELL-002 | DISS pp. 130–132 | RESEARCH | M5/M7 | binding/threshold model with FP/FN evaluation |
| CELL-003 | DISS pp. 130–132, 153–154 | SPEC | M5 | release–diffusion–binding end-to-end test |
| CELL-004 | DISS pp. 131–133, 154 | RESEARCH | M5 | ODE/SSA or distribution model against reference data |
| CELL-005 | DISS pp. 153–154 | RESEARCH | M5 | apoptosis event plus feedback to scenario |

## 5. Nano-IoT and research data

| ID | Source | Status 2026-08-27 | Target | Planned verification |
|---|---|---|---|---|
| IOT-001 | DISS pp. 113–115, 186–188 | LEGACY | M2/M7 | generic device type plus locator/collector composition |
| IOT-002 | DISS pp. 96–97 | SPEC | M6 | nano in-body → gateway → BAN → station |
| IOT-003 | DISS pp. 117–118, 187–190 | PART | M2/M6 | passive segment-bound gateway measurement site with exact total and bounded individual passages; range, detection errors, and communication follow in M6 |
| IOT-004 | RM M6 | SPEC | M6 | communication report with latency/loss/energy |
| IOT-005 | DISS pp. 96–100; MEH25 | SPEC | M6 | interchangeable network adapter without kernel dependency |
| DATA-001 | RM 6.1 | PART | M1/M2 | versioned schemas and validators for experiment, provenance, log, and checkpoint; domain M2 data schemas follow |
| DATA-002 | RM 6.4 | DONE | M1 | JSON Schema `1.0.0`, manifest and CLI negative tests |
| DATA-003 | RM 2.4, 6.4 | PART | M1 | schema `1.0.0`, automatically generated `provenance.json`, SHA-256 and contract tests; data-version catalog follows with real models |
| DATA-004 | VIS20; MEH25 | DONE | M2 | separate extraction events, measurement-site counters, bounded individual observations, time aggregates, and optional complete/first-N trajectories with explicit truncation indicators and JSON Schema |
| DATA-005 | RM 2.5 | SPEC | M0/M1 | evidence class in every model card |
| DATA-006 | RM 6.2 | SPEC | M0 | documented calibration/validation split |
| DATA-007 | RM 6.3 | SPEC | M7 | result report with intervals and sensitivity |
| DATA-008 | RM 6.2 | SPEC | M2 | statistical regression tests with justified tolerance |

## 6. Operation, scenarios, and quality

| ID | Source | Status 2026-08-27 | Target | Planned verification |
|---|---|---|---|---|
| UX-001 | RM Phase 0/10 | PART | M1 | headless CLI run in CI |
| UX-002 | VIS20 pp. 1–2 | LEGACY | M7 | standardized result format in new visualization |
| UX-003 | VIS20 pp. 1–2 | LEGACY | M7 | visual acceptance test and run comparison |
| UX-004 | RM 6.5 | SPEC | M7 | identical rendering from archived run |
| SCN-001 | DISS pp. 185–190; FP23 | LEGACY | M7 | [fingerprinting reference scenario](FINGERPRINTING_SCENARIO.md) |
| SCN-002 | DISS pp. 143–152 | LEGACY | Phase 8 | monitoring reference experiment and alert metrics |
| SCN-003 | DISS pp. 155–160 | LEGACY | Phase 8 | reproduction of published detection rates |
| SCN-004 | DISS pp. 153–154 | SPEC | Phase 8 | complete multilayer capstone |
| SCN-005 | MEH25 pp. 1–2 | LEGACY | Phase 8 | CAR-T benchmark and model comparison |
| SCN-006 | DISS pp. 140–143 | SPEC | M8 | incrementally personalized research twin |
| QUA-001 | RM M1 | DONE | M1 | green MSVC/GCC/Clang CI matrix |
| QUA-002 | RM M1 | DONE | M1 | clang-tidy, ASan/UBSan, and warnings as errors in CI |
| QUA-003 | MEH25 pp. 1–2 | SPEC | M2–M7 | versioned benchmark reports plus result comparison |
| QUA-004 | MEH25 pp. 1–2 | SPEC | M4–M7 | scaling test: agents versus populations/surrogate |
| QUA-005 | RM 6.6 | PART | M1 ongoing | API/schema/model/scenario documentation |
| QUA-006 | RM M8 | SPEC | M8 | data-protection and data-management review |

## 7. M0 coverage review

Phase 0 is complete at domain level when the following decisions and inventories
exist in addition to these documents:

- [x] four layers and responsibilities defined as binding (`ARC-001` through `ARC-007`);
- [x] legacy as reference, selective adoption, and a new kernel decided (ADR-0001);
- [x] C++20/CMake/vcpkg selected as the technical foundation (ADR-0003);
- [x] fingerprinting selected as the first vertical demonstrator (ADR-0004);
- [x] evidence and validity classes defined (ADR-0005 and `DATA-005/006`);
- [x] data inventory, including external models, complete;
- [x] target users and prioritized workflows defined as the M0 baseline;
- [x] reference organ for M3 selected: lung (ADR-0006);
- [x] partner/data gaps for proteomics, pulmonary hemodynamics, and wet-lab validation identified;
- [x] multiple licensing implemented technically: MPL-2.0 for independent Next code, GPL-2.0-only for legacy and direct ports, and CC-BY-4.0 for new original documentation and approved original data (ADR-0007).

M0 is therefore complete. Unresolved rights for individual existing data and
publications are tracked as release gates for the respective artifacts; they
block neither M1 nor independently developed Next releases. Details are in the
[M0 gate review](../m0/M0_GATE_REVIEW.md).
