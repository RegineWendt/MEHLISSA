<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0030: Trajectory-resolving Brownian channel

## Status

Accepted for M4.10.

## Context

M4.9 verified the common molecular-channel boundary with an independently
implemented endpoint sampler. Direct endpoint sampling is efficient and exact
for unbounded free diffusion but cannot expose particle paths, time-grid
behavior, surface handling, or bounded trajectory output. The next detailed
increment must add these capabilities without changing the stable M4.8 request
and response.

## Decision

Add a `brownian_trajectory_3d` adapter that:

- advances every particle through fixed three-dimensional Gaussian increments;
- implements the unchanged `MolecularChannel` interface;
- uses step-count-qualified deterministic random streams;
- applies optional exact first-order survival at each step;
- supports unbounded space and a reflecting axis-aligned box;
- retains only a configured particle subset under an absolute point bound and
  reports dropped points;
- exposes receiver count, survival count, reflection count, and mean squared
  displacement as diagnostics; and
- verifies coarse and refined time grids against the analytical receiver
  fraction, each other, and the `6Dt` mean squared displacement.

The strict profile names its compatible analytical profile. Diffusion and
degradation parameters must match before either resolution can be constructed.
The checked-in pulmonary reference uses unbounded space; reflecting walls are a
separately tested software capability.

## Consequences

MEHLISSA can now inspect bounded individual molecular paths while preserving
aggregate statistics over all particles. The stable channel contract supports
analytical, endpoint-particle, and trajectory-particle implementations. Invalid
time-grid relationships, unbounded work, incompatible profiles, oversized
receivers, and trace-buffer growth are rejected or counted explicitly.

The unbounded Gaussian endpoint distribution is exact at every time grid, so
the current refinement test is an implementation and path-statistics check, not
evidence of endpoint discretization convergence. Reflecting boxes do not
represent pulmonary anatomy. Runtime grows with sample count times step count,
and only a bounded subset of paths can be retained.

## Alternatives considered

- **Replace the M4.9 endpoint adapter:** rejected because the endpoint sampler
  remains the efficient stochastic surrogate and provides a useful independent
  comparison.
- **Store every particle position:** rejected because memory would grow with
  sample count times step count and violate the project's bounded-observation
  policy.
- **Claim conventional step convergence in free space:** rejected because sums
  of Gaussian increments already have the exact final free-diffusion law.
- **Treat the reflecting box as lung anatomy:** rejected because no anatomical
  geometry or surface parameters support that interpretation.
- **Import an external simulator now:** deferred until the internal contract can
  represent a richer surface/reaction case without coupling the core to one
  external runtime.

## Affected requirements and gates

- CAP-006 remains complete at the software-contract level and now includes a
  trajectory-resolving adapter.
- The detailed-versus-surrogate Gate M4 statement gains an executable
  trajectory comparison but remains scientifically partial while the shared
  case is synthetic and unbounded.
- The next resolution increment is a mesoscopic adapter against the same
  request and reference family.
