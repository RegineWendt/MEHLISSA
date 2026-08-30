<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Trajectory-Resolving Brownian Channel

## 1. Decision and scope

M4.10 adds a fixed-step microscopic Brownian adapter behind the unchanged M4.8
`MolecularChannel` interface. Unlike the M4.9 endpoint sampler, it advances each
particle through explicit three-dimensional positions and can retain a strictly
bounded subset of complete trajectories.

The executable profile is:

```text
examples/capillary-models/pulmonary-synthetic-tracer-trajectory-v1.json
data/schemas/trajectory-brownian-channel-profile/1.0.0.schema.json
```

The checked-in pulmonary comparison remains unbounded so that 8-step and
32-step particle paths can both be assessed against the same analytical
free-diffusion case. The implementation also supports reflecting axis-aligned
box walls, but the tested box is a software geometry and must not be interpreted
as alveolar anatomy.

## 2. Evidence and numerical method

Andrews and Bray describe Brownian dynamics as normally distributed molecular
displacements at each fixed time step and recommend repeating simulations with
smaller steps to assess whether relevant results are stable:

- Andrews SS, Bray D. *Stochastic simulation of chemical reactions with spatial
  resolution and single molecule detail*. Physical Biology. 2004;1:137-151.
  [doi:10.1088/1478-3967/1/3/001](https://doi.org/10.1088/1478-3967/1/3/001).

The later Smoldyn description covers three-dimensional Gaussian displacements,
molecule-surface interactions, and convergence toward Smoluchowski dynamics:

- Andrews SS et al. *Detailed simulations of cell biology with Smoldyn 2.1*.
  PLoS Computational Biology. 2010;6:e1000705.
  [doi:10.1371/journal.pcbi.1000705](https://doi.org/10.1371/journal.pcbi.1000705).

AcCoRD independently uses a fixed microscopic time step and analytical
reaction-diffusion reference cases:

- Noel A et al. *Simulating with AcCoRD: Actor-based communication via
  reaction-diffusion*. Nano Communication Networks. 2017;11:44-75.
  [doi:10.1016/j.nancom.2017.02.002](https://doi.org/10.1016/j.nancom.2017.02.002).

MEHLISSA does not copy or execute these simulators. For every time step
`Delta t`, it independently draws:

```text
Delta Xi ~ Normal(0, 2 D Delta t),  i in {x,y,z}
```

The accumulated path is retained only for the configured particle subset. For
first-order degradation rate `k`, a surviving signal remains active through a
step with probability:

```text
Pstep = exp(-k Delta t)
```

Diffusive motion is sampled independently for every complete latent path even
after its signal-survival flag becomes false. This keeps the diffusion
diagnostic based on all configured samples; only surviving paths can contribute
to receiver amount or concentration.

An optional reflecting box mirrors a proposed coordinate at every crossed
planar boundary and counts all reflections. The receiver must lie completely
inside that box. Receiver output still means occupancy at the requested final
observation time, not first passage or cumulative contact.

## 3. Bounded trajectories and determinism

The profile fixes:

- sample count: `400,000`;
- coarse resolution: `8` steps;
- refined resolution: `32` steps;
- experiment seed: `20260830`;
- named stream: `capillary.molecular-channel.brownian-trajectory`; and
- retained trajectories: the first `4` particles, at most `132` points.

The initial position and every refined step are retained for those four
particles, hence `4 * (32 + 1) = 132` points. All remaining particles still
contribute to aggregate receiver, survival, reflection, and displacement
statistics. If a configured point bound is smaller, excess trace points are
counted rather than allocated.

Coarse and refined simulations receive distinct step-count-qualified random
streams. Repeating one resolution with the same request produces the same local
receiver count, displacement statistic, and retained paths.

## 4. Verification gate

Both resolutions must independently satisfy the analytical receiver-fraction
gate. The comparison also assesses the coarse-refined fraction difference and
the free three-dimensional Brownian mean squared displacement:

```text
E[|X(t)|^2] = 6 D t
```

The predeclared profile requires:

1. at least `50` receiver observations at each resolution;
2. absolute analytical standardized error at most `4`;
3. analytical relative receiver-fraction error at most `25%`;
4. absolute coarse-refined standardized difference at most `4`; and
5. relative mean squared displacement error at most `1%`.

The count, standardized, relative, and displacement criteria cover different
failure modes. They are software-regression limits, not biological uncertainty
intervals.

## 5. Executable result

The observation time is `1.65375 ms`, giving step widths of `206.71875 us` and
`51.6796875 us`. The analytical receiver fraction is `0.00030836066`, and the
expected mean squared displacement is `9.9225e-12 m2`.

| Quantity | 8 steps | 32 steps |
|---|---:|---:|
| receiver observations | `121 / 400,000` | `119 / 400,000` |
| receiver fraction | `0.0003025` | `0.0002975` |
| relative fraction error | `1.9006%` | `3.5221%` |
| analytical standardized error | `-0.2111` | `-0.3912` |
| relative MSD error | `0.0137%` | `0.0334%` |

The coarse-refined standardized difference is `-0.1291`. Every gate passes,
and the 32-step evaluation repeats exactly for the fixed local seed and stream.

## 6. Interpretation boundary and next increment

For unbounded homogeneous free diffusion, the sum of Gaussian increments has
the exact final Gaussian distribution for every step count. The refinement
result therefore verifies path construction, accumulation, statistics, and
interface consistency; it does not demonstrate a decreasing endpoint
discretization bias. Step size becomes an accuracy question when surfaces,
spatially varying properties, reactions, or receiver encounters are active.

The reflecting-box test proves containment, receiver-fit validation, reflection
accounting, and bounded traces. It does not qualify a pulmonary wall geometry.
The molecular parameters, source, receiver, and environment remain synthetic.
Still absent are flow/advection, anatomical surfaces, heterogeneous diffusion,
binding and other reactions, interparticle effects, receiver kinetics, and
measured pulmonary molecular-channel data.

M4.11 fulfills the next resolution increment with a mesoscopic radial finite-
volume concentration field on the same contract. M4.13 then compares advection,
bulk reaction, and a surface-to-volume wall sink across analytical, endpoint-
particle, and axial finite-volume resolutions. Explicit trajectory-wall
encounters and receiver kinetics remain open.

See [ADR-0030](../architecture/adr/0030-trajectory-resolving-brownian-channel.md).
