<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0024: Dynamic Capillary Recruitment

- **Status:** Accepted
- **Date:** 29 August 2026
- **Applies to:** M4; `ARC-002`, `ARC-003`, `CAP-001`, `CAP-003`

## Context

M4.3 binds capillary area, velocity, and transit to a static perfused-path
count. The dissertation additionally describes activity-dependent perfusion
through precapillary sphincters. Changing path count is underdetermined unless
the runtime also knows whether total flow, pressure difference, or another
network quantity is held fixed. A state change during transit must not make the
answer depend on whether the host happened to step across the event in one
large interval or several small ones.

## Decision

Recruitment is a separate, strict, versioned overlay rather than a breaking
revision of the capillary geometry definition. Schema `1.0.0` describes
aggregate precapillary sphincter groups and a strictly ordered schedule of
open-group states for one compatible model.

Groups partition all available paths. The first state begins at zero and must
reproduce the base definition's initially perfused path count. Subsequent
states may recruit or derecruit complete groups. Events are interpreted
relative to component initialization and processed at their exact simulation
times.

Every profile chooses one of two boundary conditions:

- `fixed_total_flow`: retain base total flow and derive velocity from the new
  total open cross-section;
- `fixed_pressure_drop`: use the explicit equal-path-conductance surrogate
  `Q = Q_base * n_open / n_initial`.

The second condition is named for its modeling assumption but does not add a
pressure solver. It is only valid for the declared simplified parallel-path
interpretation.

Residents record distance travelled in their current region. Advance intervals
are split at scheduled events, so old and new velocities apply only to their
respective time spans. The runtime exposes current state, open groups, perfused
paths, boundary condition, flow, and derived region metrics. Incoming volume
flow must match the currently active state.

## Consequences

Positive:

- recruitment becomes executable data rather than an unused count or setter;
- the hemodynamic assumption is explicit and reviewable in every profile;
- grouped gates scale without pretending to identify anatomical sphincters;
- mid-transit changes preserve travelled distance and compatible-step
  determinism;
- static M4.3 cards and APIs continue to work without a profile;
- later exchange models can consume a well-defined perfused area and flow.

Negative:

- the fixed-pressure option is an equal-conductance surrogate, not a vascular
  pressure-resistance model;
- schedules are prescribed and have no physiological feedback;
- all paths in one group open and close simultaneously;
- changes can alter total flow without yet coupling that change back to an
  organ or body hemodynamic solver;
- the synthetic schedule verifies software behavior only.

## Rejected alternatives

- **Expose a mutable perfused-path setter:** this would omit provenance,
  scheduling, compatibility, and boundary-condition semantics.
- **Put the schedule into capillary schema 3.0.0:** geometry and recruitment
  have different reuse and evidence scopes; an overlay avoids duplicating the
  base model.
- **Always keep total flow fixed:** this is one valid experiment, not a neutral
  consequence of recruitment.
- **Always keep velocity fixed:** this hides the equivalent conductance
  assumption and cannot describe a fixed-flow experiment.
- **Store only elapsed regional time:** changing velocity would retroactively
  reinterpret progress and make event-crossing behavior step dependent.
- **Claim individual anatomical sphincters:** available evidence and the
  synthetic fixture support aggregate path groups only.
