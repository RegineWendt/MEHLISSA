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

UX-6.4 adds execution as a bounded application service. Scenario starts repeat
authoritative validation; campaign starts accept only the reviewed UX-4
manifest. Both require explicit plan confirmation and create a unique
repository-contained evidence directory. The Python process API owns the child
process and supports explicit cancellation. Inputs, lifecycle state, bounded
command output, available scientific artifacts, failures, and cancellation are
retained. Artifact access is by a server-created name allowlist, never by a
browser-supplied path.

UX-6.5 adds result projection without adding analysis semantics to JavaScript.
UX-6.6 adds a server-owned audit projection: the host reads retained provenance
and versioned model artifacts, verifies declared SHA-256 identities, and returns
the complete source metadata plus explicit integrity/evidence states. The
browser presents and exports that projection but does not decide whether a hash,
licence declaration, evidence class, or clinical boundary is acceptable.
`RunWorkspace` loads completed scenario and campaign artifacts through the
accepted version-guarded Python result readers. It exposes private dashboard and
two-scenario comparison responses, while jobs outside `completed` state yield
zero observations and cannot enter comparisons. The authoritative JSON/CSV and
the generated UX-3 HTML report remain retained artifacts; HTML report preview
uses an authenticated fetch followed by a script-disabled sandboxed frame.

UX-6.7 adds a server-owned campaign-analysis projection. The host loads only a
completed aggregate with the accepted versioned campaign reader, exposes its
declared groups, observations, paired differences, and sensitivity hooks, and
calculates only transparent descriptive summaries. The browser renders and
exports those exact values; it does not estimate intervals or introduce an
independent statistical model. JSON, CSV, and SVG exports record the accepted
result hash, metric, unit, included sample count, and analysis contract version.
Replicate variation, deterministic sweep contrasts, and inferential uncertainty
remain explicitly different concepts.

The browser interface targets WCAG 2.2 level AA. No telemetry, cloud service,
remote asset, patient-data workflow, or clinical claim is part of the decision.

## Consequences

- The workbench reuses the existing executable and Python boundary; model and
  scenario behavior cannot diverge silently from command-line use.
- HTML provides a cross-platform, semantic, responsive presentation layer and a
  broad accessibility-testing ecosystem.
- UX-6.1 through UX-6.7 add no mandatory third-party runtime dependency to the
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

- `UX-002` through `UX-013`
- `UX-6.1` through `UX-6.8`
- `QUA-005`, `QUA-006`
