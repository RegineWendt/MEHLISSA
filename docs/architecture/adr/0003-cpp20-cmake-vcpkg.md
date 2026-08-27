<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0003: C++20, CMake, and vcpkg as the Technical Foundation

- **Status:** Accepted
- **Date:** 26 August 2026
- **Applies to:** M0/M1; `QUA-001`, `QUA-002`, `QUA-005`

## Context

Whole-body and multiscale simulations require controllable runtime, memory
layout, and parallelization. Historical MEHLISSA versions are implemented in
C++. Python is attractive in the long term for experiments, data analysis, and
scientific workflows, but using it as the sole kernel for large agent counts
cannot be justified without prior performance comparison.

The local development environment and CI support MSVC, GCC, and Clang.
Dependencies must be pinned reproducibly.

## Decision

- The simulation kernel and performance-critical models are developed in standards-compliant C++20.
- CMake is the sole build system; builds are out of source and use presets.
- Direct C++ dependencies are declared in `vcpkg.json` with a fixed baseline commit.
- CTest orchestrates tests; Catch2 is the unit-test framework.
- CI supports MSVC, GCC, and Clang; the analysis job uses clang-tidy and the Address/UndefinedBehavior sanitizers.
- Python is added later as a versioned API for experiment creation, ensembles, and analysis. The binding must not duplicate the C++ domain model.
- New dependencies require a concrete use case, license review, and a justified ADR entry if they shape the architecture.

## Consequences

Positive:

- Performance-critical paths and memory layout remain controllable.
- Existing C++ expertise and selected legacy algorithms remain usable.
- The build is already checked automatically with three compilers.
- Python can provide a user-friendly interface without replacing kernel correctness.

Negative:

- C++ increases the demands on ownership, units, and API discipline.
- Cross-language bindings and packaging add effort.
- vcpkg requires reachable package sources; the offline smoke test covers only the kernel.

## Reassessment

The decision will be reviewed after M2 using measured profiles, API experience,
and the needs of external modelers. An alternative kernel is justified only if
a representative prototype demonstrably improves correctness, reproducibility,
and performance.
