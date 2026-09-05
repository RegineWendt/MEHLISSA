<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0051: macOS Apple Clang Source Build

- Status: Accepted
- Date: 2026-09-05
- Decision owners: MEHLISSA Next maintainers

## Context

MEHLISSA Next already builds and tests on Windows/MSVC, Linux/GCC, and
Linux/Clang. Collaborators also require a supported macOS development path.
The scientific application and local browser Workbench do not require native
desktop integration, while an application bundle would immediately add signing,
notarization, packaging, update, and multi-architecture release obligations.

The first macOS objective is therefore source portability and reproducible
compilation, not binary distribution. The build must exercise the same C++
application, Python process boundary, versioned repository artifacts, and full
CTest suite as the existing platforms.

## Decision

MEHLISSA provides a `macos-apple-clang` CMake configure preset and matching
`macos-apple-clang-debug` build and test presets. They use the Ninja generator,
the system `clang++`, C++20 without compiler extensions, warnings as errors, and
the repository's pinned vcpkg baseline.

GitHub Actions adds one required `macos-apple-clang` job on the explicitly named
`macos-15` standard runner. At the time of this decision that runner is ARM64
and uses Apple's Xcode toolchain. The job records the operating system,
architecture, compiler, CMake, and Ninja versions before configuring, building,
and running the complete suite. Naming the image explicitly avoids an
uncontrolled transition of `macos-latest` to another operating-system version.

The preset performs a native build and does not force a processor architecture,
so developers may use it on an otherwise compatible Intel or Apple Silicon Mac.
Only the ARM64 combination is CI-qualified by this decision. The Workbench
launcher recognizes the resulting command-line executable at
`build/macos-apple-clang/apps/mehlissa`.

No `.app` bundle, installer, universal binary, downloadable executable,
code-signing identity, notarization workflow, or automatic update mechanism is
part of this package.

## Consequences

- macOS contributors receive the same source-level simulator and Workbench
  workflow as Windows and Linux contributors.
- The full suite, including deterministic golden references and the isolated
  Python package check, becomes an Apple Clang portability gate.
- CI proves buildability and tested behavior on the named runner; it does not
  create or certify a redistributable macOS product.
- Intel macOS remains a reasonable native-source expectation but is not an
  accepted CI claim until separately tested.
- A future packaged macOS product requires a separate ADR covering architecture
  targets, signing, notarization, installation, updates, and release artifacts.

## Alternatives considered

- **`macos-latest`:** rejected because its operating-system mapping changes over
  time and weakens the reproducibility of the acceptance record.
- **Intel-only CI:** rejected as a declining platform with a time-limited hosted
  runner; Apple Silicon is the primary forward-looking development target.
- **Both ARM64 and Intel jobs immediately:** deferred because one complete macOS
  job meets the current collaboration need without doubling scarce macOS runner
  time.
- **A `.app` bundle now:** rejected because it adds distribution engineering but
  no scientific or simulation capability.

## Affected requirements and packages

- `QUA-007`
- Roadmap section 6.8
- CMake/vcpkg platform assurance
- Workbench executable discovery and installation documentation
