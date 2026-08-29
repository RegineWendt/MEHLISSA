<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M4 Working Plan and Evidence

## Objective

M4 turns the capillary layer into an independent co-simulation component and
then adds microvascular recruitment, balanced substance exchange, local
nanodevice behavior, and molecular communication incrementally.

The gate remains:

- a nanodevice can leave an organ, traverse a capillary bed, and return;
- substance exchange is mass-conserving and unit-consistent;
- at least one molecular channel is connected through a stable interface; and
- detailed and surrogate models are compared against the same reference cases.

## Implemented increments

### M4.1 - versioned lossless capillary transit baseline

- independent `MEHLISSA::capillary_model` library;
- strict definition schema `1.0.0`;
- ordered arteriole, capillary, and venule regions;
- explicit total and perfused parallel-path counts;
- deterministic entity transit with identity-preserving return;
- lossless population, substance-amount, and volume-flow transit;
- route, time, duplicate-ownership, topology, and recruitment validation;
- synthetic executable card with evidence and limitation metadata.

See [Capillary Transit Bed](CAPILLARY_TRANSIT_BED.md) and
[ADR-0021](../architecture/adr/0021-versioned-capillary-bed-baseline.md).

### M4.2 - conservative organ-capillary round trip

- generic four-port organ-capillary route in the co-simulation library;
- identity-preserving entity transfer from organ to capillary and back;
- lossless population, substance-amount, and volume-flow round trips;
- explicit outstanding-ownership ledgers and completion counters;
- persistent pending queues for rejected departure and return delivery;
- exact model, port, route, and synchronization-time validation;
- deterministic verification at 100 ms, 250 ms, and 500 ms host steps.

See [Organ-Capillary Round Trip](ORGAN_CAPILLARY_ROUND_TRIP.md) and
[ADR-0022](../architecture/adr/0022-organ-capillary-round-trip-coupling.md).

### M4.3 - dimension-safe geometry and volume-flow continuity

- breaking definition schema `2.0.0` with explicit SI length, diameter, and
  network volume flow;
- explicit parallel-vessel count for every serial region;
- single-vessel and total cross-section derived from diameter and count;
- mean velocity derived from total flow divided by total cross-section;
- transit derived from length divided by velocity and rounded only at the
  simulation-clock boundary;
- capillary vessel count bound to the currently perfused path count;
- rejection of invalid geometry and inconsistent volume-flow transfers;
- synthetic v2 card retaining the one-second verification route without
  claiming physiological validity.

See [Capillary Geometry and Continuity](CAPILLARY_GEOMETRY_AND_CONTINUITY.md) and
[ADR-0023](../architecture/adr/0023-dimension-safe-capillary-continuity.md).

### M4.4 - dynamic recruitment and precapillary sphincter groups

- strict recruitment-profile schema `1.0.0` layered over a compatible capillary
  definition;
- aggregate sphincter groups that partition all available parallel paths;
- scheduled rest, activity, and recovery states at exact simulation times;
- dynamic perfused-path count, total capillary area, velocity, and transit;
- explicit fixed-total-flow and simplified fixed-pressure-drop boundary
  conditions;
- distance-based in-flight progress across recruitment events;
- deterministic verification across host steps that cross an event boundary;
- synthetic profile with evidence class and explicit scientific limitations.

See [Capillary Recruitment and Precapillary Sphincter Groups](CAPILLARY_RECRUITMENT_AND_SPHINCTERS.md)
and [ADR-0024](../architecture/adr/0024-dynamic-capillary-recruitment.md).

### M4.5 - balanced blood-to-tissue substance exchange

- strict exchange-profile schema `1.0.0` for a compatible capillary model;
- substance-specific staged blood-to-endothelium, endothelium-to-interstitium,
  and interstitium-to-cell fractions;
- typed input, outgoing blood, and persistent tissue amounts;
- enforced four-term mass-balance invariant;
- explicit pass-through for unmatched substances;
- unchanged population, flow, and lossless no-profile behavior;
- drainable, profile-identified exchange records and cumulative tissue
  inventories;
- complete transformed organ-capillary-organ route with closed ownership.

See [Balanced Capillary Substance Exchange](BALANCED_CAPILLARY_EXCHANGE.md) and
[ADR-0025](../architecture/adr/0025-balanced-capillary-exchange.md).

## Planned sequence

1. introduce residence, retention, adhesion, and extravasation observables;
2. qualify an organ-specific pulmonary capillary parameter card;
3. connect and compare an analytical molecular channel and one licensable
   external or surrogate adapter;
4. implement mesoscopic and local detailed variants and compare them against
   the same reference cases;
5. perform the formal M4 gate review.

## Scientific status

M4.1 through M4.5 are software verification, not physiological validation. The
dissertation provides the layer structure, continuity requirement, recruitment
concept, and communication questions. The executable numbers in the initial
cards and recruitment profile are synthetic. Their values are internally
consistent but not physiologically qualified. Physiological geometry, flow,
transit, sphincter behavior, hematocrit, and kinetic exchange parameters require
separately sourced model cards and independent comparisons.
