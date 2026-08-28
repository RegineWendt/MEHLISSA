<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Lung model data

This directory contains executable, versioned lung-model definitions intended
as research inputs. Each JSON file has a matching SPDX sidecar and records its
population, physiological state, evidence class, source roles, uncertainty,
derivations, and limitations.

`healthy-adult-rest-supine-0d-v1.json` is the first literature-parameterized
reference candidate. It represents composite healthy-adult evidence at rest in
the supine position; it is not a patient record or a clinical model. See the
[pulmonary 0D model card](../../docs/m3/PULMONARY_0D_REFERENCE.md).
Its separate independent observations and executable comparison live under
`data/validation/`; see the
[validation report](../../docs/m3/PULMONARY_0D_INDEPENDENT_VALIDATION.md).

`healthy-adult-rest-exercise-0d-v2.json` retains the resting v1 values and adds
a bounded flow-dependent PVR/compliance rule calibrated from a healthy cohort
that is disjoint from the Bentley validation cohort. See the
[flow-adaptation model card](../../docs/m3/PULMONARY_0D_FLOW_ADAPTATION.md).

`healthy-adult-rest-exercise-age-0d-v3.json` adds the Kane-calibrated age
bands. `healthy-adult-rest-exercise-age-invasive-0d-v4.json` is its immutable
successor for ages 24–85 and replaces only the young PVR level with an invasive
aggregate calibration. See the
[young-adult resistance model card](../../docs/m3/PULMONARY_0D_YOUNG_RESISTANCE.md).

`healthy-adult-pressure-distensible-age-0d-v5.json` is a structural successor
candidate. It retains v4's resting and age calibration but replaces the
empirical flow law with a Linehan pressure-distensible vessel relationship.
The two laws cannot be enabled together. Its independent result and limits are
documented in the
[pressure-distensibility model card](../../docs/m3/PULMONARY_0D_PRESSURE_DISTENSIBILITY.md).
