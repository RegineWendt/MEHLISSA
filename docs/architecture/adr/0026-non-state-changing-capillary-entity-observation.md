<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0026: Non-State-Changing Capillary Entity Observation

- **Status:** Accepted
- **Date:** 29 August 2026
- **Applies to:** M4; `ARC-003`, `ARC-004`, `CAP-001`, `CAP-005`

## Context

The dissertation vision requires local nanodevice position, residence,
retention, adhesion, and extravasation behavior. The current entity contract
has conservative transfer ownership: an organ hands one ID to a capillary bed
and closes its ledger only after receiving that ID back. Silently retaining or
extravasating the entity would strand this ownership without saying which
component now owns it or how the route terminates.

M4 still needs inspectable local behavior and a way to compare candidate
interaction assumptions before a tissue-entry contract exists. The mechanism
must remain deterministic, memory-bounded, evidence-scoped, and compatible
with dynamic recruitment.

## Decision

M4.6 separates observation from state change.

Every resident entity exposes a current axial position derived from the same
distance state used for transport. Runtime integration accumulates exact time
spent in arteriole, capillary, and venule regions, including intervals split by
region boundaries or recruitment events.

An optional strict profile supplies constant, non-negative competing rates in
inverse seconds for retention, adhesion, and extravasation. At completion,
capillary-region residence converts those rates into four normalized
likelihoods using a competing exponential-hazard model. The profile is tied to
one model ID and carries explicit validity, sources, and limitations.

Likelihoods are observations, not sampled events. Entity identity, route, and
ownership are unchanged. Unknown entity types receive a residence record and
certain pass-through. Completed records use a bounded drainable buffer and a
cumulative dropped-record counter. Current positions are produced on demand
rather than retained as an unbounded trajectory.

The initial profile is a synthetic software-test surrogate. Its numbers are
not physiological evidence.

## Consequences

Positive:

- local position and residence become directly inspectable;
- residence remains correct across dynamic recruitment boundaries;
- candidate interaction assumptions have dimensions and normalized outputs;
- the established round-trip ownership invariant remains intact;
- bounded buffering supports large entity populations; and
- a later state-changing model has a tested reference observation contract.

Negative:

- no entity is actually retained, adhered, or extravasated;
- constant competing rates are a coarse surrogate;
- completed observations report at a host synchronization boundary;
- position snapshots are not a stored trajectory; and
- physiological parameterization and independent validation remain open.

## Rejected alternatives

- **Drop an entity when a likelihood is non-zero:** a likelihood is not an
  event, and disappearance violates ownership.
- **Sample an outcome immediately:** no terminal/tissue ownership contract
  exists yet, and synthetic rates would create misleading biological behavior.
- **Store every position at every step:** memory use would scale without a
  bound and duplicate the existing on-demand state.
- **Use dimensionless per-passage percentages:** they do not respond to actual
  residence or recruitment changes.
- **Mix entity likelihoods into substance exchange:** nanodevice disposition
  and molecular mass transfer have different identities and invariants.
