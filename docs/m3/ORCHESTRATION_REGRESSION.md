<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Body–Lung Orchestration Regression

## Composition path

The M3 regression loads each checked-in lung definition through a versioned
schema and loader, passes its typed configuration to `make_lung_model`, and
hands the resulting `ModelComponent` to `BodyOrganCoupler`. The body, coupler,
and kernel contain no branch for the selected lung resolution.

## Synchronization matrix

The identical scenario is generated for four combinations:

| Lung definition | Host step |
|---|---:|
| effective compartment | 1.0 s |
| effective compartment | 0.5 s |
| regional circulation | 1.0 s |
| regional circulation | 0.5 s |

In every combination entity 1 leaves `artery-10` at simulation time zero,
remains under explicit outside-body/organ ownership, and returns exactly once
to `vein-90` at 2.0 s. A parallel four-combination regression carries a
population of 10,000 nanodevices, 2.5 mmol, and 0.0001 m³/s over two seconds
without changing their typed payloads.

These are compatible fixed synchronization steps. They prove deterministic
step subdivision for the current transit models, not asynchronous or adaptive
multirate co-simulation. Any future scheduler must preserve the same ownership,
route, time, and conservation checks.

M3.7 extends the matrix with the literature-parameterized pulmonary 0D card at
0.1 s and 0.2 s host steps. The same entity completes the body–lung–body route
at its 6.4 s measured transit. A separate generated two-second contract setup
runs entity, population, substance, and flow payloads through all three factory
variants, while the physiological card retains its source-scoped timing.
