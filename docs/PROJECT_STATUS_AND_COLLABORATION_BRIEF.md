<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Next

## Project Status and Collaboration Brief

**Purpose:** Shareable overview for prospective contributors and research partners
**Status date:** 2 September 2026
**Development branch:** `mehlissa-next-generation`
**Base revision:** `43da0dee5a73d4515e0bd0b02cae464605daadc4`
**Milestone status:** M0 through M7 passed
**Current product focus:** UX-6 graphical research workbench

This Markdown file is the maintainable source for the shareable PDF at
`output/pdf/MEHLISSA_Next_Project_Status_and_Collaboration_Brief.pdf`.
Roadmap milestones and post-M7 user-experience packages must update this source
and regenerate and visually verify the PDF.

From the repository root, regenerate it with:

```powershell
python scripts/generate_project_status_pdf.py
```

## Executive summary

MEHLISSA Next has progressed from a set of valuable historical prototypes to a
coherent, modular research simulation platform. The project now implements the
complete architectural path envisioned for the first dissertation-driven
demonstrator: systemic transport, a replaceable lung model, capillary transport
and molecular channels, receptor and intracellular cell response, nanodevice
communication, an active gateway, a body area network, and external analysis.

**Key achievement:** All major layers can now participate in one reproducible
FP9/lung fingerprinting workflow while remaining independently replaceable
behind explicit contracts.

- Milestone gates M0 through M7 have passed their documented acceptance reviews.
- The implementation uses C++20, CMake, vcpkg, strict JSON Schemas, typed SI
  quantities, stable error contracts, deterministic random streams, and
  explicit provenance.
- The base revision passes all 284 local Windows/MSVC tests and GitHub
  Windows/MSVC, Linux/GCC, and Linux/Clang CI. The Clang path also passes
  formatting, clang-tidy, AddressSanitizer, and UndefinedBehaviorSanitizer.
- UX-2 adds a validated catalog of five model families and ten curated starter
  configurations with safe licensed copying and repository integrity checks.
- UX-3 adds a non-overwriting six-file result bundle with concise text,
  stable CSV exports, dependency-free HTML, evidence, limitations, the clinical
  non-claim, and the complete machine-readable JSON.
- UX-4 adds strict derived-experiment campaigns: retained scenario
  manifests, deterministic replicate and same-seed pair plans, bounded
  collector-count sweeps, hashed aggregate JSON, and analysis-ready CSV.
- UX-5 adds a standard-library Python process client, version-guarded
  result readers, campaign grouping and same-seed differences, optional plotting,
  and two licensed starter notebooks. The grouped UX-3 through UX-5 acceptance
  passes all 284 tests on supported CI paths in GitHub run 33668850496.
- The English User Guide, software architecture guide, model documentation,
  traceability matrix, and architecture decision records support international
  collaboration.
- The result is a reproducible research-software demonstrator. It is not a
  clinical assay model, a medical device, or a patient-specific digital twin.

| Dimension | Assessment |
|---|---|
| Software architecture | Strong modular foundation with explicit ownership, conservation, lifecycle, configuration, and evidence boundaries. |
| End-to-end integration | Complete first software vertical slice through fingerprinting Levels A-E. |
| Scientific validation | Mixed maturity: verified equations, literature-parameterized candidates, selected independent comparisons, and multiple synthetic mechanisms. |
| User experience | UX-1 exposes the complete M7 demonstrator, UX-2 provides discovery, UX-3 provides readable shareable result bundles, UX-4 provides controlled campaigns, and UX-5 provides Python and notebook access. A graphical workbench remains. |
| Clinical readiness | Not claimed. Patient prediction, diagnosis, and treatment recommendations are explicitly outside the present scope. |

## How to read project labels

Short identifiers preserve traceability across requirements, code, tests, and
evidence. They are not assumed knowledge in this report.

| Label | Plain-language meaning |
|---|---|
| M0 through M8 | Sequential milestone gates. M7 is the accepted complete fingerprinting software demonstrator. The future M8 gate concerns a governed research digital twin. |
| M7.4 | A numbered increment within a milestone. M7.4 adds concentration- and receptor-binding-based fingerprint detection. |
| Level A | Historical-timing baseline for the fingerprinting scenario; it reproduces published dissertation event semantics and is not a separate “A run”. |
| Level B | Mechanistic fingerprint detection based on concentration, receptor binding, and a threshold. |
| Level C | Explicit release and complete/incomplete assembly of nine fingerprint tiles. |
| Level D | Executed local, gateway, body-area-network, and external-station communication. |
| Level E | Sensitivity, specificity, false-positive/false-negative, robustness, and misclassification analysis. |
| P0 through P3 | Roadmap priority tiers: indispensable foundation, dissertation core, research-platform expansion, and long-term vision. They are not completion states. |
| UX-1 through UX-6 | Ordered post-M7 user-experience packages, from one-command execution to a graphical research workbench. |
| run | One reproducible execution of an experiment or scenario. A qualified term such as “baseline run” must state what makes that execution distinct. |

`FP9` is the nine-part molecular fingerprint used by the first complete
scenario. `BVS` is the historical BloodVoyagerS whole-body distribution
reference. `BAN` is the body area network between gateway and external station.
`ODE` denotes an ordinary differential-equation model, and `SSA` denotes a
stochastic simulation algorithm.

## 1. Conceptual system architecture

MEHLISSA separates the research question from the models used to answer it. A
scenario selects versioned model cards, initializes independent components,
coordinates explicit transfers and events, and produces bounded results with
provenance. This permits a fast surrogate, population model, field model, or
detailed particle model to be compared without embedding medical special cases
in the kernel.

The implemented conceptual path is:

1. **Body:** systemic circulation and whole-body transport.
2. **Lung or another organ:** replaceable organ-specific models.
3. **Capillary bed:** local transit, exchange, entity fate, and molecular channels.
4. **Cell:** receptor binding, intracellular signaling, delivery, and response.
5. **Nano-IoT:** nanodevice endpoints, local links, bounded relays, and gateway.
6. **BAN and station:** external measurement and a governed return command.

Binding engineering principles are:

- **Layer independence:** body, organ, capillary, cell, and communication
  models live in separate libraries.
- **Replaceable resolution:** model variants share stable boundaries while
  documenting their own validity scopes.
- **Explicit ownership and conservation:** identities and physical amounts
  cannot silently disappear or be duplicated at a boundary.
- **Dimension-safe internals:** public numerical APIs use typed quantities;
  files state units explicitly.
- **Reproducibility:** runs bind time, seed, named random streams, model
  versions, hashes, logs, and checkpoints.
- **Strict configuration:** versioned JSON Schemas and semantic loaders reject
  unknown or inconsistent inputs.
- **Evidence before claims:** verification, calibration, independent
  validation, historical regression, and sensitivity analysis remain distinct.
- **Bounded scale:** populations, aggregates, and record limits prevent
  uncontrolled output growth.

Independent MEHLISSA Next code is licensed under MPL-2.0. Legacy code and
direct ports remain GPL-2.0-only. New original documentation and approved
original data are licensed under CC BY 4.0. Artifact-specific provenance and
license sidecars prevent unclear data rights from entering public packages.

## 2. Milestones achieved: M0-M3

| Gate | Plain-language result | Representative capabilities |
|---|---|---|
| M0 | Project charter and architecture decisions | Dissertation requirements and traceability; new-kernel decision; four-layer architecture; lung selected as first organ; licensing, data inventory, users, workflows, validation gaps, and partner roles. |
| M1 | Trustworthy simulation kernel | Monotonic nanosecond time; 3D position; typed SI units; named deterministic random streams; component lifecycle and ownership; experiment manifests; provenance; structured errors and JSONL logs; checkpoints; byte-stable cross-platform reference. |
| M2 | Validated body transport layer | Schema-validated 95-segment vascular graph; approved legacy migration; graph and vessel-9 branching invariants; injection, extraction, and identity-preserving transport; bounded observations; rest, exercise, and posture overlays; 6,359/63,590-agent BVS regression. |
| M3 | Body-organ coupling and lung reference | Typed body-organ transfers; coarse, serial-region, pulmonary 0D, flow-adaptive, age-conditioned, pressure-distensible, and five-lobe parallel-bed models; source-disjoint validation paths; same-scenario resolution comparison; historical FP9 timer baseline. |

M0-M3 establish that a medical scenario no longer needs its own simulation
engine or vascular implementation. The kernel remains generic, a vascular
graph can change without recompilation, and a caller can select a lung model at
a declared resolution while preserving the meaning and ownership of
transported entities and quantities.

The pulmonary models include literature-parameterized and independently
evaluated endpoints, but remain zero-dimensional or equivalent representations.
They do not yet provide anatomical one-dimensional flow, pulsatile pulmonary
mechanics, patient imaging, or a complete jointly measured validation cohort.

## 3. Milestones achieved: M4-M7

| Gate | Plain-language result | Representative capabilities |
|---|---|---|
| M4 | Capillary communication layer | Executable arteriole-capillary-venule route; geometry and continuity closure; dynamic recruitment; balanced blood/endothelium/interstitium/cell exchange; residence and terminal disposition; analytical diffusion, Brownian endpoint, trajectory, radial finite-volume, and shared axial advection-reaction models. |
| M5 | Cell response layer | Analytical and time-varying receptor-ligand binding; stochastic finite-receptor SSA; capillary-to-cell signal hand-off; shared intracellular ODE/SSA network; conservative device release and uptake; synthetic apoptosis event; cohort-compressed populations up to one trillion represented cells. |
| M6 | End-to-end Nano-IoT plane | Nanodevices and local messages; one-hop delivery outcomes; bounded routing and relays; active gateway uplink/downlink; BAN and governed station loop; payload-free external network-simulator adapter; twelve fail-closed resilience and boundary-misuse cases. |
| M7 | Holistic fingerprinting vertical slice | Strict selection of 13 scenario artifacts; ten-stage causal contract; typed runtime; identity trace; artifact SHA-256 hashes; concentration-driven binding and negative control; nine FP9 tiles and incomplete control; locator-to-station communication; Level-E classification metrics and Wilson intervals; deterministic result schema 2.0.0. |

In one sentence: MEHLISSA can now execute a reproducible software workflow in
which a configured multilayer stack produces a biological detection event,
assembles an explicit fingerprint, communicates it through local and external
boundaries, and reports the outcome with complete artifact identity and
declared uncertainty limitations.

Positive, negative, invariant, numerical, integration, regression, and schema
tests cover all accepted milestones. All compiler and analysis CI jobs pass.
Gate reviews explicitly prevent a software pass from being presented as
physiological or clinical validation. The English User Guide is mandatory
review evidence at every future milestone and UX-package review.

## 4. How MEHLISSA can be used today

| Access route | Suitable uses | Current examples |
|---|---|---|
| Command line | Direct use without writing C++ | Validate manifests and vascular graphs; run the minimal workflow and BVS regression; apply body-state overlays; validate and execute the complete M7 demonstrator. |
| Reference and benchmark drivers | Reproduce accepted scientific and numerical comparisons | Pulmonary validations; coarse-versus-five-lobe comparison; molecular-channel comparisons; historical FP9 timing; benchmark campaigns. |
| C++ component APIs | Compose or extend advanced research workflows | Organ-capillary round trip; cell and communication models; external simulator adapter; direct access to the same M7 runtime used by the CLI. |
| Automated test suite | Audit contracts and reproduce gate evidence | Focused M1-M7 CTest filters, negative controls, deterministic replay, conservation, and identity checks. |

Representative research questions include:

- How does injection site or physiological state affect systemic distribution
  and transit?
- Which outputs change when a coarse lung surrogate is replaced by five
  parallel pulmonary lobe beds?
- How do binding, intracellular signaling, device release, uptake, and a
  synthetic cell response connect without violating amount ownership?
- How do loss, corruption, expiry, replay, routing errors, or station-policy
  rejection affect a Nano-IoT measurement path?
- Can the complete FP9/lung workflow preserve causal identity, report
  incomplete fingerprints, and compute classification metrics reproducibly?

The complete M7 scenario can now be discovered, validated, executed, and
summarized through the normal application:

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe scenario list
build/windows-msvc/apps/Debug/mehlissa.exe scenario run --file examples/scenarios/fp9-lung-level-a-v1.json --output results/fp9-reference
```

The separate `scenario validate` command checks the same inputs without
executing them. The output argument is a parent directory; each run creates a unique
UTC-labelled child containing `result.json`, `provenance.json`,
`run.log.jsonl`, and `summary.txt`; the application prints the exact path. An
existing result can be summarized with `mehlissa result summarize --file
<result.json>`. The command reuses the accepted M7 composer and holistic
runner, validates all thirteen selected definition/schema pairs before
execution, and preserves the non-clinical interpretation boundary.

UX-2 adds discovery without creating a second source of scientific truth:

```text
mehlissa model list
mehlissa model describe --id organ.pulmonary-zero-dimensional
mehlissa example list --model organ.pulmonary-zero-dimensional
mehlissa example copy --id scenario.fp9-complete --output work/fp9-study
```

The strict versioned catalog describes five model families and ten licensed
starters, including maturity, validity, evidence, limitations, key parameter
paths, artifacts, and documentation. Checks reject duplicate IDs, unknown model
references, missing assets, and paths outside the repository. Copying preserves
the license and refuses to overwrite work. All 281 local Windows/MSVC tests and
all supported GitHub CI jobs pass.

## 5. Current personalization capability

MEHLISSA supports parameterized research profiles, but not a complete virtual
person. A contributor can derive a versioned model card, state whether values
are measured, literature-derived, calibrated, assumed, or used only for
sensitivity analysis, and preserve units, uncertainty, provenance, and
limitations.

| Scope | Available now | Not yet available |
|---|---|---|
| Body | Load any schema-valid vascular graph; apply compatible cardiac-output and transition overlays. | Production import of segmented personal anatomy; complete pressure/compliance and disease physiology. |
| Lung | Configure pressure boundary, resistance, compliance, flow, transit, age band, distensibility, and lobe shares. | Automatic parameter inference; geometry-resolved patient lung; complete personal validation. |
| Capillary | Configure equivalent geometry, flow, recruitment, exchange, observation, and disposition. | Patient microvascular anatomy, hematocrit/rheology, qualified kinetics, and spatial tissue coupling. |
| Cell | Configure channel, receptor, reaction, delivery, threshold, response, and cohort parameters. | Patient-specific biomarker calibration, mechanistic apoptosis, and disease-specific validated families. |
| Whole run | Fix seeds, versions, checksums, output paths, and interpretation limits. | Canonical patient manifest, automatic cross-layer composition, parameter estimation, uncertainty propagation, and longitudinal updates. |

Identifiable patient data must not be committed to the repository. Clinical or
participant-specific work requires institutional approval plus explicit
consent, pseudonymization, retention, access-control, and provenance workflows.
The future M8 Research Digital Twin gate still forbids clinical decision claims
without an appropriate regulatory and validation path.

## 6. Scientific limitations after M7

| Area | What is demonstrated | What remains unproven |
|---|---|---|
| Cross-layer timing | One deterministic causal workflow with historical timing evidence. | Anatomically resolved localization and collector-return timing predicted by the coupled models. |
| Fingerprint biology | Concentration-driven receptor detection and explicit target identity. | Qualified FP9 gene-product combination, disease-marker data, and biological parameter validation. |
| Tile assembly | Nine explicit tile identities, an all-required rule, and incomplete negative control. | Executed NetTAS or molecular self-assembly physics and independently validated assembly kinetics. |
| Communication | Replaceable links, routing, gateway, BAN, station, metrics, and external-simulator boundary. | Calibrated molecular, intrabody, wearable, Bluetooth, or IEEE 802.15.6 behavior. |
| Classification | Correct sensitivity/specificity accounting, labelled cases, and Wilson intervals. | Empirical prevalence, clinical sensitivity/specificity, patient variability, and diagnostic utility. |
| Physiology | Literature-scoped lung models and selected independent endpoint comparisons. | Jointly measured cohorts, broad disease states, pulsatility, anatomy, hematocrit, and full uncertainty propagation. |
| Therapeutic response | Conservative release/uptake and a bounded synthetic cell-response event. | Mechanistic pharmacology, spatial drug transport, metabolism, toxicity, and treatment efficacy. |

**Interpretation rule:** A verified software contract shows that the declared
mechanism is implemented consistently. It does not, by itself, show that the
mechanism predicts a biological or clinical outcome.

## 7. Remaining development program

| Program | Principal work | Intended outcome |
|---|---|---|
| Usability and orchestration | UX-1 provides scenario execution, UX-2 provides discovery, UX-3 provides HTML/text/CSV result bundles, UX-4 provides controlled campaigns, and UX-5 provides Python readers, analysis, and notebooks; a workbench remains. | A researcher can find suitable models, run M7, inspect or share its result, execute reproducible parameter studies, and analyze them from Python without C++ knowledge or manual JSON copying. |
| Scientific qualification | Replace historical or synthetic assumptions with measured parameters, calibration data, independent validation, uncertainty, and sensitivity campaigns. | Defensible scenario-specific conclusions within explicit scopes. |
| Additional medical scenarios | Continuous monitoring, liquid biopsy, endocrine/adrenal venous sampling, CAR-T, and ultimately metastasis prevention. | Evidence that the architecture generalizes beyond fingerprinting. |
| Additional organs | Begin with a kidney surrogate and progress toward regional filtration, clearance, and organ-specific capillary interfaces. | Evidence that organ factories and coupling contracts are not lung-specific. |
| Personalization and digital twin | Patient manifest, imaging and SimVascular/CFD import, parameter estimation, identifiability, longitudinal updates, and governance. | Research-twin maturity from geometry to physiology and biochemistry. |
| Scaling and HPC | Bound output, profile CPU/memory/I/O, use population and field representations, and parallelize safe work. | Large ensembles and sensitivity studies without sacrificing validated behavior. |
| Visualization and analysis | Standardized result readers, plots, run comparison, uncertainty views, and reproducible figure export. | Results that non-developers can inspect and communicate. |

The roadmap priority tiers have the following meaning in the current state:

- **P0, indispensable foundation, is substantially complete:** kernel,
  reproducibility, CI, units, schemas, and the BVS baseline.
- **P1, dissertation core, is substantially complete at demonstrator level:**
  body, lung, capillary, cell, Nano-IoT, and fingerprinting vertical slice.
- **P2, platform expansion, is now the main scientific expansion space:** new
  scenarios, multiple organs, external models, and integrated visualization.
- **P3, long-term vision, remains open:** metastasis prevention,
  patient-specific models, a dynamic digital twin, HPC ensembles, and eventual
  clinical validation where appropriate.

## 8. Post-M7 usability program

`UX` means user experience. These packages improve access and interpretation;
they do not by themselves increase physiological or clinical validity.

| Package | Plain-language objective | Status |
|---|---|---|
| UX-1 - One-command M7 scenario execution | Validate and run the complete fingerprinting demonstrator and summarize its result through the normal application. | Passed; all 280 local Windows/MSVC tests and all supported GitHub CI jobs pass |
| UX-2 - Model and example discovery | List and describe available artifacts, parameters, evidence, and limitations. | Passed; five model families, ten examples, all 281 local Windows/MSVC tests, and all supported GitHub CI jobs pass |
| UX-3 - Human-readable and HTML result reporting | Provide concise terminal, tabular, and shareable HTML views over the complete machine-readable result. | Passed; six-file bundle, all 284 current tests, and all supported GitHub CI jobs pass |
| UX-4 - Derived experiments and campaigns | Create controlled variants, replicates, parameter sweeps, paired comparisons, and aggregate analyses. | Passed; six-run reference campaign, all 284 current tests, and all supported GitHub CI jobs pass |
| UX-5 - Python API and notebooks | Support common scientific analysis and plotting workflows without replacing the C++ implementation authority. | Passed; process API, readers, analysis, optional plots, two notebooks, all 284 current tests, and all supported GitHub CI jobs pass |
| UX-6 - Graphical research workbench | Add guided scenario editing, run control, comparison, provenance, and uncertainty visualization after the interfaces are stable. | Planned |

The intended first user experience is:

```text
mehlissa scenario list
mehlissa scenario validate --file examples/scenarios/fp9-lung-level-a-v1.json
mehlissa scenario run --file examples/scenarios/fp9-lung-level-a-v1.json --output results/fp9-reference
mehlissa result summarize --file results/fp9-reference/<printed-run-directory>/result.json
mehlissa result report --file results/fp9-reference/<printed-run-directory>/result.json --output reports/fp9-reference
mehlissa campaign run --file examples/campaigns/fp9-collector-count-v1.json --output results/fp9-campaign
```

UX-1 validates and runs the complete M7 composition, creates a unique directory
with result, provenance, log, and summary, and reports actionable errors. Its
end-to-end test covers positive and negative workflows. All 280 local tests and
GitHub CI run 33620386319 pass across Windows/MSVC, Linux/GCC, and Linux/Clang.

UX-2 adds model and example discovery plus fail-safe starter copying. The strict
catalog and repository references are validated before use. All 281 local tests
and cross-platform GitHub CI run 33628859417 pass.

UX-3 validates an existing result and creates a non-overwriting six-file HTML,
text, CSV, and authoritative-JSON bundle. Evidence hashes, limitations, and the
clinical non-claim remain visible.

UX-4 expands a strict campaign into retained replicates, collector-count sweeps,
and same-seed pairs. Hashed aggregate JSON and CSV bind all inputs and results;
only the reviewed `run.collector_count` parameter is allow-listed.

UX-5 adds the `mehlissa-research` process client and version-guarded readers for
scenario and campaign analysis. Matplotlib remains optional; two licensed
notebooks demonstrate complete workflows. Python cannot bypass the C++ schemas
or convert failed runs into observations. All 284 local tests pass. Grouped
UX-3 through UX-5 acceptance also passes on Windows/MSVC, Linux/GCC, and Linux/Clang
with formatting, clang-tidy, AddressSanitizer, and UndefinedBehaviorSanitizer in
GitHub CI run 33668850496.

## 9. Opportunities for collaborators

| Contributor profile | High-value starting point |
|---|---|
| C++ and simulation engineering | General scenario runner, multirate orchestration, model factories, profiling, and regression infrastructure. |
| Hemodynamics and pulmonary physiology | Review pulmonary cards, qualify cohorts, refine state dependence, and develop anatomical or pulsatile variants. |
| Renal physiology | Define and validate a kidney model family using the existing organ and capillary contracts. |
| Molecular communication | Calibrate local channels, implement assembly physics, and connect an external network simulator through the existing adapter. |
| Cell biology and pharmacology | Replace synthetic receptor, signaling, apoptosis, and delivery assumptions with evidence-qualified models. |
| Statistics and uncertainty quantification | Design independent validation, global sensitivity, uncertainty propagation, identifiability, and ensemble analysis. |
| Research UX and visualization | CLI discovery, result summaries, HTML reports, Python notebooks, and comparative visualization. |
| Research data and governance | Model-card provenance, licensing, FAIR data, patient-data controls, and digital-twin governance. |

A new module is reviewable when its public contract and units are explicit;
misuse fails predictably; time, identity, ownership, and conservation invariants
pass; randomness is reproducible; output is bounded; schemas and examples agree;
software verification is separated from calibration and validation; evidence
and limitations are documented; and all supported CI paths pass.

## 10. Key project references

| Document | Purpose |
|---|---|
| `docs/USER_GUIDE.md` | Non-technical introduction, experiment families, installation, commands, model workflows, interpretation, and troubleshooting. |
| `docs/architecture/SOFTWARE_ARCHITECTURE.md` | System structure, APIs, extension workflow, kidney outline, external simulator integration, and personalization. |
| `docs/ROADMAP.md` | Guiding principles, milestones, medical scenarios, UX packages, personalization, scaling, and cross-cutting programs. |
| `docs/requirements/SYSTEM_REQUIREMENTS.md` | Numbered requirements derived from the dissertation and project decisions. |
| `docs/requirements/TRACEABILITY_MATRIX.md` | Implementation and verification status for every tracked requirement. |
| `docs/m7/M7_GATE_REVIEW.md` | Formal decision and scientific limitations of the holistic fingerprinting demonstrator. |
| `docs/m0` through `docs/m6` | Gate reviews, model notes, validation evidence, and accepted limitations for preceding milestones. |

- **Repository:** https://github.com/RegineWendt/MEHLISSA
- **Verified CI run:** https://github.com/RegineWendt/MEHLISSA/actions/runs/33668850496

MEHLISSA Next now provides the architectural and executable backbone required
by the dissertation vision. The next phase is to make this backbone easier to
run, broaden it to further scenarios and organs, and progressively replace
synthetic assumptions with independently validated scientific models.
