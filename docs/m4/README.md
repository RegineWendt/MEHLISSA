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

## Planned sequence

1. model recruitment and precapillary sphincter state;
2. add a balanced blood/interstitium exchange contract;
3. introduce residence, retention, adhesion, and extravasation observables;
4. qualify an organ-specific pulmonary capillary parameter card;
5. connect and compare an analytical molecular channel and one licensable
   external or surrogate adapter;
6. implement mesoscopic and local detailed variants and compare them against
   the same reference cases;
7. perform the formal M4 gate review.

## Scientific status

M4.1 through M4.3 are software verification, not physiological validation. The
dissertation provides the layer structure, continuity requirement, recruitment
concept, and communication questions. The executable numbers in the initial
cards are synthetic. The v2 values are internally consistent but not
physiologically qualified. Physiological geometry, flow, transit, hematocrit,
and exchange parameters require separately sourced model cards and independent
comparisons.
