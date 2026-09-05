<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Scientific qualification

This directory contains prospective, bounded qualification plans that build on
the milestone evidence without rewriting it after results are known.

| Protocol | Status | Purpose |
|---|---|---|
| [PCQ-1 pulmonary and capillary qualification](PULMONARY_CAPILLARY_QUALIFICATION_PROTOCOL.md) | design v0.1.0 plus [PCQ-1.2 evidence-source screen](PCQ1_EVIDENCE_SOURCE_SCREEN.md); no participant outcomes acquired | qualify participant-level pulmonary hemodynamics, regional perfusion, capillary volume and transit, and their joint coherence |

The machine-readable design authority for PCQ-1 is
`data/qualification/pulmonary-capillary-qualification-protocol-v1.json`. Its
schema, frozen-asset hashes, endpoint hierarchy, negative controls, and claim
boundary are checked in continuous integration. A checked protocol demonstrates
prospective study discipline; it is not itself physiological validation.

The PCQ-1.2 source-selection authority is
`data/qualification/pulmonary-capillary-evidence-candidate-register-v1.json`.
Its separate schema and semantic checker retain thirteen candidates, rankings,
access and rights boundaries, measurement jointness, source independence,
public outcome exposure, rejection reasons, and unsent next actions. The
[D'Souza request draft](DSOUZA_2025_DATA_REQUEST.md) and the existing
[University of Arizona request](../m3/UA_ICPET_DATA_REQUEST.md) are templates;
they are not records of external contact or granted data access.

Each later protocol must preserve the separation between:

- software and numerical verification;
- literature parameterization;
- calibration;
- source-disjoint validation; and
- clinical evidence, which is outside the current MEHLISSA claim.
