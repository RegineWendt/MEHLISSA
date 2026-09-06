<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Developing MEHLISSA Next

This document describes the reproducible build of the new MEHLISSA generation.
The historical implementations in `mehlissa/` and `mehlissa2.0/` are not
changed by this build.

MEHLISSA Next is one integrated research platform with three complementary
aspects: the C++ scientific simulation stack, the Workbench/CLI/Python
research-use layer, and the schema/provenance/test/CI assurance layer. M0-M7 and
UX-1 through UX-6 describe how these capabilities were delivered; they are not
separate products or build lineages.

For the repository structure, public C++ APIs, coupling contracts, instructions
for adding modules, and the current personalization workflow, see the
[Software Architecture and Developer Guide](architecture/SOFTWARE_ARCHITECTURE.md).

## Prerequisites

- CMake 3.28 or newer
- a C++20 compiler
- vcpkg with `VCPKG_ROOT` set
- Ninja on Linux and macOS
- Python 3.10 or newer for the process API, Workbench, and release-package test

On Windows, Visual Studio Community 2026 with the “Desktop development with
C++” workload is the reference environment. Visual Studio Developer PowerShell
sets `VCPKG_ROOT` and the MSVC environment automatically.

## Windows with MSVC

In “Developer PowerShell for VS 2026,” from the repository root:

```powershell
$cmake = Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'

& $cmake --preset windows-msvc
& $cmake --build --preset windows-msvc-debug
& $ctest --preset windows-msvc-debug
```

The release build uses the `windows-msvc-release` presets accordingly.

## Integrated Python and Workbench layer

The Workbench 1.0.0 Python wheel contains the process API, versioned result
readers, local web host, browser assets, console entry point, and licence. The
C++ executable and repository-held schemas, models, evidence, examples, and
campaign definitions remain separate versioned inputs. This packaging boundary
prevents duplication of scientific authority; it does not make the Workbench an
optional or external product layer.

Install the local source into an isolated environment and verify that it can
find the accepted interfaces. On Windows PowerShell:

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install .
mehlissa-workbench --version
mehlissa-workbench --repository-root . --check
```

On Linux and macOS:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install .
mehlissa-workbench --version
mehlissa-workbench --repository-root . --check
```

After a successful check, `mehlissa-workbench --repository-root .` starts the
same local interface on all three operating systems. The launcher searches the
documented Windows MSVC, Linux GCC/Clang, and macOS Apple Clang build locations;
use `--executable` only for a non-standard build path.

The CTest named `mehlissa_python_ux6_release_package` independently builds the
wheel without dependency downloads, inspects its resources and licence,
installs it into a new temporary virtual environment, and runs the same check.
CI installs only the packaging tools before configuration; the package itself
has no mandatory third-party runtime dependency.

If vcpkg or the package registry is temporarily unavailable, a fully offline
smoke test is available:

```powershell
& $cmake --preset windows-msvc-smoke
& $cmake --build --preset windows-msvc-smoke
& $ctest --preset windows-msvc-smoke
```

This preset checks the kernel without an external test library. It does not
replace the complete Catch2 suite.

## Linux with GCC

```bash
cmake --preset linux-gcc
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

## macOS with Apple Clang

The supported macOS path is a native source build, not an application bundle.
Install the Xcode Command Line Tools, CMake 3.28 or newer, Ninja, Python 3.10 or
newer, and vcpkg. Homebrew is one convenient way to install CMake and Ninja:

```bash
xcode-select --install
brew install cmake ninja
```

Set `VCPKG_ROOT` to a pinned vcpkg checkout. From the repository root, a local
checkout matching the project baseline can be prepared with:

```bash
git clone https://github.com/microsoft/vcpkg.git .vcpkg
git -C .vcpkg checkout ddd0023b0eee70986e42ed49d9d4afb8098f212e
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
export VCPKG_ROOT="$PWD/.vcpkg"
```

Configure, build, and run the complete suite with:

```bash
cmake --preset macos-apple-clang
cmake --build --preset macos-apple-clang-debug
ctest --preset macos-apple-clang-debug
```

The command-line simulator is then available at
`build/macos-apple-clang/apps/mehlissa`. Install the Python package as described
above to use `mehlissa-workbench --repository-root .`. No `.app` package,
installer, code signing, or notarization is produced. The dedicated GitHub job
uses the pinned `macos-15` ARM64 runner image; the preset itself performs a
native build and does not hard-code the local Mac architecture.
The complete ARM64 source path is accepted by the Apple Clang job in
[CI run 33956456353](https://github.com/RegineWendt/MEHLISSA/actions/runs/33956456353).

## Analysis build with Clang

The `linux-clang-analysis` preset enables clang-tidy, AddressSanitizer, and
UndefinedBehaviorSanitizer:

```bash
cmake --preset linux-clang-analysis
cmake --build --preset linux-clang-analysis
ctest --preset linux-clang-analysis
```

## Dependencies

All direct C++ dependencies are declared in `vcpkg.json`. Its
`builtin-baseline` pins the tested vcpkg port revision. Dependencies are neither
installed globally nor copied manually into the repository.

The bootstrap uses Catch2 for unit tests. A small framework-free CTest smoke
test additionally keeps kernel invariants verifiable offline. Further libraries
are added only after a justified architecture decision.

`jsoncons` validates experiment files against the versioned JSON Schema. The
dependency is deliberately excluded from the offline smoke preset, which builds
only the general kernel without apps and experiment I/O.

`PicoSHA2` calculates the SHA-256 checksum of the loaded experiment manifest for
run provenance. Like `jsoncons`, this small header-only dependency is
reproducible through the pinned vcpkg baseline.

## Quality rules

- Builds occur only outside the source directories under `build/`.
- The build uses C++20 without compiler extensions.
- CI treats compiler warnings as errors.
- `clang-format` defines formatting; `clang-tidy` adds static analysis.
- Tests are executed through CTest.
- Controlled errors use stable `MEHLISSA-Edddd` identifiers; new meanings receive new codes instead of reinterpreting existing ones.
- The same experiment seed and stream name must produce the same raw random sequence.
- Simulation time is represented as integer nanoseconds and may advance only strictly monotonically.
- Model quantities use the dimension-safe SI types and named conversions from `core/quantity.hpp`; bare `double` values are not a public units API.
- Run-specific clock, seed, and RNG state belongs in `SimulationContext`; components are owned exclusively by `ComponentHost` and follow the documented lifecycle.
- New independently developed files carry `SPDX-License-Identifier: MPL-2.0`.
- Direct legacy ports remain `GPL-2.0-only` and are implemented in separate files with retained authorship and provenance information.
- New project documentation and approved original data use `CC-BY-4.0`; data additionally requires a provenance manifest.

## Current platform baseline

The accepted baseline comprises:

- a deliberately small neutral kernel with monotonic time, dimension-safe SI
  quantities, reproducible random streams, lifecycle ownership, and bounded
  execution;
- versioned experiment, schema, provenance, checkpoint, and structured logging
  contracts;
- independently replaceable body, lung, capillary, cell, Nano-IoT, BAN, and
  external-station implementations plus explicit cross-layer couplers;
- the complete M7 FP9/lung Levels A-E research-software demonstrator;
- CLI discovery, validation, scenario execution, reporting, and controlled
  campaigns;
- version-guarded Python process and result APIs, optional plots, and notebooks;
- MEHLISSA Research Workbench 1.0 for guided configuration, execution,
  dashboards, provenance/evidence audit, descriptive campaign analysis, and
  export; and
- 299 local Windows/MSVC tests in the current MRSQ-computationally-complete development branch;
  the published Workbench baseline retains its accepted 286-test suite across
  Windows/MSVC, Linux/GCC, Linux/Clang, and macOS/Apple Clang CI; the Clang
  analysis path also covers formatting, static analysis, and sanitizers, while
  the macOS path covers the complete native ARM64 source build and test suite.

Medical scenarios and legacy state do not belong in the kernel. Biological and
communication models depend on it as separate libraries. Browser code does not
own scientific schemas or result semantics: the Workbench delegates through the
accepted Python process boundary to the C++ executable and repository artifacts.

For API selection and extension work, continue with the
[public API map](architecture/SOFTWARE_ARCHITECTURE.md#8-public-api-map) and
[module extension workflow](architecture/SOFTWARE_ARCHITECTURE.md#9-how-to-add-a-module).

## Documentation consistency

Shared release facts, the current focus, Roadmap section mappings, selected
traceability expectations, and the ordered scientific qualification packages
are recorded in [`PROJECT_STATE.json`](PROJECT_STATE.json). It is a small
machine-readable consistency source, not a replacement for the explanatory
Roadmap or status report.

Run the documentation check before committing changes to project status,
requirements, navigation, or release evidence:

```powershell
python scripts/check_documentation_consistency.py
```

The check verifies shared facts in the canonical entry documents, one-to-one
requirement coverage in the Traceability Matrix, the separate implementation
and evidence status vocabularies and combinations, selected two-dimensional
expectations and Roadmap section mappings, local links from the main guides,
and obsolete terminology. CI runs the same command.

When changing a requirement, review both axes. Marking implementation `DONE`
does not authorize evidence `VERIFIED`: the latter requires the technical,
analytical, reference, inspection, or independent-data evidence declared by
that requirement. Record achieved evidence and the remaining qualification gap
in the same matrix row.

## Publication and scientific qualification evidence

The optional `publication` dependency installs the pinned JSON Schema validator
used by the Paper 1 evidence, protocol, release-candidate checks, and the active
pulmonary/capillary qualification design:

```powershell
python -m pip install -e ".[publication]"
python scripts/check_evidence_validity_matrix.py
python scripts/check_paper1_protocol.py
python scripts/check_pulmonary_capillary_qualification_protocol.py
python scripts/check_pulmonary_capillary_evidence_candidates.py
python scripts/check_pulmonary_capillary_preoutcome_amendment.py
python scripts/check_pulmonary_capillary_data_ingress.py
python scripts/check_pulmonary_capillary_uncertainty.py
python scripts/check_pulmonary_capillary_repository_audit.py
python scripts/check_biological_cell_model_candidates.py
python scripts/check_paper1_release_candidate.py
python -m unittest tests.test_evidence_validity_matrix -v
python -m unittest tests.test_paper1_protocol -v
python -m unittest tests.test_pulmonary_capillary_qualification_protocol -v
python -m unittest tests.test_pulmonary_capillary_evidence_candidates -v
python -m unittest tests.test_pulmonary_capillary_preoutcome_amendment -v
python -m unittest tests.test_pulmonary_capillary_data_ingress -v
python -m unittest tests.test_pulmonary_capillary_uncertainty -v
python -m unittest tests.test_pulmonary_capillary_repository_audit -v
python -m unittest tests.test_biological_cell_model_candidates -v
python -m unittest tests.test_paper1_measurement_tools -v
python -m unittest tests.test_paper1_release_candidate -v
```

The candidate checker verifies the exact source export, protocol, evidence
matrix, raw measurement archives, claim registry, SHA-256 inventory, positive
outcomes, negative controls, retained invalid setup attempt, and explicit
anti-overclaim boundaries. Regenerate `SHA256SUMS.json` only after an intentional
candidate change:

```powershell
python scripts/generate_paper1_checksums.py
```

The PCQ-1 design checker verifies the four pulmonary/capillary qualification tracks,
the six primary endpoints, no-refit and non-clinical boundaries, unique
negative controls, and SHA-256 hashes of the entering lung and capillary
candidates. Its design status cannot be changed to a successful evidence claim
without a successor protocol and actual qualification result. Numeric limits
for new primary evidence belong in a reviewed amendment committed before
validation outcomes are inspected.

The separate PCQ-1.2 candidate checker verifies the thirteen-source register,
track-specific rankings, access and participant-data rights, source overlap,
public aggregate-outcome exposure, rejected-primary decisions, anatomical
transit boundaries, and unsent external actions. It deliberately cannot turn a
source screen, public article, or draft request into physiological evidence.

The PCQ-1.3 amendment checker binds the chosen candidate-register and design
hashes to guarded source roles, eight anatomical/measurement observation
models, minimum samples, uncertainty floors, six primary numeric gates,
missingness and statistical policy, and explicit blocked outcomes. Its negative
tests prevent a small pilot from silently becoming a full track decision and
prevent the capillary-only residence time from being compared directly with a
whole-pulmonary transit measurement. Passing it proves prospective analysis
discipline, not physiological agreement.

The PCQ-1.4 ingress checker validates the policy and four outcome-blind
synthetic family adapters. Measured records are deliberately not discovered by
the repository: callers must provide an approved manifest, the exact data path,
and an absolute quarantine root outside the checkout. Authorization, privacy,
cohort disjointness, schema identity, and path containment are checked before
the data file is opened. The command emits metadata only. See the
[data-ingress guide](qualification/PCQ1_DATA_INGRESS.md) before adding a
source-specific converter; never weaken the normalized schemas to fit an
unknown or unauthorized source layout.

The PCQ-1.5 checker binds its strict plan to the PCQ-1.4 ingress hash, eight
immutable model assets, and the pulmonary/capillary implementation-source
hashes. It mirrors the accepted pulmonary equilibrium equations
using only frozen JSON values, checks v4/v7 aggregate equivalence over a fixed
flow-age grid, evaluates convergent signed local sensitivities, propagates
method floors through covariance envelopes, and verifies nine pre-calibration
design-matrix ranks. This is intentionally a dependency-light reference
analysis in the documentation job; the C++ suite remains the implementation
authority for the runtime equations. If either implementation changes, update
the model version and frozen hash rather than relaxing the checker.

Do not add invented probability distributions merely to enable a global
variance analysis. The current plan marks that analysis evidence-blocked until
joint distributions and correlations are source-backed. Likewise, do not
activate PCQ-C2 or turn synthetic design calculations into qualification
decisions. See the
[uncertainty and identifiability report](qualification/PCQ1_UNCERTAINTY_IDENTIFIABILITY.md)
for the endpoint methods, structural results, rank findings, and PCQ-1.6 handoff.

The BCQ-1.1 checker validates the ranked biological cell-model screen before
any third-party model is imported. It binds the minimal Kallenberger
`BIOMD0000000523`/`0524` source commits and SBML hashes, verifies the CC0 model
licence without extending it to the article or experimental data, keeps the
larger 525/526 pair in its non-independent structural role, and prevents the
unreleased endothelial BioModels submission from being treated as an audited
artifact. Passing BCQ-1.1 proves selection and provenance discipline only.
BCQ-1.2 now freezes solver, version, inputs, outputs, tolerances, controls, and
failure behavior before a developer executes, vendors, converts, or maps the
selected model. Its checker is:

```powershell
python scripts/check_biological_cell_model_reproduction_protocol.py
python -m unittest tests/test_biological_cell_model_reproduction_protocol.py
python scripts/check_biological_cell_model_reproduction_result.py
python -m unittest tests/test_biological_cell_model_reproduction_result.py
python -m unittest tests/test_biological_cell_model_reproduction_runner.py
python scripts/check_biological_cell_model_integration_protocol.py
python -m unittest tests/test_biological_cell_model_integration_protocol.py
python scripts/check_biological_cell_model_qualification_result.py
python -m unittest tests/test_biological_cell_model_qualification_result.py
python scripts/check_dynamic_capillary_tissue_cell_qualification_plan.py
python -m unittest tests/test_dynamic_capillary_tissue_cell_qualification_plan.py
```

The checked machine authority is
`data/qualification/biological-cell-model-reproduction-protocol-v1.json`; the
human rationale is in
[`BCQ1_REPRODUCTION_PROTOCOL.md`](qualification/BCQ1_REPRODUCTION_PROTOCOL.md).
The prospective replay amendment is
[`BCQ1_REPRODUCTION_PROTOCOL_AMENDMENT_1.md`](qualification/BCQ1_REPRODUCTION_PROTOCOL_AMENDMENT_1.md),
and the executed result is
[`BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md`](qualification/BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md).
Do not execute a different COPASI build, change LSODA tolerances, shorten the
grid, substitute initial conditions, or infer seconds/minutes/SI units from the
unit-incomplete SBML. Any material change requires a new protocol version
committed before output from the changed choice is inspected. Keep COPASI an
optional scientific reproduction dependency; BCQ-1.2 does not establish a
supported runtime contract or install it for Workbench users.

BCQ-1.3 has downloaded the two source artifacts into a controlled workspace,
verified their hashes before import, verified CopasiSE 4.46 Build 300, executed
all six declared runs, exercised all ten negative controls on disposable
copies, and retained failed/partial attempts. The original exact-zero replay
gate failed and remains failed. Amendment 1.1 was committed before a new run;
the authoritative archive then passed all nine unblocked computational gates.
Quantitative publication-curve alignment remains blocked until a rights-
compatible numeric reference is frozen.

COPASI remains optional. To generate a new non-overwriting archive from exact,
separately acquired source artifacts:

```powershell
python scripts/run_biological_cell_model_reproduction.py `
  --copasi "C:\path\to\CopasiSE.exe" `
  --source-523 "C:\path\to\BIOMD0000000523.xml" `
  --source-524 "C:\path\to\BIOMD0000000524.xml"
```

The runner fails before import on a source-hash mismatch and does not vendor
COPASI or the CC0 SBML files. The result checker needs only the checked-in
archive; it recalculates the numerical acceptance values from the raw CSV
files. BCQ-1.4–1.7 are now implemented separately under the frozen integration
protocol. The public API is
`mehlissa/models/cell/qualified_cd95_apoptosis_model.hpp`:

- `QualifiedCd95ApoptosisAdapter` is the no-refit M5 boundary;
- `KallenbergerMinimalMechanism` contains the separately reviewable 13 source
  reactions and assignment rule;
- `QualifiedCd95Request` requires an exact source case, `CD95L=16.6`, explicit
  `unresolved-model-native` semantics, and a bounded exact output grid; and
- `mehlissa_bcq1_qualification_runner` is a testing/qualification executable,
  not a general SBML or clinical interface.

Do not convert these states to SI, introduce parameter overrides into the
adapter, or substitute a population distribution. Changing source equations,
parameter identities, time semantics, or pass limits requires a new prospective
protocol version. The completed result and code-to-equation review are in
[`BCQ1_MEHLISSA_QUALIFICATION_RESULT.md`](qualification/BCQ1_MEHLISSA_QUALIFICATION_RESULT.md).
The result checker reconstructs all numerical claims from the committed CSVs
without invoking the result runner or requiring COPASI/SBML.

DCCQ means **Dynamic Capillary-Tissue-Cell Qualification**. DCCQ-1.1 through
DCCQ-1.6 are complete, while DCCQ-1.7 is partial only at the external-human-
review boundary. The parent plan authority remains
`data/qualification/dynamic-capillary-tissue-cell-qualification-plan-v1.json`;
the selected target and source authority is
`data/qualification/dynamic-capillary-tissue-cell-evidence-candidate-register-v1.json`.
The latter hash-binds the parent plan and records four ranked targets, ten
artifacts, five evidence sources, the source-to-SI bridge status, and separate
rights decisions. The selected target is human VEGF-A165a/VEGFR2 trafficking in
primary HUVECs, with NRP1 as an explicit structural choice. The prospective
equation authority is
`data/qualification/dynamic-capillary-tissue-cell-protocol-v1.json`; the
result authority is
`data/qualification/dynamic-capillary-tissue-cell-qualification-result-v1.json`.

Do not implement the new path by looping over
`CapillaryCellSignalCoupler::evaluate`: that contract is deliberately a
non-consuming uniform snapshot. Do not pass `unresolved-model-native` BCQ
states into SI transport, change the fixed CD95 stimulus, or map different
chemical identifiers merely to make the APIs connect. DCCQ-1.2 has now
selected and licence-screened the target and alternatives.
Do not copy or derive from the linked VEGFR-Trafficking-Projects source code:
its exact commit and relevant file hashes are recorded, but the repository has
no explicit licence. DCCQ-1.3 therefore independently specified the reduced
equations from the CC-BY article and Supporting Information and froze units,
parameters, NRP1 scope, synchronization, metrics, tolerances, observation
roles, and failure rules before the authoritative dynamic output. HUVEC evidence
must not be relabelled pulmonary evidence, and same-family HUVEC observations
must not be used as validation. See the [DCCQ-1 qualification
plan](qualification/DCCQ1_QUALIFICATION_PLAN.md) and [DCCQ-1.2 source
screen](qualification/DCCQ1_EVIDENCE_SOURCE_SCREEN.md), plus the [DCCQ-1
result](qualification/DCCQ1_QUALIFICATION_RESULT.md).

The public implementation API is
`mehlissa::models::cosimulation::DynamicCapillaryTissueCellModel`. Construct it
with a `DynamicCapillaryTissueCellParameters` and a
`DynamicCapillaryTissueCellInitialState`, then call `run(duration)` or
`advance_one_synchronization_interval()`. Each
`DynamicCapillaryTissueCellSnapshot` exposes the seven-owner
`DynamicLigandLedger`, receptor occupancy, the feedback multiplier used in the
completed interval, and the multiplier scheduled for the next interval.
`is_dynamically_balanced` independently checks the open-system invariant.

The reference factories `dccq1_reference_parameters()` and
`dccq1_reference_initial_state()` reproduce the frozen candidate. They are a
qualification reference, not a generic fitting interface. A different ligand,
cell context, receptor capacity, equation, source mapping, NRP1 assumption, or
acceptance threshold requires a new candidate and prospective protocol.
`mehlissa_dccq1_qualification_runner` writes a transparent CSV trajectory. The
Python qualification orchestrator executes the 41-case matrix and the result
checker reconstructs archive hashes, gate states, balance, replay and
convergence without invoking the C++ runner.

Run the DCCQ integrity checks with:

```powershell
python scripts/check_dynamic_capillary_tissue_cell_qualification_plan.py
python scripts/check_dynamic_capillary_tissue_cell_evidence_candidates.py
python scripts/check_dynamic_capillary_tissue_cell_protocol.py
python scripts/check_dynamic_capillary_tissue_cell_qualification_result.py
python -m unittest tests/test_dynamic_capillary_tissue_cell_protocol.py
python -m unittest tests/test_dynamic_capillary_tissue_cell_qualification_result.py
```

Do not edit the locked protocol or raw archives in place. A changed protocol
requires a new version and pre-measurement commit; a changed candidate requires
a new candidate identity or an explicitly reviewed replacement before tagging.

MRSQ means **Medical Reference Scenario Qualification**. MRSQ-1.1 is a
data-first selection step for the first complete injection-to-measurement
scenario. The candidate register ranks five alternatives and selects the
healthy-adult HedyPET dynamic total-body [18F]FDG PET evidence family for a
prospective protocol. It did not import participant files or make a validation
claim. The selection record intentionally separates
article, dataset, catalogue, processing-code, and fallback-source rights; in
particular, an unlicensed processing repository must not be copied merely
because the related data are advertised under CC-BY-4.0.

MRSQ-1.2 now freezes the exact public Hugging Face revision and six-object
minimum route, governance gate, source-disjoint construction/validation roles,
closed/conditional/audit tracks, cohort aggregation, four region definitions,
PET frame transformations, five primary endpoints, numeric tolerances, seven
uncertainty classes, missingness, 16 negative controls, and no-refit/amendment
policy. It explicitly excludes the full image collection and still forbids
participant CSV access. See the [MRSQ-1.1 scenario
selection](qualification/MRSQ1_SCENARIO_SELECTION.md) and [MRSQ-1.2 prospective
protocol](qualification/MRSQ1_PROSPECTIVE_PROTOCOL.md). Run their integrity
checks with:

```powershell
python scripts/check_medical_reference_scenario_candidates.py
python -m unittest tests/test_medical_reference_scenario_candidates.py
python scripts/check_medical_reference_scenario_protocol.py
python -m unittest tests/test_medical_reference_scenario_protocol.py
```

Passing these checks proves only that the choice and prospective protocol are
internally consistent. It cannot turn public metadata, a data licence, or a
planned observation model into external medical validation.

MRSQ-1.3 through MRSQ-1.7 now close the computational path. Run the full local
checks with:

```powershell
python scripts/check_medical_reference_scenario_data_ingress.py
python -m unittest tests/test_medical_reference_scenario_data_ingress.py
cmake --build --preset windows-msvc-debug --target mehlissa_fdg_pet_reference_runner mehlissa_fdg_pet_scenario_tests
build/windows-msvc/tests/Debug/mehlissa_fdg_pet_scenario_tests.exe
build/windows-msvc/scenarios/fdg_pet/Debug/mehlissa_fdg_pet_reference_runner.exe
python -m unittest tests/test_mrsq_cohort_evaluator.py
python scripts/check_medical_reference_scenario_closeout.py
python -m unittest tests/test_medical_reference_scenario_closeout.py
```

`MEHLISSA::fdg_pet_scenario` exposes `Administration`, `TissueKinetics`,
`CandidateParameters`, `Frame`, `FramePrediction`, `DecayReference`,
`source_disjoint_reference_candidate`, and `simulate` from
`scenarios/fdg_pet/include/mehlissa/scenarios/fdg_pet/fdg_pet_model.hpp`.
The candidate is deterministic RK4 and models a population blood input,
reversible two-tissue lung/liver/kidney kinetics, renal transfer, a one-way
bladder accumulator, an explicit fluorine-18 decay convention, and requested-
frame averaging. Add alternative kinetics by constructing a new parameter set
or a separately named implementation; never alter the frozen v1 candidate or
fit it to HedyPET outcomes.

The CSV ingress remains fail-closed for measured data. Authorization and
manifest validation occur before any source CSV is opened. A future prospective
amendment must record a local institutional determination, outside-repository
quarantine/retention/access approval, exact content SHA-256 values, and an
authoritative exact frame-duration mapping. Do not fetch participant files into
the checkout or infer durations from outcome curves. The cohort evaluator in
`scripts/mrsq_cohort_evaluator.py` is intentionally separate from access code
and currently qualified only on arbitrary values. See the [bounded MRSQ-1
result](qualification/MRSQ1_QUALIFICATION_RESULT.md).
