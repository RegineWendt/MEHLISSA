<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0018: Bounded Transport Observation and Passive Measurement Sites

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M2.5; `BODY-005`, `IOT-003`, `DATA-004`

## Context

Individual trajectories are valuable for rare cells and selected nanodevices,
but do not scale to large populations. Pure aggregates, on the other hand, lose
identity and event order. The dissertation also requires extraction and a wrist
gateway, while a genuine communication model can be coupled meaningfully only
in M6.

## Decision

1. Extractions are scheduled transport events and remove either up to a fixed number or all entities available in a segment.
2. Deterministic extraction prefers the lowest active particle IDs.
3. In M2.5, `sample` and `gateway` are passive segment-bound measurement sites. They count arrivals but do not alter transport.
4. Measurement-site totals remain complete; individual observations have an independent hard limit.
5. Trajectories can be disabled, complete, or selected as a `first_n` sample and always require a hard record limit.
6. Segment populations are recorded at a configurable time interval and are also bounded.
7. Every bounded category reports truncation explicitly. Reaching an output limit neither terminates nor changes the simulation.
8. A versioned JSON format validated before writing separates observation data from visualization and later orchestration.

## Consequences

Positive:

- Large runs can operate with a constantly bounded volume of detailed output.
- Identity remains available for deliberately selected entities.
- Aggregates are not distorted by detail limits.
- A passive gateway site can already be used in body and scenario tests without anticipating M6.
- Extraction becomes part of verified population conservation.

Limitations:

- `first_n` is a reproducible technical sample, not a random or statistically representative sample.
- Extractions scheduled between two outer steps are processed at the first step end that reaches them and are reported accordingly.
- The JSON format is intended for moderate verification artifacts; very large data volumes later require streaming or a compressed columnar format.
- A passive gateway measurement site models neither a radio channel nor detection errors.

## Alternatives

- **Always write all trajectories:** rejected because memory and I/O would grow without bound.
- **Store aggregates only:** rejected because identity and rare events are needed for later multiscale coupling.
- **Random sample by default:** deferred because selection method, weighting, and scientific interpretation are scenario-specific.
- **Implement the gateway as a network device already:** rejected because this would improperly couple body transport and nano-IoT communication.

## Verification

- `docs/m2/TRANSPORT_OBSERVATION.md`
- `data/schemas/transport-observation-report/1.0.0.schema.json`
- `compartment_transport_tests`
