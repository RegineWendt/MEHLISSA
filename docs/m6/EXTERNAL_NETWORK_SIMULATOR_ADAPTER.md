<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# External Network-Simulator Adapter

## Purpose

M6.6 allows the M6.5 BAN path to obtain one transfer result from ns-3 or
another separately built network simulator without making that simulator a
dependency of the MEHLISSA kernel or any physiological model.

The composition is:

```text
BanFrame
  -> ExternalNetworkSimulatorAdapter
  -> versioned metadata-only NetworkSimulationRequest
  -> NetworkSimulatorClient / JSON exchange implementation
  -> external simulator
  -> versioned NetworkSimulationResponse
  -> checked BanTransferResult
  -> BanCommunicationSession metrics
```

No external simulator is vendored, downloaded, or linked by the default build.
M6.6 establishes and verifies the integration contract. A project-specific
client may call an in-process simulator binding, a child process, a service, or
an offline co-simulation coordinator behind the provided interfaces.

## Dependency boundary

`ExternalNetworkSimulatorAdapter` implements the existing M6.5
`BanTransportAdapter`. It depends only on the IoT library and an injected
`NetworkSimulatorClient`. Body, organ, capillary, cell, gateway, station, and
BAN-frame code neither include simulator headers nor receive simulator-native
objects.

Two client levels are available:

- `NetworkSimulatorClient` exchanges typed request/response objects with an
  in-process integration; and
- `JsonNetworkSimulatorClient` uses `NetworkSimulatorJsonExchange`, allowing a
  thin project-specific boundary to exchange versioned JSON with an external
  executable or service.

This repository deliberately does not prescribe process creation, sockets,
HTTP, or a simulator installation path. Those are deployment concerns and can
be added without changing the MEHLISSA contracts.

## Metadata-only request

The `1.0.0` request contains:

- request, adapter, simulator, simulator-version, and scenario identities;
- BAN frame ID and measurement-uplink or governed-command-downlink kind;
- source and target endpoint IDs;
- correlation and source-event IDs;
- exact departure and absolute-validity times in integer nanoseconds; and
- frame size in bytes.

It deliberately excludes measurement content, command content, biological
state, patient attributes, and the typed BAN payload. The external network
simulator receives only what is required to evaluate transport behavior.

The schemas are:

```text
data/schemas/network-simulation-request/1.0.0.schema.json
data/schemas/network-simulation-response/1.0.0.schema.json
```

## Response and result mapping

The response must echo request, adapter, simulator, version, scenario, and
frame identities. It reports a completion time, delivered/lost/corrupted
outcome, and separate transmitter, receiver, and link energy. The adapter
rejects malformed JSON, extra fields, identity mismatches, incompatible
simulator versions/scenarios, time reversal, non-finite or negative energy,
and exhausted attempt capacity.

Completion after the BAN frame's absolute validity becomes an `expired` BAN
drop regardless of the simulator's nominal outcome. Otherwise delivered,
lost, and corrupted map directly into the M6.5 result. The unchanged
`BanCommunicationSession` validates the adapter result and aggregates its
counts, bytes, latency, outcome, and energy. This also acts as a conformance
test for future simulator clients.

## Reference profile

The strict reference files are:

```text
examples/iot-models/synthetic-external-network-simulator-v1.json
examples/iot-models/synthetic-network-simulator-response-v1.json
data/schemas/network-simulator-adapter-profile/1.0.0.schema.json
```

The fixture returns one 12 ms delivery for the M6.5 256-byte measurement frame
and supplies synthetic transmitter, receiver, and link energy. Tests verify
the generated request and example response against their JSON Schemas, prove
that payload content is absent, and cover loss, corruption, validity expiry,
identity mismatch, unexpected JSON fields, and attempt capacity.

## Connecting ns-3 or another simulator

An integration module should:

1. implement `NetworkSimulatorClient` or `NetworkSimulatorJsonExchange`;
2. select and document one external simulator version and scenario/configuration
   identity in the adapter profile;
3. map request endpoints, size, direction, departure, and deadline to the
   external model without accessing physiological state;
4. return the exact echoed identities and measured/simulated result fields;
5. distinguish simulator execution failure from a modeled packet loss;
6. add adapter conformance tests plus provenance for the simulator binary,
   configuration, seed, and data; and
7. qualify every latency, energy, capacity, error, interference, mobility, and
   protocol claim using evidence appropriate to that external model.

An ns-3 client may remain GPL-2.0-only in a separate integration module. The
independent MPL-2.0 MEHLISSA Next library contains no ns-3 code or mandatory
link dependency.

## Limits

M6.6 verifies an exchange boundary, not network science. The checked client is
an in-process fixture and does not establish Bluetooth, IEEE 802.15.6,
molecular, optical, acoustic, cellular, or Internet behavior. MEHLISSA checks
response identity, time, validity, and metric consistency; it cannot by itself
validate an external simulator's topology, protocol, propagation, queue,
interference, capacity, mobility, or energy model.

See [ADR-0047](../architecture/adr/0047-versioned-external-network-simulator-boundary.md)
for the decision.
