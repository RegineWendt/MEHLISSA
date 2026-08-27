<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0010: Dimension-Safe SI Quantity System

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M1/M2; `SYS-001`, `SYS-004`, `BODY-002`, `DATA-001`

## Context

Historical MEHLISSA models sometimes encode units only in variable names or
comments. This permits unnoticed confusion between metres and millimetres and
dimensionally invalid calculations. Future body, organ, cell, and communication
models exchange quantities across several scales; a documentation convention
alone is insufficient.

The simulation clock additionally needs integer nanoseconds for exact,
monotonic event sequences. Physical model equations, in contrast, require
dimension-safe multiplication and division with derived quantities.

## Decision

1. The kernel defines `Quantity<Dimension<L, T, N>>`. The exponents represent length, time, and amount of substance in SI base dimensions.
2. Public alias types cover `Length`, `Time`, `Area`, `Volume`, `Speed`, `Amount`, and `Concentration` in M1.
3. Quantities of different dimensions cannot be added or subtracted. Multiplication and division derive the result dimension at compile time.
4. There is no implicit conversion between `double` and a physical quantity. Named factory and output functions make the unit visible at the call site.
5. SI values are used internally: metre, second, cubic metre, metre per second, mole, and mole per cubic metre. Common medical prefixes are scaled exactly at the boundary.
6. `SimulationClock` retains integer `std::chrono::nanoseconds`. Model equations use the dimension-safe time type; adapters between event time and model quantity are introduced where needed.
7. `Position3D` stores dimension-safe lengths and returns a `Length`. Bare coordinate values are no longer permitted in the new API.
8. Positivity, finiteness, and domain-specific value ranges are invariants of the respective model or input schema, not of the dimension itself.

## Consequences

Positive:

- Dimensional errors become visible at compile time rather than during an experiment.
- Input, model, and output code states conversions explicitly.
- Derived quantities such as speed and concentration result from ordinary but type-safe arithmetic.
- The system is small, header-only, and remains part of the dependency-free offline kernel.

Negative:

- Existing code using bare `double` values must be migrated deliberately at boundaries.
- Internal representation as `double` does not solve uncertainty or numerical-conditioning problems.
- Further base dimensions such as mass or temperature require a controlled extension of the dimension vector.
- Clock time and physical time intentionally remain separate types and require explicit adapters.

## Alternatives

- **Unit only in the variable name:** rejected because the compiler cannot verify invariants.
- **Unit as a runtime string:** still needed for metadata, but too late and error-prone for kernel arithmetic.
- **All values as SI `double`:** avoids internal conversions, but not confusion between dimensions.
- **External units library:** technically possible, but adds API, build, and migration complexity for the currently small set. The decision will be reassessed when extended dimensions, formatting, or standards interoperability exceed the small internal contract.
