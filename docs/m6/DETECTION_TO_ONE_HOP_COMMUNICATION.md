<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Detection to One-Hop Communication (M6.2)

## Purpose

M6.2 closes the first executable boundary between biological detection and
communication. A successful M5 receptor-threshold response becomes a neutral
detection event, a locator emits a traceable local message, and an
interchangeable one-hop link either delivers it to a collector or reports why
it was dropped.

This is not yet an external measurement. The reference ends at an in-process
local collector.

## Dependency boundary

```text
M5 ReceptorLigandResponse
        |
        | MEHLISSA::iot_cosimulation adapter
        v
MolecularDetectionEvent  ->  LocalMessageRequest
                                   |
                            locator emission
                                   |
                            OneHopLinkModel
                                   |
                      delivered / loss / corruption / expiry
                                   |
                        collector reception if delivered
```

The cell library has no M6 dependency. The IoT library has no cell dependency.
Only the dedicated adapter library depends on both. A different biological
detector or a different link can therefore be introduced independently.

## Versioned profile

The executable composition is defined by:

```text
examples/iot-models/synthetic-local-communication-v1.json
data/schemas/local-communication-profile/1.0.0.schema.json
```

The profile configures:

- source locator and target collector IDs;
- stable message prefix, experiment correlation, content type, validity, hop
  limit, and declared size;
- link kind and ID;
- fixed latency and per-attempt link energy;
- a repeating sequence of `delivered`, `lost`, and `corrupted` outcomes; and
- evidence scope, sources, and explicit limitations.

Unknown fields are rejected. The reference outcome, latency, and link energy
must agree exactly with the executable link configuration.

## Checked detection-to-collector reference

The M5.1 synthetic receptor profile reaches its threshold after
`2.746530721 s` in the integer simulation clock. The adapter preserves the
source cell model, binding request, ligand, compartment, locator, final bound
fraction, and event ID. It creates a 320-byte logical detection message whose
content repeats the source model, request, signal, compartment, and observed
fraction so that the collector record remains self-describing.

The first scheduled link outcome is delivered:

| Quantity | Checked value |
|---|---:|
| detection time | `2.746530721 s` |
| link latency | `0.025 s` |
| collector completion time | `2.771530721 s` |
| locator transmission energy | `0.5 µJ` |
| collector reception energy | `0.25 µJ` |
| link-model energy | `0.1 µJ` |
| attempted/delivered messages | `1 / 1` |
| attempted/delivered bytes | `320 B / 320 B` |

The message's `source_event_id` is
`detection.m6-2.receptor-threshold.1`, so the collector record can be traced
back to the precise M5 response.

## Communication metrics

`CommunicationMetrics` is deliberately separate from receptor, cell, organ,
and capillary results. It records:

- attempts, deliveries, prescribed channel losses, corruptions, and validity
  expiries;
- attempted and delivered bytes;
- total, maximum, and mean latency for delivered messages; and
- transmitter, receiver, and link energy as three separately owned quantities.

A three-attempt verification sequence (`delivered`, `lost`, `corrupted`)
produces delivery fraction `1/3`, total drop fraction `2/3`, channel-loss
fraction `1/3`, and corruption fraction `1/3`. All three attempts consume
transmitter and link energy; only the delivered attempt consumes receiver
energy.

## Interpretation boundary

All latency, energy, size, and outcome values are synthetic. A repeating
outcome sequence verifies deterministic semantics; it is not an estimated loss
or bit-error probability. The link has no geometry, range, throughput,
queueing, contention, interference, noise spectrum, encoding, or physical
carrier. M6.3 must add clusters, relay selection, and bounded multi-hop
forwarding without weakening the M6.2 event trace or communication metrics.
