<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0033: Shared axial advection-reaction case

## Status

Accepted for M4.13.

## Context

M4.8 through M4.11 compare analytical, particle, trajectory, and field models
through free diffusion. Those cases deliberately exclude directional blood
flow and surfaces. The M4 gate also requires detailed and surrogate models to
be compared through shared reference cases, and the pulmonary M4.7 card now
provides an evidence-bound equivalent radius, path length, and flow closure.

Adding advection or wall behavior as optional fields on the M4.8 request would
change its physical semantics and make existing implementations silently
incomplete. A new case must also distinguish numerical verification from
physiological validation.

## Decision

Define a separate versioned axial advection-diffusion-reaction profile. Bind its
radius, local-domain extent, and mean velocity to the executable M4.7 pulmonary
capillary definition.

Use one cross-section-averaged cylindrical model. Convert a wall interaction
velocity into the homogeneous sink `k_wall = 2 v_wall / R`, keep bulk and wall
reaction destinations separate, and require every resolution to close active,
reacted, and escaped amount.

Provide three independent evaluations:

1. an infinite-line advected Gaussian with exact reaction attenuation and
   receiver integration;
2. deterministic endpoint particles with explicit competing reaction outcomes;
3. a conservative one-dimensional finite-volume field with explicit boundary
   loss and a nested coarse/refined grid.

Use predeclared statistical, analytical-error, refinement, conservation, and
boundary-influence gates. Keep molecular and kinetic parameters synthetic and
label the profile as a software-test surrogate.

## Consequences

MEHLISSA now compares microscopic and mesoscopic models on one case that includes
flow, diffusion, volume reaction, and surface-derived reaction. The profile
cannot drift away from its capillary radius or flow without being rejected.
Bulk reaction, wall reaction, and boundary escape remain auditable rather than
being collapsed into generic loss.

The model is only one-dimensional and cross-section averaged. Its evidence-bound
radius does not make its synthetic wall kinetics physiological, and the endpoint
particles do not resolve actual wall encounters. The infinite-line analytical
reference requires the finite boundaries to be far enough away, so boundary
escape remains an explicit gate.

## Alternatives considered

- **Extend the M4.8 free-diffusion request:** rejected because advection and wall
  semantics would break the meaning of the stable contract.
- **Implement only a new finite-volume solver:** rejected because it would not
  provide a shared microscopic/mesoscopic comparison.
- **Start with a full three-dimensional alveolar sheet:** deferred because no
  licensable reconstructed geometry and jointly qualified kinetics are yet
  available, and numerical complexity would obscure the first shared gate.
- **Use an absorbing wall without separate accounting:** rejected because
  surface reaction must be distinguishable from bulk reaction and domain escape.
- **Treat the equivalent tube as anatomical:** rejected; M4.7 explicitly limits
  it to volume-flow-transit closure.

## Affected requirements and gates

- CAP-006 now includes a shared advection-diffusion-bulk/surface-reaction case
  across analytical, particle, and field resolutions.
- The final M4 comparison gate has a richer executable reference beyond free
  diffusion.
- A formal M4 gate review is the next milestone action.
- Explicit radial wall encounters, receptor kinetics, external adapters, and
  physiological signal qualification remain future increments.
