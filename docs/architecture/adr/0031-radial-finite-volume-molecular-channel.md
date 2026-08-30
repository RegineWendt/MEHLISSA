<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0031: Radial finite-volume molecular channel

## Status

Accepted for M4.11.

## Context

M4.8 through M4.10 established analytical and particle-based implementations
behind one molecular-channel contract. The roadmap also requires a mesoscopic
resolution in which a population is represented as a concentration field.
That implementation must preserve the existing request and response, expose
its spatial state, conserve signal amount, and have an executable refinement
argument rather than merely another point estimate.

## Decision

Add a `radial_finite_volume_diffusion_3d` adapter that:

- evolves normalized active amount in concentric spherical control volumes;
- implements the unchanged `MolecularChannel` interface;
- exchanges amount conservatively across internal shell faces;
- uses a symmetry boundary at the origin and an absorbing far boundary whose
  escaped amount is recorded;
- applies exact split first-order degradation and records degraded amount;
- derives an explicit stable step count from cell width, diffusion, and a CFL
  safety factor;
- exposes the complete final field plus active, degraded, escaped, receiver,
  grid, time-step, and conservation diagnostics; and
- verifies coarse and refined grids against the analytical M4.8 case and each
  other under predeclared gates.

The strict profile names its compatible analytical profile. Diffusion and
degradation values must match. The receiver must lie wholly within the finite
domain, and configured grid work is bounded.

## Consequences

MEHLISSA now supports analytical, stochastic particle, explicit trajectory,
and deterministic field views of one channel experiment. Conservation and
boundary loss are explicit, a complete bounded spatial profile is available,
and grid refinement can expose source or discretization sensitivity.

The radial field cannot represent advection, asymmetric geometry, anatomical
walls, or spatially localized reactions. Its innermost-shell source and
interpolated off-center receiver are grid-dependent regularizations. Explicit
time stepping becomes more expensive quadratically with refinement in this
one-dimensional radial grid, so hard cell, step, and cell-step limits remain
part of the contract.

## Alternatives considered

- **Use a Cartesian three-dimensional field immediately:** deferred because it
  would make the first mesoscopic regression much larger without improving the
  shared spherically symmetric free-diffusion reference.
- **Return only the receiver concentration:** rejected because a mesoscopic
  implementation must make its spatial state and conservation auditable.
- **Renormalize after every update:** rejected because it would hide escaped or
  degraded amount and could conceal numerical errors.
- **Use a stochastic reaction-diffusion master equation first:** deferred; the
  deterministic field isolates spatial discretization and mass balance before
  adding population noise.
- **Treat the finite outer boundary as unbounded:** rejected; escaped amount is
  a first-class diagnostic with a profile gate.

## Affected requirements and gates

- CAP-006 remains complete and now includes the planned mesoscopic field
  implementation behind the stable molecular-channel contract.
- The Gate M4 multi-resolution evidence now spans analytical, endpoint,
  trajectory, and field adapters on the same synthetic reference case.
- Physiological channel validation remains open because the shared case has no
  anatomical surface, flow, measured signal parameters, or receiver kinetics.
- The next M4 increment is a conservative terminal/tissue ownership contract.
