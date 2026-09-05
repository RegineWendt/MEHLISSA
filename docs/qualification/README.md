<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Scientific qualification

This directory contains prospective, bounded qualification plans that build on
the milestone evidence without rewriting it after results are known.

| Protocol | Status | Purpose |
|---|---|---|
| [PCQ-1 pulmonary and capillary qualification](PULMONARY_CAPILLARY_QUALIFICATION_PROTOCOL.md) | design v0.1.0, [PCQ-1.2 source screen](PCQ1_EVIDENCE_SOURCE_SCREEN.md), [PCQ-1.3 amendment](PCQ1_PRE_OUTCOME_AMENDMENT.md) v0.2.0, [PCQ-1.4 rights-aware ingress](PCQ1_DATA_INGRESS.md), [PCQ-1.5 uncertainty/identifiability analysis](PCQ1_UNCERTAINTY_IDENTIFIABILITY.md), and [PCQ-1.5a repository-first data audit](PCQ1_REPOSITORY_FIRST_DATA_AUDIT.md); no participant outcomes acquired | qualify participant-level pulmonary hemodynamics, regional perfusion, capillary volume and transit, and their joint coherence |

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

The PCQ-1.3 analysis authority is
`data/qualification/pulmonary-capillary-preoutcome-amendment-v1.json`. It
freezes eight observation models, source activation guards, sample and
precision floors, six primary numeric gates, missingness and statistical
rules, and explicit inconclusive or blocked states. In particular, the current
capillary residence time cannot be compared directly with Lassen's pulmonary-
trunk-to-left-atrium transit measurement. A machine-checked amendment is
prospective analysis discipline, not a physiological qualification result.

The PCQ-1.4 ingress authority is
`data/qualification/pulmonary-capillary-data-ingress-policy-v1.json`. Its
manifest-first checker requires approved rights, privacy, confirmed cohort
independence, and an explicit outside-repository quarantine path before opening
measured data. Four strict family schemas and CC0 synthetic fixtures verify the
normalized adapters without using outcomes. Command output contains metadata,
never raw observations; whole-pulmonary transit remains blocked.

The PCQ-1.5 uncertainty authority is
`data/qualification/pulmonary-capillary-uncertainty-plan-v1.json`. It covers
all six protocol uncertainty classes and all nine endpoints, registers
distribution and correlation availability, compares seven pulmonary structures
without winner selection, verifies seven convergent local sensitivities, and
tests nine identifiability designs. Missing joint distributions block global
variance attribution, and the whole-pulmonary transit observation model stays
blocked. These are prospective design and software results, not participant-
level qualification evidence.

The PCQ-1.5a repository-availability authority is
`data/qualification/pulmonary-capillary-repository-audit-v1.json`. It binds the
unchanged PCQ-1.2 source register and records exact target-study findings, five
non-equivalent repository alternatives, access/licence constraints, an
outcome-blind file boundary, and the still-unsent contact queue. It found no
drop-in primary dataset and changes no frozen source role or numeric rule.

Each later protocol must preserve the separation between:

- software and numerical verification;
- literature parameterization;
- calibration;
- source-disjoint validation; and
- clinical evidence, which is outside the current MEHLISSA claim.
