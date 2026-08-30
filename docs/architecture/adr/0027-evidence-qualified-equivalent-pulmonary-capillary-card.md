<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0027: Evidence-qualified equivalent pulmonary capillary card

## Status

Accepted for M4.7.

## Context

The M4.1-M4.6 capillary cards are synthetic software fixtures. A pulmonary
card needs organ-specific evidence, but the available human studies measure
different constructs. Physiological diffusing-capacity methods estimate
functional capillary blood volume, whereas post-mortem morphometry estimates
lumen capacity and surface. Total alveolar-capillary length and discrete vessel
count are not robustly identifiable because the network is sheet-like and
branched. The current runtime nevertheless requires a parallel-tube geometry.

## Decision

Introduce capillary definition schema 3.0.0 with:

- explicit `equivalent_parallel_tubes` semantics;
- parameter-level values, SI units, uncertainties, source IDs, evidence roles,
  and derivations;
- separate functional blood volume and morphometric lumen capacity;
- a round, explicitly non-anatomical perfused path count and derived
  representative length;
- transit derived as functional volume divided by the existing M3 reference
  flow;
- numerical-transition semantics for unqualified arteriole and venule
  boundaries; and
- loader-enforced closure among metadata and executable geometry.

Schema 2.0.0 remains loadable for the synthetic reference. Regional blood
volume becomes a first-class derived runtime metric.

## Consequences

The pulmonary candidate can reproduce a cited volume-residence scale without
claiming anatomical fidelity that the evidence cannot support. Morphometric
capacity is not confused with resting perfused volume, and every calibration
or numerical choice remains visible.

The card is still not physiological validation. Its main flow and capillary
measurements are not jointly observed, the recumbent functional cohort has
four subjects, hematocrit remains unresolved, and boundary-region geometry is
only a software transition. Later detailed networks may replace the equivalent
geometry behind the same comparison case.
