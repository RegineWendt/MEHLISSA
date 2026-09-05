<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Scientific qualification

This directory contains prospective, bounded qualification plans that build on
the milestone evidence without rewriting it after results are known.

| Protocol | Status | Purpose |
|---|---|---|
| [PCQ-1 pulmonary and capillary qualification](PULMONARY_CAPILLARY_QUALIFICATION_PROTOCOL.md) | design v0.1.0 established before new validation outcomes | qualify participant-level pulmonary hemodynamics, regional perfusion, capillary volume and transit, and their joint coherence |

The machine-readable authority for PCQ-1 is
`data/qualification/pulmonary-capillary-qualification-protocol-v1.json`. Its
schema, frozen-asset hashes, endpoint hierarchy, negative controls, and claim
boundary are checked in continuous integration. A checked protocol demonstrates
prospective study discipline; it is not itself physiological validation.

Each later protocol must preserve the separation between:

- software and numerical verification;
- literature parameterization;
- calibration;
- source-disjoint validation; and
- clinical evidence, which is outside the current MEHLISSA claim.
