<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Validation data

This directory contains versioned observations reserved for validation rather
than model calibration. Validation loaders reject reuse of model evidence
sources where an executable independence check exists.

The first case is the
[healthy-adult pulmonary 0D aggregate validation](../../docs/m3/PULMONARY_0D_INDEPENDENT_VALIDATION.md).
It stores only attributed aggregate facts, not source articles or subject
records. Read each case's limitations and source terms before reuse.

Version v1 evaluates the immutable resting model. Version v2 repeats the same
observations after calibrating bounded flow adaptation on the disjoint
Claessen cohort. Bentley remains validation-only in both cases.

M3.10 adds a separate schema for subject-level multipoint measurements at
`data/schemas/pulmonary-zero-dimensional-multipoint-validation/1.0.0.schema.json`.
No clinical subject file is checked in: the preferred UA iCPET control data
require author approval and explicit reuse terms. The only current multipoint
record is a clearly marked synthetic software fixture under `tests/data`; the
default evidence loader rejects it. See the
[subject-level validation plan](../../docs/m3/PULMONARY_0D_SUBJECT_MULTIPOINT_VALIDATION.md).

M3.11 adds an independent published-population route at
`healthy-population-multipoint-v1.json`, governed by
`data/schemas/pulmonary-zero-dimensional-population-multipoint-validation/1.0.0.schema.json`.
It transcribes attributed Kovacs and Wolsk aggregate series, normalizes the
reported units to SI during loading, and evaluates the immutable v2 model
without fitting. Its 10/18 stage agreement is a qualified partial result with
age-stratified failures, not subject-level evidence. See the
[population multipoint report](../../docs/m3/PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md).

M3.12 preserves that v1 case as the immutable v2-model result and adds
`healthy-population-multipoint-v2.json` for the age-conditioned v3 model. The
Wolsk series carry predeclared representative ages that select age bands
calibrated independently from Kane et al.; Kovacs and Wolsk remain
validation-only. Agreement improves to 14/18 stages without refitting. See the
[age-conditioning model card](../../docs/m3/PULMONARY_0D_AGE_CONDITIONING.md).
The v2 validation case uses population schema 1.1.0; the immutable M3.11 case
continues to validate against schema 1.0.0.

M3.13 adds `healthy-population-multipoint-v3.json` for the v4 invasive
young-resistance candidate. It contains only the three disjoint Wolsk age
series (15 stages). Kovacs 2009 is omitted because the v4 Kovacs 2012
calibration reanalysed part of the same historical literature corpus. All
15 Wolsk stages agree without refitting; see the
[young-adult resistance model card](../../docs/m3/PULMONARY_0D_YOUNG_RESISTANCE.md).

M3.14 adds `healthy-pressure-distensible-population-v1.json`, which binds the
same 15 Wolsk stages to the structural v5 candidate under a pre-locked,
no-fitting protocol. Agreement is 5/5, 5/5, and 1/5 by increasing age stratum,
or 11/15 overall. The older failures are retained as a model limitation; see
the [pressure-distensibility model card](../../docs/m3/PULMONARY_0D_PRESSURE_DISTENSIBILITY.md).

M3.15 adds `healthy-pressure-distensible-population-v2.json`. It copies the v5
Wolsk stages and protocol unchanged and binds them to the age-conditioned v6
candidate. The older-stratum RMSE improves from 5.411 to 4.603 mmHg, but stage
agreement remains 1/5 older and 11/15 overall. See the
[age-distensibility model card](../../docs/m3/PULMONARY_0D_AGE_DISTENSIBILITY.md).
