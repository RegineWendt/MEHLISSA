<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0001: New Kernel and Legacy Strategy

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M0, M1; `SYS-005`, `SYS-006`, `QUA-003`

## Context

MEHLISSA 1.x extends ns-3 and contains valuable body, scenario, and
fingerprinting logic, but is tightly coupled and carries technical and domain
debt. MEHLISSA 2.0 removes ns-3 and significantly improves runtime, but remains
a small simulation kernel close to a single scenario. No historical version
fully implements the four layers described in the dissertation.

Continuing either implementation directly would make existing assumptions,
data formats, and class couplings the foundation of the new architecture.
Ignoring the legacy code completely would lose published reference runs, data
sets, and established rules.

## Decision

MEHLISSA Next receives a new, small, scenario-independent simulation kernel.
The `mehlissa/` and `mehlissa2.0/` directories remain available at the legacy
tag as scientific and technical references.

Code is ported selectively only when:

1. its domain behavior is understood and mapped to a requirement;
2. ownership and licensing permit adoption;
3. units, data dependencies, and assumptions are documented;
4. automated tests secure the intended behavior;
5. the port respects the new separation of layers and scenarios.

Historical result values are used for regression or comparison, not accepted as
truth without review.

## Consequences

Positive:

- The kernel and architecture can be reproducible and testable from the outset.
- ns-3 remains an optional communication adapter instead of a mandatory dependency.
- Published results and data remain available for comparison.
- Scenarios no longer have to dictate architectural decisions in the kernel.

Negative:

- Some work is duplicated in the short term.
- Historical functions become available only after a qualified port.
- Deviations from legacy results must be investigated systematically.

## Rejected alternatives

- **Modernize MEHLISSA 1.x directly:** too tightly coupled to ns-3 and scenario-specific classes.
- **Extend MEHLISSA 2.0 unchanged:** a better starting point than 1.x, but still lacks robust layer, data, and evidence contracts.
- **Complete greenfield start without legacy comparison:** would lose scientific traceability and valuable reference cases.
