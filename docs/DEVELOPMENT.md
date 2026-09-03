<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Developing MEHLISSA Next

This document describes the reproducible build of the new MEHLISSA generation.
The historical implementations in `mehlissa/` and `mehlissa2.0/` are not
changed by this build.

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

## Workbench package and clean-install check

The Workbench 1.0 Python wheel contains the process API, versioned result
readers, local web host, browser assets, console entry point, and licence. The
C++ executable and repository-held schemas, models, evidence, examples, and
campaign definitions remain separate versioned inputs.

Install the local source into an isolated environment and verify that it can
find the accepted interfaces:

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install .
mehlissa-workbench --version
mehlissa-workbench --repository-root . --check
```

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

## Current bootstrap

The current system builds verifiable technical foundations around a deliberately
small neutral kernel:

- monotonic simulation time with nanosecond resolution and overflow protection;
- dimension-safe SI quantities and three-dimensional Euclidean geometry;
- named, reproducible random streams;
- versioned experiment validation and automatic run provenance;
- structured JSONL run logging and a versioned checkpoint manifest;
- validated vascular graphs, deterministic compartment transport, and schema-validated scientific reference reports;
- bounded transport observation and flow-conserving body-state profiles; and
- a versioned M6.1 nanodevice/profile contract with independently replaceable
  local messages, lifecycle, energy, storage, and count accounting; and
- an M6.2 neutral detection adapter and replaceable one-hop link with explicit
  delivery/drop results and communication-only latency, loss/error, byte, and
  energy metrics; and
- an M6.3 strict directed cluster with bounded fewest-hop/lowest-latency route
  selection, checked store-and-forward relays, and aggregate per-hop metrics;
  and
- an M6.4 active gateway with a neutral measurement publication boundary and a
  traceable command-to-local-control downlink.

Medical scenarios and legacy state do not belong in the kernel. Biological and
communication models depend on it as separate libraries.
