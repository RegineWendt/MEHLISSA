<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M6 Implementation and Evidence

## Objective

M6 adds a communication plane above the existing physiological and cellular
layers. It connects in-body nanodevices, local communication, relays, an active
gateway, a body-area network, and an external analysis/control station while
keeping communication models replaceable and their metrics separate from
biological results.

Gate M6 is **in progress**. M6.1 supplies the device and message foundation;
M6.2 connects a checked M5 detection to a local collector over an explicit
one-hop link. M6.3 adds bounded cluster routing and relay forwarding. M6.4 adds
an active local gateway, a neutral measurement publication boundary, and a
routed local command downlink. M6.5 adds replaceable BAN transport, an external
analysis/control station, explicit transport-policy governance, and a causal
return path to a local actuator. Authenticated or clinical control and
command-driven actuation effects do not exist yet.

## Implemented increments

### M6.1 - nanodevice and local-message contract

- independent `MEHLISSA::iot_model` library with no body, organ, capillary,
  cell, or ns-3 dependency;
- free device-type ID plus composable sensing, transmission, reception, relay,
  collection, actuation, and payload-release capabilities;
- explicit target and amount- or unit-count payload inventories;
- dimension-safe energy, message size, storage, transmission, and reception
  budgets;
- dormant, active, depleted, and failed lifecycle with checked transitions;
- local detection, fingerprint-tile, measurement, control, and acknowledgement
  message kinds;
- stable source, target, correlation, source-event, time, hop, size, and
  content identities;
- strict profile schema `1.0.0` and separate synthetic locator and collector
  examples;
- one checked locator-to-collector hand-off with exact energy, count, and
  storage accounting; and
- negative tests for capability combinations, ambiguous payloads, invalid
  lifecycle use, routing, expiry, duplicates, and resource overruns.

See [Nanodevice and Local-Message Contract](NANODEVICE_AND_LOCAL_MESSAGE_CONTRACT.md)
and [ADR-0042](../architecture/adr/0042-versioned-nanodevice-and-local-message-contract.md).

### M6.2 - detection adapter, one-hop link, and metrics

- neutral, versioned molecular-detection event preserving biological source,
  request, detector, signal, compartment, time, and observed fraction;
- separate `MEHLISSA::iot_cosimulation` adapter from a successful causal M5
  receptor response, leaving cell and IoT libraries mutually independent;
- data-configured detection-message adapter preserving exact event and
  experiment correlation in the M6.1 envelope;
- replaceable `OneHopLinkModel` contract with explicit delivery results;
- deterministic scheduled link reporting delivery, loss, corruption, or
  validity expiry without treating modeled non-delivery as a software error;
- exact attempted/delivered counts and bytes, delivered latency, channel loss,
  corruption, expiry, transmitter energy, receiver energy, and link energy;
- strict local-communication profile schema and synthetic executable example;
  and
- checked M5 threshold-to-locator-to-link-to-collector reference plus negative
  causality, source, configuration, and expiry cases.

See [Detection to One-Hop Communication](DETECTION_TO_ONE_HOP_COMMUNICATION.md)
and [ADR-0043](../architecture/adr/0043-neutral-detection-event-and-replaceable-one-hop-link.md).

### M6.3 - cluster, relay, and bounded multi-hop routing

- strict directed cluster topology with explicit member roles and hop bound;
- deterministic `fewest_hops` and `lowest_total_latency` route strategies;
- loop-free route selection with stable tie-breaking;
- immediate store-and-forward through checked relay-capable devices;
- preservation of payload, correlation, source event, and absolute expiry;
- unique per-hop sender/message identity and decremented hop budget;
- explicit terminal delivery/drop state plus all individual hop results;
- aggregated message, byte, latency, loss/error/expiry, and endpoint/link energy
  metrics; and
- positive two-hop, alternative-route, second-hop-loss, hop-bound, and relay
  capability tests.

See [Cluster, Relay, and Bounded Multi-Hop Communication](CLUSTER_RELAY_AND_MULTI_HOP.md)
and [ADR-0044](../architecture/adr/0044-bounded-cluster-routing-and-store-forward-relay.md).

### M6.4 - active gateway, measurement uplink, and command downlink

- `ActiveGateway` composed with a normal M6.1 resource-bounded endpoint;
- strict endpoint-profile identity and receive/collect/transmit capabilities;
- versioned network-neutral `GatewayMeasurement` preserving complete local and
  biological trace identity;
- configured accepted uplink kinds and bounded publication capacity;
- versioned `GatewayCommand` with target, correlation, time, validity, hop,
  size, content type, and content;
- checked mapping to a local `control` message with duplicate, self-target, and
  capacity rejection;
- M6.3-routed collector-to-gateway measurement and gateway-to-relay-to-actuator
  command references; and
- exact latency, byte, endpoint/link energy, storage, and count verification.

See [Active Gateway Measurement Uplink and Command Downlink](ACTIVE_GATEWAY_UPLINK_AND_DOWNLINK.md)
and [ADR-0045](../architecture/adr/0045-active-gateway-measurement-and-command-boundary.md).

### M6.5 - BAN adapters and external analysis/control station

- versioned `BanFrame` envelope for gateway measurements and governed commands;
- stateful gateway adapter preserving previously published measurement and
  correlation identities;
- external station with explicit gateway, target, content-type, correlation,
  duplication, and capacity policy;
- normal typed denial outcomes and downlink creation only after approval;
- replaceable `BanTransportAdapter` boundary with a deterministic scheduled
  reference implementation;
- separate BAN count, byte, latency, loss, corruption, expiry, and transmitter,
  receiver, and link-energy metrics;
- complete station-to-gateway-to-relay-to-actuator reference composition; and
- strict `1.0.0` profile schema plus causality, policy, replay, loss,
  corruption, and expiry tests.

See [BAN and External Analysis/Control Station](BAN_AND_EXTERNAL_STATION.md) and
[ADR-0046](../architecture/adr/0046-ban-adapter-and-governed-station-loop.md).

## Planned sequence

1. ~~Device, resource, lifecycle, and local-message foundation.~~ M6.1 complete.
2. ~~Molecular-detection-to-message adapter and interchangeable one-hop link
   with explicit delivery result and communication metrics.~~ M6.2 complete.
3. ~~Cluster, relay, and bounded multi-hop routing.~~ M6.3 complete.
4. ~~Active nano/micro gateway with measurement uplink and command downlink.~~
   M6.4 complete.
5. ~~BAN and external station adapters, including closed command-to-device
   path.~~ M6.5 complete.
6. Optional external network-simulator adapter without physiological
   dependencies.
7. Loss, latency, error, energy, capacity, failure/security scenarios, User
   Guide review, and formal Gate M6 review.

## Current scientific boundary

All M6 profiles, device budgets, topology, link values, and outcome sequences
remain synthetic software-test values. M6.3 represents deterministic routing,
relay, latency, loss, corruption, expiry, and energy-accounting semantics, and
M6.5 adds a bidirectional BAN/station software path, but its scheduled links
are not physical channels or calibrated probability models. It does not model
anatomical placement, range, diffusion, interference, noise physics, capacity,
queues, retransmission, a concrete BAN protocol, authentication, cryptographic
authorization or integrity, encryption, clinical safety policy, or
command-driven biological actuation. The gateway, station, policy, and BAN
adapter are implementation-neutral software boundaries, not validated
hardware or medical-device controls.
