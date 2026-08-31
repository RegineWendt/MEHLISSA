<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0040: Synthetic apoptosis commitment and higher-layer feedback

- Status: Accepted
- Date: 2026-08-31

## Context

M5.6 ends with an explicitly owned intracellular drug amount but does not alter
cell state. Gate M5 requires a measurable event or state change to return to a
higher layer, and `CELL-005` requires apoptosis plus a generic measurable
cell-state event. Embedding scenario or gateway types in the cell model would
couple biology to later M6/M7 implementations. Claiming biological kinetics
from the synthetic M5.6 amount would exceed the available evidence.

## Decision

Introduce an independently replaceable synthetic Hill-response model that
maps final intracellular amount to a bounded effect fraction and changes one
cell from `viable` to the irreversible state `apoptosis_committed` when a
declared threshold is reached. Use a log-ratio evaluation for numerical
stability. The response preserves delivery, drug, model, cell, request, and
observation-time identity.

Define a separate, versioned `CellStateEvent` in the implementation-neutral
coupling library. A co-simulation adapter validates the configured source and
maps only a committed response to `cell.apoptosis_committed`; viable states do
not emit an event. Target model and port remain data, so future scenario,
gateway, or reporting layers can consume the contract without becoming cell
model dependencies.

Bind model parameters, reference values, feedback route, validity, sources,
and limitations in one strict profile. Treat every parameter and result as a
synthetic software-verification value.

## Consequences

- M5 now has a complete executable path from intracellular detection through
  delivery to a measurable higher-layer cell-state event.
- Cell biology, the neutral event contract, and scenario consumption remain
  independently replaceable.
- No activation or a subthreshold amount cannot produce an apoptosis event.
- The event time is an observation timestamp, not a reconstructed biological
  commitment time.
- The amount-based Hill surrogate has no biological, pharmacodynamic,
  treatment, toxicity, or clinical validity.
- M5.8 can add population scaling and evidence qualification without changing
  the stable event contract.

## Alternatives considered

- A direct amount threshold was rejected because a bounded continuous effect
  value provides a measurable state and an analytical half-maximal reference.
- A mechanistic apoptosis pathway was deferred because no qualified pathway,
  cell type, drug, concentration history, or kinetic data set has yet been
  selected.
- Returning a scenario-specific command directly from the cell model was
  rejected because it would reverse the layer dependency and couple M5 to M7.
- Emitting viable events was rejected for this first transition contract;
  viable state remains available in the cell response without creating noisy
  higher-layer state-change traffic.

## Affected requirements and gates

This implements Roadmap increment M5.7 and advances `CELL-005` from research to
a partial executable contract. The second Gate M5 statement is satisfied at the
synthetic software-contract level. `CELL-005` remains partial because the model
has no biological qualification, pathway dynamics, uncertainty, population
heterogeneity, or executable scenario consumer. M5 remains open for M5.8
population scaling, evidence qualification, User Guide gate review, and formal
gate review.
