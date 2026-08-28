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
