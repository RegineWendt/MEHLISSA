<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2 – Validated Body Layer

**Status:** complete

**Started:** 27 August 2026

M2 replaces the implicit legacy topology with a versioned, dimension-safe, and
semantically validated vascular graph. Particle transport and scientific BVS
regressions are built on this contract.

## Planned increments

| Increment | Status | Gate verification |
|---|---|---|
| M2.1 vascular graph contract | complete | JSON Schema, SI data types, graph/geometry/flow invariants, synthetic reference graph, and platform CI |
| M2.2 legacy-1995 migration | complete | deterministic converter, CC BY provenance, 95-segment SI graph, supported split for vessel 9, and profile separation |
| M2.3 deterministic compartment transport | complete | scheduled injection, data-driven transit times, named transition random stream, single-movement and population-conservation tests |
| M2.4 BVS reference run | complete | 6,359/63,590 runs, equilibrium, injection location, perfusion, exact conservation, and schema-validated golden reference within predefined tolerances |
| M2.5 output and measurement sites | complete | bounded complete/first-N trajectories, time aggregates, deterministic extraction, passive sample/gateway measurement sites, and schema-validated result format |
| M2.6 alternative body models and states | complete | schema-validated rest, exercise, and head-up-tilt profiles, flow-conserving recomputation, and CLI application without rebuilding; physiological limitations explicit |

## M2.1 principles

- IDs are stable strings and need not be contiguous.
- The file format uses only explicitly named SI fields.
- Topology is explicit; coordinate equality alone does not create an edge.
- A closed reference circulation must be strongly connected.
- Length, cross section, volume, velocity, and flow are cross-checked.
- Transition probabilities must be complete, unambiguous, and normalized to one.
- Sources, license, evidence quality, uncertainty, and validity scope are part of the model contract.
- Medical IDs and organ knowledge remain outside `core/`.

The legacy findings and migration boundary are documented in
[`LEGACY_95_DATA_AUDIT.md`](LEGACY_95_DATA_AUDIT.md).

## M2.2 – 1995 migration

The authorized sources are transformed by a strict converter into the canonical
profile `data/body-models/bvs95-dissertation-rest-v1.json`. Source files remain
unchanged. IDs, types, and coordinates are mapped bijectively, units are
explicitly converted to SI, flows are conserved, and all edges are stored. For
rest/supine state, vessel 9 uses the supported `0.2875/0.7125` split to the left
and right internal jugular veins.

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe migrate-legacy-95 `
  --vasculature mehlissa2.0/data/95_vasculature.csv `
  --transitions mehlissa2.0/data/95_transitions.csv `
  --output data/body-models/bvs95-dissertation-rest-v1.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

License, hashes, and transformations are recorded in
`data/legacy/bvs95/release-v1.json`. The methodological separation between the
executable reproduction profile and the future adult-female rest/supine
reference is described in
[`PHYSIOLOGICAL_BASELINE.md`](PHYSIOLOGICAL_BASELINE.md) and made binding in
[ADR-0016](../architecture/adr/0016-legacy-95-release-and-reference-profiles.md).

## Using M2.1

From the repository root, the CLI first validates the JSON Schema and then all
semantic graph and flow invariants:

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe validate-body `
  --model examples/body-models/synthetic-branching-circuit.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

Without `--schema`, `data/schemas/vascular-graph/1.0.0.schema.json` is used
relative to the current working directory.

## M2.3 – deterministic compartment transport

The first transport component treats every vessel segment as a perfused
compartment while retaining the stable identity of every mobile entity. An
entity accumulates residence time in its current segment. After the transit
time calculated from `length / mean velocity`, it is transferred to exactly one
successor according to the transition probabilities stored in the graph.

“Deterministic” means that branching remains stochastic, but an experiment with
the same master seed, events, and time step produces exactly the same sequence.
The component therefore uses its own named random stream,
`body.compartment-transport.transitions`, and no implementation-dependent
standard distribution algorithm.

The invariants, limitations, and API are documented in
[`COMPARTMENT_TRANSPORT.md`](COMPARTMENT_TRANSPORT.md). The architecture
decision is documented in
[ADR-0015](../architecture/adr/0015-deterministic-compartment-transport.md).

## M2.4 – BVS reference regression

The event-driven reference runner operationalizes published BVS claims about
equilibrium after about seven minutes, injection location, and a tenfold
population. Separately, it checks the dissertation's 23 perfusion targets
against the 95-segment graph. All gates were fixed before the first complete
run; the generated JSON report is schema-validated and checked byte for byte as
a golden reference.

The method, results, and limitations—especially separation of 94-vessel BVS,
the 95-vessel dissertation profile, and physiological validation—are documented
in [`BVS_REFERENCE_REGRESSION.md`](BVS_REFERENCE_REGRESSION.md) and
[ADR-0017](../architecture/adr/0017-bvs-reference-regression.md).

## M2.5 – output and measurement sites

Transport now supports scheduled partial and complete extraction, passive
sample and gateway measurement sites, and independently bounded individual
observations, trajectories, and temporal population aggregates. Total counters
and the conservation invariant `active + extracted = injected` remain exact
even when detailed output is truncated.

The result is written as schema-validated JSON. Semantics, event order,
truncation rules, and the deliberate boundary between a passive M2.5
measurement site and an active M6 gateway are documented in
[`TRANSPORT_OBSERVATION.md`](TRANSPORT_OBSERVATION.md) and
[ADR-0018](../architecture/adr/0018-bounded-transport-observation.md).

## M2.6 – alternative models and states

Versioned overlays now separate the stable vascular graph from rest, exercise,
and posture. A profile is bound to a model ID and version, can change cardiac
output and existing branching ratios, and then resolves the complete stationary
flow graph. Geometry remains fixed, velocity follows consistently from
`v = Q/A`, and the derived graph must again satisfy all M2.1 invariants.

The distribution includes a historical resting identity, a literature-based
1.9-fold exercise sensitivity, and a head-up-tilt sensitivity from 6.0 to
4.7 L/min. Missing regional territories and the vertebral venous plexus are
reported as limitations rather than replaced with invented transitions.
Details and reproduction instructions are in
[`BODY_STATE_PROFILES.md`](BODY_STATE_PROFILES.md) and
[ADR-0019](../architecture/adr/0019-versioned-body-state-overlays.md).

The continuously expanded English [User Guide](../USER_GUIDE.md) describes
practical use of all functions implemented so far.
