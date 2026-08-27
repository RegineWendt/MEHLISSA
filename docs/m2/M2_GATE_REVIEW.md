<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2 Gate Review – Validated Body Layer

**Review date:** 27 August 2026

**Reviewed baseline:** `6292a22606585ab7e44f9bef01d448864fcfb455`

**Result:** passed

## 1. Review method

The review checked every Gate M2 statement against executable verification,
versioned data, and methodological documentation. A requirement was not treated
as satisfied merely because a class or document existed. The review also
separated software verification, historical regression, calibration
reproduction, and independent physiological validation.

The first reviewed candidate, `df36ced`, passed the complete Windows build and
70 tests but exposed two Linux quality failures: formatting drift and GCC
`-Wmissing-field-initializers` diagnostics. These findings were not waived.
Commit `db57261` reformatted the affected sources and replaced the two partial
aggregate initializations with explicit assignments. Subsequent analysis found
ambiguous same-type helper parameters in the BVS runner and legacy converter;
commits `108294e` and `6292a22` replaced them with typed or type-distinct
arguments. No finding was waived.

## 2. Gate acceptance criteria

| Gate M2 criterion | Status | Evidence |
|---|---|---|
| 1995 model is schema-validated and fully documented | satisfied | canonical 95-segment SI graph, strict vascular JSON Schema, deterministic lossless migration, provenance/release record, graph and migration tests |
| BVS and dissertation results are reproduced within defined tolerances or deviations are explained | satisfied | 6,359/63,590-agent BVS regression, minute-window comparisons, 23-target dissertation perfusion comparison, predefined gates, schema-valid golden report, methodological separation in `BVS_REFERENCE_REGRESSION.md` |
| particle, flow, and transition invariants are automatic | satisfied | single-edge-per-advance, exact injected/active/extracted balance, reproducible named-stream branching, strongly connected topology, normalized transitions, geometry and junction-flow validation |
| a new body model can be loaded without code changes | satisfied | the same schema, loader, validator, CLI, and transport accept the synthetic branching circuit and the canonical 95-segment graph; versioned state overlays also load without rebuilding |
| output can be aggregated and bounded for large experiments | satisfied | exact aggregates with independent bounds for trajectories, measurements, and snapshots; explicit truncation indicators; schema-validated report |

## 3. Verification evidence

- local Visual Studio 2026 Debug build: passed;
- local CTest suite: 70/70 passed;
- local formatting gate: passed with repository `.clang-format`;
- `git diff --check`: passed;
- Windows GitHub job for the complete M2 candidate: build and 70/70 tests passed;
- Linux GCC build and 70/70 tests: passed;
- Linux Clang format verification, Clang-Tidy build, ASan/UBSan build, and 70/70 tests: passed;
- complete GitHub matrix: [run 33076167919](https://github.com/RegineWendt/MEHLISSA/actions/runs/33076167919), passed.

## 4. Scientific scope and accepted limitations

The term “validated body layer” applies to the declared M2 transport contract
and reference regressions. It is not a claim of complete anatomical or
physiological validation.

- BVS95 geometry is schematic rather than subject-specific anatomy.
- Equivalent diameters are transport parameters, not measured vascular radii.
- The 23-target perfusion comparison is a dissertation calibration regression,
  not independent physiological validation.
- The rest, exercise, and head-up-tilt profiles are transparent sensitivity
  profiles. Regional exercise redistribution, the vertebral venous plexus,
  pressure, compliance, and dynamic transitions remain open.
- The implemented flow model is a deterministic perfused compartment model;
  virtual laminar and imported streamline/CFD variants remain future work.
- Complete experiment-level orchestration and body–organ hand-off begin in M3.

These limitations are documented in model data and M2 method documents and do
not contradict the explicit Gate M2 criteria. They remain traceable as `PART`
or `RESEARCH` requirements rather than being silently marked complete.

## 5. Exit decision

The implementation, local evidence, and complete cross-platform CI matrix
satisfy Gate M2. M2 is closed. M3 may build on the body-layer contracts without
freezing the declared follow-on research work.
