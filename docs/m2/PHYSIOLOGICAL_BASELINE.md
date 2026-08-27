<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Physiological Basis of the M2 Body Models

**As of:** 27 August 2026

**Purpose:** Separation of historical reproduction, physiological reference,
and later validation

## Two profiles with different claims

| Profile | Status | Claim |
|---|---|---|
| `bvs95-dissertation-rest-v1` | executable and validated | historical 1995 topology and dissertation transitions as a reproducible transport baseline |
| `reference-adult-female-rest-supine-v1` | defined at domain level, not yet mapped to all 95 segments | normative reference for a healthy adult woman at rest and supine |

The first profile is a software and publication comparison. The second is
intended to support later physiological assessment. Results from the first must
not be described as validation of the second.

## Executable dissertation profile

The converter reproducibly generates
`data/body-models/bvs95-dissertation-rest-v1.json` from the authorized CSV
sources. Its rules are:

- all 95 IDs, types, and coordinates remain bijectively mapped;
- legacy centimetres are converted to metres with factor `0.01`;
- edges are derived once from the legacy coordinate rule and stored explicitly afterward;
- all 23 existing branching ratios remain unchanged;
- vessel 9 is added with supported values of `0.2875` left and `0.7125` right;
- vessel 2 is normalized to `0.0001 m³/s`, or 6.0 L/min;
- segment flows are propagated along all branches and added at merges;
- historical mean velocities are used in SI;
- equivalent area, diameter, and volume follow from `A = Q/v`, `d = sqrt(4A/pi)`, and `V = A*L`.

The 6.0 L/min is a transparent normalization close to typical resting values,
not a measurement of the historical reference person. Derived diameters are
transport parameters, not anatomical vessel radii. Unquantified uncertainty is
left empty in the data set instead of being assigned invented, falsely precise
percentages.

## Vessel 9 and validity scope

Legacy vessel 9 is the head organ bed. Its successors are:

| Successor | Anatomy | Probability |
|---:|---|---:|
| 81 | left internal jugular vein | 0.2875 |
| 83 | right internal jugular vein | 0.7125 |

The values derive from cohort means of 161 and 399 ml/min measured by
phase-contrast MRI in healthy young adults. In the model they apply only at
rest and supine. Interindividual side and posture differences will later be
modeled as parameter sets or through additional vertebral and extrajugular
drainage paths.

Planned sensitivity cases are `0.5/0.5`, pronounced right dominance, and left
dominance. They do not replace the reference value.

## Normative female reference profile

The historical BVS geometry was derived from a woman 1.72 m tall and weighing
69 kg. A female reference therefore represents the smallest discontinuity with
the existing data. Initial values for
`reference-adult-female-rest-supine-v1` are:

| Quantity | Value | Evidence |
|---|---:|---|
| cardiac output | 5.9 L/min | ICRP 89, healthy adult woman, rest/supine |
| total blood volume | 3.9 L | ICRP 89 female reference value |
| direct organ shares | ICRP 89 table | literature values; must be mapped to disjoint terminal beds before use |
| head side drainage | 0.2875/0.7125 | Stoquart-ElSankari et al. 2009; state- and population-specific |

Regional BVS compartments such as shoulder, arm, pelvis, and leg do not map
directly to tissue-based ICRP categories. Until a traceable aggregation rule
exists, no purportedly physiological numbers are written into an executable
graph for them. Dissertation shares remain in the reproduction profile; a
later mapping will use vascular territories from a whole-body atlas or an
appropriate MRA data set.

## Derivation and validation rule

For a complete stationary profile:

```text
Q_organ = Q_heart * f_organ
Q_segment = sum of all downstream terminal Q_organ
p(i -> j) = Q_j / sum(Q_all_successors)
```

Portal flow and organ values that already contain inflow from other organs
must be modeled explicitly. They must not be summed twice as terminal shares.

A profile is called physiologically validated only when:

1. flow and mass are conserved at every branch and merge;
2. all organ shares are disjoint or their overlap is modeled explicitly;
3. calibration and validation data are separate;
4. cardiac output, organ flows, transit times, and selected vessel values lie within predefined tolerances;
5. sensitivities for cardiac output, organ shares, velocity, and jugular side ratio are reported.

## Sources and next data level

- Regine Wendt, *Einsatz von Nanotechnologien in der Präzisionsmedizin*, 2024.
- Stoquart-ElSankari et al., DOI `10.1038/jcbfm.2009.29`.
- ICRP Publication 89, 2002.
- For later anatomical radii and additional paths: ADAVN and an appropriate closed 1D/0D whole-body model. Such third-party data is adopted only after its own license, version, and provenance review.

The model is a research reference, not a patient-specific model or medical
device.
