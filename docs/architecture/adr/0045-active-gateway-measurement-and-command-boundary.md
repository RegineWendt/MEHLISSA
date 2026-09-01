<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0045: Active gateway measurement and command boundary

- **Status:** Accepted
- **Date:** 2026-09-01
- **Decision owners:** MEHLISSA maintainers
- **Roadmap scope:** M6.4

## Context

M6.3 can route local messages among nanodevices, relays, and collectors but has
no active transition between the local communication plane and a future BAN or
external station. Treating a gateway as a passive measurement label would not
represent resource consumption, state, duplicate control, or a command return
path. Coupling it directly to one network library would make the physiological
and local IoT layers depend on that technology.

## Decision

An `ActiveGateway` composes a normal resource-bounded `Nanodevice` endpoint
with two versioned, network-neutral boundaries:

- `GatewayMeasurement` publishes one accepted buffered detection or measurement
  while preserving gateway, source message/device, correlation, source event,
  observation time, size, content type, and content;
- `GatewayCommand` is validated and translated into an M6.1 `control`
  `LocalMessageRequest`, preserving target, correlation, command identity,
  creation time, validity, hop limit, size, content type, and content.

The endpoint profile identity must match the gateway profile. Its device must
receive, collect, and transmit. Measurement and command counts are bounded;
duplicate commands and self-targeting commands are rejected. Invalid buffered
uplink kinds remain buffered and unpublished.

M6.3 performs the local transport on either side. Publishing a measurement is
the upper boundary of M6.4, not a BAN transmission. A prepared command starts
at that boundary and may be routed to a local actuator; receipt confirms only
communication, not authorization or a physiological effect.

## Consequences

- Gateway state and endpoint energy are explicit and testable.
- Local routing, upper-network transport, and external analysis remain
  independently replaceable.
- Uplink measurements and downlink commands preserve causal trace identities.
- Command duplication and capacity are visible before local transmission.
- M6.5 can attach BAN and station adapters without changing local messages.
- An upper-layer caller can currently originate a command without authentication
  or clinical safety policy; this is a declared non-claim, not production
  control behavior.

## Alternatives considered

- **Passive gateway measurement site only:** rejected because it cannot model
  bidirectional communication, endpoint resources, or command state.
- **BAN-specific gateway API:** rejected because the BAN implementation is not
  yet selected and must remain replaceable.
- **Execute actuation inside the gateway:** rejected because command transport
  and biological/payload effects have distinct ownership and evidence needs.

## Verification

- strict gateway, endpoint, and cluster profile validation;
- one exact collector-to-gateway measurement hop and publication;
- one exact gateway-to-relay-to-actuator control downlink;
- preservation of causal and experiment trace across both boundaries;
- exact endpoint/link latency, byte, energy, storage, and count accounting; and
- rejection of empty publication, unsupported uplink kind, duplicate/self
  command, mismatched endpoint profile, and incapable endpoint.
