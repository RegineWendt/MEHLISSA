<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M5 Gate Review – Cell Response

**Review date:** 31 August 2026

**Reviewed implementation baseline:** the M5.8 commit containing this review

**Result:** passed — all five Roadmap Gate M5 statements are executable or
documented at the declared synthetic software-contract level

## 1. Review method

The review required executable code, strict profiles, automated positive and
negative tests, architecture decisions, evidence qualification, and an explicit
User Guide impact review. It distinguished the narrow Roadmap gate from the
broader long-term `CELL-*` research requirements and separated analytical
software verification from external biological validation.

## 2. Roadmap gate criteria

| Gate M5 criterion | Status | Evidence and finding |
|---|---|---|
| a molecular signal from the capillary model can trigger a cell reaction | satisfied at synthetic contract level | M5.2 samples a typed, time-scoped M4 tissue inventory without consuming it and maps amount/volume to an M5.1 receptor request; the end-to-end test produces binding and threshold detection while preserving M4 ownership |
| a cell model can return a measurable event or state change to higher layers | satisfied at synthetic contract level | M5.7 changes a cell to `apoptosis_committed` and a separate adapter emits a versioned neutral event with source, target, measurement, and time identity; viable responses remain silent |
| receptor binding and reaction network are tested against analytical or external reference data | satisfied for software verification | M5.1/M5.3 use closed-form binding references, M5.4 uses analytical binomial moments, and M5.5 converges to exact constant-input messenger/effector equilibria while binding ODE and SSA implementations to one conserved topology; no external biological validation is claimed |
| single-cell and population variants have documented validity scopes | satisfied | M5.1/M5.3/M5.4/M5.5/M5.7 single-cell scopes, the explicit-cell M5.4/M5.5 populations, and the M5.8 one-trillion-cell compressed population are compared in one model-selection table with limitations and scaling semantics |
| English User Guide passed gate-impact review | satisfied | metadata, capability/non-claim text, experiment catalog, decision aid, guided M5.8 workflow, evidence links, validity guidance, and next-milestone text were reviewed and updated |

## 3. Population and sensitivity findings

M5.8 evaluates one trillion synthetic cells as four weighted cohorts and
retains only two cohort details. The aggregate uses four Hill evaluations,
reports 750 billion viable and 250 billion committed cells, a committed
fraction of `0.25`, and mean effect `0.385`. Scaling the same weights to 1,000
cells preserves the fractions. Predeclared half-max and threshold variations
produce committed fractions from `0.0` to `0.5`, demonstrating that the
synthetic decision is assumption-sensitive.

The compressed model does not supersede M5.4. M5.4 is the accepted resolution
for finite-receptor stochasticity; M5.8 is accepted only for deterministic,
internally homogeneous amount cohorts.

## 4. Evidence disposition

The complete M5 evidence audit is in
[M5 Evidence Qualification](M5_EVIDENCE_QUALIFICATION.md). Analytical and
cross-implementation checks support gate acceptance. The BioModels
`BIOMD0000000525` CD95/caspase-8 model and the Eissing et al. caspase model are
recorded only as future external comparison candidates. No external model or
paper is counted as reproduced M5 evidence.

## 5. Verification evidence

- MSVC Debug build with warnings as errors: passed;
- focused M5.8 population suite: 31 assertions in 5 test cases, passed;
- analytical intracellular equilibrium plus population tags: 62 assertions in
  9 test cases, passed;
- complete local CTest regression: 228/228 passed; and
- strict schemas, semantic validation, overflow rejection, bounded-output
  checks, deterministic named streams, and exact conservation remain active.

## 6. User Guide impact review

The mandatory review found user-visible impact and updated:

- covered-software and last-updated metadata from open M5.1–M5.7 to accepted M5;
- the introductory cell-layer capability and explicit non-claims;
- experiment 5.9 inputs, workflow, outputs, interpretation, and limitations;
- the first-experiment decision aid with the M5.8 population profile;
- a new technical M5.8 section with runnable paths, expected values, focused
  checks, selection guidance, and evidence links; and
- maintenance text so M6 is the next open gate.

No CLI command was invented: M5 remains a component/developer reference
workflow, and that access level stays explicit.

## 7. Requirements disposition

| Requirement | Review disposition |
|---|---|
| `CELL-001` | `SPEC` retained: M5 accepts a time-scoped capillary snapshot, but a dynamic biomarker release/concentration field remains unimplemented |
| `CELL-002` | `PART` retained: deterministic and stochastic binding, thresholds, misclassification, and response coupling are executable; biological kinetics and context remain open |
| `CELL-003` | `PART` retained: device release and uptake conserve amount; spatial diffusion, drug-target binding, metabolism, and calibration remain open |
| `CELL-004` | `PART`: a shared ODE/SSA intracellular network and population comparisons are executable; externally reproduced biological reaction data remain open |
| `CELL-005` | `PART` retained: single-cell and compressed-population apoptosis states plus higher-layer event semantics are executable; mechanistic qualification and a scenario consumer remain open |

The broader requirements intentionally remain visible. Their incomplete
physiological scope does not contradict the five narrower gate statements.

## 8. Accepted limitations and exit decision

M5 does not establish a validated biomarker field, named biological pathway,
pharmacodynamic response, therapeutic dose, toxicity, safety, clinical outcome,
or patient population. Cohort compression does not create evidence or
heterogeneity. All executable M5 references remain synthetic software-test
surrogates.

Gate M5 passes as a technical milestone. M6 may build Nano-IoT, gateway, and
external communication on the accepted neutral events and model boundaries.
Any later biological qualification must add a new versioned model/profile and
must not silently reinterpret the M5 references.
