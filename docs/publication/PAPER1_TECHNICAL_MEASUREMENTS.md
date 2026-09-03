<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Paper 1 Technical Measurement Report

Status: completed technical baseline for release-candidate review  
Protocol: Paper 1 Technical Experiment Protocol v2.0.0  
Measurement date: 3 September 2026

## 1. Claim boundary

These experiments test the MEHLISSA software platform as software.  They cover
deterministic execution, state-neutral observation, bounded artifacts,
resource behavior in two small M7 configurations, and agreement of CLI,
Python, and Workbench access paths.  They do **not** establish physiological,
biological, diagnostic, therapeutic, patient-specific, or clinical validity.

The evidence and validity matrix remains authoritative for the separate
maturity of each executable model family.

## 2. Experiment P1-E1: frozen body-observation campaign

The accepted run used the frozen RQ4 campaign with seed `20260827`, seven
measured blocks, Release build, a clean worktree, populations 1,000, 10,000,
100,000, and conditionally 1,000,000, plus the 6,359-particle two-hour anchor.
The conditional million-particle population was enabled because the predeclared
O0 thresholds passed: median simulation time was 2.405 s at 100,000 particles
and median peak RSS used 0.000348 of physical memory.

All 112 scheduled attempts completed.  The campaign report records
`status = complete`, `suitable_for_analysis = true`, and zero correctness
violations.  Across every same-population, same-block observation policy, the
transition count, injected/active/extracted population, final-population hash,
and random-stream state were exactly equal.  Detailed policies also agreed on
population snapshots and measurement totals.  Observation therefore did not
change the simulated state in this campaign.

Median results are reported descriptively:

| Condition | Simulation time | Transition throughput/s | Peak RSS | Observation output |
|---|---:|---:|---:|---:|
| 6,359, O0, two-hour anchor | 2.640 s | 8,887,484 | 7.58 MiB | 911 B |
| 1,000, O0 | 18.69 ms | 11,599,555 | 7.57 MiB | 909 B |
| 1,000, O1 | 18.84 ms | 11,508,485 | 7.57 MiB | 0.09 MiB |
| 1,000, O2 | 20.23 ms | 10,716,076 | 8.53 MiB | 0.27 MiB |
| 1,000, O3 | 39.91 ms | 5,432,676 | 175.39 MiB | 38.51 MiB |
| 10,000, O0 | 204.88 ms | 10,579,854 | 7.57 MiB | 912 B |
| 10,000, O1 | 232.17 ms | 9,336,533 | 7.59 MiB | 0.09 MiB |
| 10,000, O2 | 221.78 ms | 9,773,937 | 8.71 MiB | 0.27 MiB |
| 100,000, O0 | 2.405 s | 9,010,646 | 11.26 MiB | 915 B |
| 100,000, O1 | 2.143 s | 10,112,524 | 11.82 MiB | 0.10 MiB |
| 100,000, O2 | 2.074 s | 10,448,999 | 11.79 MiB | 0.27 MiB |
| 1,000,000, O0 | 28.068 s | 7,721,693 | 46.51 MiB | 918 B |
| 1,000,000, O1 | 28.918 s | 7,494,763 | 45.78 MiB | 0.10 MiB |
| 1,000,000, O2 | 28.056 s | 7,725,129 | 45.80 MiB | 0.28 MiB |

O0 disables detailed observation, O1 retains bounded aggregates and passive
counters, O2 adds bounded first-N trajectories and measurement records, and O3
is the deliberately detailed all-particle 1,000-particle condition.  The O3
result demonstrates why output policy is a first-class experimental choice:
even at 1,000 particles it raised median retained output to 38.51 MiB and peak
RSS to 175.39 MiB.  The remaining timing differences are descriptive medians,
not inferential performance claims.

### Setup deviation

An initial attempt used an incorrectly transcribed temporary O1 field and was
rejected by the driver in all 32 O1 attempts.  It was marked unsuitable for
analysis, excluded from the results above, and retained as
`P1-E1-invalid-setup-report.json`.  The accepted rerun used a machine plan that
matched the frozen plan except for the required machine label.  This is an
operator setup deviation, not an override or an experimental exclusion.

## 3. Experiment P1-E2: bounded M7 resource and replay study

After two unmeasured warm-ups, baseline 1,000-collector and 10,000-collector
conditions were alternated and repeated three times each with the same master
seed.  Exact result hashes and deterministic scientific projections agreed
within each condition.

| Condition | Median wall time (range) | Median peak memory (range) | Artifact bytes |
|---|---:|---:|---:|
| 1,000 collectors | 0.241 s (0.214–0.260) | 8,486,912 (8,458,240–8,585,216) | 28,816 |
| 10,000 collectors | 0.388 s (0.326–0.596) | 8,486,912 (8,462,336–8,544,256) | 29,160 |

The invalid-scenario control was rejected and a deliberately altered result
hash was detected.  This deliberately small study supports a technical
reproducibility/resource statement only.  It is not a scaling law, HPC result,
or biological feasibility result.

## 4. Experiment P1-E3: access-path parity

The same accepted M7 result was read twice through the CLI, twice through the
version-guarded Python reader, and through the Workbench dashboard and audit.
The CLI and Python results were repeatable, the Workbench explicitly reported
`mehlissa.load_result` as its reader, its scientific summary matched the Python
summary exactly, and the authoritative result hash remained unchanged.  The
Workbench audit reported `verified`; a tampered copy reported `attention`.

This supports parity of the scientific information exposed through the three
access paths.  It is not a usability study and does not independently validate
the underlying models.

## 5. Reproduction and archive

The locked protocol, machine-readable reports, complete ZIP archives, scripts,
source/build identities, hashes, and the invalid setup report are collected in
`publication/paper1/release-candidates/paper1-platform-methods-rc1-20260903`.
The release candidate must be checked with the repository publication checker
and the complete supported CI matrix before manuscript claims are accepted.

