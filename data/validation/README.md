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
