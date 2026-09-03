<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.8 Usability, Accessibility, Packaging, and Release Acceptance

**Status:** locally accepted on 3 September 2026. Publication acceptance is
complete only when the required GitHub Windows/MSVC, Linux/GCC, and
Linux/Clang-analysis checks pass for the published commit.

## Release outcome

UX-6.8 closes the planned UX-6 graphical-workbench program with **MEHLISSA
Research Workbench 1.0.0**. The release entry point integrates guided scenario
editing, corrective validation, guarded scenario and campaign execution,
reader-backed dashboards and comparison, provenance/evidence audit, and
descriptive campaign visualization/export.

This is a research-software release, not a scientific-validation or clinical-
readiness claim. It uses curated examples, synthetic inputs, and declared
parameter sweeps. It does not ingest patient records, infer patient-specific
parameters, diagnose disease, recommend treatment, or quantify population
uncertainty.

## Installation and packaging contract

The PEP 517 wheel contains:

- the `mehlissa` Python process and result-reader API;
- the `mehlissa_workbench` local host and browser assets;
- the `mehlissa-workbench` console entry point;
- the MPL-2.0 software licence.

The wheel deliberately does not embed the C++ executable, examples, schemas,
model metadata, evidence records, or campaign definitions. Those artifacts are
versioned together in the repository and remain the scientific authority. A
supported source installation therefore needs Python 3.10 or newer, a built
MEHLISSA executable, and the matching repository checkout. Windows/MSVC and
Linux/GCC or Linux/Clang are the CI-supported platform/toolchain combinations.

The acceptance test builds a wheel without downloading dependencies, inspects
its assets and licence, installs it into a newly created isolated virtual
environment, verifies `mehlissa-workbench --version`, imports packaged browser
resources, and executes `--check` against the built C++ application and
repository. The check must report Workbench 1.0.0, scenario editing, five
models, ten examples, a valid reference scenario, and one accepted run plan.

## Representative task review

| Role and task | Acceptance observation |
|---|---|
| novice researcher: understand the product | purpose, research-only boundary, maturity, evidence, and workflow are visible before execution |
| novice researcher: repair and run a scenario | schema-derived controls, located validation feedback, non-overwriting save-as, exact-plan confirmation, progress, dashboard, and retained artifacts form one route |
| expert researcher: execute and interpret a campaign | allowlisted campaign design, replicate/sweep/paired views, units, sample counts, exact values, source identity, and no-inference boundary remain together |
| reviewer: verify a result | authoritative artifacts, source licences, hashes, missing/altered states, software identity, seeds, assumptions, audit JSON, and analysis exports are accessible without rerunning |
| contributor: extend the system | the architecture guide, versioned interfaces, example workspace, requirements, tests, and ADR identify the C++/Python/browser boundaries and extension path |

## Keyboard and screen-reader checks

The automated semantic gate verifies an English page language, one primary
heading, unique identifiers, explicit names for all form controls, explicit
button types, no positive tab order, labelled dialogs, valid fragment targets,
three or more live regions, visible focus styling, narrow-viewport reflow, and
the absence of focus-outline suppression.

The task review additionally covers:

- skip-link navigation and logical `Tab`/`Shift+Tab` order;
- `Enter`/`Space` activation and `Escape` dialog dismissal;
- focus placement on the save filename and run-confirmation checkbox;
- named navigation, main, section, form, dialog, table, figure, status, and
  alert semantics in the accessibility tree;
- persistent text equivalents for status and charts, so color and geometry are
  not the sole information carriers;
- desktop and 390-pixel-wide operation, browser zoom, table overflow
  containment, and absence of global horizontal overflow.

These checks are a reproducible engineering baseline. They do not replace
future evaluation with a diverse group of researchers and people using their
preferred assistive technologies.

## Error recovery and safety review

- invalid values produce stable, located repair guidance and close save/run
  gates without discarding edits;
- validation and start operations repeat authority checks on the complete
  candidate rather than trusting browser state;
- paths, job identifiers, artifacts, logs, and outputs remain bounded by the
  server-owned allowlists and repository roots;
- cancellation retains its input and lifecycle evidence but creates no result
  observation;
- incomplete, failed, cancelled, malformed, or tampered results fail closed in
  dashboards, comparison, audit, and analysis;
- refresh or reconnect recovers retained run state; no remote service,
  telemetry, or external browser dependency is introduced;
- the host remains loopback-only and protected by an ephemeral capability.

## Acceptance evidence

UX-6.8 is locally accepted when all of the following are true:

1. the isolated wheel installation and packaged-resource smoke test passes;
2. the semantic accessibility contract and representative desktop/mobile
   browser tasks pass without console errors or global horizontal overflow;
3. the complete Windows/MSVC test suite, including all earlier M- and UX-gates,
   passes;
4. the English User Guide, Roadmap, architecture guide, requirements and
   traceability, status brief and generated PDF describe Workbench 1.0.0;
5. the example workspace explains both a novice single-run route and an expert
   campaign route;
6. the published commit passes all required GitHub CI jobs.

The first five criteria constitute local acceptance. Criterion six constitutes
publication acceptance and must be checked against the immutable commit, not
inferred from a prior run.

## Follow-up beyond UX-6

Further workbench changes are maintenance or new explicitly scoped UX packages.
The next product work should be driven by scientific expansion and external
user feedback: richer evidence-qualified models and responses, additional
organs, larger study designs, and later privacy-governed personalization. Every
future M-gate still requires a User Guide, Roadmap, architecture, status, and
release-impact review.
