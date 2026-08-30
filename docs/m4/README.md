<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M4 Working Plan and Evidence

## Objective

M4 turns the capillary layer into an independent co-simulation component and
then adds microvascular recruitment, balanced substance exchange, local
nanodevice behavior, and molecular communication incrementally.

The gate remains:

- a nanodevice can leave an organ, traverse a capillary bed, and return;
- substance exchange is mass-conserving and unit-consistent;
- at least one molecular channel is connected through a stable interface; and
- detailed and surrogate models are compared against the same reference cases.

## Implemented increments

### M4.1 - versioned lossless capillary transit baseline

- independent `MEHLISSA::capillary_model` library;
- strict definition schema `1.0.0`;
- ordered arteriole, capillary, and venule regions;
- explicit total and perfused parallel-path counts;
- deterministic entity transit with identity-preserving return;
- lossless population, substance-amount, and volume-flow transit;
- route, time, duplicate-ownership, topology, and recruitment validation;
- synthetic executable card with evidence and limitation metadata.

See [Capillary Transit Bed](CAPILLARY_TRANSIT_BED.md) and
[ADR-0021](../architecture/adr/0021-versioned-capillary-bed-baseline.md).

### M4.2 - conservative organ-capillary round trip

- generic four-port organ-capillary route in the co-simulation library;
- identity-preserving entity transfer from organ to capillary and back;
- lossless population, substance-amount, and volume-flow round trips;
- explicit outstanding-ownership ledgers and completion counters;
- persistent pending queues for rejected departure and return delivery;
- exact model, port, route, and synchronization-time validation;
- deterministic verification at 100 ms, 250 ms, and 500 ms host steps.

See [Organ-Capillary Round Trip](ORGAN_CAPILLARY_ROUND_TRIP.md) and
[ADR-0022](../architecture/adr/0022-organ-capillary-round-trip-coupling.md).

### M4.3 - dimension-safe geometry and volume-flow continuity

- breaking definition schema `2.0.0` with explicit SI length, diameter, and
  network volume flow;
- explicit parallel-vessel count for every serial region;
- single-vessel and total cross-section derived from diameter and count;
- mean velocity derived from total flow divided by total cross-section;
- transit derived from length divided by velocity and rounded only at the
  simulation-clock boundary;
- capillary vessel count bound to the currently perfused path count;
- rejection of invalid geometry and inconsistent volume-flow transfers;
- synthetic v2 card retaining the one-second verification route without
  claiming physiological validity.

See [Capillary Geometry and Continuity](CAPILLARY_GEOMETRY_AND_CONTINUITY.md) and
[ADR-0023](../architecture/adr/0023-dimension-safe-capillary-continuity.md).

### M4.4 - dynamic recruitment and precapillary sphincter groups

- strict recruitment-profile schema `1.0.0` layered over a compatible capillary
  definition;
- aggregate sphincter groups that partition all available parallel paths;
- scheduled rest, activity, and recovery states at exact simulation times;
- dynamic perfused-path count, total capillary area, velocity, and transit;
- explicit fixed-total-flow and simplified fixed-pressure-drop boundary
  conditions;
- distance-based in-flight progress across recruitment events;
- deterministic verification across host steps that cross an event boundary;
- synthetic profile with evidence class and explicit scientific limitations.

See [Capillary Recruitment and Precapillary Sphincter Groups](CAPILLARY_RECRUITMENT_AND_SPHINCTERS.md)
and [ADR-0024](../architecture/adr/0024-dynamic-capillary-recruitment.md).

### M4.5 - balanced blood-to-tissue substance exchange

- strict exchange-profile schema `1.0.0` for a compatible capillary model;
- substance-specific staged blood-to-endothelium, endothelium-to-interstitium,
  and interstitium-to-cell fractions;
- typed input, outgoing blood, and persistent tissue amounts;
- enforced four-term mass-balance invariant;
- explicit pass-through for unmatched substances;
- unchanged population, flow, and lossless no-profile behavior;
- drainable, profile-identified exchange records and cumulative tissue
  inventories;
- complete transformed organ-capillary-organ route with closed ownership.

See [Balanced Capillary Substance Exchange](BALANCED_CAPILLARY_EXCHANGE.md) and
[ADR-0025](../architecture/adr/0025-balanced-capillary-exchange.md).

### M4.6 - nanodevice residence and interaction observations

- current entity position with region, axial distance, and axial fraction;
- exact arteriole, capillary, and venule residence accumulation;
- strict evidence-scoped entity-observation profile schema `1.0.0`;
- residence-sensitive competing pass-through, retention, adhesion, and
  extravasation likelihoods;
- normalized results without changing entity identity, route, or ownership;
- residence-only observations for unmatched entity types; and
- bounded drainable completion records with an explicit drop counter.

See [Capillary Nanodevice Residence and Interaction Observations](CAPILLARY_ENTITY_OBSERVATION.md)
and [ADR-0026](../architecture/adr/0026-non-state-changing-capillary-entity-observation.md).

### M4.7 - evidence-qualified pulmonary capillary candidate

- healthy-adult resting recumbent pulmonary reference state;
- strict schema `3.0.0` with parameter-level evidence roles, uncertainty,
  derivations, source IDs, and executable closure checks;
- separate functional perfused blood volume and morphometric lumen capacity;
- equivalent diameter derived from human morphometry without claiming tubular
  alveolar-sheet anatomy;
- 0.859 s capillary transit derived from 85.9 mL functional volume and the M3
  nominal 6 L/min reference flow;
- explicit non-anatomical path-count and representative-length semantics;
- numerical arteriole and venule transitions kept separate from qualified
  capillary evidence; and
- regional blood volume exposed as a derived runtime metric.

See [Pulmonary Capillary Reference Candidate](PULMONARY_CAPILLARY_QUALIFICATION.md)
and [ADR-0027](../architecture/adr/0027-evidence-qualified-equivalent-pulmonary-capillary-card.md).

### M4.8 - interchangeable analytical molecular channel

- stable typed request/response boundary independent of channel implementation;
- amount, concentration, length, receiver volume, and observation time in SI;
- strict molecular-channel profile schema `1.0.0` with evidence and limitations;
- analytical point-source free-diffusion implementation with optional
  first-order degradation and a guarded passive small-receiver approximation;
- pulmonary-bound reference request derived from the M4.7 equivalent diameter
  and checked against its capillary residence ceiling;
- factory construction through the abstract channel interface; and
- deterministic impulse-response regression plus invalid-context and
  invalid-receiver tests.

See [Interchangeable Molecular-Channel Interface](MOLECULAR_CHANNEL_INTERFACE.md)
and [ADR-0028](../architecture/adr/0028-interchangeable-molecular-channel-contract.md).

### M4.9 - deterministic Brownian particle comparison

- independent particle-surrogate implementation behind the unchanged M4.8
  channel request and response;
- direct three-dimensional Brownian endpoint sampling with coordinate variance
  `2Dt` and optional first-order survival;
- explicit experiment seed, named random stream, uniform conversion, and
  Box-Muller transform;
- strict particle-profile schema `1.0.0` bound to the analytical profile;
- two million samples for the shared pulmonary-bound synthetic tracer case;
- diagnostics for sample count, receiver observations, and Monte Carlo standard
  error; and
- predeclared minimum-count, standardized-error, and relative-error gates.

See [Brownian Particle-Channel Comparison](BROWNIAN_PARTICLE_CHANNEL_COMPARISON.md)
and [ADR-0029](../architecture/adr/0029-deterministic-brownian-particle-comparison.md).

### M4.10 - trajectory-resolving Brownian channel

- fixed-step three-dimensional Brownian paths behind the unchanged channel
  request and response;
- strict trajectory-profile schema `1.0.0` bound to the analytical profile;
- deterministic, step-count-qualified random streams and exact per-step
  first-order survival;
- unbounded and reflecting-box boundary modes with receiver containment checks;
- bounded trace retention with an explicit dropped-point counter;
- diagnostics for receiver observations, survival, reflections, and mean
  squared displacement;
- 8-step and 32-step verification against the shared pulmonary-bound synthetic
  reference; and
- predeclared count, analytical, refinement, and `6Dt` displacement gates.

See [Trajectory-Resolving Brownian Channel](TRAJECTORY_BROWNIAN_CHANNEL.md) and
[ADR-0030](../architecture/adr/0030-trajectory-resolving-brownian-channel.md).

### M4.11 - conservative radial concentration field

- deterministic mesoscopic population field behind the unchanged channel
  request and response;
- strict radial finite-volume profile schema `1.0.0` bound to the analytical
  profile;
- conservative fluxes between spherical shells, symmetry at the origin, and an
  absorbing far boundary with separately accounted escape;
- exact split first-order degradation and an enforced active-degraded-escaped
  amount balance;
- CFL-derived time steps plus hard cell, step, and cell-step work bounds;
- complete bounded final field and explicit receiver-interpolation diagnostics;
- 128-to-256-shell verification against the shared pulmonary-bound analytical
  case; and
- predeclared analytical, refinement, conservation, and boundary-loss gates.

See [Radial Finite-Volume Molecular Channel](RADIAL_FINITE_VOLUME_CHANNEL.md) and
[ADR-0031](../architecture/adr/0031-radial-finite-volume-molecular-channel.md).

### M4.12 - conservative terminal entity ownership

- versioned terminal-disposition transfer and explicit source/sink interfaces;
- strict state-changing profile bound to the compatible M4.6 observation
  profile;
- one deterministic named-stream draw for each completed matched entity;
- exclusive pass-through, retained, adhered, or extravasated result;
- explicit target owner model and distinct target compartment per terminal
  result;
- pending retry after rejected delivery without closing the original ledger;
- persistent terminal store with duplicate-ID and target validation; and
- population closure plus identical decisions at 100, 250, and 500 ms host
  steps.

See [Conservative Terminal Entity Ownership](TERMINAL_ENTITY_OWNERSHIP.md) and
[ADR-0032](../architecture/adr/0032-conservative-terminal-entity-ownership.md).

### M4.13 - shared axial advection-diffusion-reaction case

- separate versioned contract for physics that deliberately exceeds the M4.8
  free-diffusion request;
- strict binding to the M4.7 equivalent pulmonary capillary radius, path extent,
  and resting mean flow speed;
- one shared axial source, receiver, diffusivity, bulk reaction, and
  surface-to-volume wall reaction;
- exact advected-Gaussian analytical reference with competing reaction balance;
- deterministic 200,000-sample microscopic endpoint-particle evaluation;
- conservative 256- and 512-cell mesoscopic finite-volume fields with explicit
  boundary escape and complete final output;
- predeclared statistical, analytical, refinement, conservation, and boundary
  gates; and
- synthetic kinetic parameters and explicit cross-section-averaged limitations.

See [Shared Axial Advection-Diffusion-Reaction Case](SHARED_AXIAL_ADVECTION_REACTION_CASE.md)
and [ADR-0033](../architecture/adr/0033-shared-axial-advection-reaction-case.md).

## Planned sequence

1. perform the formal M4 gate review; and
2. record any remaining M4 work as explicit post-gate qualification or external-
   adapter increments rather than silently extending the milestone.

## Scientific status

M4.1 through M4.13 are software verification, not physiological validation. The
dissertation provides the layer structure, continuity requirement, recruitment
concept, and communication questions. The initial cards, recruitment profile,
exchange fractions, and interaction rates remain synthetic. M4.7 adds the first
literature-parameterized capillary candidate, but its evidence comes from small
different cohorts and an equivalent rather than anatomical geometry. M4.8 adds
evidence-based channel architecture but deliberately synthetic molecular
parameters. M4.9 independently reproduces that synthetic case within
predeclared Monte Carlo gates; M4.10 adds bounded explicit paths, an 8-to-32-step
verification, and reflecting-box software support without claiming that the box
is pulmonary anatomy. M4.11 adds a conservative spherical field, explicit
boundary-loss accounting, and 128-to-256-shell refinement, but no anatomical
field geometry. M4.12 conserves identity across sampled terminal hand-offs, but
its rates and target compartments are synthetic. M4.13 adds a shared
analytical/particle/field case with pulmonary-card-bound equivalent radius and
flow, directed advection, bulk reaction, and a cylindrical surface-derived
sink. Its kinetic parameters are synthetic, its geometry remains equivalent,
and its particles sample endpoints rather than explicit wall encounters. These
are implementation comparisons, not biological validation.
Jointly measured flow and capillary volume, sphincter behavior,
hematocrit, kinetic exchange parameters, and independent physiological
comparison remain open.
