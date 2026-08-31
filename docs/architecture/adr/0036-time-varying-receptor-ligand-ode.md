<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0036: Time-varying receptor-ligand ODE baseline

## Status

Accepted for M5.3.

## Context

M5.1 solves reversible one-to-one receptor binding exactly when ligand
concentration is constant. M5.2 can derive such a constant exposure from one
time-scoped M4 tissue snapshot. Neither increment can represent a pulse,
withdrawal, or another prescribed concentration trajectory.

The next model must introduce time dependence without weakening the exact M5.1
reference, implying ligand consumption that is not modeled, or selecting an
external solver before MEHLISSA has a verified internal numerical baseline.

## Decision

Add a separate time-varying request and response to `MEHLISSA::cell_model`.
The request contains a strictly increasing series of concentration knots whose
first offset is zero. Each value applies on the left-closed interval beginning
at its knot. The cell model integrates

```text
df/dt = kon L(t) (1 - f) - koff f
```

for bound receptor fraction `f` with fixed-step classical fourth-order
Runge-Kutta. A step is shortened to end exactly at every concentration knot and
at the observation boundary, so a discontinuity is never integrated across.

The profile declares the step size and a maximum number of steps. Evaluation
rejects a request when `step * (kon * max(L) + koff) > 1`, when a trajectory is
unordered or out of range, or when the step budget is exhausted. The response
contains bounded samples, final and peak occupancy, conserved final receptor
amounts, and the first upward threshold crossing. Crossing time is interpolated
between the two numerical states bracketing the threshold.

Keep ligand as a prescribed, non-depleting external reservoir. Do not modify the
M5.1 interface or claim that this ODE is an intracellular reaction network.

## Verification references

The profile contains two independently calculable cases:

1. A constant `0.0003 mol/m3` exposure for ten seconds must converge to the
   exact M5.1 solution.
2. A pulse that is off for two seconds, on for five seconds, and off for five
   seconds is solved analytically segment by segment. It verifies activation,
   first threshold crossing, peak occupancy, and post-pulse dissociation.

Tests require decreasing final-state error under step halving, agreement with
both analytical references, exact final receptor balance, bounded output, and
rejection of unsafe inputs.

## Consequences

MEHLISSA can now evaluate prescribed time-dependent cell-surface exposure and
distinguish peak response from the final state. The constant M5.1 solution
remains an independent oracle rather than becoming an internal special case of
the numerical solver.

The result is still deterministic and spatially homogeneous. It does not
consume ligand, propagate an M4 tissue field through time, persist cell state
between requests, model stochastic molecules, or trigger an intracellular or
higher-layer state change. Fixed-step RK4 is not presented as a stiff or
adaptive biological reaction-network solver.

## Alternatives considered

- **Extend the M5.1 request with optional trajectory fields:** rejected because
  it would mix exact and numerical semantics behind one model kind.
- **Forward Euler:** rejected because RK4 provides a stronger convergence
  baseline at modest implementation cost.
- **Adaptive or external ODE solver now:** deferred until reaction-network
  stiffness, error-control needs, and an external comparison case are known.
- **Linearly interpolate concentration:** deferred; piecewise-constant pulses
  have unambiguous segment-wise analytical references.
- **Subtract bound ligand from the source:** rejected because the current model
  prescribes an external reservoir and has no ligand mass-balance contract.

## Affected requirements and gates

- CELL-002 now includes deterministic time-dependent binding and detection.
- The receptor-binding part of the third M5 gate statement has analytical-limit
  and convergence evidence for constant and pulsed inputs.
- CELL-004 remains open because no intracellular reaction network is yet
  implemented or compared with external data.
- Stochastic single-cell and population variants, biological qualification,
  higher-layer feedback, release, uptake, and apoptosis remain open.
