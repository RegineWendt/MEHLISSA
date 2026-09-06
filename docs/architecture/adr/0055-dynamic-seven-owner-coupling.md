<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0055: Typed seven-owner dynamic ligand coupling

- Status: Accepted
- Date: 2026-09-06
- Decision owners: MEHLISSA maintainers

## Context

The M5 capillary-to-cell adapter intentionally observes a non-consuming,
homogeneous snapshot. DCCQ-1 requires a true dynamic boundary in which binding,
dissociation, internalization, degradation, clearance and outlet transit move
one named ligand between exclusive owners. It also requires feedback to remain
causal across explicit synchronization boundaries.

## Decision

Add `DynamicCapillaryTissueCellModel` to the co-simulation layer. Its public API
uses core SI quantity types and a stable VEGF-A165a/VEGFR2/HUVEC identity. The
model owns blood-free, endothelial-free, interstitial-free, receptor-bound,
internalized, cleared/degraded and outlet amounts and exposes the full
open-system balance at every output.

Use fixed-step classical RK4 internally and a distinct synchronization
interval externally. Occupancy-derived feedback computed after interval `n`
may be applied only during interval `n+1`. Invalid ligand identity, nonphysical
SI values, inconsistent initial ownership, unsupported NRP1 effects, negative
states and balance violations fail closed.

Treat NRP1 as an explicit structural dimension. The reference mode is
tracked-neutral because the selected sources justify biological inclusion but
not a reduced-model facilitation multiplier. Excluded and labelled exploratory
facilitation modes exist only for structural sensitivity.

Keep the model programmatic and modular. The qualification runner is a testing
and evidence executable, not a new clinical or general-purpose Workbench
workflow. The existing snapshot API remains unchanged for its documented use.

## Consequences

- DCCQ-G1 through DCCQ-G5 can be evaluated without changing M4 or M5 contracts.
- The complete ligand balance can be independently reconstructed from CSV
  output.
- The reduced model is deliberately smaller than the 281-state literature
  reference and does not copy its unlicensed repository code.
- Transfer and feedback assumptions remain visible as engineering parameters;
  computational qualification does not establish pulmonary or biological
  validity.
- New ligands, cell types or fitted feedback rules require a new candidate and
  prospective evidence protocol rather than aliases or silent overrides.
