<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0042: Versioned nanodevice and local-message contract

- Status: Accepted
- Date: 2026-09-01

## Context

M4 can transport and observe an entity named `nanodevice`, and M5 can address a
device/payload activation boundary. Neither layer owns a general device model
or a logical communication protocol. M6 must introduce device capabilities,
resources, messages, relays, gateways, and external links without making the
physiological kernel or models depend on ns-3 or one network implementation.

The first increment needs a stable semantic boundary before propagation, loss,
latency, clustering, or gateway behavior can be compared. Otherwise every
communication implementation would define device state and message identity
differently.

## Decision

Add an independent `MEHLISSA::iot_model` library depending only on the neutral
kernel. A strict profile defines one device's freely named type, target,
initial lifecycle state, composable standard capabilities, payload inventory,
energy, message-size/count limits, and reception storage.

The runtime device uses the lifecycle `dormant -> active -> depleted` with a
one-way failure transition. It accounts for transmission and reception energy,
message counts, and stored bytes. Invalid state, missing capability, expired or
misaddressed messages, duplicates, and exceeded budgets are rejected before
partial resource mutation.

The local-message contract carries version, stable message/source/target
identity, message kind, experiment correlation, source event, creation and
validity time, hop limit, byte size, content type, and content. M6.1 models only
explicit endpoint emission and reception. Later communication adapters own
delivery probability, latency, noise, interference, routing, energy additions,
and metrics. Physiology may produce or consume neutral events but does not
depend on a communication implementation.

## Consequences

- Scenarios can define locator, collector, relay, actuator, or new specialized
  types by capability composition and a free type ID.
- Device resources and lifecycle are testable before network behavior exists.
- Message provenance can remain intact from a biological event through later
  gateway and external-report adapters.
- Energy becomes a dimension-safe core quantity in joules.
- M6.1 does not claim physical transmission, propagation, reception
  probability, gateway operation, or physiological device parameters.
- A later network adapter may drop a message as a modeled result; the strict
  endpoint currently throws only for invalid use or violated contracts.

## Alternatives considered

- Extending capillary entities with communication fields was rejected because
  it would couple physiology and network semantics.
- Starting directly with ns-3 packet objects was rejected because it would
  make ns-3 a dependency of all communication callers and obscure stable domain
  identity.
- An unrestricted string map for device properties was rejected because units,
  capability combinations, and resource invariants would not be checkable.
- Hard-coded locator and collector subclasses were rejected because M6 and M7
  require scenario-defined specialized types.

## Affected requirements and gates

This completes the software-contract portion of `IOT-001` and starts Gate M6.
It prepares `IOT-002` through `IOT-005` but does not satisfy their networking,
gateway, metric, or adapter requirements.
