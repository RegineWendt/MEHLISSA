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
- 286 local Windows/MSVC tests plus accepted Windows/MSVC, Linux/GCC,
  Linux/Clang, and macOS/Apple Clang CI; the Clang analysis path also covers
  formatting, static analysis, and sanitizers, while the macOS path covers the
  complete native ARM64 source build and test suite.

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
BCQ-1.2 must freeze solver, version, inputs, outputs, tolerances, controls, and
failure behavior before a developer executes, vendors, converts, or maps the
selected model. Keep any external SBML engine an optional scientific
reproduction dependency unless a later architecture decision establishes a
supported runtime contract.

Do not edit the locked protocol or raw archives in place. A changed protocol
requires a new version and pre-measurement commit; a changed candidate requires
a new candidate identity or an explicitly reviewed replacement before tagging.
