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

## Planned sequence

1. add dimension-safe capillary geometry and continuity-derived velocity;
2. model recruitment and precapillary sphincter state;
3. add a balanced blood/interstitium exchange contract;
4. introduce residence, retention, adhesion, and extravasation observables;
5. qualify an organ-specific pulmonary capillary parameter card;
6. connect and compare an analytical molecular channel and one licensable
   external or surrogate adapter;
7. implement mesoscopic and local detailed variants and compare them against
   the same reference cases;
8. perform the formal M4 gate review.

## Scientific status

M4.1 and M4.2 are software verification, not physiological validation. The
dissertation provides the layer structure, continuity requirement, recruitment
concept, and communication questions. The executable numbers in the initial
card are synthetic. Physiological geometry, flow, transit, hematocrit, and
exchange parameters require separately sourced model cards and independent
comparisons.
