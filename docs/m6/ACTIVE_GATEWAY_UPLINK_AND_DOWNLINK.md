<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Active gateway measurement uplink and command downlink (M6.4)

## Purpose and boundary

M6.4 introduces an active nano/micro gateway between the routed local cluster
and the subsequently added M6.5 BAN/external-station adapters. It provides two
bidirectional software contracts:

```text
local measurement message -> active gateway -> GatewayMeasurement boundary
GatewayCommand boundary -> active gateway -> routed local control message
```

The first line is called the measurement uplink even though M6.4 ends at the
gateway's implementation-neutral upper boundary. The second is a local command
downlink from that boundary to a target nanodevice. BAN transport and an
external station are attached by M6.5; authentication, cryptographic
authorization, and clinical decision logic remain later work.

## Active gateway contract

`ActiveGateway` owns one ordinary `Nanodevice`. Its strict endpoint must match
the configured gateway and have receive, collect, and transmit capabilities.
This reuses M6.1 lifecycle, storage, count, message-size, and energy rules rather
than creating privileged communication behavior.

`GatewayMeasurement` version `1.0.0` preserves:

- measurement and gateway identities;
- source message and source-device identities;
- experiment correlation and biological/source event;
- gateway observation time; and
- logical size, content type, and content.

Only configured detection and/or measurement message kinds can be published.
Exactly one message must be buffered. Publication capacity is bounded and an
invalid kind remains buffered rather than being silently discarded.

`GatewayCommand` version `1.0.0` carries command and target identities,
experiment correlation, creation time, validity, hop limit, logical size,
content type, and content. The gateway maps it to a local `control` message and
uses the command identity as the source event. Duplicate commands,
self-targeting, invalid contracts, and exhausted capacity are rejected before
local transmission.

## Strict synthetic reference

The profiles are:

```text
examples/iot-models/synthetic-active-gateway-v1.json
examples/iot-models/synthetic-gateway-endpoint-v1.json
examples/iot-models/synthetic-uplink-collector-v1.json
examples/iot-models/synthetic-actuator-v1.json
examples/iot-models/synthetic-gateway-cluster-v1.json
data/schemas/active-gateway-profile/1.0.0.schema.json
```

### Measurement uplink

The uplink collector sends one 256-byte measurement over a `20 ms` scheduled
link. The exact communication report is one attempt/delivery, `0.35 µJ`
collector transmitter energy, `0.4 µJ` gateway receiver energy, and `0.1 µJ`
link-model energy. The gateway publishes it at the exact completion time and
releases its local receive storage.

### Command downlink

The gateway converts one 128-byte activation command into a local control
message. Lowest-latency routing selects:

```text
gateway.synthetic.wrist -> relay.synthetic.1 -> actuator.synthetic.1
```

The two scheduled hops take `8 ms + 12 ms`. Their exact report is two
attempts/deliveries, 256 hop-bytes, `1.1 µJ` transmitter energy (`0.8 µJ`
gateway plus `0.3 µJ` relay), `0.45 µJ` receiver energy (`0.2 µJ` relay plus
`0.25 µJ` actuator), and `0.1 µJ` link-model energy.

The actuator stores a control message that preserves the command and experiment
trace. M6.4 does not interpret or execute that command.

## Scientific and operational limits

All device, capacity, energy, timing, topology, and outcome values are
synthetic software-test values. The gateway is not a validated wearable or
implant. There is no skin/tissue coupling, radio or molecular channel, BAN,
external analysis station, human operator, command decision algorithm,
authentication, authorization, encryption, clinical safety policy, retry, or
fail-safe control. A delivered command does not release payload or change a
biological model. M6.5 introduces BAN and external-station adapters while
retaining these explicit boundaries and non-claims.
