<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2.4 – BVS Reference Regression

**Status:** passed

**Reference profile:** `bvs95-dissertation-rest-v1`

**Seed:** `2018`

**Machine-readable verification:**
`data/reference-results/bvs95-dissertation-rest-m2.4.json`

## Purpose and scientific boundary

M2.4 tests two different claims that must not be conflated:

1. **Dynamic BVS claims:** Does particle distribution approach a long-term value after about seven minutes, become largely independent of injection location, and remain similar with a tenfold population?
2. **Dissertation perfusion:** Does the 95-segment graph reproducibly represent the 23 organ and regional shares specified in Table 4.1?

The dynamic comparison is not a byte- or number-identical reproduction of the
2018 BloodVoyagerS run. That run used 94 vessels, predominantly 1:1 branches,
some 1:3 branches, and randomly varied velocities. MEHLISSA Next instead uses
the flow-conserving 95-segment graph reconstructed from the dissertation. M2.4
therefore tests the published qualitative and order-of-magnitude claims under
the new model contract.

## Reconstructed reference conditions

The following were adopted from the BloodVoyagerS paper:

- 6,359 particles as the reference population and 63,590 for the tenfold run;
- injection into vessel 1, the ascending aorta;
- comparison of vessel populations averaged over the preceding minute;
- minute 7 as the equilibrium check and minute 120 as the long-term reference;
- alternative injection in the left popliteal region;
- published comparison values of 3.11% mean normalized deviation and 3.95% injection-location deviation.

The dissertation supplies the added 95th coronary branch and target and
simulation shares for 23 regions. These values are tested separately against
stationary flows in the canonical graph.

## Metrics

For two averaged distributions `x` and `y` with `S = 95` segments:

```text
MAD = (1/S) * sum_i |x_i - y_i|
normalized MAD [%] = MAD / (N/S) * 100
```

At one point, the paper calls its averaged absolute vessel deviation “standard
deviation,” but describes an average of absolute differences. M2.4 therefore
unambiguously names the implemented measure **mean absolute difference (MAD)**
and does not claim identity with a statistical standard deviation.

The population comparison uses total-variation distance between minute-7
distributions normalized to their respective particle counts. Perfusion errors
are calculated in percentage points.

## Gates fixed before the run

| Gate | Limit | Rationale |
|---|---:|---|
| minute 7 versus minute 120 | at most 5% normalized MAD | includes the published 3.11% and model differences |
| aorta versus popliteal at minute 7 | at most 5% normalized MAD | includes the published 3.95% |
| 6,359 versus 63,590 particles | at most 2% total variation | tests scaling stability rather than identical counts |
| mean error versus perfusion target | at most 0.01 percentage points | deterministic stationary graph |
| maximum error versus perfusion target | at most 0.01 percentage points | prevents locally hidden outliers |
| mean difference from dissertation simulation | at most 0.5 percentage points | historical stochastic run, not a target |
| population conservation | exact | hard transport invariant |

The limits were fixed before the first complete M2.4 run and were not changed
afterward.

## Result

| Verification | Result | Limit | Status |
|---|---:|---:|---|
| minute 7 versus minute 120 | 1.843506% | 5% | passed |
| aorta versus popliteal, minute 7 | 1.741359% | 5% | passed |
| population scaling | 0.760720% | 2% | passed |
| mean perfusion-target error | 0.002140 percentage points | 0.01 | passed |
| maximum perfusion-target error | 0.007685 percentage points | 0.01 | passed |
| mean versus dissertation actual values | 0.314012 percentage points | 0.5 | passed |
| population conservation | exact | exact | passed |

At 10.886912%, minute 1 is still substantially farther from the long-term
distribution as expected. Minute 15 at 1.847935% is not monotonically below
minute 7, which is plausible for a finite stochastic population; the gate
requires convergence into a stable dispersion range, not a monotonic sequence.

## Implementation and reproduction

The reference runner processes segment transitions as events. It integrates
exact particle residence time per segment and derives the 60-second averages
from it. The long run is therefore not distorted by an arbitrarily chosen
global time step. Transitions are drawn in stable order from a named random
stream derived from the master seed. Total population is conserved exactly
after every complete run.

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe reference-bvs `
  --model data/body-models/bvs95-dissertation-rest-v1.json `
  --output data/reference-results/bvs95-dissertation-rest-m2.4.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json `
  --report-schema data/schemas/bvs-reference-report/1.0.0.schema.json
```

Before writing, the report is validated against
`data/schemas/bvs-reference-report/1.0.0.schema.json`. A test regenerates it and
compares it byte for byte with the committed golden reference.

## Claim and remaining risks

M2.4 demonstrates that Next transport is mass-conserving, reproducible, and
robust across the three BVS scenarios for this profile. It also demonstrates
that stationary flows migrated from the dissertation meet its target values.

M2.4 is **not independent physiological validation**: the target shares
calibrated the 1995 transitions and therefore cannot simultaneously serve as
external validation data. Anatomical vessel radii, pulsatile flow, within-
segment velocity distributions, and an independent female resting profile
remain tasks for M2.6 and later validation increments.

## Sources

- Wendt et al., *BloodVoyagerS – Simulation of the work environment of medical
  nanobots*, 2018, especially pp. 4–5.
- Regine Wendt, *Einsatz von Nanotechnologien in der Präzisionsmedizin*, 2024,
  Section 4.3.1, Table 4.1, and Table A.1.
- Public reference implementation:
  <https://github.com/RegineWendt/blood-voyager-s>.
