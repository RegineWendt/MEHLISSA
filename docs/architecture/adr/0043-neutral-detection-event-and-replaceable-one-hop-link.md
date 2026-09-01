<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0043: Neutral detection event and replaceable one-hop link

- Status: Accepted
- Date: 2026-09-01

## Context

M5 produces a checked receptor-threshold response, while M6.1 can emit and
receive a traceable local message. Connecting those objects directly inside the
cell or device model would make physiology depend on one communication
implementation. Calling the receiving device directly would also provide no
place to represent latency, loss, corruption, or link energy and could mix
communication measurements into biological output.

M6 needs a minimal executable link before it adds relays or gateways. The link
must be replaceable, must report modeled non-delivery as data rather than as a
software exception, and must not claim a physical channel before one is
qualified.

## Decision

Introduce a versioned, communication-neutral `MolecularDetectionEvent`. A
separate `MEHLISSA::iot_cosimulation` adapter converts a successful, causal M5
`ReceptorLigandResponse` into that event. The cell library does not include or
depend on M6 headers.

The M6 detection-message adapter converts the neutral event into the existing
`LocalMessageRequest`, preserving source model, request, event, detector,
signal, compartment, time, and observed fraction across the event and message
envelope.

Define `OneHopLinkModel` as the replaceable delivery boundary. Its explicit
result distinguishes delivery from loss, corruption, and validity expiry. The
first implementation, `ScheduledOneHopLink`, uses a repeating declared outcome
sequence, fixed latency, and fixed per-attempt link energy. This is a
deterministic verification model, not a probabilistic or physical channel.

`OneHopCommunicationSession` composes device emission, link evaluation, and
conditional receiver acceptance. It collects a communication-only metric
snapshot: message and byte counts, delivered latency, channel loss, corruption,
expiry, transmitter energy, receiver energy, and link energy. Biological model
values are not included.

## Consequences

- M5 and M6 remain independently replaceable and are connected only in a
  dedicated adapter library.
- A modeled lost or corrupted message does not mutate the receiving device and
  is not treated as an invalid program state.
- Every delivered message remains traceable to the exact M5 request and
  threshold event.
- Alternative analytical, stochastic, molecular, radio, or external-simulator
  links can implement `OneHopLinkModel` without changing physiology or message
  identity.
- The scheduled link can verify accounting and failure semantics but cannot
  establish range, capacity, noise physics, or a calibrated error probability.
- M6.2 ends at the local collector; relay, gateway, BAN, and external reporting
  remain open.

## Alternatives considered

- Adding communication fields to `ReceptorLigandResponse` was rejected because
  it reverses the intended dependency direction.
- Treating loss as an exception was rejected because loss is a valid simulated
  outcome, whereas exceptions are reserved for invalid contracts or state.
- Beginning with a stochastic Bernoulli link was rejected for this increment
  because it would require a probability and calibration claim before the
  deterministic semantics and metrics were frozen.
- Counting all energy in one scalar was rejected because transmitter,
  receiver, and link-model energy have different owners and interpretations.

## Affected requirements and gates

This implements the first local-link portion of `IOT-002`, advances `IOT-003`,
and partially satisfies the latency/loss/error/energy reporting requirement
`IOT-004`. It does not yet satisfy Gate M6's external measurement or downlink
criteria.
