<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0016: Release of the 1995 Data and Separation of Reference Profiles

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M2.2; `BODY-001`, `BODY-002`, `BODY-005`, `BODY-006`, `DATA-001`

## Context

ADR-0007 and ADR-0014 left migration of the 1995 data set open because explicit
data authorization, the missing transition from vessel 9, and absolute
hemodynamic parameterization were absent. Project leadership has now confirmed
that `95_vasculature.csv`, `95_transitions.csv`, and `95_fingerprints.csv` may
be used and relicensed.

A single file cannot both reproduce historical results and pose as an
anatomical-physiological reference. The legacy geometry is schematic; its
transitions represent the resting perfusion calibrated in the dissertation,
while many absolute vessel parameters are missing.

## Decision

1. The three authorized source files and Next data generated from them are licensed under `CC-BY-4.0`. The manifest, checksums, and sidecars document authorization and transformation.
2. The source files remain byte-identical. The converter does not modify them and maps every legacy ID bijectively to `bvs95-NNN`.
3. The executable profile `bvs95-dissertation-rest-v1` serves reproduction and regression. It adopts topology and transitions, converts coordinates from centimetres to metres, and explicitly normalizes the circulation to 6.0 L/min.
4. The historical velocity classes of 10 cm/s for arteries, 3.7 cm/s for veins, and 1 cm/s for organ beds—with 5 cm/s for the two heart segments—are retained for transit-time comparisons. Cross section and diameter are derived from `A = Q/v` as equivalent transport parameters and are not described as anatomical measurements.
5. In the canonical migration, vessel 9 has successors 81 on the left and 83 on the right. The rest/supine value is derived from published cohort means of 161 ml/min left and 399 ml/min right: `0.2875/0.7125`.
6. The legacy run with implicit `1/0` remains traceable only through the unchanged source as `legacy-as-run`. Because of the zero probability, it is not emitted as a valid Next vascular graph.
7. A separate future profile, `reference-adult-female-rest-supine-v1`, uses normative physiological values. Its starting points are 5.9 L/min cardiac output and 3.9 L blood volume from ICRP 89. It becomes executable only when organ and regional flows have been mapped to the graph without double counting and independently reviewed.
8. Every parameter carries an evidence class, source, validity scope, and—where quantifiable—uncertainty. A calibrated value is not independent validation evidence.

## Consequences

Positive:

- M2.2 has a reproducible, schema- and semantics-validated 95-segment data set.
- The correction of vessel 9 is supported and visible as a state-dependent assumption.
- Historical regression remains possible without presenting schematic parameters as clinical physiology.
- The converter strictly checks the documented legacy structure and generates flow, geometry, and transition invariants deterministically.

Negative and limitations:

- Despite consistent SI quantities, the M2.2 data set is not an anatomically validated whole-body circulation.
- The jugular mean represents neither individual left/right ratios nor altered extrajugular drainage when upright.
- A complete physiological profile requires further mapping and domain validation; this work belongs to M2.6 and later validation increments.

## Sources

- Regine Wendt, *Einsatz von Nanotechnologien in der Präzisionsmedizin*, 2024,
  Section 4.3.1 and Table A.1.
- Stoquart-ElSankari et al., *A phase-contrast MRI study of physiologic
  cerebral venous flow*, J Cereb Blood Flow Metab 29 (2009),
  DOI `10.1038/jcbfm.2009.29`.
- ICRP Publication 89, *Basic Anatomical and Physiological Data for Use in
  Radiological Protection: Reference Values*, 2002.

## Alternatives

- **Set vessel 9 to 50/50:** simple, but unrelated to the identified cohort mean; suitable only as a sensitivity variant.
- **Use arterial head inflow as the venous side ratio:** rejected because a mixed organ bed and collateral venous drainage prevent it from representing a valid jugular ratio.
- **Describe the legacy graph directly as physiological:** rejected because its geometry, velocities, and regional mapping are not sufficiently validated.
