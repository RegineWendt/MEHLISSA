<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 Gate Review – Body–Organ Coupling

**Review date:** 27 August 2026

**Reviewed baseline:** `ea2b2932f9e6268e70f0494507a3ab7ceae9ea41`

**Result:** not passed — technical coupling candidate complete; scientific gate
remains open

**Post-review update, 28 August 2026:** M3.7 adds the first executable,
literature-parameterized pulmonary 0D reference candidate. This closes the
absence of source-scoped mean pressure, flow, resistance, compliance, total
transit, right/left perfusion, and parameter uncertainty at implementation
level. It does not change the gate result because its nominal values form a
composite calibration reference rather than a jointly measured subject set,
independent quantitative validation and anatomical refinement remain open,
and the FP9 scenario-layer baseline is still not executable. See
[Pulmonary 0D Reference Candidate](PULMONARY_0D_REFERENCE.md).
The post-review local Visual Studio Debug suite passed 91/91 tests; CI evidence
is recorded separately when the increment is pushed.

## 1. Review method

The review checked the Roadmap Gate M3 statements and the additional acceptance
criteria in ADR-0006 against executable evidence. Software verification was
kept separate from anatomical fidelity, physiological parameterization, and
independent validation. A second class was not treated as a “detailed organ
model” merely because it contained more regions than the coarse surrogate.

## 2. Roadmap gate criteria

| Gate M3 criterion | Status | Evidence and finding |
|---|---|---|
| an agent moves reproducibly from the body graph into an organ and back | satisfied | `BodyOrganCoupler`, explicit outside-body ownership ledger, stable ID and named-port validation, coarse/regional × 0.5/1.0 s matrix |
| flow, populations, and substance amounts are conserved across the layer boundary | satisfied for lossless transit | dimension-safe contracts, exact boundary ledger, both lung endpoints preserve transfer ID and typed payload; changing biochemical exchange is intentionally not claimed |
| the organ has an independent, interchangeable model implementation | partially satisfied | two independent `ModelComponent` implementations and one factory/definition path exist; the regional implementation remains a structural test surrogate rather than an independently qualified physiological model |
| a coarse compartment and more detailed organ model use the same scenario | partially satisfied | the same externally selected body–organ regression runs both implementations without kernel/coupler branches; “more detailed” is structural only, not anatomical 0D/1D or vascular geometry |

## 3. Additional ADR-0006 criteria

| Criterion | Status | Finding |
|---|---|---|
| agent and substance flow complete body–lung–body traversal | satisfied for entity; organ traversal satisfied for conserved quantities | the complete entity round trip reaches the body return ledger; conserved population, substance, and flow traverse both lung endpoints without payload change |
| no agent or relevant amount is duplicated or lost | satisfied | positive and negative ownership, duplicate-ID, route, time, and exact-payload tests |
| both lung variants implement the same contract | satisfied | one `ModelComponent` interface, one `LungModelConfig`, schema-selected definitions, generated cross-variant tests |
| flow, pressure/transit time, and perfusion share have units, sources, uncertainty, and a reference comparison | partially satisfied after M3.7 | the 0D candidate has executable SI pressure, flow, PVR, compliance, measured total transit, right/left perfusion, uncertainty, evidence roles, and a normal-range pressure comparison; its composite calibration values are not yet independently validated as one joint physiological state |
| historical FP9 timer baseline runs without scenario-specific kernel logic | not satisfied | the dissertation baseline is specified and traced, but fingerprint detection/assembly/collection is not executable; implementing it prematurely in the organ kernel would violate the architecture |

## 4. Verification evidence

- local Visual Studio 2026 Debug build: passed;
- local CTest suite: 85/85 passed;
- local formatting and targeted Clang-Tidy/bugprone checks: passed;
- `git diff --check`: passed for the reviewed M3 changes;
- complete M3.5 GitHub matrix: [run 33081276396](https://github.com/RegineWendt/MEHLISSA/actions/runs/33081276396), passed on Windows/MSVC, Linux/GCC, and Linux/Clang with Clang-Tidy, ASan, and UBSan;
- final M3.6 orchestration matrix:
  [run 33081839637](https://github.com/RegineWendt/MEHLISSA/actions/runs/33081839637),
  passed on Windows/MSVC, Linux/GCC, and Linux/Clang with Clang-Tidy, ASan,
  and UBSan.

## 5. What is complete and reusable

- versioned entity, population, substance-amount, and volume-flow contracts;
- deterministic identity and quantity ownership at model boundaries;
- explicit body hand-off, organ ownership, and body return;
- coarse and regional pulmonary transit implementations behind one interface;
- schema-validated executable model cards with validity, evidence, sources,
  licenses, limitations, and optional external-data axes/units/provenance;
- one externally selected scenario across both resolutions and compatible host
  steps; and
- cross-platform build, analysis, sanitizer, and regression infrastructure.

These contracts are suitable for continued software integration. They do not
make the current lung models physiological reference models.

## 6. Blocking scientific closure package

M3 can pass only after the following evidence is added and reviewed:

1. qualify and license a concrete pulmonary reference case or a sourced 0D/1D
   parameterization, including population and physiological state; the initial
   [SimVascular candidate review](SIMVASCULAR_PULMONARY_CANDIDATE_REVIEW.md)
   narrows the open questions but does not qualify the archive;
2. ~~provide sourced pressure, resistance/flow, perfusion, and transit targets
   in SI units with uncertainty and explicit calibration/validation
   separation~~ — implemented by M3.7; independent subject-level validation
   remains under items 1 and 3;
3. implement the anatomical/hemodynamic regional variant and compare it with
   both independent targets and the effective compartment;
4. decide whether rest/exercise pulmonary redistribution belongs in the M3
   reference set, then provide reviewed parameters rather than global scaling;
5. implement the historical FP9 timing regression at the appropriate scenario
   layer, or formally revise the ADR/M3 boundary if that executable baseline is
   deferred to M7; and
6. rerun this gate review on the resulting immutable data/model baseline and
   complete CI matrix.

## 7. Exit decision

The M3 software architecture and lossless coupling slice are verified, but the
milestone is not closed. The gate remains open on scientific evidence and the
FP9 executable reference, not on a hidden software failure. Downstream
prototyping may use the verified interfaces provided it retains the explicit
`software_test_surrogate` validity label and does not cite the current outputs
as pulmonary physiology.
