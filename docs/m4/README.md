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

## Planned sequence

1. connect organ -> capillary -> organ through a dedicated orchestrator;
2. add dimension-safe capillary geometry and continuity-derived velocity;
3. model recruitment and precapillary sphincter state;
4. add a balanced blood/interstitium exchange contract;
5. introduce residence, retention, adhesion, and extravasation observables;
6. qualify an organ-specific pulmonary capillary parameter card;
7. connect and compare an analytical molecular channel and one licensable
   external or surrogate adapter;
8. implement mesoscopic and local detailed variants and compare them against
   the same reference cases;
9. perform the formal M4 gate review.

## Scientific status

M4.1 is software verification, not physiological validation. The dissertation
provides the layer structure, continuity requirement, recruitment concept, and
communication questions. The executable numbers in the initial card are
synthetic. Physiological geometry, flow, transit, hematocrit, and exchange
parameters require separately sourced model cards and independent comparisons.
