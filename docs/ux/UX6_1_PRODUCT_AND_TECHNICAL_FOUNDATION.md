<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.1: Product Scope and Technical Foundation

**Status:** locally accepted; publication and supported CI remain pending

**Date:** 3 September 2026

## 1. Decision summary

The first graphical MEHLISSA workbench will be a **local browser application
hosted by a small Python process**. It is a presentation client of the accepted
MEHLISSA command and Python process APIs. Scientific execution, validation,
schemas, provenance, and result semantics remain owned by the existing C++
application and versioned artifacts.

UX-6.1 proves this boundary with a read-only catalog browser. It lists the five
implemented model families and ten curated examples by calling
`MehlissaClient.list_models()` and `MehlissaClient.list_examples()`. It neither
reads the catalog JSON directly nor duplicates discovery or simulation logic.
The browser interface has no remote dependencies or telemetry.

The binding architecture decision is [ADR-0050](../architecture/adr/0050-local-browser-research-workbench.md).

## 2. Intended users and their first outcomes

| Role | Starting knowledge | Outcome the workbench must make easy |
|---|---|---|
| Scenario author or scientific researcher | Understands the research question, but should not need to know every JSON contract | Find a suitable model and example, construct a valid experiment, run it reproducibly, and interpret its evidence boundary. |
| Student or first-time user | May know neither the repository nor the MEHLISSA abbreviations | Understand what can be investigated, open a curated example safely, and see why a result is not a clinical prediction. |
| Biomedical model developer | Knows a domain model and its evidence | Inspect model parameters and provenance, add a model through the documented contracts, and compare it with an accepted reference. |
| Experimental partner or reviewer | Primarily evaluates data and claims | Inspect run inputs, hashes, evidence, limitations, uncertainty, and exportable results without changing the run. |
| Platform developer | Extends C++, Python, schemas, packaging, or the interface | Reuse one API boundary, reproduce failures, and test graphical workflows without introducing a second simulator. |

The primary UX-6 path is the scenario author. The other roles constrain
terminology, evidence display, accessibility, and auditability from the start.

## 3. Prioritized end-to-end workflows

1. **Discover and choose:** search model families and curated examples; compare
   maturity, evidence, limitations, and related artifacts. UX-6.1 implements
   the first read-only form of this workflow.
2. **Create or open safely:** start from a curated scenario or open a supported
   scenario; edit schema-derived fields; preserve unknown fields; save to a new
   file explicitly. This is UX-6.2.
3. **Understand and correct:** validate continuously through the authoritative
   contracts, locate every error, and obtain bounded corrective guidance. This
   is UX-6.3.
4. **Plan and run:** review the exact scenario, seed, destination, and campaign
   design before starting; monitor progress and retain failures or cancellation
   evidence. This is UX-6.4.
5. **Inspect and compare:** summarize one result or a campaign, compare paired
   runs, and drill through to authoritative JSON and generated reports. This is
   UX-6.5.
6. **Audit meaning:** keep hashes, model versions, sources, licences,
   limitations, and the non-clinical boundary adjacent to displayed outcomes.
   This is UX-6.6.
7. **Explore uncertainty:** visualize declared replicates, sweeps, paired
   differences, and sensitivity hooks with units and sample counts; export the
   exact plotted data. This is UX-6.7.
8. **Install and recover:** install, launch, complete representative novice and
   expert tasks, recover from errors, and use the interface with keyboard and
   assistive technology. This is UX-6.8.

## 4. First-release scope and explicit non-goals

The integrated UX-6 release is intended to support existing, versioned MEHLISSA
scenarios, campaigns, reports, and result readers. It may provide contextual
explanations, but an explanation must not silently change an input or create a
scientific inference.

UX-6 does not:

- implement numerical simulation, schema validation, campaign expansion, or
  result calculations in browser code;
- claim diagnosis, treatment guidance, patient prediction, medical-device
  status, or biological validation beyond the linked evidence;
- accept participant or patient data before the M8 governance, consent,
  pseudonymization, retention, and access-control work exists;
- make arbitrary repository files available through the local host;
- allow an unreviewed parameter path merely because a generic form can render
  it; or
- conceal a failed, missing, unsupported, or scientifically unqualified result.

## 5. Low-fidelity screen concepts

The designs below describe information hierarchy, not a fixed visual style.

### 5.1 Discover

```text
+ MEHLISSA Workbench --------------------------- [local] [read-only]
| What do you want to investigate?
| [ Search models, layers, or examples... ] [Layer: all]
|
| Model families                  Starter examples
| + Lung / organ ------------+    + FP9 complete ----------------+
| | maturity · evidence      |    | related models · source path |
| | limitations · details    |    | open a safe copy (later)     |
| +--------------------------+    +-------------------------------+
|
| Research demonstrator · not for clinical decisions
+----------------------------------------------------------------
```

### 5.2 Scenario workspace

```text
+ Scenario: FP9 lung reference ------------ Valid / 0 errors / 2 notes
| Models | Parameters | Evidence | Source JSON
| Body model       [Systemic vascular transport  v]
| Lung model       [Five-lobe pulmonary 0D       v]
| Collector count  [9]  devices  (range, source, limitation)
| Master seed      [1209]         (reproducibility explanation)
| [Save as...]                         [Review run plan]
+----------------------------------------------------------------
```

### 5.3 Run and campaign

```text
+ Run plan -------------------------------------------------------
| Scenario hash · models · seed · output directory · limitations
| [Start run]  Stage 3 of 8  [=========>       ]  [Cancel safely]
| Bounded log | retained manifest | retained failure evidence
| Campaign: replicates / sweep / same-seed paired comparison
+----------------------------------------------------------------
```

### 5.4 Results and provenance

```text
+ Result: completed ----------------------------------------------
| Outcome summary | Compare | Uncertainty | Provenance | Files
| Metric [unit]     baseline   variant   paired difference  n
| [accessible plot with table equivalent]
| Evidence class · model limits · hashes · seed · source artifacts
| [Open UX-3 report] [Export figure] [Export exact table data]
+----------------------------------------------------------------
```

## 6. Technology evaluation

| Candidate | Strengths for MEHLISSA | Costs and risks | Decision |
|---|---|---|---|
| Local browser UI plus Python standard-library host | Reuses the UX-5 process boundary directly; cross-platform browser rendering; semantic HTML and established accessibility testing; no new runtime dependency for the spike; straightforward notebook/Python ecosystem integration. | Local HTTP must be protected against network exposure, cross-site requests, unsafe file serving, and untrusted strings; later packaging must make Python and executable discovery dependable. | **Selected.** Start with a deliberately small loopback host and expand the API only with reviewed workflows. |
| Qt for Python / PySide6 | Mature native desktop widgets, accessibility interfaces, and one-language Python host. | Adds sizeable platform binaries and deployment work; Qt for Python is offered under LGPLv3/GPLv3 or commercial terms, which requires packaging and compliance review. | Not selected for the first workbench; reconsider if native integration becomes a demonstrated requirement. |
| Tauri 2 | Small webview-based desktop shell with a capability model and native packaging potential. | Adds Rust and platform-specific build dependencies; Windows development depends on Microsoft C++ Build Tools and WebView2. It would create a second host layer before workflows are stable. | Deferred as a possible later packaging shell, not as the scientific API. |
| Electron | Large ecosystem and consistent Chromium/Node behavior. | Bundles a Chromium/Node application stack and requires careful main/renderer isolation, navigation control, and dependency security. This is excessive for the present bounded local client. | Rejected for the initial workbench. |

Primary technology references:

- Python [`http.server`](https://docs.python.org/3/library/http.server.html) and
  [`webbrowser`](https://docs.python.org/3/library/webbrowser.html)
- [Qt for Python](https://doc.qt.io/qtforpython-6/) and its
  [accessibility overview](https://doc.qt.io/qtforpython-6.8/overviews/qtdoc-accessible.html)
- [Tauri prerequisites](https://v2.tauri.app/start/prerequisites/) and
  [architecture](https://v2.tauri.app/concept/architecture/)
- Electron [process model](https://www.electronjs.org/docs/latest/tutorial/process-model)
  and [security guidance](https://www.electronjs.org/docs/latest/tutorial/security)
- W3C [Web Content Accessibility Guidelines (WCAG) 2.2](https://www.w3.org/TR/WCAG22/)

## 7. Technical boundary of the UX-6.1 prototype

```text
Browser HTML/CSS/JavaScript
        | GET /api/catalog + ephemeral session capability
        v
Python loopback host (mehlissa_workbench)
        | MehlissaClient.list_models / list_examples
        v
MEHLISSA executable discovery commands
        | validates catalog and repository references
        v
Versioned model catalog, model cards, examples, and licences
```

The Python adapter parses the stable human-readable discovery rows into the
prototype response `api_version=1.0.0`. This response is deliberately private
to the workbench prototype; it is not a promise to external clients. UX-6.2
must prefer existing schemas and add explicit machine-readable application
commands where round-trip editing needs more structure.

## 8. Threat, privacy, and safety baseline

| Threat or failure | UX-6.1 control | Later obligation |
|---|---|---|
| Accidental network exposure | Host accepts only `127.0.0.1` or `localhost`; binding to `0.0.0.0` is rejected. | Packaging tests must retain loopback-only defaults and make any future remote mode a separate reviewed product decision. |
| Cross-site request or DNS-rebinding attempt | Each process creates a high-entropy session capability; the API requires it in a header; host headers are allow-listed; responses are `no-store` and `no-referrer`. | State-changing endpoints require explicit origin, capability, method, and user-confirmation design before implementation. |
| Cross-site scripting through catalog text | Browser content is created with `textContent`; no `innerHTML` is used; Content Security Policy allows only same-origin assets. | Add adversarial fixtures for every future rich text or file preview. |
| Path traversal or arbitrary file disclosure | Four static assets are served from an explicit allow-list; unknown and encoded traversal paths return `404`. | Scenario/result file access must use reviewed roots or an operating-system file picker and must never become a generic file server. |
| Session capability disclosure | Capability is generated in memory, removed from browser history immediately, excluded from referrers and server logs, and dies with the process. | Do not persist it, include it in reports, or reuse it across launches. |
| Unbounded work or output | UX-6.1 exposes discovery only and performs no simulation. | Runs and campaigns must retain existing limits, cancellation evidence, bounded logs, and non-overwriting outputs. |
| Misinterpretation as clinical software | The interface permanently states that it is a research demonstrator and not validated for clinical decisions. | Evidence and non-claims must stay visible beside future results, not only in an about screen. |

Privacy baseline:

- no telemetry, analytics, account, cloud service, or remote font is used;
- catalog data stays on the local machine;
- UX-6.1 accepts no personal, participant, or patient data and stores no UI
  state; and
- any future participant-specific workflow is blocked until the M8 governance
  requirements are satisfied.

This baseline is defensive design, not a claim that the prototype has passed a
professional penetration test or medical-software security assessment.

## 9. Accessibility baseline

WCAG 2.2 level AA is the target for the workbench interface. UX-6.1 establishes:

- semantic landmarks, headings, labels, table headers, and status messages;
- a keyboard-accessible skip link and native search/select controls;
- visible keyboard focus, non-color status text, high-contrast foregrounds,
  responsive reflow, and a horizontally scrollable data table at narrow widths;
- no essential motion, audio, timed interaction, or pointer-only control; and
- concise international English without assuming milestone abbreviations.

Automated accessibility checks cannot prove usability. UX-6.8 therefore retains
manual keyboard, screen-reader, zoom/reflow, contrast, error-recovery, novice,
and expert task reviews as release evidence.

## 10. Acceptance evidence

The UX-6.1 increment is locally accepted because:

- the product scope, roles, workflows, screen concepts, technology choice,
  threat baseline, privacy baseline, and accessibility target are recorded;
- the workbench lists five model families and ten starter examples through the
  existing `MehlissaClient` discovery methods;
- unit/integration tests cover successful discovery, malformed output,
  loopback binding, session enforcement, read-only methods, static security
  headers, and traversal rejection;
- all 285 local Windows/MSVC tests pass against the current Debug build;
- browser review confirms correct content, functioning search and layer
  filters, semantic structure, responsive single-column layout without page
  overflow, and no console warnings or errors; and
- the application contains no simulation or independent catalog-validation
  implementation.

Cross-platform CI is intentionally not claimed yet. The current changes are
local until the repository owner explicitly requests a push. UX-6.1 becomes
published acceptance evidence only after that push and the supported GitHub CI
jobs complete successfully.
