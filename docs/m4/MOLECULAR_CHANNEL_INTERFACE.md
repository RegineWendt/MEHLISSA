<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Interchangeable Molecular-Channel Interface

## 1. Decision and scope

M4.8 adds the first stable molecular-channel boundary and connects one
analytical implementation to the M4.7 pulmonary capillary reference candidate.
It completes the analytical half of requirement CAP-006. It does **not** yet
qualify a pulmonary messenger molecule or compare an external simulator.

The executable reference files are:

```text
examples/capillary-models/pulmonary-synthetic-tracer-diffusion-v1.json
data/schemas/molecular-channel-profile/1.0.0.schema.json
```

The capillary definition supplies a context model, named capillary region,
equivalent diameter, and residence-time ceiling. The channel profile supplies
all propagation and signaling assumptions. This separation prevents a
synthetic diffusion coefficient or receiver from becoming an undocumented
property of the pulmonary model.

## 2. Stable request and response

Every `MolecularChannel` implementation consumes the same typed request:

| Field | Meaning |
|---|---|
| request and signal IDs | traceable experiment and molecular-signal identity |
| context model and region IDs | spatial provenance without ownership of the capillary model |
| emitted amount | SI amount in mol |
| transmitter-receiver separation | SI length in m |
| passive receiver volume | SI volume in m3 |
| observation time | simulation-clock duration |

It returns the implementation ID, expected receiver concentration, expected
amount within the passive receiver, and the corresponding fraction of the
emitted amount. The interface does not expose implementation-specific particle
state. An analytical model, particle simulator, measured response surface, or
surrogate distribution can therefore produce the same high-level result.

## 3. First analytical implementation

`AnalyticalDiffusionChannel` implements the Green's-function solution for an
instantaneous point release into an unbounded, homogeneous, isotropic
three-dimensional medium with constant diffusion coefficient `D` and optional
homogeneous first-order degradation rate `k`:

```text
c(r,t) = n0 / (4 pi D t)^(3/2) * exp(-r^2 / (4 D t) - k t)
```

Here `n0` is emitted amount, `r` is transmitter-receiver separation, and
`c(r,t)` is expected concentration at the receiver center. For a sufficiently
small passive receiver of volume `Vrx`, M4.8 uses the uniform-concentration
approximation:

```text
nrx(t) = c(r,t) * Vrx
```

The reference observation is made at the analytical concentration peak. With
`a = r^2/(4D)`:

```text
tpeak = 2a / (3/2 + sqrt(9/4 + 4ka))
```

For `k = 0`, this reduces to `r^2/(6D)`. The implementation rejects zero or
non-finite quantities, receivers outside the declared small-receiver ratio,
non-finite results, and receiver amounts greater than the emitted amount.

## 4. Evidence review and adapter choice

Pierobon and Akyildiz describe molecular communication as distinct transmitter,
propagation, and receiver modules and develop a physical diffusion-based
end-to-end model. M4.8 preserves this separation but deliberately implements
only a narrow propagation-and-passive-observation contract:

- Pierobon M, Akyildiz IF. *A physical end-to-end model for molecular
  communication in nanonetworks*. IEEE JSAC. 2010;28(4):602-611.
  [doi:10.1109/JSAC.2010.100509](https://doi.org/10.1109/JSAC.2010.100509).

N3Sim is a particle-oriented diffusion simulator with transmitters, receivers,
and harvesting nodes. Its Brownian implementation is scientifically useful as
an independent comparison target, rather than code to silently embed in the
analytical component:

- Llatser I et al. *N3Sim: Simulation framework for diffusion-based molecular
  communication nanonetworks*. Simulation Modelling Practice and Theory.
  2014;42:210-222.
  [doi:10.1016/j.simpat.2013.11.004](https://doi.org/10.1016/j.simpat.2013.11.004).

The current implementation therefore provides a clean adapter seam. The next
channel increment should run a licensable external or independently
implemented particle surrogate against exactly the same request and response,
not introduce another application-specific API.

## 5. Pulmonary-bound contract case

The reference profile derives:

- separation as half the M4.7 equivalent capillary diameter:
  `6.30 um / 2 = 3.15 um`;
- passive receiver radius as one tenth of that separation: `0.315 um`;
- observation time as the analytical peak for the synthetic
  `D = 1e-9 m2/s`: `1.65375 ms`;
- receiver volume from the spherical radius: `1.309243e-19 m3`; and
- a residence check against the M4.7 capillary ceiling of `0.859 s`.

For the synthetic `1e-18 mol` impulse with no degradation, the executable
regression expects:

| Result | Value |
|---|---:|
| peak concentration | `2.355259e-3 mol/m3` |
| expected amount in passive receiver | `3.083607e-22 mol` |
| expected receiver fraction | `3.083607e-4` |

Only the capillary equivalent diameter and residence ceiling come from the
M4.7 evidence card. The tracer identity, amount, diffusion coefficient,
transmitter-receiver placement, receiver size, and lack of degradation are
synthetic verification choices. The result must not be interpreted as oxygen
exchange, a biological detection probability, or a clinical prediction.

## 6. Verification and remaining work

Automated tests verify:

- strict schema and metadata loading;
- factory construction through the abstract `MolecularChannel` interface;
- derivation from the named pulmonary capillary region;
- analytical peak time and numerical impulse-response result;
- dimensional concentration-to-amount closure;
- rejection of incompatible capillary definitions; and
- rejection of a receiver outside the approximation boundary.

Still open are advection, bounded or heterogeneous geometry, reactions and
binding, stochastic counting noise, symbol sequences and intersymbol
interference, detection thresholds, a physiologically identified signal, and
comparison with an independent detailed or surrogate implementation.

See [ADR-0028](../architecture/adr/0028-interchangeable-molecular-channel-contract.md).
