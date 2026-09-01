<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0046: Replaceable BAN adapter and governed station loop

- **Status:** Accepted
- **Date:** 2026-09-01
- **Decision owners:** MEHLISSA maintainers
- **Roadmap scope:** M6.5

## Context

M6.4 exposes a network-neutral gateway measurement and command boundary but
does not transport either object through a body-area network or represent an
external station. Letting an arbitrary caller return a command cannot prove
that the command responds to an observed measurement, passed an explicit
policy, came from the expected station, or was not replayed. Binding the
gateway directly to one BAN technology would also obstruct the M6.6 external
network-simulator adapter.

## Decision

Introduce a versioned `BanFrame`, a polymorphic `BanTransportAdapter`, a
stateful `GatewayBanAdapter`, and an `ExternalAnalysisControlStation`.

The gateway adapter records each uniquely published measurement. The station
records each uniquely received measurement and its receipt time, then evaluates
command requests against configured gateway, time order, target, content-type,
correlation, duplication, and capacity rules. An approved
`GovernedGatewayCommand` retains the decision,
station, gateway, and source-measurement identities. The gateway accepts it
only if it returns from the configured station, addresses the configured
gateway, remains valid, has not been replayed, and matches a measurement and
correlation previously published by that adapter.

The transport is an injected interface. M6.5 provides a deterministic
scheduled implementation with explicit outcomes and communication-only
metrics. Policy denial is a typed modeled result. Malformed contracts and
broken invariants remain software errors.

The lower `ActiveGateway::prepare_downlink` boundary remains responsible for
mapping an accepted gateway command into the local network. The M6.5 reference
composes all boundaries and routes the resulting control message to an
actuator, but it does not execute a biological or payload-release effect.

## Consequences

- Measurement-to-command causality is explicit and testable across the BAN.
- Denied commands cannot be serialized into a station downlink frame.
- Station policy and gateway replay checks are stateful and capacity-bounded.
- BAN loss, corruption, expiry, latency, bytes, and energy remain distinct from
  local communication and physiological results.
- M6.6 can implement `BanTransportAdapter` without changing other layers.
- The policy is transport governance only; security and clinical-safety claims
  remain explicitly excluded.

## Alternatives considered

- **Direct station call into `ActiveGateway`:** rejected as the reference path
  because it omits BAN outcomes and causal governance.
- **Protocol-specific BAN objects:** rejected because they would couple the
  architecture to a technology before evidence and requirements select one.
- **Parse and authorize medical actions in the transport layer:** rejected
  because transport policy, clinical decision logic, and actuation effects have
  different owners and validation obligations.
- **Treat policy denial as an exception:** rejected because denial is an
  expected scenario outcome that must remain reportable.

## Verification

- strict BAN/station JSON Schema and semantic profile validation;
- exact measurement and command identity across both adapter directions;
- complete station-to-gateway-to-relay-to-actuator delivery;
- separate BAN and local-route latency, count, byte, and energy assertions;
- explicit unknown-measurement, correlation, target, duplicate, and replay
  rejection; and
- prescribed loss, corruption, and expiry accounting.
