<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0028: Interchangeable molecular-channel contract

## Status

Accepted for M4.8.

## Context

Gate M4 requires at least one molecular channel behind a stable interface and
a comparison between detailed and surrogate models. Putting one analytical
formula directly into the pulmonary capillary component would satisfy neither
interchangeability nor scientific traceability. Adopting an external simulator
as the interface would instead expose tool-specific particle state and make
MEHLISSA scenarios depend on that tool's availability and license.

## Decision

Introduce a model-neutral `MolecularChannel` request/response contract with
typed amount, length, volume, concentration, and time. Implement a first
`analytical_free_diffusion_3d` adapter and a strict versioned profile that:

- keeps channel parameters separate from capillary physiology;
- binds a reference request to one named capillary definition and region;
- derives local separation and receiver size from explicit fractions;
- evaluates at the analytical peak and checks it lies inside regional
  residence time;
- records evidence class, sources, and limitations; and
- constructs implementations through a factory returning the abstract
  channel interface.

The M4.7 pulmonary equivalent diameter and transit ceiling provide the first
context. All molecular-signal parameters remain a software-test surrogate.

## Consequences

Analytical, particle-based, external, and distributional models can now be
compared using the same inputs and outputs. Organ geometry and molecular
propagation assumptions remain independently replaceable. Invalid context and
small-receiver assumptions fail before a result is used.

M4.8 does not model advection, barriers, reactions, stochastic molecule counts,
modulation, or detection. It also does not complete the detailed-versus-
surrogate comparison. The interface may need a future version for sampled
trajectories or symbol sequences, but those additions must not weaken the v1
single-impulse semantics.

## Alternatives considered

- **Embed diffusion in `CapillaryBed`:** rejected because organ transport and
  channel physics would no longer be independently replaceable.
- **Use JSON maps as the runtime API:** rejected because units and required
  provenance would become implicit.
- **Adopt N3Sim's internal objects as the contract:** rejected because this
  couples MEHLISSA to one external implementation and license boundary.
- **Claim a pulmonary oxygen channel immediately:** rejected because the
  current point-source free-diffusion assumptions do not represent alveolar
  barrier transport and the required parameters are not jointly qualified.

## Affected requirements and gates

- CAP-006 becomes partially executable: one analytical channel is connected
  through a stable interface.
- The third Gate M4 statement is satisfied at the software-contract level.
- The shared-reference comparison with an independent detailed or surrogate
  implementation remains open.
