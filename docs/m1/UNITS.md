<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Kernel Unit System

MEHLISSA Next uses SI internally and distinguishes physical dimensions in the
C++ type system. A value without a visible unit should occur only at a tightly
bounded serialization or mathematics boundary.

## Types and canonical units

| C++ type | Dimension | Internal SI value | Named inputs |
|---|---:|---|---|
| `Length` | L | metre | `meters`, `millimeters`, `micrometers` |
| `Time` | T | second | `seconds`, `milliseconds`, `minutes` |
| `Area` | L² | square metre | derived from `Length * Length`, `square_meters` |
| `Volume` | L³ | cubic metre | `cubic_meters`, `liters`, `milliliters` |
| `Speed` | L/T | metre per second | derived from `Length / Time`, `meters_per_second`, `millimeters_per_second` |
| `Amount` | N | mole | `moles`, `millimoles`, `micromoles` |
| `Concentration` | N/L³ | mole per cubic metre | derived from `Amount / Volume`, `moles_per_cubic_meter`, `millimoles_per_liter` |

`1 mmol/l` is exactly equal to `1 mol/m³`. Integer nanoseconds in the
`SimulationClock` remain separate so that events can be ordered without
rounding drift.

## Binding rules

- No implicit conversion from or to `double`.
- Addition and subtraction only for equal dimensions.
- Multiplication and division derive the result dimension.
- Public model APIs use quantity types, not unit suffixes on bare numbers.
- JSON and data formats state the unit and value separately and convert to SI
  exactly once during decoding.
- Models check finiteness, sign, and domain-specific value ranges at their input
  boundary; dimension safety does not replace this validation.

## Verification

`tests/quantity_tests.cpp` contains runtime and compile-time tests for all M1
quantities. Geometry tests use the migrated `Position3D` API, which is also
checked by the dependency-free smoke test.
