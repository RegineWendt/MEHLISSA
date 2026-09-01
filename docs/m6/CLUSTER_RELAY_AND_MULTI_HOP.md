<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Cluster, relay, and bounded multi-hop communication (M6.3)

## Purpose

M6.3 extends the replaceable M6.2 one-hop contract to deterministic local
clusters. It answers three software questions without claiming a physical
in-body network:

1. which declared route is selected for a given objective;
2. whether a relay preserves message trace and bounded validity; and
3. how per-hop delivery outcomes and resource costs combine end to end.

## Contracts

`NanodeviceClusterConfig` declares:

- a stable cluster identity and maximum hop count;
- members with endpoint, relay, collector, or gateway roles; and
- unique directed links between known members.

`NanodeviceCluster::select_route` enumerates loop-free paths only up to the
configured bound. `fewest_hops` minimizes edge count; `lowest_total_latency`
minimizes the sum of configured link delays. Stable secondary comparisons make
ties reproducible.

`BoundedMultiHopSession` checks the selected route against the message hop
limit before changing a device. Intermediate devices must be active,
relay-capable, and initially free of stored messages. The session preserves the
payload, content type, experiment correlation, source event, and absolute
expiry while producing a new sender and message identity at each hop.

The end-to-end result contains the selected device route, every M6.2 hop result,
the terminal delivery/drop state, and aggregated communication-only metrics.
A loss, corruption, or expiry stops the route normally and does not call later
receivers.

## Strict reference profile

The checked inputs are:

```text
examples/iot-models/synthetic-relay-v1.json
examples/iot-models/synthetic-cluster-communication-v1.json
data/schemas/cluster-communication-profile/1.0.0.schema.json
```

The topology has a direct locator-to-collector link of `40 ms` and a two-hop
locator-to-relay-to-collector path of `10 ms + 15 ms`. Consequently:

| Strategy | Route | Total configured latency |
|---|---|---:|
| fewest hops | locator → collector | 40 ms |
| lowest total latency | locator → relay → collector | 25 ms |

The two-hop reference sends one 320-byte logical payload across two links. Its
exact report is:

- 2 attempts and 2 deliveries;
- 640 attempted and delivered hop-bytes;
- 25 ms summed delivered latency and 15 ms maximum hop latency;
- 0.8 µJ transmitter energy: 0.5 µJ locator plus 0.3 µJ relay;
- 0.45 µJ receiver energy: 0.2 µJ relay plus 0.25 µJ collector; and
- 0.12 µJ link-model energy.

These are separate accounting owners and must not be added to a biological
energy balance.

## Failure and boundary cases

- a second-hop loss leaves the collector unchanged and reports one delivered
  and one lost hop;
- an insufficient message hop limit is rejected before the first transmission;
- an intermediate device without relay capability is rejected before mutation;
- unknown endpoints, duplicate members or links, self-links, unreachable
  targets, and routes beyond the configured bound are invalid data.

All topology, delay, energy, and outcome values are synthetic. The reference
does not model anatomical distance, propagation, range, queues, contention,
interference, retransmission, mobility, security, capacity, or hardware. M6.4
adds an active gateway boundary; it does not retroactively calibrate this local
cluster.
