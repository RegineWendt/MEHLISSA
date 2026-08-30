<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Shared Axial Advection-Diffusion-Reaction Case

## 1. Decision and scope

M4.13 adds one executable reference case shared by an analytical solution, a
microscopic endpoint-particle model, and a mesoscopic finite-volume field. All
three use the same:

- equivalent pulmonary capillary radius and resting mean flow speed;
- source, receiver, axial domain, and observation time;
- diffusion coefficient;
- first-order bulk reaction; and
- cross-section-averaged cylindrical wall reaction.

The strict profile is:

```text
examples/capillary-models/pulmonary-shared-axial-transport-v1.json
data/schemas/axial-advection-reaction-profile/1.0.0.schema.json
```

This case does not alter the M4.8 free-diffusion request. Advection and surface
semantics materially change the physical question, so M4.13 uses a separate
versioned contract instead of adding hidden optional behavior to that stable
interface.

## 2. Geometry and flow binding

The profile is bound to the M4.7 definition
`pulmonary-healthy-adult-rest-supine-v1` and its
`alveolar-capillary-network` region. Loading succeeds only when:

- the configured lumen radius is half the executable `6.3 um` equivalent
  diameter;
- the configured `0.801990139 mm/s` speed equals volume flow divided by the
  total perfused cross-section; and
- the `400 um` local domain fits within the `688.909529 um` representative
  path.

The source begins at `100 um`. After `0.1 s`, advection places the analytical
mean at `180.199014 um`, which is also the center of a `20 um` receiver.
Diffusion with `D = 1e-9 m2/s` gives an axial standard deviation of
`14.142136 um`.

The radius and flow are evidence-bound equivalent geometry, not a reconstructed
alveolar sheet. The local source, receiver, and molecular parameters are
synthetic.

## 3. Shared reaction model

Bulk reaction uses the synthetic first-order rate

```text
k_bulk = 0.1 s^-1.
```

The wall uses a synthetic interaction velocity

```text
v_wall = 1e-7 m/s.
```

For a cross-section-averaged cylinder, surface per unit length divided by
volume per unit length is `2/R`. The equivalent first-order wall sink is

```text
k_wall = 2 v_wall / R = 0.06349206349 s^-1.
```

The shared active fraction is therefore

```text
S(t) = exp(-(k_bulk + k_wall)t).
```

Competing reaction loss is partitioned in proportion to the two rates. At
`0.1 s`, the analytical balance is:

| Destination | Amount fraction |
|---|---:|
| active | `0.9837837165` |
| bulk reacted | `0.0099186976` |
| wall reacted | `0.0062975858` |
| total | `1.0000000000` |

This is a homogenized surface sink. It does not resolve radial wall encounters,
binding sites, reversible adhesion, or receptor kinetics.

## 4. Three resolutions

### 4.1 Analytical reference

The infinite-line axial solution is the advected Gaussian

```text
c(x,t) = S(t) / sqrt(4 pi D t)
         exp(-(x - x0 - u t)^2 / (4 D t)).
```

The receiver fraction is the exact integral across the receiver interval. Its
reference value is `0.5120593043`.

### 4.2 Microscopic endpoint particles

The microscopic adapter samples 200,000 independent final axial positions from
the same Gaussian. A deterministic named stream also samples survival and, for
reacted particles, the competing bulk or wall destination. It records receiver,
active-in-domain, escaped, bulk-reacted, and wall-reacted counts.

The checked seed produces:

- 102,250 receiver observations, or `0.5112500000`;
- 1,977 bulk-reaction events;
- 1,224 wall-reaction events; and
- a receiver difference of `-0.7241` analytical standard errors.

Repeating the case reproduces every count exactly.

### 4.3 Mesoscopic finite-volume field

The field stores normalized active amount in uniform axial control volumes.
Conservative face fluxes combine mean advection with centered diffusion. Zero
external concentration supplies finite-domain boundary fluxes, which are
accounted separately as left and right escape. Exact per-step reaction splitting
then records bulk and wall loss independently.

The stable time step is derived from cell width, advection, diffusion, and a
configured CFL factor. Cell count, step count, and total cell-step work are hard
bounded. The final complete field is returned.

## 5. Verification result

The profile predeclares 256- and 512-cell fields:

| Quantity | Analytical | Particles | 256 cells | 512 cells |
|---|---:|---:|---:|---:|
| receiver fraction | `0.5120593043` | `0.5112500000` | `0.5239759053` | `0.5155977421` |
| relative reference error | — | statistical gate | `2.3272%` | `0.6910%` |
| time steps | — | endpoint sample | `593` | `1,913` |

The coarse-refined difference is `1.5990%`. Finite-volume boundary escape is
zero at displayed precision. Absolute balance residuals are `1.44e-15` and
`3.66e-15` for the two fields; analytical and particle counts close exactly.
Every predeclared gate passes.

## 6. What this establishes

M4.13 is the first MEHLISSA case in which the same geometry and physical
parameters simultaneously exercise:

- directed blood-flow advection;
- molecular diffusion;
- a bulk reaction;
- a surface-derived reaction;
- stochastic microscopic sampling;
- a conservative mesoscopic field; and
- an analytical solution with statistical and grid-refinement gates.

It verifies cross-resolution semantics and accounting. It does not validate a
pulmonary molecule, wall chemistry, or physiological receiver. A future richer
model may replace the cross-section average with explicit radial geometry and
receptor kinetics while retaining this case as a regression baseline.

See [ADR-0033](../architecture/adr/0033-shared-axial-advection-reaction-case.md).
