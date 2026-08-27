<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2.6 – Interchangeable Body-State Profiles

**Status:** complete with documented physiological model limitations

**Profile schema:** `data/schemas/body-state-profile/1.0.0.schema.json`

## Purpose

M2.6 separates stable vascular topology and geometry from a hemodynamic state.
The same compiled program can load a compatible vascular graph and a versioned
state profile, calculate a new flow-conserving graph, and fully validate it
again.

A profile contains:

- a stable ID, version, and title;
- the exact compatible model ID and version;
- population, state, and validity description;
- an anchor segment and cardiac-output factor;
- optional topology-preserving transition ratios;
- sources and known limitations.

## Flow-conserving application

Freely scaling individual vessels is prohibited because it would violate
branch and merge balances. After profile changes, M2.6 solves the stationary
system for all segments:

```text
Q_j = sum_i(Q_i * p(i -> j))
Q_anchor = Q_anchor,baseline * profile factor
```

The solver uses partial pivoting. The solution must be finite and strictly
positive. Geometry remains unchanged; mean velocity is recomputed from
`v = Q/A`. All M2.1 vascular-graph invariants then apply again, including flow
conservation and consistency between transition probabilities and successor
flows.

A transition override may change probabilities but may neither add nor remove
successors. A new anatomical path requires a new body model and must not be
hidden in a state profile.

## Included profiles

| Profile | Factor | Result at vessel 2 | Evidence role |
|---|---:|---:|---|
| `bvs95-rest-supine-v1` | 1.0 | 6.0 L/min | historical identity baseline |
| `bvs95-supine-cycle-exercise-1.9x-v1` | 1.9 | 11.4 L/min | literature-based global exercise sensitivity |
| `bvs95-head-up-tilt-70deg-v1` | 0.783333 | 4.7 L/min | literature-based global orthostatic sensitivity |

### Rest/supine

The identity profile reproduces the M2.2 graph. The 6.0 L/min remains a
transparent normalization of the historical dissertation profile, not a
measurement of a reference individual.

### Supine cycle exercise

Wong et al. studied 24 healthy adults using phase-contrast MRI. During cycle
exercise, cardiac output increased 1.9-fold. Absolute upper-body flow increased
about 1.5-fold and lower-body flow 2.3-fold; the lower-body share rose from 71%
to 84%.

M2.6 adopts only the global factor of 1.9. The asymmetric BVS95 branches are not
a validated map of upper and lower vascular territories. Enforcing regional
redistribution on them would invent anatomy. The profile is an exercise
sensitivity, not a complete exercise-physiology model.

### 70° head-up tilt

Harms et al. reported a median of 6.0 L/min supine and 4.7 L/min during
sustained 70° head-up tilt in nine healthy adults. This gives the factor
`4.7/6.0 = 0.783333…`.

Gisolf et al. demonstrated an important structural limitation: when humans are
upright, the internal jugular veins largely collapse and the vertebral venous
plexus becomes an important cerebral drainage path. This path is absent from
the 1995 graph. The profile therefore changes only global flow, not a jugular
ratio. A genuine orthostatic variant requires an extended vascular graph.

## Reproduction

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe apply-body-state `
  --model data/body-models/bvs95-dissertation-rest-v1.json `
  --profile data/body-states/bvs95-supine-cycle-exercise-1.9x-v1.json `
  --output build/bvs95-exercise.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json `
  --profile-schema data/schemas/body-state-profile/1.0.0.schema.json
```

The same command processes the rest or head-up-tilt profile without recompiling
the program.

## Automated verification

`body_state_profile_tests` verifies:

- schema and semantics of all three profiles;
- model/version compatibility;
- unchanged geometry;
- complete recomputation of flow and velocity;
- flow conservation throughout the 95-segment graph;
- genuine branch redistribution in the synthetic reference graph;
- renewed validity of the generated vascular graph;
- CLI application without rebuilding.

## Sources

- Wong DT et al., *Changes in systemic and pulmonary blood flow distribution
  in normal adult volunteers in response to posture and exercise*, J Physiol
  Sci 64 (2014), DOI `10.1007/s12576-013-0298-z`.
- Harms MPM et al., *Postural effects on cardiac output and mixed venous oxygen
  saturation in humans*, Exp Physiol 88 (2003), DOI `10.1113/eph8802580`.
- Gisolf J et al., *Human cerebral venous outflow pathway depends on posture
  and central venous pressure*, J Physiol 560 (2004), DOI
  `10.1113/jphysiol.2004.070409`.

## Remaining work

Despite completion of M2.6, `BODY-008` remains partially satisfied. A
physiologically complete exercise profile requires robust vascular territories
and regional resistances; an orthostatic profile requires the vertebral venous
plexus, pressure, compliance, and baroreflex dynamics. These extensions belong
in M3/M4 or a new anatomical body model and will not be replaced with further
scaling factors.
