<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Architecture Decisions

Architecture Decision Records (ADRs) document decisions that shape the further
development of MEHLISSA Next. They explain context, decision, consequences, and
rejected alternatives. The English translations preserve the accepted
decisions; a new decision supersedes an existing one through a new ADR.

| ADR | Status | Decision |
|---|---|---|
| [ADR-0001](adr/0001-new-kernel-and-legacy-policy.md) | Accepted | new kernel; legacy code as a reference and source of selective ports |
| [ADR-0002](adr/0002-four-layer-cosimulation.md) | Accepted | four independent layers with explicit coupling contracts |
| [ADR-0003](adr/0003-cpp20-cmake-vcpkg.md) | Accepted | C++20 kernel with CMake/vcpkg; Python later as the experiment API |
| [ADR-0004](adr/0004-fingerprinting-first-vertical-slice.md) | Accepted | fingerprinting as the first vertical demonstrator |
| [ADR-0005](adr/0005-model-evidence-and-validity.md) | Accepted | explicit separation of evidence, calibration, validation, and hypotheses |
| [ADR-0006](adr/0006-lung-reference-organ.md) | Accepted | lung as the first reference organ and an incremental pulmonary model |
| [ADR-0007](adr/0007-repository-license.md) | Accepted | MPL-2.0 for independent Next code, GPL-2.0-only for legacy/ports, and CC-BY-4.0 for new original documentation and approved data |
| [ADR-0008](adr/0008-versioned-experiment-manifest.md) | Accepted | strict, versioned JSON experiment manifest using JSON Schema 2020-12 |
| [ADR-0009](adr/0009-run-provenance.md) | Accepted | automatically generated, schema-validated run provenance with cryptographic checksums |
| [ADR-0010](adr/0010-dimensional-quantity-system.md) | Accepted | dimension-safe SI quantities in the kernel and explicit units at system boundaries |
| [ADR-0011](adr/0011-simulation-context-and-lifecycle.md) | Accepted | explicit run context and uniquely owned component lifecycle |
| [ADR-0012](adr/0012-errors-logs-and-checkpoints.md) | Accepted | stable error identifiers, JSONL run logs, and versioned checkpoint manifests |
| [ADR-0013](adr/0013-cross-platform-determinism-reference.md) | Accepted | byte-identical golden-reference run for MSVC, GCC, and Clang |
| [ADR-0014](adr/0014-validated-vascular-graph-contract.md) | Accepted | versioned SI vascular graph with geometry, topology, and flow invariants |
| [ADR-0015](adr/0015-deterministic-compartment-transport.md) | Accepted | deterministic, data-driven compartment transport |
| [ADR-0016](adr/0016-legacy-95-release-and-reference-profiles.md) | Accepted | CC BY release and validated migration of the 1995 model; historical and physiological profiles remain separate |
| [ADR-0017](adr/0017-bvs-reference-regression.md) | Accepted | separate BVS dynamics and dissertation perfusion regressions with predefined gates |
| [ADR-0018](adr/0018-bounded-transport-observation.md) | Accepted | bounded trajectories and aggregates, deterministic extraction, and passive sample/gateway measurement sites |
| [ADR-0019](adr/0019-versioned-body-state-overlays.md) | Accepted | versioned body states with a cardiac-output anchor and flow-conserving recomputation without topology changes |
| [ADR-0020](adr/0020-model-component-and-entity-exchange.md) | Accepted | versioned model-component boundary and conservative entity hand-off for M3 |
| [ADR-0021](adr/0021-versioned-capillary-bed-baseline.md) | Accepted | strict arteriole-capillary-venule component and lossless M4 reference baseline |
| [ADR-0022](adr/0022-organ-capillary-round-trip-coupling.md) | Accepted | explicit four-port organ-capillary route with pending queues and an ownership ledger |
| [ADR-0023](adr/0023-dimension-safe-capillary-continuity.md) | Accepted | geometry and flow as inputs; area, velocity, and transit derived with dimensional quantities |
| [ADR-0024](adr/0024-dynamic-capillary-recruitment.md) | Accepted | versioned sphincter-group schedules with explicit fixed-flow or fixed-pressure boundary semantics |
| [ADR-0025](adr/0025-balanced-capillary-exchange.md) | Accepted | optional staged substance partition with explicit four-compartment mass balance |
| [ADR-0026](adr/0026-non-state-changing-capillary-entity-observation.md) | Accepted | exact regional residence and bounded competing-interaction likelihood observations without changing entity ownership |
| [ADR-0027](adr/0027-evidence-qualified-equivalent-pulmonary-capillary-card.md) | Accepted | evidence-qualified pulmonary volume-flow-transit closure with explicit equivalent-geometry semantics |

## Status model

- `Proposed`: submitted for discussion;
- `Accepted`: binding for new work;
- `Superseded`: replaced by a named, later ADR;
- `Rejected`: considered but not selected;
- `Deprecated`: still documented but should no longer be used.

## New ADRs

New files receive the next four-digit number. They should contain at least:

1. status and date;
2. context and forces to resolve;
3. concrete decision;
4. positive and negative consequences;
5. alternatives considered;
6. affected requirements or roadmap gates.
