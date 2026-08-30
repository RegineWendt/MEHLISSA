<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0029: Deterministic Brownian particle comparison

## Status

Accepted for M4.9.

## Context

M4.8 introduced a stable molecular-channel contract and one analytical
free-diffusion implementation. Gate M4 also requires implementations at
different resolutions to be compared against the same reference. Importing an
external simulator immediately would add installation and licensing boundaries
before MEHLISSA had demonstrated that its own adapter seam and statistical
comparison were sufficient.

## Decision

Add an independently implemented `brownian_particle_endpoint_3d` adapter that:

- implements the unchanged M4.8 `MolecularChannel` request and response;
- samples individual three-dimensional Brownian endpoints with coordinate
  variance `2Dt`;
- handles optional first-order degradation through independent survival draws;
- uses an explicit seed, named stream, 53-bit uniform conversion, and
  Box-Muller normal transform;
- exposes sample count, receiver count, and estimated standard error as
  diagnostics outside the stable common response; and
- compares its receiver fraction with the analytical model using predeclared
  observation-count, standardized-error, and relative-error gates.

The particle profile must name its compatible analytical profile and match its
diffusion and degradation parameters before construction.

## Consequences

MEHLISSA now has two independently implemented molecular channels behind one
interface and a reproducible comparison result. Monte Carlo uncertainty is
visible rather than mistaken for physiological uncertainty. External simulators
can later map to the same contract and comparison record.

The endpoint sampler is exact only for the current unbounded homogeneous
free-diffusion case. It does not retain paths or resolve collisions, boundaries,
flow, or reactions. Exact cross-platform hit counts are not a scientific gate;
statistical agreement is. A trajectory-resolving or external adapter remains
necessary before claiming a generally detailed molecular environment.

## Alternatives considered

- **Copy N3Sim or AcCoRD into the repository:** rejected because their code and
  license boundaries should remain explicit and independently replaceable.
- **Assert one fixed particle count on every platform:** rejected because the
  scientifically relevant invariant is agreement within predeclared sampling
  uncertainty, not transcendental-library bit identity.
- **Use a time-stepped walk for the unbounded reference:** rejected because it
  adds discretization cost and error without testing any boundary interaction.
- **Compare only relative error:** rejected because statistical resolution also
  depends on sample count and event rarity.

## Affected requirements and gates

- CAP-006 is complete at the software-contract level.
- Analytical and particle-surrogate channels are compared against the same
  pulmonary-bound reference request.
- The broader detailed-versus-surrogate Gate M4 statement remains partial until
  trajectory-resolving or external-tool comparison covers a richer local case.
