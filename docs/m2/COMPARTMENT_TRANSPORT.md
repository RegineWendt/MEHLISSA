<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Deterministic Compartment Transport

## Purpose and model boundary

`CompartmentTransport` is the first executable transport interpretation of the
validated vascular graph. Its purpose is to demonstrate critical software
invariants early without claiming spatially resolved laminar flow or a
physiologically validated 95-segment parameterization.

Every segment is a directed transit compartment. Mobile entities continue to be
represented individually so that their identity can later be preserved across
the body, organ, capillary, and cell layers. The current component does not know
biological entity types; it transports only stable numeric IDs.

## Time and transition model

For segment `i`, the nominal transit time is

`t_i = length_i / mean velocity_i`.

During construction, it is converted to integer nanoseconds and rounded up
conservatively. An `advance(delta)` may be no longer than the shortest segment
transit time in the loaded graph. No entity can therefore traverse two edges in
one call. All due transitions are first collected in a separate buffer and then
committed together. Segment order in the JSON cannot cause cascading movement
at the same simulation instant.

At a branch, every due entity draws exactly one 64-bit value from the named
stream `body.compartment-transport.transitions`. The upper 53 bits are mapped
to the exactly representable grid `[0, 1)` and compared with cumulative
probabilities. A segment with one successor consumes no random value. This
method avoids the cross-standard-library reproducibility that
`std::discrete_distribution` does not guarantee.

## Injections

Injections are prescheduled events with

- simulation time in nanoseconds,
- a start segment ID, and
- a positive number of new entities.

Events at time zero execute during `initialize()`. Later events are activated in
the time step whose closed endpoint reaches their time. Input order remains
stable for equal times. IDs are assigned strictly monotonically from one.

## Automatically verified invariants

- Injection times are nonnegative, counts positive, and start segments present.
- Every time step is positive and does not exceed the safe upper bound.
- An entity changes segment at most once per `advance()`.
- A transition changes neither identity nor total entity count.
- The sum of all segment populations always equals the number injected minus entities demonstrably extracted.
- The same seed and input produce identical locations, residual times, populations, transition counts, and random-stream counters.
- Different seeds can produce different paths at genuine branches.

Verification resides in `tests/compartment_transport_tests.cpp`.

## Extension in M2.5

M2.5 adds scheduled extraction, passive sample/gateway measurement sites,
bounded trajectories, bounded individual observations, population aggregates,
and a schema-validated result format. The precise semantics are documented in
[`TRANSPORT_OBSERVATION.md`](TRANSPORT_OBSERVATION.md).

## Remaining limitations

- Transit time is identical for all entities in a segment; there is no radial velocity profile yet.
- Transport state does not yet have checkpoint serialization; integration with experiment orchestration and co-simulation follows in M3.
- Time-dependent flows, vascular compliance, pulsatility, and physiological states are not yet modeled.
- The synthetic four-segment model is a software test, not physiological evidence.
- Domain distribution and equilibrium regressions begin with M2.4.
