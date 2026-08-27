<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0020: Model Component and Versioned Entity Exchange

- **Status:** Accepted
- **Date:** 27 August 2026
- **Applies to:** M3–M5; `ARC-001` through `ARC-006`, `ORG-001`, `ORG-003`, and `ORG-005`

## Context

The M2 body transport owns its agents and vascular state. M3 must move selected
state into an independent organ model without allowing either layer to mutate
the other's internals. The four-layer decision requires versioned exchange
objects, explicit synchronization, preserved identity, and interchangeable
model variants.

The complete future boundary includes agents, populations, substance flows,
physiological states, and events. Implementing all semantics in one untyped or
mostly optional message would make conservation and validation ambiguous.

## Decision

`models/coupling` defines a domain-level `ModelComponent` interface above the
medical-neutral `core::SimulationComponent`. It adds:

- a stable model ID;
- explicit queries for named entity entry and exit ports;
- acceptance of a versioned `EntityTransfer`;
- explicit collection of outbound transfers.

Entity-transfer contract version `1.0.0` carries the stable entity ID and type,
source and target model/port IDs, and emission time. Receivers validate the
contract version, route, synchronization time, and duplicate identity before
changing state. A transfer is ownership hand-off: the sender must no longer
count the entity after acceptance, while the receiver must count it exactly
once until it emits the next transfer.

Population, substance-flow, physiological-state, and event transfers will use
separate typed contracts with their own conservation and compatibility rules.
They are not encoded as optional fields in `EntityTransfer`.

The first receiver is `LungCompartment`. It implements a fixed-transit
pulmonary surrogate with named arterial entry and venous exit. It does not add
lung-specific logic to `core` or the body graph.

## Consequences

Positive:

- boundary errors are rejected before silent loss or duplication;
- organ variants can implement the same contract without sharing state;
- identity, route, and temporal-order tests are independent of anatomy detail;
- later external simulators can be wrapped behind the same component boundary.

Negative:

- the body transport needs an adapter before a complete round trip exists;
- explicit ownership and synchronization require orchestrator bookkeeping;
- additional exchange categories require additional versioned types and tests;
- fixed transit is quantized to synchronization boundaries in the first slice.

## Rejected alternatives

- **Direct access to body particle containers:** couples both implementations and prevents independent replacement.
- **Medical hand-off methods in `core`:** violates the neutral-kernel boundary.
- **One generic map or JSON message for every exchange:** weakens compile-time units and makes conservation semantics implicit.
- **Immediate detailed pulmonary geometry:** postpones testing the architectural boundary and mixes coupling defects with model-data defects.
