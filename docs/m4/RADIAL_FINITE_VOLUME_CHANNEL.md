<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Radial Finite-Volume Molecular Channel

## 1. Decision and scope

M4.11 adds a deterministic mesoscopic concentration-field adapter behind the
unchanged M4.8 `MolecularChannel` interface. It represents normalized active
signal amount in concentric spherical control volumes rather than as individual
molecules. The complete final field and its active, degraded, and escaped
amount fractions are observable.

The executable profile is:

```text
examples/capillary-models/pulmonary-synthetic-tracer-radial-field-v1.json
data/schemas/radial-finite-volume-channel-profile/1.0.0.schema.json
```

The field uses the same emission, diffusion coefficient, degradation rate,
receiver, separation, and observation time as the analytical M4.8 pulmonary-
bound synthetic tracer case. It therefore adds a third modeling resolution
without changing experiment-facing request or response semantics.

## 2. Evidence and numerical method

Bernstein derives mesoscopic subvolume diffusion rates from a finite-volume
discretization of the heat equation and shows convergence to that equation in
the appropriate limit:

- Bernstein D. *Simulating mesoscopic reaction-diffusion systems using the
  Gillespie algorithm*. Physical Review E. 2005;71:041103.
  [doi:10.1103/PhysRevE.71.041103](https://doi.org/10.1103/PhysRevE.71.041103).

FiPy documents a cell-centered finite-volume framework for conservative PDE
simulation, while AcCoRD demonstrates why analytical, mesoscopic, and
microscopic reaction-diffusion descriptions should be compared through shared
cases:

- Guyer JE, Wheeler D, Warren JA. *FiPy: Partial Differential Equations with
  Python*. Computing in Science & Engineering. 2009;11(3):6-15.
  [doi:10.1109/MCSE.2009.52](https://doi.org/10.1109/MCSE.2009.52).
- Noel A et al. *Simulating with AcCoRD: Actor-based communication via
  reaction-diffusion*. Nano Communication Networks. 2017;11:44-75.
  [doi:10.1016/j.nancom.2017.02.002](https://doi.org/10.1016/j.nancom.2017.02.002).

MEHLISSA independently implements the radial diffusion equation

```text
partial c / partial t = D (1 / r^2) partial/partial r (r^2 partial c/partial r) - k c
```

on uniform spherical shells. For an interface at radius `r`, the explicit
amount transfer during `Delta t` is

```text
Delta a = D 4 pi r^2 (c_left - c_right) Delta t / Delta r.
```

The same transfer is subtracted from one shell and added to its neighbor, so
internal fluxes conserve amount by construction. The origin has zero flux by
spherical symmetry. A zero-concentration absorbing outer boundary accounts for
escaped amount separately. First-order degradation uses the exact split factor
`exp(-k Delta t)` and is also accumulated separately.

The explicit time step is chosen automatically from

```text
Delta t_max = safety (Delta r)^2 / (6D),
```

with a configured safety factor of `0.45`. Cell count, time-step count, and
their product have hard bounds. Every update must remain finite and nonnegative.

## 3. Source, receiver, and boundary semantics

The point release is regularized by placing one normalized emitted amount in
the innermost shell. This approximation shrinks with grid refinement. At the
observation time, MEHLISSA linearly interpolates the two neighboring shell-
center concentrations at the transmitter-receiver separation. It then applies
the same passive small-receiver approximation as M4.8:

```text
receiver amount fraction = normalized local concentration * receiver volume.
```

This is final-time occupancy, not cumulative first passage or binding. The
receiver must fit wholly inside the radial domain.

The outer radius is `25.2 um`, eight times the `3.15 um` transmitter-receiver
separation. The reference profile requires escaped amount at the observation
time to be no more than `1e-12`; thus the finite boundary is explicitly checked
rather than silently treated as infinity.

## 4. Verification gate

The profile predeclares coarse and refined grids of 128 and 256 shells. Both are
compared with the analytical free-diffusion response, and the two field results
are compared with each other. It requires:

1. coarse analytical relative error at most `10%`;
2. refined analytical relative error at most `3%`;
3. coarse-refined relative difference at most `10%`;
4. absolute amount-balance residual at most `1e-12`; and
5. escaped amount fraction at most `1e-12`.

The balance identity is checked as

```text
active fraction + degraded fraction + escaped fraction = 1.
```

Separate tests activate degradation and a nearby absorbing boundary to prove
that both loss destinations are nonzero, distinct, and included in the same
balance.

## 5. Executable result

At `1.65375 ms`, the analytical receiver amount fraction is
`0.00030836065960745`.

| Quantity | 128 shells | 256 shells |
|---|---:|---:|
| time steps | `569` | `2,276` |
| receiver fraction | `0.00030846877995154` | `0.00030838720474097` |
| relative analytical error | `0.035063%` | `0.008608%` |
| escaped fraction | `0` | `0` |
| absolute balance residual | `4.44e-16` | `0` |

The coarse-refined relative difference is `0.026452%`. Every gate passes, and
the refined evaluation repeats exactly because the field solver is
deterministic.

## 6. Interpretation boundary and next increment

This result verifies a conservative population/field resolution and the common
channel contract. It does not validate pulmonary molecular transport. Spherical
symmetry excludes directional blood flow, asymmetric walls, branching geometry,
and localized tissue. The source and off-center receiver are numerical
approximations, and all molecular parameters remain synthetic.

M4 now contains analytical, endpoint-particle, trajectory-particle, and radial-
field implementations that can answer the same final-time receiver request.
M4.12 now defines conservative terminal ownership for state-changing retention,
adhesion, and extravasation. A later shared case can introduce qualified
surfaces, advection, reactions, and receiver kinetics across the mesoscopic and
microscopic resolutions.

See [ADR-0031](../architecture/adr/0031-radial-finite-volume-molecular-channel.md).
