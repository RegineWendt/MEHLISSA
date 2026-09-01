<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0047: Versioned external network-simulator boundary

- **Status:** Accepted
- **Date:** 2026-09-01
- **Decision owners:** MEHLISSA maintainers
- **Roadmap scope:** M6.6

## Context

M6.5 defines a replaceable `BanTransportAdapter`, but a direct ns-3 dependency
would again couple communication technology, build tooling, and GPL-licensed
simulator types to the primary model library. External simulators also vary in
whether they run in process, as a separate executable, as a service, or under
a co-simulation coordinator. The stable MEHLISSA boundary must therefore
describe the transport question and result without prescribing deployment.

Sending the complete `BanFrame` would unnecessarily expose measurement or
command content to a simulator that only needs network metadata. Accepting an
unstructured callback result would make identity, time, validity, and metrics
impossible to verify consistently.

## Decision

Add `ExternalNetworkSimulatorAdapter` as a `BanTransportAdapter`. It translates
one validated BAN frame into a versioned `NetworkSimulationRequest` containing
only identities, direction, endpoints, timing, deadline, and size. It obtains
a versioned `NetworkSimulationResponse` through an injected
`NetworkSimulatorClient`, checks complete identity echo and temporal/metric
consistency, applies BAN validity expiry, and returns a normal
`BanTransferResult`.

Provide both a typed client interface and a JSON client over the narrower
`NetworkSimulatorJsonExchange` interface. Publish strict request, response,
and adapter-profile JSON Schemas. Leave process, socket, HTTP, simulator API,
and installation decisions to separate integration modules.

External failures or malformed responses are software/integration errors.
Modeled delivery, loss, corruption, and validity expiry are normal BAN results.
Every initiated external call consumes a bounded adapter attempt even if its
response is invalid.

## Consequences

- Physiological and local communication models retain no simulator dependency.
- External simulators never need biological or command payload content.
- Exact simulator/scenario/version provenance is part of every request.
- Existing M6.5 sessions and metrics work unchanged with external results.
- ns-3, other libraries, processes, and services can use the same contract.
- The default build remains reproducible without an external simulator.
- A concrete external model still requires its own evidence, configuration,
  execution connector, provenance, and validation.

## Alternatives considered

- **Link ns-3 directly into `MEHLISSA::iot_model`:** rejected because it would
  introduce a mandatory technology and licensing/build dependency.
- **Pass complete BAN payloads:** rejected because network simulation requires
  metadata, not medical or command content.
- **Standardize one process or HTTP transport:** rejected because deployment
  environments differ and transport mechanics are below the semantic contract.
- **Accept latency alone:** rejected because outcome, identity, time, bytes, and
  separate energy are required for traceable M6 metrics.

## Verification

- strict adapter profile and request/response JSON Schemas;
- exact request generation and response identity echo;
- proof that measurement/payload content is absent from JSON requests;
- mapping of delivered, lost, corrupted, and expired results;
- integration through the unchanged `BanCommunicationSession` metrics; and
- rejection of malformed/extra JSON, mismatched identity, time reversal,
  invalid energy, and exhausted attempt capacity.
