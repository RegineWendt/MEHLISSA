<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0022: Organ-Capillary Round-Trip Coupling

- **Status:** Accepted
- **Date:** 29 August 2026
- **Applies to:** M4; `ARC-001`, `ARC-003`, `ARC-004`, `CAP-001`

## Context

M3 established independently owned organ components and typed entity and
conserved-transfer contracts. M4.1 added an independent capillary component,
but the two layers could only be exercised separately. A complete hand-off
needs to distinguish four boundaries: departure from the organ, entry into the
capillary bed, exit from the capillary bed, and return to the organ.

The coupling mechanism must preserve identity and conserved payloads, make
temporary ownership observable, reject stale or misrouted transfers, and avoid
embedding lung-specific physiology in the orchestration layer.

## Decision

`OrganCapillaryCoupler` is a generic production component in
`models/cosimulation`. It receives explicitly named organ and capillary
endpoints plus a four-port route. It does not own, initialize, advance, or
finalize either model; the host remains responsible for synchronized model
lifecycle and time advancement.

At each synchronization boundary, the coupler:

1. collects outbound entity and conserved transfers from the source endpoint;
2. retains them in a pending buffer until the target accepts them;
3. validates contract version, model IDs, route ports, and emission time;
4. records accepted departure identifiers in an outstanding-ownership ledger;
5. accepts a return only if its identifier is outstanding; and
6. removes the identifier and records a completed round trip only after the
   organ accepts the return.

Rejected transfers remain pending so the caller can inspect the failure and,
where appropriate, retry at the correct synchronization boundary. Entity IDs
and conserved-transfer IDs use separate ledgers because they belong to
different contract namespaces. Population, substance-amount, and volume-flow
transfers use the same generic conserved-transfer path.

The executable verification uses a scripted synthetic organ and the synthetic
M4.1 capillary card. This is an architectural contract test, not a pulmonary
physiology model. Organ-specific gateways, transforming tissue exchange, and
biological parameterization remain separate increments.

## Consequences

Positive:

- the first M4 gate statement is executable at the software-contract level;
- no entity or conserved transfer is simultaneously owned by both endpoints;
- wrong route, time, endpoint, duplicate departure, and unknown return are
  rejected at a single audited boundary;
- failed delivery does not silently discard a transfer;
- future lung, kidney, surrogate, mesoscopic, and detailed components can use
  the same coupling contract.

Negative:

- the caller must advance both components to compatible synchronization times;
- pending queues currently stop at the first invalid transfer;
- the in-memory ownership ledger is not yet checkpointed;
- completion counters prove technical round trips, not physiological validity;
- no transformation or mass exchange occurs in this lossless reference path.

## Rejected alternatives

- **Put the coupler inside the lung model:** this would collapse organ and
  capillary responsibilities and prevent reuse by other organs.
- **Let the capillary call the organ directly:** this creates hidden ownership
  and lifecycle dependencies between otherwise interchangeable components.
- **Infer ports from model names:** explicit routes are versionable and fail
  early when component cards are incompatible.
- **Drop rejected transfers:** a temporary synchronization or target rejection
  must not become silent entity or mass loss.
- **Add exchange to the round-trip coupler:** exchange needs an explicit
  balanced blood/endothelium/interstitium/cell contract and belongs inside an
  exchange-capable model, not the ownership bridge.
