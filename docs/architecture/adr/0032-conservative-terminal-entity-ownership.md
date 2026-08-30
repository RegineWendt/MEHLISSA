<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0032: Conservative terminal entity ownership

## Status

Accepted for M4.12.

## Context

M4.6 computes residence-sensitive probabilities for pass-through, retention,
adhesion, and extravasation but deliberately does not sample them. The existing
organ-capillary coupler opens an ownership ledger when an organ emits an entity
and previously could close it only when the entity returned. Sampling a
non-return outcome without a new owner would strand or delete that identity.

The new mechanism must preserve deterministic replay, make every state change
auditable, survive target rejection, and leave the observational mode intact.

## Decision

Introduce a versioned `EntityDispositionTransfer` and explicit source/sink
interfaces in the coupling layer. A transfer identifies one terminal outcome,
the decision profile and draw, its capillary source, and one target owner model
and compartment.

An optional capillary disposition profile is bound to a compatible M4.6
observation profile. At capillary completion, matched entities consume one draw
from a named simulation-context stream. Pass-through follows the existing organ
return. Retained, adhered, and extravasated results follow distinct cumulative
probability intervals and produce terminal transfers.

The organ-capillary coupler holds taken dispositions in a pending queue. It
validates the original outstanding entity, synchronization time, source, target,
and target compartment before delivery. It closes the original ledger only
after a terminal sink accepts the entity. Failed delivery remains pending.

Provide a generic `TerminalEntityStore` as the first sink. It persistently owns
accepted IDs, rejects duplicates and wrong targets, and exposes counts by
outcome and compartment. A later M5 component can implement the same sink
interface and replace the store without changing capillary sampling.

## Consequences

MEHLISSA can now execute rather than merely report retention, adhesion, and
extravasation while maintaining one owner per entity. Terminal and return paths
jointly conserve the injected population, failed hand-offs are retryable, and
fixed-seed decisions are stable across compatible host steps.

Terminal records are intentionally not droppable or drainable, so memory grows
with terminally owned entities until a richer owner manages their lifecycle.
The current terminal semantics do not model reversibility or subsequent tissue
behavior. Synthetic rates remain unsuitable for biological interpretation.

## Alternatives considered

- **Erase non-returning IDs in the capillary:** rejected because disappearance
  is not an ownership transfer.
- **Close the ledger when the capillary emits:** rejected because target
  rejection would leave no owner.
- **Store terminal outcomes only as observations:** rejected because this would
  reproduce M4.6 rather than implement state change.
- **Make the capillary permanently own every outcome:** rejected because
  extravasated entities belong beyond the blood component and M5 needs a clean
  receiving interface.
- **Use per-entity random streams:** deferred; one named stable-order stream is
  sufficient for the current deterministic batch contract.
- **Allow separate owner models for each outcome in one profile:** deferred to
  a routed sink registry; M4.12 uses one owner with distinct compartments so a
  failed FIFO transfer cannot block behind a different sink.

## Affected requirements and gates

- CAP-005 now includes sampled outcomes and a conservative tissue-ownership
  hand-off; physiological rates and executable tissue dynamics remain open.
- The first M4 gate path now closes either by organ return or by acknowledged
  terminal ownership without losing identity.
- M5 can replace the terminal store through the disposition-sink interface.
- The next M4 increment is a richer shared multi-resolution case followed by
  the formal M4 gate review.
