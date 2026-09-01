<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0044: Bounded cluster routing and store-and-forward relay

- **Status:** Accepted
- **Date:** 2026-09-01
- **Decision owners:** MEHLISSA maintainers
- **Roadmap scope:** M6.3

## Context

M6.2 terminates every communication attempt at one local receiver. The
dissertation vision also requires clusters, relays, and multi-hop reachability.
Adding these semantics inside `Nanodevice` or a biological model would couple
endpoint state, routing policy, and channel behavior and would make later
replacement by a network simulator difficult.

## Decision

MEHLISSA represents a local cluster as a validated directed topology whose
members have explicit endpoint, relay, collector, or gateway roles. Each edge
owns one replaceable M6.2 link model. `NanodeviceCluster` selects a loop-free
route bounded by both the cluster maximum and the message hop limit.

The first two deterministic strategies are:

- `fewest_hops`, with configured latency and route identity as tie-breakers;
- `lowest_total_latency`, with hop count and route identity as tie-breakers.

`BoundedMultiHopSession` performs immediate store-and-forward communication.
Every relay must expose receive, transmit, and relay capabilities and must have
an empty receive buffer before the exchange. It receives one hop, releases that
buffer after checking the exact delivered message, and creates the next local
message while preserving content, correlation, source-event identity, and the
absolute expiry time. Each hop has a unique message identity and an explicitly
decremented hop budget.

Per-hop results and endpoint/link energy remain visible. An end-to-end result
aggregates communication metrics without converting loss, corruption, or
expiry into software exceptions. A dropped hop terminates forwarding and does
not mutate later route devices.

## Consequences

- Cluster policy remains independent from biology and endpoint internals.
- Bounded simple-path search prevents loops and unbounded forwarding.
- Alternative route objectives are comparable on the same topology.
- Traceability and expiry survive relay boundaries.
- The synthetic scheduled links still do not establish physical reachability.
- Immediate forwarding has no queue, contention, scheduling, congestion, or
  retransmission semantics.
- A relay consumes endpoint energy on both reception and retransmission.

## Alternatives considered

- **Rewrite the destination in one shared message object:** rejected because it
  obscures per-hop sender identity and endpoint accounting.
- **Embed routing in `Nanodevice`:** rejected because device capabilities and
  topology policy have different replacement and validation boundaries.
- **Introduce ns-3 immediately:** rejected because M6 first requires a stable,
  independently testable interface that an external simulator can implement.

## Verification

- strict schema and semantic validation of the synthetic cluster profile;
- deterministic divergence of fewest-hop and lowest-latency routes;
- exact two-hop trace, latency, byte, and energy accounting;
- second-hop loss without collector mutation; and
- pre-mutation rejection of insufficient hop limits and incapable relays.
