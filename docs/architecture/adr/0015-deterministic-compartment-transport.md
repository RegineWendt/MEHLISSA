<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0015: Deterministic Identity-Preserving Compartment Transport

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M2.3; `BODY-005`, `BODY-006`, `BODY-007`, `SYS-003`

## Context

After the vascular graph contract, MEHLISSA needs its first executable transport
semantics. Depending on iteration order, the legacy implementation can forward
an entity several times within the same simulation instant. It also mixes
movement, hard-coded vessel types, random decisions, and medical device classes.

Later multilayer coupling must demonstrate both population conservation and the
identity of individual nanodevices or rare cells. At the same time, the first
component must not claim flow physics that have not yet been validated.

## Decision

1. The first flow model is an identity-preserving transit-compartment model in `models/body`.
2. Residence times derive exclusively from the validated segment fields `length / mean velocity` and are represented internally as integer nanoseconds.
3. Injections are scheduled, validated events. Every entity receives a stable, strictly monotonic ID.
4. Transitions execute in two phases: determine first, then commit together. An entity can therefore traverse at most one edge per `advance()`.
5. The permitted time step is limited to the shortest segment transit time. This prevents the single-movement rule from discarding physical transit time; excess residence time is carried into the successor segment.
6. Branches use only the named random stream `body.compartment-transport.transitions`. Selection uses a specified 53-bit grid instead of `std::discrete_distribution`.
7. A single successor is deterministic and consumes no random number. RNG consumption therefore depends only on genuine branching decisions.
8. Exact population conservation is checked after every step.

## Consequences

Positive:

- Iteration order can no longer cause multiple movements.
- Identity and population are preserved across branches and merges.
- Random decisions are reproducible and verifiable through seed, stream name, and draw count.
- The same transport model works without rebuilding with every graph conforming to the M2.1 contract.
- Later flow models can be added behind their own component and tested against the same invariants.

Negative and limitations:

- Individual entities use more memory than pure population vectors. A population-based mode remains necessary for scaling tests.
- Fixed transit per segment represents neither laminar profiles nor transit-time dispersion.
- Very large simulation steps are deliberately rejected. A later event-driven scheduler may execute multiple physical transitions within an outer coupling step, but must continue to guarantee unambiguous event times.
- Extraction, measurement sites, trajectory bounding, and checkpoints are not part of this decision.

## Alternatives

- **Port the legacy movement directly:** rejected because it would retain iteration dependence and special medical logic.
- **Only segment populations without IDs:** rejected for now because multilayer scenarios need the identity of individual devices and rare cells. An aggregated implementation may later exist in parallel.
- **`std::discrete_distribution`:** rejected because its concrete mapping from engine output to results is not specified across platforms.
- **Arbitrarily large steps with multiple transitions:** deferred until an explicit event scheduler cleanly defines order and coupling times.
