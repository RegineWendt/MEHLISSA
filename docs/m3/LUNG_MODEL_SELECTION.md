<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Lung Model Selection

## Purpose

`make_lung_model` is the composition boundary for selecting an organ-model
resolution. A `LungModelConfig` selects one of:

- `effective_compartment`, implemented by `LungCompartment`; or
- `regional_circulation`, implemented by `PulmonaryCirculation`; or
- `pulmonary_zero_dimensional`, implemented by
  `PulmonaryZeroDimensionalModel`.

All results are returned as `ModelComponent` and therefore expose the same
named entry, named exit, lifecycle, and entity-transfer contract. The body
coupler contains no variant-specific branch.

## Configuration safety

Variant-specific parameters are mutually exclusive. A compartment requires a
positive compartment transit time and no regional definitions. A regional
model requires one or more validated regions and no compartment transit time.
The 0D model requires its typed flow, pressure, resistance, compliance,
transit, and right-perfusion parameters and rejects either surrogate's timing
structure.
The concrete model constructors continue to validate identifiers, ports, and
durations after selection.

## Regression evidence

The body–lung–body regression is generated for both software surrogates from
the same two-second test scenario. The literature 0D definition runs the same
contract at its measured 6.4-second transit using compatible 0.1- and
0.2-second host steps. In every case the same entity leaves body segment
`artery-10` and returns to `vein-90` without loss or duplication. A generated
conserved-transfer regression covers all three implementations. Dedicated
factory tests also reject mixed parameter sets.

Schema-validated standalone lung definitions now feed this same typed factory;
see [Versioned Lung Model Definitions](LUNG_MODEL_DEFINITIONS.md). Composition
inside the general experiment manifest remains future work and must preserve
this path rather than introduce a second model-selection mechanism.
