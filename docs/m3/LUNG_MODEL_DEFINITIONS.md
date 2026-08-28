<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Versioned Lung Model Definitions

## Executable model card

Schemas `lung-model-definition/1.0.0` through `1.4.0` bind model selection and
parameters to the evidence needed to interpret them. A definition contains:

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
Variant timing structures cannot be mixed. Version 1.1.0 adds a pulmonary 0D
variant and source-linked SI quantities with explicit uncertainty, evidence
role, and derivation.
Version 1.2.0 adds optional, evidence-linked flow adaptation with an explicit
reference flow, resistance and compliance exponents, and a maximum supported
flow ratio. Derived exponents can state that source uncertainty was not
propagated rather than inventing bounds.
Version 1.3.0 adds optional evidence-linked age conditioning with explicit
supported age bounds, band boundaries, a scenario age, and young/older PVR
multipliers. The executable loader rejects ages outside the calibrated range.
Version 1.4.0 adds a Linehan pressure-distensibility option with locked
reference flow, reference left-atrial pressure, and an inverse-pressure
coefficient. It is mutually exclusive with empirical flow adaptation.

## Checked-in definitions

Two examples make the existing software regressions externally reproducible:

- `lung-compartment-contract-v1.json`; and
- `lung-regional-contract-v1.json`.

Both are deliberately classified as `software_test_surrogate`. Their two-second
total transit and regional split are verification values. The definitions state
that they do not represent a human population or physiological state.

The first `literature_parameterized` definition is
`data/lung-models/healthy-adult-rest-supine-0d-v1.json`. It is executable and
records mean flow, left-atrial pressure sink, PVR, arterial compliance,
right/left perfusion, pulmonary transit, a mean-pressure comparison target,
uncertainty, and derivations. Its model card is
[Pulmonary 0D Reference Candidate](PULMONARY_0D_REFERENCE.md).

The v2 definition
`data/lung-models/healthy-adult-rest-exercise-0d-v2.json` adds bounded
rest-to-exercise flow adaptation while retaining v1 as an immutable resting
baseline. Its derivation, independent source selection, and post-calibration
test are documented in
[Bounded Pulmonary 0D Flow Adaptation](PULMONARY_0D_FLOW_ADAPTATION.md).

The v3 definition
`data/lung-models/healthy-adult-rest-exercise-age-0d-v3.json` preserves v2 and
adds a Kane-calibrated, three-band age multiplier on PVR. Its derivation and
untouched Kovacs/Wolsk result are documented in
[Pulmonary 0D Age Conditioning](PULMONARY_0D_AGE_CONDITIONING.md).

The v4 definition
`data/lung-models/healthy-adult-rest-exercise-age-invasive-0d-v4.json` keeps
schema 1.3.0 and replaces only the young PVR multiplier with an invasive
24–50-year calibration. Its overlap-aware Wolsk-only validation is documented
in [Pulmonary 0D Young-Adult Resistance Qualification](PULMONARY_0D_YOUNG_RESISTANCE.md).

The v5 definition
`data/lung-models/healthy-adult-pressure-distensible-age-0d-v5.json` keeps the
v4 resting and age evidence but replaces empirical flow adaptation with the
Linehan pressure-distensible relationship and a separately sourced healthy
coefficient. Its frozen 11/15 Wolsk comparison and older-stratum limitation
are documented in
[Pulmonary 0D Pressure Distensibility](PULMONARY_0D_PRESSURE_DISTENSIBILITY.md).

## External-data boundary

An `externally_derived` definition can identify an immutable source file by
SHA-256 and preserve its native format, axes, length unit, flow unit, and every
conversion applied. Definition loading validates this metadata; a future import
adapter must additionally resolve the file, verify its checksum, apply the
declared transformations, and validate the derived network.

No SimVascular/VMR geometry is included or qualified in M3 yet. The metadata
test uses a nonexistent dummy path and an all-
zero checksum solely to exercise decoding. It is not a data release.

## Qualification rule

A software-test definition may enter CI immediately. A
`literature_parameterized` or `externally_derived` definition may be described
and tested technically, but cannot be presented as physiologically validated
until the source, population, state, licenses, transformations, calibration,
independent validation target, uncertainty, and limitations have been reviewed.
