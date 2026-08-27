<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Versioned Lung Model Definitions

## Executable model card

Schema `lung-model-definition/1.0.0` binds model selection and parameters to
the evidence needed to interpret them. A definition contains:

- stable definition and model identities;
- the selected coarse or regional implementation;
- named entry, exit, and body-return ports;
- SI transit durations;
- population and physiological-state validity;
- an evidence class, sources, licenses, roles, and limitations; and
- optional external-data identity, checksum, format, coordinate system, units,
  and an ordered transformation record.

The loader validates JSON structure and semantic consistency before producing a
typed `LungModelConfig`. The factory then performs the concrete model checks.
Variant timing structures cannot be mixed.

## Checked-in definitions

Two examples make the existing software regressions externally reproducible:

- `lung-compartment-contract-v1.json`; and
- `lung-regional-contract-v1.json`.

Both are deliberately classified as `software_test_surrogate`. Their two-second
total transit and regional split are verification values. The definitions state
that they do not represent a human population or physiological state.

## External-data boundary

An `externally_derived` definition can identify an immutable source file by
SHA-256 and preserve its native format, axes, length unit, flow unit, and every
conversion applied. Definition loading validates this metadata; a future import
adapter must additionally resolve the file, verify its checksum, apply the
declared transformations, and validate the derived network.

No SimVascular/VMR geometry or hemodynamic parameter set is included or
qualified in M3 yet. The metadata test uses a nonexistent dummy path and an all-
zero checksum solely to exercise decoding. It is not a data release.

## Qualification rule

A software-test definition may enter CI immediately. A
`literature_parameterized` or `externally_derived` definition may be described
and tested technically, but cannot be presented as physiologically validated
until the source, population, state, licenses, transformations, calibration,
independent validation target, uncertainty, and limitations have been reviewed.
