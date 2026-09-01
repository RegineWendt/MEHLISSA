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

Gate M6 is **in progress**. M6.1 supplies the device and message foundation; no
end-to-end external measurement or downlink exists yet.

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

## Planned sequence

1. ~~Device, resource, lifecycle, and local-message foundation.~~ M6.1 complete.
2. Molecular-detection-to-message adapter and interchangeable one-hop link with
   explicit delivery result and communication metrics.
3. Cluster, relay, and bounded multi-hop routing.
4. Active nano/micro gateway with measurement uplink and command downlink.
5. BAN and external station adapters, including closed command-to-device path.
6. Optional external network-simulator adapter without physiological
   dependencies.
7. Loss, latency, error, energy, capacity, failure/security scenarios, User
   Guide review, and formal Gate M6 review.

## Current scientific boundary

All M6.1 profiles and budgets are synthetic software-test values. Direct API
emission followed by direct reception verifies semantics and accounting, not a
physical channel. It does not model propagation delay, range, diffusion,
interference, noise, loss, reception probability, security, a gateway, a BAN,
or an external station.
