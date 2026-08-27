<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0009: Machine-Readable Run Provenance

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M1; `DATA-003`, `SYS-002`, `SYS-007`, `UX-001`

## Context

An identical seed alone does not make a simulation run reproducible. The
experiment file, software revision, uncommitted changes, build, compiler,
platform, and reached simulation time can also affect the result. These details
must not depend on manual documentation and must support automated comparison
and archiving.

## Decision

1. Every successfully completed CLI run automatically writes `<outputs.directory>/provenance.json`.
2. The document follows a strict, semantically versioned JSON Schema using Draft 2020-12. The first version is `1.0.0`.
3. Provenance identifies the experiment and schema version and stores the path and SHA-256 hash of the manifest file actually read.
4. It records the MEHLISSA version, Git commit, dirty status, build type, compiler ID and version, operating system, and architecture.
5. It records the master seed, UTC start and end times, run status, and reached simulation time in integer nanoseconds.
6. Git and build information is embedded during configuration or compilation. A build outside a Git worktree explicitly uses `unknown` instead of invented values.
7. PicoSHA2 calculates the checksum as a small, reviewed header-only dependency obtained through the pinned vcpkg baseline.
8. Version and checksums of real models and data sets extend the same contract as soon as they are integrated in M2.

## Consequences

Positive:

- A result can be mapped automatically to its input manifest and software revision.
- Dirty builds are visible and are not silently represented as a published commit.
- The versioned schema provides a stable contract for later result archives, comparison tools, and the Python API.
- Standard-vector and schema tests verify hash calculation and document structure.

Negative:

- The configured Git state can become stale if sources are changed without rerunning CMake; normal builds rerun the CMake check when inputs change.
- UTC timestamps are observational data and therefore differ between otherwise identical runs.
- The first version does not yet cover external data or model catalogs.

## Alternatives

- **Human-readable log only:** rejected because structure, required fields, and automatic validation are missing.
- **Hash only:** rejected because it does not explain build and platform differences.
- **Git commit without dirty status:** rejected because a local build could then falsely appear to be an exactly reproducible commit.
- **Platform-specific cryptography API:** rejected because it would impede the portable build and identical checksum paths.
- **Provenance only during export:** rejected because run context could be lost or misassigned before then.
