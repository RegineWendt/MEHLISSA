<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0002: Four Layers as Explicit Co-Simulation

- **Status:** Accepted
- **Date:** 26 August 2026
- **Applies to:** M0, M3–M5; `ARC-001` through `ARC-007`

## Context

The dissertation defines body, organ, capillary, and cell layers as connected
but independent modules with different spatial and temporal scales. The
historical implementations focus primarily on whole-body transport and partly
represent layer transitions through special logic in shared classes.

A comprehensive simulation cannot calculate all biological scales at the same
resolution simultaneously. The architecture must therefore activate detailed
models locally, continue running coarser models alongside them, and exchange
state in a controlled manner.

## Decision

Each layer is implemented as an independent `ModelComponent` with its own state,
time-step/event model, and validity scope. Components communicate exclusively
through versioned exchange objects and an orchestrator.

At least the following exchange types are provided:

- individual entities;
- aggregated populations and flows;
- physiological states;
- molecular signals;
- detection and cell events;
- actuation commands and measurements;
- evidence and uncertainty metadata.

The orchestrator coordinates synchronization points. Exchanges verify temporal
ordering, identity, and relevant conservation laws. Direct changes to another
layer's internal state are not permitted.

## Consequences

Positive:

- Model variants can be developed and validated independently.
- Coarse and detailed models can be combined according to the research question.
- External simulators can be connected through the same contracts.
- Errors at layer boundaries become observable and testable.

Negative:

- Coupling contracts and synchronization add development effort.
- Temporal and spatial interpolation can introduce new numerical errors.
- Conservative exchanges of populations and substances require clear semantics.

## Rejected alternatives

- **One global time step for all models:** simple, but inefficient and partly numerically unsuitable for widely different scales.
- **Shared monolithic object graph:** impedes interchangeability, validation, and scalable abstractions.
- **Only a loosely coupled file-based pipeline:** useful for offline coupling, but insufficient for bidirectional runtime scenarios.
