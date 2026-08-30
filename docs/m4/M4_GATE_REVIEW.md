<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M4 Gate Review – Capillary Communication

**Review date:** 30 August 2026

**Reviewed implementation baseline:**
`7ba05eab408ddd42cd6be1eea32e7c18de2ea267`

**Result:** passed — all four Roadmap Gate M4 statements are executable at the
software-contract level

## 1. Review method

The review checked each Gate M4 statement against executable code, strict
versioned profiles, automated positive and negative tests, architecture
decisions, and the declared evidence scope. A class or document alone was not
accepted as evidence. The review also checked cross-cutting ownership,
conservation, units, determinism, bounded work and output, and cross-platform
quality evidence.

Software verification was kept separate from anatomical fidelity,
physiological parameter qualification, and independent biological validation.
The explicit Roadmap gate is narrower than the complete long-term `CAP-*`
research requirements. A remaining `PART` or `RESEARCH` item therefore blocks
M4 only when it contradicts one of the four gate statements.

## 2. Roadmap gate criteria

| Gate M4 criterion | Status | Executable evidence and finding |
|---|---|---|
| a nanodevice can leave an organ, traverse a capillary bed, and return | satisfied | the organ–capillary coupler transfers one stable entity ID through explicit organ-outlet, capillary-inlet, capillary-outlet, and organ-return ports; exact ownership ledgers, pending-delivery queues, and tests across 100, 250, and 500 ms host steps close the route without loss or duplication; optional M4.12 dispositions either return the entity or transfer it exactly once to an acknowledged terminal owner, and rejected delivery remains retryable |
| substance exchange conserves mass and is unit-consistent | satisfied for the declared M4 exchange contract | typed SI substance amounts are partitioned among outgoing blood, endothelium, interstitium, and cell; the implementation rejects incompatible profiles and non-conservative partitions, records an explicit balance, prevents duplicate processing, and completes the full organ–capillary–organ route |
| at least one molecular channel is connected through a stable interface | satisfied | the implementation-neutral `MolecularChannel` request/response contract supports analytical free diffusion, endpoint Brownian particles, trajectory-resolving Brownian motion, and a radial finite-volume field without changing the M4.8 boundary; strict profiles reject incompatible contexts and approximations |
| detailed and surrogate models are compared against the same reference cases | satisfied | the M4.8 free-diffusion request is shared by analytical, endpoint-particle, trajectory, and radial-field implementations; M4.13 additionally compares analytical, 200,000-particle, and 256/512-cell field resolutions under one pulmonary-card-bound axial advection–diffusion–reaction profile with predeclared statistical, refinement, boundary, and conservation gates |

## 3. Detailed evidence by capability

### 3.1 Transit, geometry, and ownership

- M4.1 provides an independent capillary library, three explicit
  microvascular regions, strict topology and ownership validation, and lossless
  entity and conserved-payload transit.
- M4.2 closes the organ–capillary–organ route with four named ports and
  acknowledged, retryable transfers.
- M4.3 derives area, velocity, and transit from typed SI geometry and volume
  flow rather than accepting mutually inconsistent prescribed values.
- M4.12 turns the earlier non-state-changing outcome probabilities into one
  exclusive, deterministic terminal disposition with an explicit owner.

Primary tests include:

- `An entity and conserved payloads complete an organ capillary organ round trip`;
- `The organ capillary round trip is stable across compatible host steps`;
- `Terminal dispositions close the organ route into exactly one tissue owner`;
  and
- `Rejected terminal ownership remains pending until the correct store accepts it`.

### 3.2 Recruitment, exchange, and local nanodevice behavior

- M4.4 changes aggregate perfused-path groups at exact scheduled boundaries and
  preserves in-flight distance under fixed-flow and equal-path fixed-pressure
  surrogates.
- M4.5 uses dimension-safe amount contracts and an enforced four-destination
  balance; repeated exchange accumulates inventory without reprocessing a
  record.
- M4.6 exposes axial position, exact regional residence, bounded observations,
  and normalized competing outcome probabilities without hidden state change.
- M4.12 performs optional state change only after the separate observation and
  ownership contracts agree.

Primary tests include:

- `Capillary exchange balances blood endothelium interstitium and cell`;
- `Balanced substance exchange completes the organ capillary organ route`;
- `Capillary exchange rejects incompatible or non-conservative profiles`;
- `Recruitment preserves in-flight distance across synchronization step sizes`;
  and
- `Sampled capillary dispositions are conservative deterministic and step independent`.

### 3.3 Stable channel boundary and multi-resolution comparison

M4.8 defines the stable channel request and response. M4.9 verifies an
independent endpoint-particle implementation against its analytical result.
M4.10 resolves bounded paths and compares 8- and 32-step trajectories. M4.11
adds a conservative 128/256-shell radial field. M4.13 adds a separate richer
contract because advection and wall reaction materially change the physical
question and must not silently change the M4.8 semantics.

For the M4.13 shared case, all implementations use one source, receiver,
diffusivity, mean flow, bulk-reaction rate, wall-derived reaction rate,
observation time, and equivalent pulmonary radius. The checked results are:

| Result | Analytical | Particles | 256 cells | 512 cells |
|---|---:|---:|---:|---:|
| receiver fraction | `0.5120593043` | `0.5112500000` | `0.5239759053` | `0.5155977421` |
| relative analytical error | — | statistical gate | `2.3272%` | `0.6910%` |

The particle result is `-0.7241` analytical standard errors from the reference,
the field refinement difference is `1.5990%`, and amount-balance residuals are
below `4e-15`. Every predeclared gate passes.

Primary tests include:

- `The analytical implementation satisfies the free-diffusion impulse response`;
- `Brownian endpoints reproducibly agree with the analytical pulmonary case`;
- `Time-resolved Brownian paths pass analytical and step-refinement gates`;
- `Radial concentration fields conserve amount and pass grid-refinement gates`;
- `Particles and finite volumes pass one advection diffusion and surface-reaction case`;
  and
- `Every axial resolution conserves active reacted and escaped signal`.

## 4. Cross-cutting verification evidence

- local Visual Studio 2026 Debug build: passed;
- complete local CTest suite at the implementation baseline: 188/188 passed;
- local targeted Clang-Tidy analysis: passed without findings;
- deterministic named random streams and fixed profile seeds cover every
  stochastic M4 comparison;
- strict schemas, semantic loaders, incompatible-context tests, and declared
  evidence and limitation fields cover every executable M4 profile;
- bounded particle/path retention and hard finite-volume work limits prevent
  unbounded reference runs; and
- complete cross-platform GitHub matrix:
  [run 33327513888](https://github.com/RegineWendt/MEHLISSA/actions/runs/33327513888),
  passed on Windows/MSVC, Linux/GCC, and Linux/Clang with Clang-Tidy, ASan, and
  UBSan.

## 5. Requirements disposition

The following statuses are intentional and consistent with passing the
Roadmap gate:

| Requirement | Review disposition |
|---|---|
| `CAP-001` | `PART` accepted: the executable equivalent arteriole–capillary–venule route satisfies transit and continuity; anatomical network refinement remains later research |
| `CAP-002` | `PART` accepted: M4.7 supplies an evidence-qualified pulmonary equivalent candidate; jointly measured cohorts, hematocrit, and independent physiological validation remain open |
| `CAP-003` | `PART` accepted: aggregate recruitment and two declared flow surrogates are verified; anatomical sphincter control and feedback are not claimed |
| `CAP-004` | `PART` accepted: the M4 balance and unit contract passes; physiological bidirectional kinetics, metabolism, and qualification remain open |
| `CAP-005` | `PART` accepted: residence, outcome sampling, and terminal ownership pass; reversible interactions and executable tissue response belong to M5 |
| `CAP-006` | `DONE`: one stable channel contract and shared analytical, microscopic, and mesoscopic comparisons are executable |
| `CAP-007` | `RESEARCH` accepted outside the gate: cluster formation, reachability, and multi-hop communication remain planned for later communication work |

No `PART` or `RESEARCH` item above invalidates an explicit Gate M4 statement.
The labels must remain visible so a software gate cannot be mistaken for a
completed physiological model.

## 6. Accepted limitations and post-gate work

M4 does **not** establish any of the following:

- anatomical pulmonary capillary topology or subject-specific geometry;
- a jointly measured flow, capillary-volume, and transit cohort;
- explicit red-blood-cell or hematocrit dynamics;
- validated precapillary sphincter anatomy or physiological feedback;
- physiological bidirectional barrier transport, metabolism, adhesion,
  extravasation, receptor, or receiver kinetics;
- explicit radial wall encounters in the M4.13 particle model;
- a complete tissue or cell response after terminal hand-off;
- an external BiNS2, BNSim2, N3Sim, CFD, or other solver adapter; or
- clinical, patient-specific, or predictive biological validity.

These are retained as M5/M6 work, external-adapter increments, or scientific
qualification packages. Changes that alter an accepted contract or the meaning
of a checked reference profile require a new schema/profile version and an
architecture decision; they must not rewrite the M4 baseline silently.

## 7. Exit decision

M4 passes. The repository contains a conservative organ–capillary route,
dimension-safe balanced exchange, a stable molecular-channel boundary, and
shared analytical, microscopic, and mesoscopic comparisons with explicit
statistical and numerical gates. M5 may build receptor, biomarker, binding,
release, and cell-response behavior on these interfaces.

The exit decision is a technical milestone acceptance. It does not elevate the
equivalent pulmonary geometry or synthetic kinetic profiles to physiological
validation. Downstream work must retain their evidence classes, validity
domains, limitations, and versioned reference results.
