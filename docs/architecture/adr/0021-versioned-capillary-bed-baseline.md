<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0021: Versioned Capillary-Bed Baseline

- **Status:** Accepted
- **Date:** 29 August 2026
- **Applies to:** M4; `ARC-001`, `ARC-002`, `CAP-001` through `CAP-006`

## Context

The dissertation separates arterioles, capillaries, and venules and assigns
substance exchange and local nanodevice communication to the capillary layer.
It also requires volume-flow continuity across the highly parallel bed and
describes activity-dependent recruitment through precapillary sphincters.

M3 established a typed `ModelComponent` boundary and conservative entity,
population, substance-amount, and volume-flow hand-offs. M4 needs to exercise
the fourth architectural layer without prematurely mixing unvalidated
physiology, biochemical exchange, and molecular-channel behavior.

## Decision

M4 begins with an independent `models/capillary` component. Definition contract
version `1.0.0` requires:

- named arteriole entry and venule exit ports;
- exactly three ordered regions: arteriole, capillary, and venule;
- a positive total parallel-path count and a positive perfused-path count not
  exceeding the total;
- positive transit time for every region;
- explicit validity, source, and limitation metadata.

The first implementation is a deterministic, lossless transit surrogate.
Individual entities and the existing conserved population, substance-amount,
and volume-flow transfers retain their payload while crossing the three
regions. Transfers are accepted and emitted only at synchronization points.
The configured path counts record the recruitment state but do not yet alter
flow distribution or transit.

The distributed reference card uses synthetic timing and path counts solely
for software verification. It is not a physiological capillary model.

Transforming exchange will use a later typed balance contract rather than
silently changing `SubstanceAmountTransfer`. Sphincter dynamics, local
positions, retention, adhesion, extravasation, hematocrit, stochastic transit,
and molecular channels remain separate increments behind the same component
boundary.

## Consequences

Positive:

- the capillary layer becomes a real interchangeable component immediately;
- serial microvascular semantics and ownership are executable and testable;
- the lossless baseline provides a conservation oracle for later exchange;
- data files cannot conceal missing regions, closed beds, or unsupported
  physiology;
- later surrogate, mesoscopic, and detailed variants can share ports and
  contracts.

Negative:

- the first model cannot support biological interpretation;
- path recruitment is declarative rather than dynamic;
- synchronization still quantizes completion to the selected step boundary;
- the organ-to-capillary-to-organ orchestrator remains to be implemented.

## Rejected alternatives

- **Implement exchange in the first increment:** this would make transport and
  mass-balance defects difficult to distinguish.
- **Reuse the pulmonary `capillary-surrogate` region:** an internal organ region
  is not an independent fourth-layer component and cannot host channel models.
- **Represent all capillaries individually:** this is unnecessary for the
  contract baseline and would make physiological assumptions look more precise
  than their evidence.
- **Allow arbitrary region order:** this weakens the arteriole-capillary-venule
  boundary and permits anatomically invalid cards.
