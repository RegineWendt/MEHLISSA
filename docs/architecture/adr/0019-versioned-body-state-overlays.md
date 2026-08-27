<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0019: Versioned Body States as Flow-Conserving Overlays

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M2.6; `BODY-004`, `BODY-006`, `BODY-008`, `DATA-001`

## Context

Rest, exercise, and posture change cardiac output, regional perfusion, and
transit times. Copying complete graphs would duplicate geometry, topology, and
sources; freely scaling individual vessels would violate flow conservation. At
the same time, the schematic 1995 model lacks anatomical paths that are
essential for some states.

## Decision

1. A body state is a schema-validated overlay on exactly one model ID and version.
2. The profile changes cardiac output and optionally existing branching ratios, but never topology.
3. Complete stationary flow is recomputed from all transitions and a cardiac-output anchor as a linear system of equations.
4. Cross sections and geometry remain fixed; mean velocities follow from `v = Q/A`.
5. The derived graph receives a new model ID, profile version, validity description, and all profile sources.
6. After application, the complete M2.1 validator must pass again.
7. Missing anatomical paths are not simulated through transition ratios. Such changes require a new body model.
8. Profiles explicitly distinguish historical reproduction, literature-based sensitivity, and physiological validation.

## Consequences

Positive:

- States can be exchanged and compared without rebuilding.
- Geometry and state assumptions remain versioned separately.
- Branch changes cannot locally violate flow conservation.
- Sources and model limitations travel with the derived graph.
- The same method works with alternative compatible graphs.

Limitations:

- The stationary overlay contains no time-dependent state transition.
- Fixed cross sections represent vascular compliance and vasodilation only indirectly through changed mean velocity.
- The exercise profile does not yet include regional redistribution.
- Without a vertebral venous plexus, the head-up-tilt profile cannot represent complete cerebral orthostatic physiology.

## Alternatives

- **Copy the complete graph for each state:** rejected because of data duplication and drift.
- **Arbitrary flow factors per segment:** rejected because they can violate local and global conservation.
- **Map missing paths as probabilities onto existing vessels:** rejected because this would conceal absent anatomy.
- **Keep only the historical resting graph:** rejected because state transitions are a central objective of the body layer.

## Verification

- `data/schemas/body-state-profile/1.0.0.schema.json`
- `data/body-states/`
- `docs/m2/BODY_STATE_PROFILES.md`
- `body_state_profile_tests`
- `mehlissa_cli_applies_body_state_without_rebuild`
