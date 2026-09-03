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
- Ninja on Linux
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

On Linux:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install .
mehlissa-workbench --version
mehlissa-workbench --repository-root . --check
```

After a successful check, `mehlissa-workbench --repository-root .` starts the
same local interface on either platform. The launcher searches the documented
Windows MSVC and Linux GCC/Clang build locations; use `--executable` only for a
non-standard build path.

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
- 286 local Windows/MSVC tests plus accepted Windows/MSVC, Linux/GCC, and
  Linux/Clang CI with formatting, static analysis, and sanitizers.

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
requirement coverage in the Traceability Matrix, selected status and Roadmap
section mappings, local links from the main guides, and obsolete terminology.
CI runs the same command.

## Paper 1 publication evidence

The optional `publication` dependency installs the pinned JSON Schema validator
used by the Paper 1 evidence, protocol, and release-candidate checks:

```powershell
python -m pip install -e ".[publication]"
python scripts/check_evidence_validity_matrix.py
python scripts/check_paper1_protocol.py
python scripts/check_paper1_release_candidate.py
python -m unittest tests.test_evidence_validity_matrix -v
python -m unittest tests.test_paper1_protocol -v
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

Do not edit the locked protocol or raw archives in place. A changed protocol
requires a new version and pre-measurement commit; a changed candidate requires
a new candidate identity or an explicitly reviewed replacement before tagging.
