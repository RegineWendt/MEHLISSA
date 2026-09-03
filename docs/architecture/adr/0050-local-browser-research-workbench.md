<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0050: Local Browser Research Workbench

- Status: Accepted
- Date: 2026-09-03
- Decision owners: MEHLISSA Next maintainers

## Context

UX-1 through UX-5 provide stable command, report, campaign, and Python process
interfaces, but new researchers still need to understand paths, JSON structure,
and command syntax before completing an experiment. UX-6 needs a guided visual
workspace without creating a second simulation implementation or allowing a
desktop framework to become the scientific contract.

The first technical choice must support Windows and Linux, remain approachable
to international contributors, expose evidence and non-claims clearly, and
allow accessible task-based interaction. It must also be proportionate: the
initial product risk is workflow design and trustworthy round trips, not native
desktop integration.

## Decision

MEHLISSA will implement the first graphical research workbench as a local
browser application hosted by a Python process. The host delegates to the
accepted `python/mehlissa` process API, which in turn invokes the authoritative
C++ application. Browser code owns presentation and interaction only.

The host uses Python's standard library, binds only to loopback, serves an
explicit static-asset allow-list, and protects its API with an ephemeral
per-process capability. UX-6.1 exposed only read-only catalog discovery.
UX-6.2 adds a reviewed, bounded scenario workspace: the browser submits only
allow-listed scalar changes, the host retains the complete source object,
delegates candidate validation to the existing application, and creates only a
new file inside a repository-local workspace. Existing files are never
overwritten.

UX-6.3 adds a private validation operation. It rebuilds the complete candidate
from an approved source and allow-listed changes, delegates the validity
decision to `mehlissa scenario validate`, and returns located issues plus a
candidate SHA-256. Schema-derived browser feedback may improve correction but
does not replace that decision. Save repeats validation and future execution
must require an accepted candidate state.

The browser interface targets WCAG 2.2 level AA. No telemetry, cloud service,
remote asset, patient-data workflow, or clinical claim is part of the decision.

## Consequences

- The workbench reuses the existing executable and Python boundary; model and
  scenario behavior cannot diverge silently from command-line use.
- HTML provides a cross-platform, semantic, responsive presentation layer and a
  broad accessibility-testing ecosystem.
- UX-6.1 through UX-6.3 add no mandatory third-party runtime dependency to the
  Python package.
- Local HTTP introduces a security boundary. Loopback restriction, host-header
  validation, session capability, restrictive response headers, safe text
  insertion, and bounded file serving are requirements, not optional polish.
- Later packaging must locate Python and the MEHLISSA executable reliably or
  embed them without changing the workbench API boundary.
- The private prototype catalog response is not a general public REST API.
  Machine-readable editing and execution endpoints must be designed from the
  existing versioned schemas and commands rather than expanding it casually.

## Alternatives considered

- **Qt for Python/PySide6:** credible native option with mature widgets, but it
  adds platform binaries, packaging complexity, and licence/compliance review
  before the main workflows are proven.
- **Tauri 2:** potentially attractive as a later packaging shell, but it adds a
  Rust and platform-toolchain layer and would not replace the Python/C++ process
  boundary.
- **Electron:** strong ecosystem but an unnecessarily large Chromium/Node
  application and security surface for the bounded local research client.
- **Direct browser access to repository files:** rejected because browsers must
  not interpret local paths or reimplement schema and repository validation.
- **Native C++ GUI:** rejected for the first release because it couples product
  iteration to C++ UI development and offers no scientific-authority benefit.

## Affected requirements and packages

- `UX-002` through `UX-009`
- `UX-6.1` through `UX-6.8`
- `QUA-005`, `QUA-006`
