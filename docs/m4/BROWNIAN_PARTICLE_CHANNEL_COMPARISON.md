<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Brownian Particle-Channel Comparison

## 1. Decision and scope

M4.9 adds an independently implemented stochastic channel adapter behind the
M4.8 `MolecularChannel` interface. It uses the same pulmonary-bound request as
the analytical channel and compares both implementations with a predeclared
statistical gate.

The executable particle profile is:

```text
examples/capillary-models/pulmonary-synthetic-tracer-brownian-v1.json
data/schemas/brownian-particle-channel-profile/1.0.0.schema.json
```

This closes CAP-006 at the software-contract level: an analytical model and a
particle surrogate are interchangeable and run against one reference case. It
does not yet provide a trajectory-resolving, bounded-tissue, or external-tool
comparison.

## 2. Evidence and implementation choice

N3Sim represents molecular propagation with Brownian particle dynamics and was
created to validate diffusion-channel models and communication protocols:

- Llatser I et al. *N3Sim: Simulation framework for diffusion-based molecular
  communication nanonetworks*. Simulation Modelling Practice and Theory.
  2014;42:210-222.
  [doi:10.1016/j.simpat.2013.11.004](https://doi.org/10.1016/j.simpat.2013.11.004).

AcCoRD combines microscopic and mesoscopic reaction-diffusion simulation and
explicitly verifies simulation results against analytical reference cases:

- Noel A et al. *Simulating with AcCoRD: Actor-based communication via
  reaction-diffusion*. Nano Communication Networks. 2017;11:44-75.
  [doi:10.1016/j.nancom.2017.02.002](https://doi.org/10.1016/j.nancom.2017.02.002).

M4.9 does not copy or execute either simulator. It implements the minimal
Brownian endpoint experiment needed to test MEHLISSA's channel boundary. In an
unbounded homogeneous medium, each coordinate after observation time `t` is
sampled independently as:

```text
Xi(t) ~ Normal(0, 2 D t),  i in {x,y,z}
```

The receiver is a sphere centered at the request's transmitter-receiver
separation. A sample is observed when its endpoint lies inside that sphere and,
for degradation rate `k`, it survives an independent event with probability:

```text
Psurvival = exp(-k t)
```

Direct endpoint sampling is exact for this simple propagation case and avoids
introducing a microscopic time-step error. It does not retain trajectories, so
it is a particle surrogate rather than the final detailed local model.

## 3. Determinism

The profile fixes:

- sample count: `2,000,000`;
- experiment seed: `20260830`; and
- named random stream: `capillary.molecular-channel.brownian-endpoint`.

MEHLISSA derives a request-specific stream and converts its stable 53-bit
uniform grid to normal pairs with an explicit Box-Muller transform. Repeating
the same request therefore produces the same observation count on one supported
runtime. Scientific acceptance does not depend on an exact cross-platform hit
count; it depends on the predeclared statistical gate.

## 4. Comparison statistics

For `H` receiver observations among `N` particle samples:

```text
phat = H / N
SE(phat) = sqrt(phat (1 - phat) / N)
```

The standardized comparison uses the analytical reference fraction `pref` as
the binomial null:

```text
z = (phat - pref) / sqrt(pref (1 - pref) / N)
```

The profile was fixed before executing the regression and requires all three:

1. at least `200` receiver observations;
2. `|z| <= 4`; and
3. relative fraction error at most `15%`.

The count floor prevents a superficially small relative result with inadequate
Monte Carlo support. The standardized and relative bounds guard complementary
failure modes. These are software-verification thresholds, not confidence
limits for pulmonary physiology.

## 5. Executable result

For the shared M4.8 request:

| Quantity | Analytical reference | Brownian endpoint result |
|---|---:|---:|
| receiver fraction | `0.00030836066` | `0.0002955` |
| expected/observed sample count | `616.72` | `591` |
| relative fraction error | - | `4.1707%` |
| absolute standardized error | - | `1.0359` |

The result passes all predeclared gates and repeats exactly for the fixed local
seed and stream. The comparison also confirms that both implementations return
concentration and amount through the same `MolecularChannelResponse` contract.

## 6. Scientific boundary and next increment

Agreement here means that two independent implementations are consistent for
one synthetic, unbounded free-diffusion problem. It does not validate the M4.8
small-receiver approximation in all geometries and does not qualify a real
pulmonary signaling molecule.

M4.10 subsequently adds bounded particle trajectories, fixed time steps,
coarse-refined verification, and reflecting-box software support without
changing the v1 request/response semantics. Anatomical walls, advection,
heterogeneous media, reactions and binding, interparticle effects, receiver
kinetics, symbol sequences, and measured biological parameters remain absent.

See [ADR-0029](../architecture/adr/0029-deterministic-brownian-particle-comparison.md)
and [Trajectory-Resolving Brownian Channel](TRAJECTORY_BROWNIAN_CHANNEL.md).
