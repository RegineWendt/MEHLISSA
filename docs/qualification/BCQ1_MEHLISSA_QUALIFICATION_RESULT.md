<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# BCQ-1.4–1.7 MEHLISSA Qualification Result

## Decision

BCQ-1.4 through BCQ-1.7 are complete. Every unblocked computational gate
passes. MEHLISSA now contains a typed, no-refit implementation of the selected
minimal Kallenberger 2014 CD95L–CD95–caspase-8 mechanism, and both source cases
reproduce the independently generated COPASI trajectories within the
prospectively frozen limits.

The result is deliberately narrower than biological validation. Publication-
curve alignment, a reusable population ensemble, and external human reviewer
attestation remain `BLOCKED`; biological qualification is
`NOT_ESTABLISHED`. The authoritative machine record is
`data/qualification/biological-cell-model-qualification-result-v1.json`.

## BCQ-1.4 — typed adapter

The public C++ entry point is
`mehlissa/models/cell/qualified_cd95_apoptosis_model.hpp`.
`QualifiedCd95ApoptosisAdapter` accepts:

- one of the exact `BIOMD0000000523` or `BIOMD0000000524` source cases;
- the unchanged `CD95L = 16.6` stimulus;
- the explicit `unresolved-model-native` unit semantics;
- an end time, output interval, and bounded internal RK4 step.

It returns all 18 source states and the named `PrER_mGFP`, `PrNES_mCherry`,
`p43`, and `p18` observables. The adapter rejects a changed stimulus, invented
SI units, nonphysical grids, and public parameter overrides.

The `KallenbergerMinimalMechanism` class is a separate equation layer. It
implements the source `CD95act` assignment and 13 reaction fluxes. This keeps
M5 input/output mapping distinct from source mathematics and lets the
qualification-only runner perturb parameters without widening the no-refit
adapter.

## Code-to-equation review

| Source operation | MEHLISSA flux | State effect |
|---|---|---|
| FADD recruitment and dissociation | `v1`, `v2` | `FADD ⇄ DISC` |
| p55 recruitment | `v3` | `p55free + DISC → DISCp55` |
| D216 processing | `v4`, `v5` | `DISCp55 → p30`; `p43 → p18 + DISC` |
| p55-driven D374 trans cleavage | `v6`, `v8` | `DISCp55 → p43`; `p30 → p18 + DISC` |
| p43-driven D374 trans cleavage | `v7`, `v9` | the corresponding `p43`-dependent conversions |
| p18 inactivation | `v10` | `p18 → p18inactive` |
| BID cleavage | `v11` | `Bid → tBid` driven by `p43 + p18` |
| NES reporter cleavage | `v12` | `PrNES_mCherry → PrNES + mCherry` |
| ER reporter cleavage | `v13` | `PrER_mGFP → PrER + mGFP` driven by `p18` |

The source identities, eleven fixed kinetic parameters, two initial vectors,
assignment-rule algebra, state order, and implementation hashes are bound by
the [prospective protocol](BCQ1_MEHLISSA_QUALIFICATION_PROTOCOL.md). Tests
exercise every source-derived conservation family and all 18 cross-engine
states.

## BCQ-1.5 — cross-engine and structural checks

For each source case, MEHLISSA produced primary, byte-identical replay, and
tightened-step trajectories with 961 points and all 18 states. Primary RK4 uses
24,000 internal steps; tightened RK4 uses 48,000. The independent reference is
the already frozen COPASI 4.46 Build 300/LSODA archive from BCQ-1.3.

| Case | Maximum absolute MEHLISSA–COPASI difference | Maximum fraction of prospective limit | Maximum primary–tightened difference | Convergence fraction of limit |
|---|---:|---:|---:|---:|
| `BIOMD0000000523` | `2.62697e-7` | `0.023060` | `7.73070e-11` | `1.22874e-5` |
| `BIOMD0000000524` | `7.30472e-7` | `0.018904` | `2.68301e-11` | `6.36569e-6` |

The largest invariant residual is `4.00178e-11`; the minimum state is zero;
both uncleaved reporters are non-increasing. The magnitude of an absolute
difference must not be confused with the scale-aware pass rule: every one of
the 34,596 state/time comparisons lies below its frozen tolerance.

The official CC0 525/526 companions were checked by commit, Git-blob SHA-256,
licence file, SBML structure, and initial vector. Each has the expected 18
species, 19 reactions, 15 global parameters, and one assignment rule—six
reactions and three global parameters more than the selected minimal family.
They remain same-publication structural context and were not used as an
independent outcome source or an additional M5 runtime variant.

## BCQ-1.6 — population and uncertainty decision

The audited public package does not establish a reusable row-level ensemble of
initial protein values with correlations and reuse rights. MEHLISSA therefore
does not fabricate a population by sampling independent marginals. The two
source cases remain explicitly average-cell models.

As a diagnostic, all eleven kinetic parameters were perturbed centrally by
1% and 0.5% for both cases. Normalized sensitivities were evaluated for the
four final-time observables. All 88 step-stability comparisons pass; the worst
relative step disagreement is `0.003376`, well below the prospective `0.05`
limit. These values describe local model behavior only. They are not a
probability distribution, confidence interval, patient variability, or
population prediction.

## BCQ-1.7 — independent checks and bounded claim

The result checker is separate from the result runner. It reconstructs every
CSV comparison, convergence value, invariant, reporter direction, sensitivity
stability result, archive hash, source identity, structural role, review state,
and documentation boundary. Twelve mutation/overclaim negative controls fail
closed. The C++ tests independently exercise the adapter, equations,
conservation, COPASI agreement, and step refinement.

External human scientific reviewer attestation has not been received. The
review outcome is therefore
`COMPUTATIONALLY_QUALIFIED_AVERAGE_CELL_WITH_BLOCKED_BIOLOGICAL_GATES`, not
biological qualification.

The allowed statement is:

> MEHLISSA contains a typed, no-refit reimplementation of the selected
> Kallenberger minimal average-cell SBML mechanism whose trajectories reproduce
> the independently generated COPASI reference within the frozen numerical
> limits.

The result must not be described as quantitative publication-curve
reproduction, population-ensemble reproduction, individual-cell prediction,
normal endothelial-cell realism, independent experimental validation, organ
or patient validity, treatment evidence, safety evidence, clinical validity,
or biological qualification.

## Reproduction

Build the project with cell models and testing enabled. The checked C++ runner
is `mehlissa_bcq1_qualification_runner`. The archive-generation command is:

```powershell
python scripts/run_biological_cell_model_qualification.py `
  --runner build/windows-msvc/models/cell/Debug/mehlissa_bcq1_qualification_runner.exe `
  --structural-525 C:\controlled\BIOMD0000000525.xml `
  --structural-526 C:\controlled\BIOMD0000000526.xml
```

The two structural files must match the frozen official Git commits and blob
hashes; they are inspected in the controlled workspace and are not copied into
the result archive. The committed result checker requires no external model or
solver.

## Archive and retained deviations

The authoritative archive is
`results/bcq1/kallenberger-mehlissa/20260906T090213Z-e3e71e6ce6c2`.
Its canonical checksum manifest is bound by the machine result. Two earlier
post-protocol attempts remain committed and explicitly non-authoritative:

- `20260906T084542Z-e3e71e6ce6c2` contains valid computations but was produced
  before the required living-document review was complete;
- `20260906T085845Z-e3e71e6ce6c2` contains valid computations but its integrity
  summary directly aggregated five rather than all nine frozen invariants.

The authoritative rerun directly reports all nine invariants and binds the
completed User Guide, Roadmap, Requirements Matrix, project status, and
code-to-equation review. Retaining the superseded attempts makes the review
history reproducible and prevents a passing rerun from erasing process defects.
