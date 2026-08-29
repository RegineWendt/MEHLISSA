<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Capillary Recruitment and Precapillary Sphincter Groups

## Purpose

M4.4 makes the number of perfused capillary paths a time-dependent model
state. A versioned profile groups parallel paths behind aggregate
precapillary sphincter gates and declares which groups are open at each
scheduled state boundary. The capillary bed then recomputes flow, area,
velocity, and transit from the selected hemodynamic boundary condition.

This increment implements the recruitment mechanism described conceptually in
the dissertation. Its distributed values are synthetic software fixtures, not
a claim about the count, timing, or behavior of anatomical sphincters.

## Versioned profile contract

The strict schema and executable example are:

```text
data/schemas/capillary-recruitment-profile/1.0.0.schema.json
examples/capillary-models/synthetic-recruitment-fixed-flow-v1.json
```

A profile identifies one compatible capillary model and provides:

- a profile identity, version, title, validity scope, sources, and limitations;
- one explicit boundary condition;
- sphincter groups with positive path counts; and
- a strictly increasing schedule of states, beginning at zero, with the IDs of
  all groups that are open in each state.

Groups must partition the capillary definition's total path count exactly.
Every scheduled group must exist and may occur only once per state. At least
one group remains open, and the first state's open-path count must equal the
base definition's `perfused_path_count`. These rules make the base card the
unambiguous initial condition while allowing later recruitment and
derecruitment.

The synthetic profile partitions eight paths into groups of two, two, and
four. It starts with four paths at `rest`, opens all eight at `activity` after
one second, and returns to four at `recovery` after two seconds.

## Boundary conditions

Recruitment cannot determine both total flow and pressure from geometry alone.
M4.4 therefore requires the model card to state what remains fixed.

### Fixed total flow

`fixed_total_flow` keeps the base network flow `Q_base` unchanged. If `n_open`
identical representative capillaries are open, then:

```text
A_total = n_open * A_single
Q       = Q_base
v       = Q / A_total
t       = L / v
```

Opening more paths distributes the same flow over more cross-sectional area;
capillary velocity falls and capillary transit increases. Feeder and drainer
metrics remain unchanged because their geometry and total flow are unchanged.

### Fixed pressure drop surrogate

`fixed_pressure_drop` represents equal parallel paths with unchanged
per-path conductance under an imposed pressure difference. It scales total
flow relative to the initial state:

```text
Q = Q_base * n_open / n_initial
```

Capillary area and total flow therefore change by the same factor, preserving
capillary velocity and transit. The serial representative arteriole and venule
receive the changed total flow, so their derived velocities and transits
change. This is deliberately a conductance-scaling surrogate. It does not
solve pressure, resistance, viscosity, or a Poiseuille network and must not be
presented as such.

## Runtime semantics

Schedule times are relative to component initialization. The initial state is
applied before the first transfer. Later states are applied at their exact
simulation times even when a host step crosses a boundary.

Residents store distance already travelled within the current region rather
than elapsed time under one velocity. An advance is split at each recruitment
event, movement is integrated with the old velocity up to that event, metrics
are recomputed, and movement continues with the new velocity. Thus an
in-flight nanodevice neither restarts its region nor retroactively travels at a
new speed. The synthetic test obtains the same 1.4-second completion with
100 ms and 350 ms host steps when recruitment occurs at 400 ms.

The public runtime state reports:

- current recruitment-state ID;
- current number of open sphincter groups and perfused paths;
- selected boundary condition;
- current total flow; and
- recomputed region metrics.

A `VolumeFlowTransfer` entering at a synchronization boundary must match the
flow for the state active at that boundary. Entity identity and conserved
payload behavior from M4.1–M4.3 remain unchanged.

## Validation and verification

The runtime rejects incomplete group partitions, incompatible model IDs,
unknown or duplicate groups, duplicate state or source IDs, empty states,
non-increasing schedules, an initial-state mismatch, invalid metadata, and
unrepresentable state times.

The M4.4 tests verify schema loading, state exposure, fixed-flow redistribution,
fixed-pressure conductance scaling, exact event handling, in-flight distance
preservation, step-size independence, and invalid-profile rejection. Existing
static transit, conservation, and organ-capillary round-trip tests remain
unchanged and passing.

## Scientific boundary and next work

M4.4 is software verification of an aggregate recruitment mechanism. It does
not yet provide individual sphincter anatomy, feedback control, vasomotion,
autoregulation, pressure-resistance dynamics, heterogeneous path conductance,
hematocrit, or physiological parameter qualification. Those features require
separate evidence and model cards.

M4.5 now adds a typed, mass-balanced exchange contract among blood,
endothelium, interstitium, and cell compartments. Recruitment and exchange can
be configured together without silently coupling synthetic fractions to area
or flow. See [Balanced Capillary Substance Exchange](BALANCED_CAPILLARY_EXCHANGE.md).

See [ADR-0024](../architecture/adr/0024-dynamic-capillary-recruitment.md) for
the binding architectural decision.
