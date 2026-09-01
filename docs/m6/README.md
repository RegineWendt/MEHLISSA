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
one-hop link. No gateway, end-to-end external measurement, or downlink exists
yet.

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

## Planned sequence

1. ~~Device, resource, lifecycle, and local-message foundation.~~ M6.1 complete.
2. ~~Molecular-detection-to-message adapter and interchangeable one-hop link
   with explicit delivery result and communication metrics.~~ M6.2 complete.
3. Cluster, relay, and bounded multi-hop routing.
4. Active nano/micro gateway with measurement uplink and command downlink.
5. BAN and external station adapters, including closed command-to-device path.
6. Optional external network-simulator adapter without physiological
   dependencies.
7. Loss, latency, error, energy, capacity, failure/security scenarios, User
   Guide review, and formal Gate M6 review.

## Current scientific boundary

All M6 profiles, device budgets, link values, and outcome sequences remain
synthetic software-test values. M6.2 now represents explicit latency, loss,
corruption, expiry, and energy-accounting semantics, but its scheduled link is
not a physical channel or calibrated probability model. It does not model
range, diffusion, interference, noise physics, capacity, routing, security, a
gateway, a BAN, or an external station.
