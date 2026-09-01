<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Nanodevice and Local-Message Contract (M6.1)

## Purpose

M6.1 defines what an in-body communication device is and what a logical local
message must preserve. It deliberately stops before choosing how the message
travels. This lets an analytical link, molecular channel, packet network, or
external simulator implement delivery later without changing device identity
or physiological models.

## Device profile

The strict profile and two executable examples are:

```text
data/schemas/nanodevice-profile/1.0.0.schema.json
examples/iot-models/synthetic-locator-v1.json
examples/iot-models/synthetic-collector-v1.json
```

A profile contains:

- stable profile, device, type, and target identities;
- initial `dormant` or `active` state;
- a unique combination of `sense`, `transmit`, `receive`, `relay`, `collect`,
  `actuate`, and `release_payload` capabilities;
- payload inventory quantified either as molar amount or discrete units;
- initial energy in joules;
- transmit and receive energy per message;
- maximum message size, reception storage, transmissions, and receptions; and
- validity, evidence class, sources, licenses, and limitations.

The type ID is not an enum. A scenario can add a specialized device type by
choosing a new stable type ID and a valid capability combination without adding
a C++ subclass. Relay requires transmit plus receive; collection requires
receive. Messaging resources without the corresponding capability are
rejected.

## Lifecycle and resources

```text
dormant --activate--> active --exact energy exhaustion--> depleted
   |                    |                                  |
   `--------------------+--------------fail---------------->` failed
```

Only an active device may emit or receive. A failed or depleted device cannot
be reactivated. Before a message changes counters, storage, or energy, all
identity, lifecycle, capability, timing, and capacity conditions are checked.
An operation that is rejected therefore leaves resource state unchanged.

Reception storage is bounded. `take_received_messages()` transfers queued
messages to the caller and frees occupied bytes, while the device retains the
message IDs so replaying an already accepted message is still rejected.

## Local-message envelope

Contract `1.0.0` defines five initial kinds:

- `detection`;
- `fingerprint_tile`;
- `measurement`;
- `control`; and
- `acknowledgement`.

Every message carries source and target device, experiment correlation,
source-event identity, creation time, validity duration, hop limit, byte size,
content type, and content. The content remains opaque at this neutral envelope;
later typed detection, measurement, and command adapters must validate their
own payload schemas.

The envelope is not a packet-format claim. `size_bytes` is the declared logical
message size used for budgets and later metrics, not evidence of a realizable
molecular or radio encoding.

## Checked reference hand-off

At `100 ms`, the synthetic locator emits a 64-byte detection message addressed
to the collector and valid for one second. It references source event
`cell.binding.threshold.1` and correlation
`experiment.m6-1.local-message`.

| Quantity | Before | After |
|---|---:|---:|
| locator energy | `2.00 µJ` | `1.50 µJ` |
| locator transmissions | `0` | `1` |
| collector energy | `1.00 µJ` | `0.75 µJ` |
| collector receptions | `0` | `1` |
| collector occupied storage | `0 B` | `64 B` |

Taking the message returns occupied storage to zero. Separate tests prove that
an oversized emission does not consume the locator budget and an expired,
misaddressed, or duplicate reception does not consume the collector budget.

## Interpretation boundary

The reference devices, target, payload, energy, sizes, and counts are synthetic.
Direct method calls do not establish physical reachability or successful
communication. M6.2 must add an interchangeable link result with latency, loss,
error, and energy accounting and must connect a real M4/M5 detection event
without adding communication dependencies to those layers.
