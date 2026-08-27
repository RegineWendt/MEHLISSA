<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0008: Versioned Experiment Manifest

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M1; `DATA-001`, `DATA-002`, `SYS-002`, `SYS-007`, `UX-001`

## Context

Reproducible MEHLISSA runs require a stable, machine-readable contract for
runtime, random seed, models, and outputs. Free-form or implicit configuration
would reveal invalid units, unknown fields, and unreconstructable defaults only
during simulation.

## Decision

1. Experiments are described as UTF-8 JSON documents.
2. Every document states a semantically versioned `schema_version`.
3. Version `1.0.0` is defined by a JSON Schema using Draft 2020-12.
4. The schema is strict: unknown fields are prohibited, required fields never disappear silently, and units are stated explicitly.
5. Time values consist of an integer value and a unit. Conversion to internal nanoseconds additionally checks for overflow.
6. The master seed is part of the manifest; named random streams are later derived from it deterministically.
7. A manifest is fully loaded and validated before simulation state is created or an output directory is changed.
8. `jsoncons` handles parsing and schema validation. The header-only BSL-1.0 dependency is obtained reproducibly through vcpkg.

## Consequences

Positive:

- Invalid configurations are rejected with context before the run.
- Other tools and the later Python API can use the same public schema.
- Schema versions enable controlled migration instead of silent semantic changes.
- The general kernel remains free of JSON and file-system dependencies; the function resides in a separate experiment library.

Negative:

- Every breaking change requires a new schema version and possibly a migrator.
- Contract tests must keep the schema and C++ decoding synchronized.
- The full build requires the additional jsoncons dependency; the offline smoke test still checks only the dependency-free kernel.

## Alternatives

- **Free-form JSON without a schema:** rejected because errors and unknown fields would be detected too late.
- **YAML:** better for large hand-written configurations, but has more complex type and parser semantics; it can later be added as a converting front end.
- **Custom JSON and schema parsing:** rejected because it would duplicate security- and maintenance-critical infrastructure without domain value.
- **Protobuf as the primary input format:** conceivable later for internal interfaces, but currently unnecessary for versionable, human-readable experiments.
