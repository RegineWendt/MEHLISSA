<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Paper 1 Technical Experiment Protocol v2.0.0

**Status:** locked before new publication measurements

**Lock date:** 3 September 2026

**Machine-readable protocol:**
`publication/paper1/technical-experiment-protocol-v2.0.0.json`

**Schema:** `data/schemas/paper1-technical-protocol/2.0.0.schema.json`

## Amendment and change history

Paper 1 protocol v1.1 is an externally frozen Paper artifact and is not present
in this software repository. It has not been changed, overwritten, recreated or
silently reinterpreted. This v2 protocol is a new amendment for the repositioned
platform-and-methods paper.

The scope changes because MEHLISSA now provides the full M2-M7 stack and three
research access paths. Consequently, v2:

1. retains the frozen RQ4 body-transport experiment unchanged;
2. adds conservative and causally traceable multiscale composition;
3. adds end-to-end evidence, provenance and validity integrity;
4. adds a bounded full-M7 resource/replay study; and
5. adds CLI/Python/Workbench access-path parity.

The FP9 workflow remains a software and integration demonstrator. It is not a
biological, diagnostic, device or clinical feasibility experiment.

## Preliminary technical evaluation questions

| ID | Technical question | Boundary |
|---|---|---|
| RQ-T1 | Are versioned inputs and named random streams reproducible across supported platforms and access paths? | no claim of equal performance or scientific validity across platforms |
| RQ-T2 | Does composition of replaceable multiscale models preserve conservation and causal identities? | only implemented contracts and reference cases |
| RQ-T3 | Are results traceable to inputs, evidence roles, limitations, versions and hashes with calibration/validation separation? | traceability cannot upgrade evidence quality |
| RQ-T4 | How do bounded observation and research access paths affect resources and result integrity? | descriptive measurements on recorded machines; no universal scalability or usability claim |

These are working evaluation questions, not manuscript wording.

## Locked execution conditions

- Measure the clean Git commit containing this protocol.
- Use a Release build with the repository-pinned vcpkg revision.
- Record UTC time, OS/version, architecture, CPU, logical processors, physical
  RAM, compiler/version, CMake, Python, build type, commit, branch, remote and
  exact worktree state.
- Execute each measured attempt in a fresh process and preserve stdout, stderr,
  exit status, timestamps and partial artifacts.
- Never pool measurements across machines, operating systems or compilers.
- Preserve raw precision. Derived reports may round only for presentation.

The primary measurement host is assigned one stable, non-personal machine
label immediately before execution. Supported-platform reproducibility is
reported per CI or independent host, not inferred from the primary Windows
machine.

## P1-E1: frozen body-observation campaign

The authoritative plan remains
`examples/benchmarks/rq4-primary-campaign.json`: master seed 2018, ordering seed
20260827, seven measured blocks, ten mandatory population/policy combinations,
one two-hour 6,359-entity anchor per block and a conditional one-million-entity
phase. O0 retains no detail; O1 retains bounded sites/aggregates; O2 adds a
bounded trajectory sample; O3 retains complete detail within frozen pilot
limits.

The campaign tests exact equality of transitions, final states and hashes, RNG
draw counts, populations, passive measurements and common aggregates. Timing,
peak RSS, throughput and output volume are descriptive. Median, inclusive
quartiles, IQR and range are reported without significance claims. The
one-million phase runs only if the predeclared O0 eligibility thresholds pass.

The checked-in placeholder machine label is replaced only in a retained
measurement copy. The frozen source plan itself remains unchanged.

## P1-E2: bounded full-M7 resource and replay study

Use `fp9-lung-level-a-v1` with the same master seed for two conditions:

- baseline: 1,000 collectors, three measured fresh-process repetitions;
- controlled variation: 10,000 collectors, three repetitions.

After one unreported warm-up of each condition, alternate baseline and
variation. Record wall time, peak process memory, recursive output bytes/file
count, result hash, all input hashes, stage and causal trace, classifications
and communication outputs. Within a condition, result hashes, causal trace,
input hashes and conservation/population invariants must be exactly equal.
Across conditions, only causally collector-dependent fields may differ. Resource
values are descriptive replicate values plus median and range.

Positive controls validate the input and every result through authoritative
readers. Negative controls reject one schema-invalid scenario and one tampered
result copy. Neither negative control is a measured scientific run.

## P1-E3: research-access parity

Create one authoritative result with the C++ CLI. Read or project that same
file twice through:

- CLI `result summarize`;
- Python `mehlissa.load_result`; and
- Workbench audit/dashboard code, which must declare the same reader and source
  hash.

Scenario/run IDs, seed, schema version, stage order/count, detection, assembly,
communication, classification, limitations and source SHA-256 must remain
exact. Presentation-only fields may differ if listed. Input mutation is
forbidden. Existing Python and Workbench negative tests remain controls for
wrong document type, malformed inputs, missing artifacts and hash tampering.
This is an integrity test, not a usability study.

## Failures, partial runs and exclusions

Every attempt receives a unique ID. Failed, interrupted and resource-limited
attempts remain in the append-only ledger; retries receive new IDs and reasons.
Missing values remain missing and are never imputed. Runs with the wrong
revision, dirty tree, non-Release binary, invalid input, incomplete environment
metadata, operator interruption or failed integrity checks are excluded from
completed-run summaries but still reported.

Unexpected outcomes are retained without post-hoc tolerance changes. A count
of passing tests is verification metadata, not a scientific result.

## Raw-data and archive contract

Each release candidate uses:

```text
publication/paper1/release-candidates/<candidate-id>/
  metadata/
  protocol/
  measurements/<experiment-id>/raw/<attempt-id>/
  measurements/<experiment-id>/analysis/
  tests/
  ci/
  SHA256SUMS.json
```

The manifest records normalized paths, byte sizes and SHA-256 hashes. Analysis
scripts are retained with their inputs. Temporary directories, local build
trees and scratch benchmark workspaces are never committed.

No DOI, final public release tag or final 100% claim audit is authorized by
this protocol. Those actions remain a later Paper gate after manuscript text
exists.
