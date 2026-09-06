<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# ADR-0053: Dynamic capillary-tissue-cell qualification boundary

- **Status:** Accepted
- **Date:** 2026-09-06
- **Applies to:** DCCQ-1; `ARC-003`, `ARC-004`, `ARC-006`, `CAP-004`,
  `CAP-006`, `CELL-001`, `CELL-002`, and `CELL-003`

## Context

MEHLISSA has three useful but non-equivalent foundations: balanced staged
capillary exchange, a non-consuming homogeneous capillary-to-cell snapshot,
and a receptor solver for a prescribed concentration history. BCQ-1 also adds
one computationally qualified published CD95 average-cell mechanism, but its
adapter locks the source stimulus and retains unresolved model-native time and
state units.

Repeated snapshots would not account for binding depletion, dissociation,
internalization, clearance, outlet transport, or feedback. Directly connecting
the BCQ adapter to SI tissue transport would hide an unresolved unit conversion
and change the frozen source input. Either shortcut would create a more
biological-looking diagram without stronger evidence.

## Decision

Create a seven-increment **Dynamic Capillary-Tissue-Cell Qualification
(DCCQ-1)** programme before adding the dynamic implementation. DCCQ-1.1 binds
the existing baselines and requires:

- one chemical or biological ligand identity across all layers;
- compatible explicit units at every boundary;
- exactly one owner for blood-free, endothelial-free, interstitial-free,
  receptor-bound, internalized, cleared/degraded, and outlet amounts;
- an open-system amount balance including cumulative inlet and outlet;
- delayed, bounded feedback at declared synchronization boundaries;
- separate software, mathematical, literature, source-disjoint experimental,
  and participant/clinical evidence levels; and
- prospective equations, parameters, metrics, tolerances, controls, and asset
  hashes before dynamic or validation output is inspected.

The existing `ExtracellularSignalSample` and `CapillaryCellSignalCoupler`
remain supported snapshot contracts. A new dynamic API must not change their
semantics or reuse their contract version. DCCQ-1.2 selects and licence-screens
the biological target and alternatives before DCCQ-1.3 freezes the new API and
equation protocol.

## Consequences

- Dynamic coupling cannot be claimed by scheduling the current snapshot more
  frequently.
- Ligand association, dissociation, internalization, clearance, degradation,
  and outlet flow must be reflected in one auditable amount ledger.
- The qualified CD95 mechanism remains available for its BCQ claim but is
  blocked from the new SI dynamic path until a new source-compatible input and
  unit protocol exists.
- VEGF-A/VEGFR is a candidate, not a predetermined selection.
- DCCQ-1.1 improves prospective discipline and architectural clarity but does
  not increase biological validation by itself.

## Alternatives considered

- **Poll the existing snapshot at every step:** rejected because observation
  is non-consuming and cannot close a dynamic ligand balance.
- **Treat oxygen as a generic ligand alias:** rejected for biological work
  because chemical identity changes at the boundary.
- **Immediately vary the BCQ CD95L input:** rejected because it would widen a
  frozen no-refit adapter and silently interpret unresolved units.
- **Implement a generic dynamic solver before selecting evidence:** rejected
  as the scientific path because target biology and observable evidence should
  constrain equations and parameter roles. A source-neutral mathematical
  reference may still be used later for software verification under the frozen
  DCCQ-1.3 protocol.
