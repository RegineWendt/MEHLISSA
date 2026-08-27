<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Lung Model Selection

## Purpose

`make_lung_model` is the composition boundary for selecting an organ-model
resolution. A `LungModelConfig` selects either:

- `effective_compartment`, implemented by `LungCompartment`; or
- `regional_circulation`, implemented by `PulmonaryCirculation`.

Both results are returned as `ModelComponent` and therefore expose the same
named entry, named exit, lifecycle, and entity-transfer contract. The body
coupler contains no variant-specific branch.

## Configuration safety

Variant-specific parameters are mutually exclusive. A compartment requires a
positive compartment transit time and no regional definitions. A regional
model requires one or more validated regions and no compartment transit time.
The concrete model constructors continue to validate identifiers, ports, and
durations after selection.

## Regression evidence

The body–lung–body regression is generated for both selections from the same
test scenario. In both cases the same entity leaves body segment `artery-10`,
is owned by the selected lung for two seconds, and returns to `vein-90` without
loss or duplication. Dedicated factory tests also reject mixed parameter sets.

Schema-validated standalone lung definitions now feed this same typed factory;
see [Versioned Lung Model Definitions](LUNG_MODEL_DEFINITIONS.md). Composition
inside the general experiment manifest remains future work and must preserve
this path rather than introduce a second model-selection mechanism.
