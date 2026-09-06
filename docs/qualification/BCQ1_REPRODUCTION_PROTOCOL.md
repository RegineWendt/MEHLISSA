<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BCQ-1.2 Prospective No-Refit Reproduction Protocol

## Protocol decision

BCQ-1.2 freezes the first execution protocol for the minimal Kallenberger 2014
CD95L-CD95-caspase-8 model pair before a trajectory is generated. The
machine-readable authority is
`data/qualification/biological-cell-model-reproduction-protocol-v1.json`.
Its parent is the content-hashed
[BCQ-1.1 selection](BCQ1_BIOLOGICAL_CELL_MODEL_SELECTION.md).

This increment was a protocol result, not a simulation result. At its freeze,
no external SBML file was bundled in MEHLISSA, no model had been run, and no
parameter had been fitted. The later
[BCQ-1.3 result](BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md) preserves this protocol,
its failed exact-zero replay outcome, and the committed prospective amendment.
M5 remains `software_test_surrogate`.

## Question

Can an external, mature SBML engine execute the two unchanged public
average-cell artifacts reproducibly, with stable numerical behavior and the
source mechanism's conservation properties, without fitting or silently
supplying missing unit semantics?

This deliberately narrower question comes before MEHLISSA integration. It
separates three otherwise easily confused claims:

1. **source-artifact reproduction:** an independent engine executes the public
   equations and initial values consistently;
2. **publication-curve reproduction:** numeric trajectories agree with a
   rights-compatible, frozen publication reference series; and
3. **biological qualification:** the model predicts observations in a declared
   validity domain with adequate independent evidence.

BCQ-1.3 may establish only the first claim. The second is blocked until a
machine-readable reference series and its use basis are frozen. The third
requires later mapping, comparison, uncertainty work, and review.

## Frozen lineage

| Item | Frozen identity |
|---|---|
| protocol baseline | MEHLISSA commit `3f7c555bdc604b1acb9da20cd632b1b7a2770d62` |
| BCQ-1.1 parent | `biological-cell-model-candidate-register-v1.json`, SHA-256 `3cd72bc3e3f71f26089b448d3ae6e80000a2d1156abc99cf95669a2f18a723be` |
| CD95-overexpressing HeLa average cell | `BIOMD0000000523.xml`, source commit `8605e43f8e2fd364f122d579341891c0058ef778`, SHA-256 `2afe6758ab396038e71fcb1716fefcfec67656b8bd0bfb3da8d4e1eda9524ff4` |
| wild-type HeLa average cell | `BIOMD0000000524.xml`, source commit `d091308a14fb4301a4a2b1b567ea874484bb97e6`, SHA-256 `4bf4a5bcda5b43a551bcdda09fca91a5e777d2c5db1eafcb17dcb6f1574221bc` |

The artifacts are SBML Level 2 Version 4 under CC0 1.0. Article, figure,
supplement, and experimental-data rights remain separate. The larger
`BIOMD0000000525`/`0526` pair is not part of this execution; it remains a
same-publication structural-sensitivity option for BCQ-1.5.

## Unit audit and mandatory guard

Both selected SBML artifacts omit model-level time, substance, volume, and
extent units. Their single `cell` compartment has size one but no declared
unit, and the species do not declare substance units. BioModels documentation
relates `CD95L = 16.6` to `500 ng/mL` and labels the converted value `16.6 nM`,
but that statement does not fill every missing SBML unit or prove the time-axis
unit.

BCQ-1.3 must therefore:

- execute every source number unchanged;
- call the independent variable `model_time` and its unit
  `unresolved-model-native`;
- call state values `unresolved-model-native`;
- perform no molar, SI, seconds, or minutes conversion; and
- retain the unconverted run if a later citable unit mapping is added.

This guard prevents a numerically successful import from becoming a falsely
precise biological interpretation. A converter's default display unit is not
evidence of the original model's intended unit.

## Frozen model cases

The two artifacts share one compartment, 18 species, 13 reactions, 12 global
parameters, one `CD95act` assignment rule, and the same kinetic parameters.
Only their source-defined initial values differ.

| Source value | `0523` | `0524` |
|---|---:|---:|
| CD95 | 116 | 12 |
| FADD | 93 | 90 |
| p55free | 155 | 127 |
| Bid | 236 | 224 |
| PrNES_mCherry | 973 | 1909 |
| PrER_mGFP | 5178 | 3316 |
| CD95L | 16.6 | 16.6 |

All other dynamic product or intermediate species begin at zero. The complete
18-species initial vectors and all 11 constant kinetic parameters are frozen in
the machine record. `CD95act` remains the source assignment:

```text
(CD95^3 * KDL^2 * CD95L)
-----------------------------------------------
(CD95L + KDL) * (CD95^2 * KDL^2
                  + KDR * CD95L^2
                  + 2 * KDR * KDL * CD95L
                  + KDR * KDL^2)
```

No value may be changed to improve a result.

## Frozen solver and execution matrix

The external engine is **COPASI command-line engine (CopasiSE) 4.46 Build
300**, release tag `Build-300`, source commit
`e9c47d912b55eccd56f70b72e52f19d61f5ab2e2`, under Artistic License 2.0.
COPASI remains an optional scientific reproduction tool; it is not introduced
as a MEHLISSA runtime dependency.

The deterministic time-course task uses LSODA without the reduced model.

| Setting | Relative tolerance | Absolute tolerance | Maximum internal steps |
|---|---:|---:|---:|
| primary and replay | `1e-9` | `1e-12` | 100,000 |
| tightened convergence run | `1e-11` | `1e-13` | 200,000 |

All six runs use the exact output grid `0` through `240` inclusive at `0.25`
model-time intervals, giving 961 rows. The matrix comprises primary, exact
replay, and tightened runs for each artifact. Optimization and parameter
estimation tasks must be disabled.

The 240-unit horizon is a frozen nominal reproduction domain, not a claim that
the experiment lasted 240 seconds or minutes. Changing it after seeing a curve
requires a versioned pre-outcome amendment and cannot erase this protocol or
its results.

## Primary observables and diagnostics

The four primary outputs are direct SBML species values; no normalization,
baseline subtraction, ratio, interpolation, or unit conversion is allowed:

| Species | Interpretation within the source mechanism |
|---|---|
| `PrER_mGFP` | uncleaved ER-localized mGFP reporter substrate |
| `PrNES_mCherry` | uncleaved NES-localized mCherry reporter substrate |
| `p43` | caspase-8 p43 cleavage intermediate |
| `p18` | active caspase-8 p18 species |

All 18 species must also be exported for conservation, nonnegativity, and
initial-state diagnostics. The observable list is selected from the source and
BioModels description before execution. It is not chosen from favorable
output.

## Acceptance rules

BCQ-1.3 passes source-artifact reproduction only if every unblocked rule passes
for both artifacts:

1. Source file hashes and commits match before import.
2. COPASI imports the unchanged expected structure without a fatal semantic
   error.
3. Each run contains exactly the 961 ordered grid points, all primary outputs,
   and finite numeric values.
4. Every initial value agrees with the frozen SBML value to absolute
   `1e-12`.
5. Parsed primary and replay values agree exactly at every grid point.
6. Primary and tightened results satisfy
   `abs(primary-tightened) <= 1e-8 + 1e-6 * max(1, abs(tightened))`.
7. Each source-derived conservation sum satisfies
   `abs(residual) <= 1e-10 + 1e-8 * max(1, abs(initial_total))`.
8. No species is less than `-1e-10` times the larger of one and the largest
   absolute source initial value for its artifact.
9. Both uncleaved reporter substrates are non-increasing within the declared
   numerical margin.
10. Quantitative publication-curve alignment stays **BLOCKED**, not failed and
    not passed, until a rights-compatible numeric reference is frozen.

The conservation diagnostics cover the FADD/DISC complex chain, the
procaspase-8 fragment family, Bid/tBid, both reporter-substrate/product pairs,
and constant CD95/CD95L inputs. Their exact species sets and initial totals are
machine checked.

A successful BCQ-1.3 result may say that the frozen public average-cell SBML
artifacts were executed independently in COPASI with reproducible, converged
trajectories and source-derived invariants. It may not say that a published
curve, population distribution, new experiment, or biological/clinical claim
has been independently validated.

## Required negative controls

Ten controls must demonstrate failure for source-hash drift, artifact or case
substitution, changed initial/kinetic/stimulus values, solver drift, invented
units, malformed output grids, non-finite or nonconservative output, enabled
fitting, and overclaiming. Negative controls operate on disposable copies or
synthetic result fixtures; they must never modify the frozen source artifact.

## Result archive and failure policy

Each attempt receives a non-overwriting UTC-and-protocol-hash run directory
under `results/bcq1/kallenberger-minimal/`. The archive must contain:

- the exact protocol snapshot and source manifest;
- solver binary/version/method/settings provenance;
- six raw trajectory tables and solver logs;
- reproduction metrics and machine plus human reports; and
- hashes covering the complete archive.

Started runs, failed imports, partial outputs, deviations, and failed controls
are retained. Raw article material and participant data are forbidden in this
archive. The executable BCQ-1.3 tooling must fail closed on any identity,
configuration, output, or claim inconsistency.

## Amendment rule

Any material change to source identity, model case, units, solver, numerical
settings, grid, observable, tolerance, control, archive, or claim must become a
new protocol version committed before inspecting output produced by the new
choice. It must explain the reason, name the affected decision, and keep this
version and every earlier result traceable. A post-outcome threshold relaxation
cannot be represented as if it were this prospective protocol.

## Execution handoff

BCQ-1.3 acquired the two content-hashed CC0 artifacts in a controlled
workspace, verified COPASI 4.46 Build 300, generated the six frozen runs,
exercised the negative controls, and published the complete pass/partial/
blocked/fail history. The external result does not add a MEHLISSA runtime
adapter; that begins in BCQ-1.4.

## Sources

- [COPASI 4.46 Build 300 release](https://github.com/copasi/COPASI/releases/tag/Build-300)
- [COPASI command-line documentation](https://copasi.org/Support/User_Manual/Model_Creation/Commandline_Version_and_Commandline_Options/)
- [COPASI deterministic time-course method documentation](https://copasi.org/Support/User_Manual/Methods/Time_Course_Calculation/Deterministic_Simulation/)
- [BioModels `BIOMD0000000523` source](https://github.com/biomodels/BIOMD0000000523)
- [BioModels `BIOMD0000000524` source](https://github.com/biomodels/BIOMD0000000524)
- [Kallenberger et al. 2014 article](https://pmc.ncbi.nlm.nih.gov/articles/PMC4208692/)
