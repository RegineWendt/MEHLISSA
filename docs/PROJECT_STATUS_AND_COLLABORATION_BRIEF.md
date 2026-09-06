<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Next

## Project Status and Collaboration Brief

**Purpose:** Shareable overview for prospective contributors and research partners
**Status date:** 6 September 2026
**Development branch:** `mehlissa-next-generation`
**Verified published release commit:** `5821c7358f490c1c92e9ec79eaed783f80851297`
**Workbench release:** 1.0.0; UX-6.1 through UX-6.8 accepted
**Milestone status:** M0 through M7 passed
**Current product focus:** externally validated medical reference scenario and pending DCCQ source-disjoint evidence plus external review

This Markdown file is the maintainable source for the shareable PDF at
`output/pdf/MEHLISSA_Next_Project_Status_and_Collaboration_Brief.pdf`.
Scientific milestones and research-use capabilities must update this source and
regenerate and visually verify the PDF.

Shared release facts and the ordered next scientific packages are recorded in
the machine-readable [`PROJECT_STATE.json`](PROJECT_STATE.json) and checked
against the canonical documentation in CI.

From the repository root, regenerate it with:

```powershell
python scripts/generate_project_status_pdf.py
```

## Executive summary

MEHLISSA Next has progressed from a set of valuable historical prototypes to a
coherent, modular research simulation platform. Its scientific simulation
stack, research-use interfaces, and assurance mechanisms together constitute
the platform. The scientific stack now implements the complete architectural
path envisioned for the first dissertation-driven demonstrator: systemic
transport, a replaceable lung model, capillary transport and molecular
channels, receptor and intracellular cell response, nanodevice communication,
an active gateway, a body area network, and external analysis.

**Key achievement:** All major layers can now participate in one reproducible
FP9/lung fingerprinting workflow while remaining independently replaceable
behind explicit contracts.

- Milestone gates M0 through M7 have passed their documented acceptance reviews.
- The implementation uses C++20, CMake, vcpkg, strict JSON Schemas, typed SI
  quantities, stable error contracts, deterministic random streams, and
  explicit provenance.
- The published Workbench 1.0 commit passes all 286 local Windows/MSVC tests and
  GitHub Windows/MSVC, Linux/GCC, and Linux/Clang CI. The Clang path also passes
  formatting, clang-tidy, AddressSanitizer, and UndefinedBehaviorSanitizer.
- The native macOS/Apple Clang CMake path, automatic Workbench executable
  discovery, and required `macos-15` ARM64 GitHub job are accepted by GitHub CI
  run 33956456353. The path produces a source-built command-line simulator,
  not a `.app` bundle.
- The current development branch contains 295 Windows/MSVC tests after the
  completed DCCQ-1 typed dynamic-coupling and computational qualification work;
  the new commit's cross-platform CI status is reported separately after push.
- The integrated research-use layer provides a validated five-family,
  ten-example catalog; safe scenario editing and validation; direct and
  campaign execution; non-overwriting reports; process-based Python readers;
  optional plots; and two notebooks.
- MEHLISSA Research Workbench 1.0 provides graphical discovery, guided
  configuration, corrective validation, guarded execution, result comparison,
  provenance and evidence audit, descriptive campaign analysis, exact-value
  export, isolated installation, and tested accessibility and recovery paths.
  It calls the accepted C++ and Python interfaces and does not create a second
  scientific implementation.
- English user, architecture, model, traceability, and decision documentation
  supports international collaboration.
- A reviewable Paper 1 platform/methods candidate now binds a locked technical
  protocol, a schema-validated six-family evidence matrix, three complete
  experiment archives, source export, claim registry, and SHA-256 inventory.
- The next scientific cycle has started with PCQ-1 design v0.1.0. It freezes
  the entering lung and capillary artifacts by SHA-256, a bounded non-clinical
  claim, four separately decidable tracks, six primary endpoints, no-refit and
  source-disjointness rules, six uncertainty classes, seven negative controls,
  and an amendment gate before new participant-level outcomes are inspected.
  PCQ-1.2 now adds a machine-checked thirteen-candidate source register with
  track rankings, access and rights boundaries, measurement jointness, source
  overlap, rejected-primary reasons, and transparent public-outcome exposure.
  PCQ-1.3 analysis amendment v0.2.0 now freezes four guarded source roles,
  eight observation models, sample and precision floors, six primary numeric
  gates, 90% equivalence intervals, missingness/statistical rules, and explicit
  blocked states. It prevents the five-person Arizona cohort from being called
  a full hemodynamic qualification and prevents whole-pulmonary transit from
  being compared directly with capillary-only residence. No request has been
  sent and no candidate participant record acquired. This is prospective
  study discipline and governed source selection, not yet new physiological
  evidence.
- PCQ-1.4 adds a machine-checked, manifest-first data boundary for all four
  selected measurement families. Measured records are forbidden in Git and
  may only be opened from an explicit outside-repository quarantine directory
  after rights, privacy, release, schema identity, content hash, and cohort
  independence pass. Four arbitrary synthetic fixtures verify normalization
  and failure behavior; command output contains metadata only. No measured
  participant record was acquired or inspected.
- PCQ-1.5 now binds all six uncertainty classes and all nine endpoints to a
  strict outcome-blind plan. Seven immutable pulmonary structures are compared
  over a fixed flow-age grid; all seven signed local sensitivities converge;
  and nine pre-calibration rank tests show what the planned observations can
  and cannot identify. Missing joint distributions explicitly block global
  variance attribution, equilibrium data cannot identify compliance, capillary
  volume cannot separate its equivalent geometry factors, and the unmatched
  whole-pulmonary transit remains blocked. No measured outcome or
  qualification decision was used.
- PCQ-1.5a now records a repository-first, metadata-only availability audit
  before direct requests. Five intended studies and five repository-backed
  alternatives are machine-checked against the unchanged PCQ-1.2 register.
  The audit found useful healthy posture/cardiac-output, invasive disease, and
  lung-water resources, but no drop-in dataset for the frozen primary
  pressure, regional-perfusion, capillary-volume, or transit endpoints. No new
  participant file was opened and no source role or numeric limit changed;
  D'Souza and Arizona remain the first contacts, followed by Bailey and
  conditionally Lassen.
- A coordinated PCQ-1 request package now supplies institution-reviewable
  drafts for D'Souza, Arizona, and Bailey, plus a deliberately feasibility-only
  Lassen inquiry. It also fixes the outcome-blind waiting-period plan: build a
  source-neutral PCQ-1.6 execution and reporting shell with synthetic fixtures,
  dry-run governance and provenance, and resolve the transit observation model
  independently. No request has yet been sent and no access is implied.
- BCQ-1.1 now provides a machine-checked four-candidate biological cell-model
  and licence screen. It selects the minimal Kallenberger 2014 CD95L-CD95-
  caspase-8 pair `BIOMD0000000523` and `BIOMD0000000524`, freezes the public
  Git commits and SBML hashes, and identifies the respective CD95-
  overexpressing and wild-type HeLa roles. Both encoded model artifacts are
  CC0 1.0; article, figure, supplement, and experimental-data rights remain
  separate. Rehm 2006 is the public downstream fallback, the larger
  Kallenberger 525/526 pair is a same-publication structural companion, and a
  VEGF-A/VEGFR endothelial model is deferred to the later dynamic coupling
  program.
- BCQ-1.2 froze the no-refit reproduction protocol before a first trajectory.
  The machine record binds both source hashes and full initial
  cases, COPASI command line 4.46 Build 300 with LSODA, a primary/replay/
  tightened six-run matrix on a 961-point grid, four direct observables,
  conservation and numerical gates, ten negative controls, and a non-
  overwriting failure-retaining archive. The selected SBML files omit explicit
  time and substance units, so the protocol preserves the source numbers as
  `unresolved-model-native` and forbids silently calling them seconds, minutes,
  molar, or SI.
- BCQ-1.3 independently executed both unchanged artifacts in COPASI
  4.46 Build 300. Six primary, replay, and tightened runs each contain 961
  model-time points and all 18 species. Nine unblocked computational gates and
  all ten negative controls pass. The worst replay difference used 30.84% of
  its prospective strict limit; the worst tightened-solver difference used
  0.339% of its limit; source-derived invariants stayed within
  `1.00045e-11`; and no negative state or reporter-direction violation occurred.
  The original exact-zero replay protocol remains a disclosed failed result;
  its versioned amendment was committed before the passing run. Quantitative
  publication-curve alignment remains blocked and units remain source-native
  and unresolved.
- BCQ-1.4 through BCQ-1.7 are now complete. A typed no-refit M5 adapter and a
  separate 13-reaction source-equation layer cover both selected average-cell
  cases. Six 961-point MEHLISSA trajectories compare all 18 states with the
  COPASI archive, replay byte-identically, converge under RK4 step halving, and
  preserve all source invariants. The worst normalized cross-engine result uses
  2.31% of its prospective tolerance and the worst MEHLISSA convergence result
  uses 0.00123%.
- The 525/526 pair passes its source/licence/structure audit but remains
  same-publication context rather than independent evidence. BCQ-1.6 retains
  the average-cell limitation because no reusable joint protein distribution
  was established; all 88 local sensitivity step checks pass as diagnostics.
  BCQ-1.7 closes with a separate result checker and twelve negative controls.
  One named variant is now a `computationally-qualified published average-cell
  mechanism`; publication curves, population evidence, external human
  attestation, biological qualification, endothelial realism, and clinical
  validity remain blocked.
- The machine authority is
  `data/qualification/biological-cell-model-qualification-result-v1.json`; it
  binds final archive `20260906T090213Z-e3e71e6ce6c2`, its canonical checksum
  manifest, all nine source invariants, and two transparently superseded but
  retained attempts.
- DCCQ-1.1 established the next qualification programme. DCCQ means
  **Dynamic Capillary-Tissue-Cell Qualification**. Its machine-checked
  prospective plan audits six existing baselines, defines a seven-owner ligand
  ledger, five evidence levels, eight execution/evidence gates,
  six uncertainty classes, ten negative controls, and seven increments. It
  expressly prevents the non-consuming homogeneous M5.2 snapshot, the
  oxygen-to-synthetic-ligand test alias, or the fixed-input, unit-unresolved
  BCQ mechanism from being reported as a dynamic biological coupling.
- DCCQ-1.2 now selects human VEGF-A165a binding to and trafficking with VEGFR2
  in primary HUVECs, with NRP1 as an explicit structural choice. Four targets,
  ten exact artifacts, five evidence roles, source units, and rights boundaries
  are machine checked. The article and Supporting Information are CC-BY-4.0;
  the linked repository has no explicit licence, so its code and standalone
  CSV files may be inspected and hashed but are not copied or bundled.
- DCCQ-1.3 froze nine independently specified reduced equations, 15 explicit
  parameters, exact molecule-to-SI conversions, the conservative
  tracked-neutral NRP1 reference choice, five internal-step resolutions, four
  synchronization resolutions, metrics, limits, and ten negative controls
  before the authoritative output. The Rab11 source surface was corrected from
  325 to the S12 value of 350 square micrometres per cell during source review.
- DCCQ-1.4 adds `DynamicCapillaryTissueCellModel`: human VEGF-A165a moves
  through blood-free, endothelial-free, interstitial-free, receptor-bound,
  internalized, cleared/degraded and outlet owners. Binding consumes free
  ligand, all sinks remain visible, and bounded occupancy feedback is applied
  only in the following synchronization interval.
- DCCQ-1.5 and DCCQ-1.6 execute 41 retained trajectories and 24 local
  sensitivity comparisons. Maximum balance residual is
  `7.857794173099922e-32 mol`; replay is exact; 1-second versus 0.5-second RK4
  error is `1.0649136586993484e-14`; and 30-second versus 15-second coupling
  error is `1.2144341029339634e-5`. Identity/units, balance, limiting cases,
  numerical convergence and causal timing pass.
- DCCQ-1.7 reaches its machine-verifiable close-out. G1-G5 pass, G6 and G8 are
  partial, and G7 is blocked. Numerical, synchronization, local-parameter and
  NRP1/feedback structural effects are reported, but joint distributions,
  observational covariance and global identifiability are unavailable. HUVEC
  culture remains human endothelial in-vitro evidence, not pulmonary
  physiology. The source-disjoint kinetic challenge uses engineered HEK293T
  cells and the independent HUVEC source has one directional endpoint; no
  condition-matched dynamic series or external human attestation exists.
- The result is a reproducible research-software demonstrator. It is not a
  clinical assay model, a medical device, or a patient-specific digital twin.

| Dimension | Assessment |
|---|---|
| Software architecture | Strong modular foundation with explicit ownership, conservation, lifecycle, configuration, and evidence boundaries. |
| End-to-end integration | Complete first software vertical slice through fingerprinting Levels A-E. |
| Scientific validation | Mixed maturity: verified equations, literature-parameterized candidates, selected independent comparisons, and multiple synthetic mechanisms. |
| Integrated research-use layer | Commands, discovery, reports, campaigns, Python and notebook access, and Workbench 1.0 form the supported delivery surface of MEHLISSA. They expose the scientific stack without duplicating its schemas, models, validation, or result authority. |
| Clinical readiness | Not claimed. Patient prediction, diagnosis, and treatment recommendations are explicitly outside the present scope. |

### Requirements in two dimensions

The 83 baseline requirements now carry separate implementation and evidence
statuses. This prevents software completion from being read as scientific
validation. Functionally, 58 requirements are complete, 16 are partly
implemented, five remain legacy-only, and four are specified but not yet
implemented. For evidence, 39 are verified for their declared bounded claim,
35 have partial evidence, seven are unverified in MEHLISSA Next, and two need a
dedicated research evidence programme. The BCQ updates deliberately retain
`PART` evidence for `CELL-002`, `CELL-004`, and `CELL-005`: computational
qualification of one published average-cell mechanism does not supply the
still-missing population, publication-series, external-review, or biological
evidence.

`DONE/PART` is an important and legitimate state: executable behavior and its
software contracts are present, but physiological, biological, external-data,
scale, or release-wide qualification remains incomplete. `VERIFIED` refers to
the verification mode required by the individual requirement; it does not
automatically mean clinical or participant-level validation. The canonical
row-level assessment and remaining gaps are maintained in the Traceability
Matrix.

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
| `DONE/PART` | Two-dimensional requirement status: functional implementation is complete, while the required evidence is only partial. Other rows independently combine `DONE`, `PART`, `LEGACY`, or `SPEC` implementation with `VERIFIED`, `PART`, `UNVERIFIED`, or `RESEARCH` evidence. |
| UX-1 through UX-6 | Delivery packages used to build MEHLISSA's cross-cutting research-use layer, from one-command execution to a graphical research workbench. They were implemented after M7, but their accepted outputs are integral platform capabilities rather than a separate add-on. UX-6.1 through UX-6.8 are reviewable increments of the workbench. |
| P1-E1 through P1-E3 | Predeclared Paper 1 technical experiments: body-observation neutrality/resources, small M7 resource/replay behavior, and CLI/Python/Workbench result parity. They are experiment identifiers, not evidence grades. |
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

That scientific path is one of three integrated aspects of MEHLISSA:

1. **Scientific simulation stack:** the body, organ, capillary, cell,
   Nano-IoT, BAN, and station components that represent the investigated
   mechanisms.
2. **Research-use layer:** the command line, catalog, reports, campaigns,
   Python readers, notebooks, and Workbench that let researchers configure,
   execute, inspect, compare, and share experiments.
3. **Assurance layer:** schemas, typed units, provenance, evidence boundaries,
   deterministic tests, gate reviews, and CI that keep both other aspects
   reproducible and auditable.

The historical M0-M7 gates primarily record growth of the scientific stack and
its runtime contracts. UX-1 through UX-6 record delivery of the research-use
layer. This distinction is useful for traceability, but neither sequence alone
defines the product: MEHLISSA is the integrated platform formed by all three
aspects.

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

Independent MEHLISSA Next code is licensed under MPL-2.0. Legacy code and direct
ports remain GPL-2.0-only; new original documentation and approved original data
are licensed under CC BY 4.0. Artifact-specific provenance and license sidecars
prevent unclear data rights from entering public packages.

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

## 4. How the integrated MEHLISSA platform can be used today

| Access route | Suitable uses | Current examples |
|---|---|---|
| MEHLISSA Research Workbench 1.0 | Guided use by new and experienced researchers | Discover models and examples; edit, validate, and run FP9/lung scenarios; run the curated campaign; inspect results, comparisons, provenance, evidence, and descriptive analyses; export exact tables and figures. |
| Command line | Scriptable and transparent direct use without writing C++ | Validate manifests and vascular graphs; run the minimal workflow and BVS regression; apply body-state overlays; discover, validate, execute, summarize, and report the complete M7 demonstrator. |
| Python API and notebooks | Reproducible analysis and integration into research workflows | Launch accepted processes; read versioned scenario and campaign results; create optional plots; reproduce the two licensed notebook workflows. |
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

After installing the Workbench package and building the matching simulator, the
integrated graphical route starts with:

```powershell
mehlissa-workbench --repository-root . --check
mehlissa-workbench --repository-root .
```

The first command checks the repository, simulator, schemas, examples, and
other prerequisites. The second starts the local Workbench and opens its
address in the browser. The complete M7 scenario also remains available through
the scriptable command-line route:

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

The integrated discovery commands use the same catalog and do not create a
second source of scientific truth:

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
the license and refuses to overwrite work. The complete published platform
passes all 286 local Windows/MSVC tests and all supported GitHub CI jobs.

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
| Cell | Configure synthetic channel, receptor, reaction, delivery, threshold, response, and cohort parameters; use the typed no-refit Kallenberger minimal average-cell mechanism through its C++ API. | Workbench selection of the qualified mechanism, patient-specific biomarker calibration, reusable correlated cell populations, endothelial mechanisms, and biologically validated disease-specific families. |
| Whole run | Fix seeds, versions, checksums, output paths, and interpretation limits. | Canonical patient manifest, automatic cross-layer composition, parameter estimation, uncertainty propagation, and longitudinal updates. |

Identifiable patient data must not be committed to the repository. Clinical or
participant-specific work requires institutional approval plus explicit
consent, pseudonymization, retention, access-control, and provenance workflows.
The future M8 Research Digital Twin gate still forbids clinical decision claims
without an appropriate regulatory and validation path.

## 6. Current scientific limitations

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

The evidence-and-validity inventory and the first Paper 1 technical baseline
are no longer future work. Candidate `paper1-platform-methods-rc1-20260903`
contains the six-family matrix, protocol v2.0.0, 112-attempt body campaign,
small repeated M7 resource study, access-path parity check, complete raw
archives, source export, hashes, and a claim registry. The suggested tag is
`paper1-platform-methods-rc1`; it has not been created, and no DOI or final
release is implied. The remaining programs begin by using and maintaining this
qualification infrastructure.

The access-pending pulmonary/capillary design, completed public cell-model path,
and computationally closed DCCQ-1 path are indexed in
`docs/qualification/README.md`. While PCQ-1 and the DCCQ external gates await
rights-compatible evidence or review, the next implementation focus is one
externally validated medical reference scenario.

| Program | Principal work | Intended outcome |
|---|---|---|
| Pulmonary and capillary qualification | **PCQ-1.1 through PCQ-1.5a completed locally:** design, source selection, observation models, numeric gates, safe ingress, uncertainty semantics, and repository availability are machine-checked. PCQ-1.5 covers six uncertainty classes and nine endpoints, seven pulmonary structures, covariance envelopes, local sensitivities, and identifiability. PCQ-1.5a finds no drop-in primary repository dataset, retains five non-equivalent alternatives, opens no participant file, and leaves the frozen source roles unchanged. A coordinated, still-unsent request package covers D'Souza, Arizona, Bailey, and feasibility-only Lassen. PCQ-1.6 next builds the outcome-blind execution/reporting shell and runs the frozen evaluator only when authorized source-disjoint observations exist. | Present result: auditable prospective design, source selection, safe data boundary, uncertainty/identifiability plan, repository-first access audit, and governed outreach drafts - not a validation pass. Intended exit: bounded pulmonary and capillary claims evaluated without refitting, with uncertainty and all partial or negative findings retained. |
| Biological cell-model qualification | **BCQ-1.1 through BCQ-1.7 completed locally:** source and licence selection, prospective COPASI and MEHLISSA protocols, retained deviations, a typed no-refit 18-state M5 adapter, separate source equations, all-state cross-engine and convergence checks, 525/526 structural audit, average-cell population decision, stable local sensitivities, twelve negative controls, and a runner-independent close-out review. | One published average-cell mechanism is computationally qualified. Publication-curve alignment, a reusable population ensemble, external human attestation, biological qualification, endothelial/organ realism, individual or patient prediction, and clinical validity remain blocked. |
| Dynamic capillary-tissue-cell coupling | **DCCQ-1.1 through DCCQ-1.6 complete; DCCQ-1.7 machine close-out complete:** a pre-output nine-equation/SI protocol, typed seven-owner VEGF-A165a/VEGFR2 HUVEC-informed model, consumptive binding, explicit sinks, delayed feedback, 41 trajectories, five passing computational gates, 24 local-sensitivity comparisons, NRP1/feedback variants, archive integrity and bounded claim are runner-independently machine checked. | Computational dynamic-coupling qualification passes. G6 remains partial, G7 blocked and G8 partial: no joint distributions, condition-matched source-disjoint HUVEC dynamics or external human attestation exist. Pulmonary, in-vivo, patient, biological-validation and clinical claims remain prohibited. |
| Externally validated medical reference scenario | Freeze one measurable complete protocol and compare its injection-to-measurement outputs with independent physiological or experimental observations. | The first reproducible end-to-end validation report with a bounded claim and explicit limitations. |
| Additional medical scenarios | Continuous monitoring, liquid biopsy, endocrine/adrenal venous sampling, CAR-T, and ultimately metastasis prevention. | Evidence that the architecture generalizes beyond fingerprinting. |
| Additional organs | Begin with a kidney surrogate and progress toward regional filtration, clearance, and organ-specific capillary interfaces. | Evidence that organ factories and coupling contracts are not lung-specific. |
| Personalization and digital twin | Patient manifest, imaging and SimVascular/CFD import, parameter estimation, identifiability, longitudinal updates, and governance. | Research-twin maturity from geometry to physiology and biochemistry. |
| Scaling and HPC | Bound output, profile CPU/memory/I/O, use population and field representations, and parallelize safe work. | Large ensembles and sensitivity studies without sacrificing validated behavior. |
| Advanced analysis and visualization | Extend the implemented dashboards and exports with qualified ensemble statistics, uncertainty propagation, spatiotemporal views, and larger-study comparison. | Evidence-backed analyses for study designs and scales beyond the bounded Workbench 1.0 demonstrators. |

The roadmap priority tiers have the following meaning in the current state:

- **P0, indispensable foundation, is substantially complete:** kernel,
  reproducibility, CI, units, schemas, and the BVS baseline.
- **P1, dissertation core, is substantially complete at demonstrator level:**
  body, lung, capillary, cell, Nano-IoT, and fingerprinting vertical slice.
- **P2, platform expansion, is now the main scientific expansion space:** new
  qualification evidence, scenarios, multiple organs, external models, and
  advanced scientific visualization beyond Workbench 1.0.
- **P3, long-term vision, remains open:** metastasis prevention,
  patient-specific models, a dynamic digital twin, HPC ensembles, and eventual
  clinical validation where appropriate.

## 8. Integrated research-use capabilities and delivery record

`UX` means user experience and identifies the implementation history of
MEHLISSA's research-use layer. The resulting commands, reports, campaign tools,
Python interfaces, notebooks, and Workbench 1.0 are supported parts of the
platform, not a separate future program. They make the scientific capabilities
operable, reproducible, and auditable, but do not by themselves increase
physiological or clinical validity.

| Package | Plain-language objective | Status |
|---|---|---|
| UX-1 - One-command M7 scenario execution | Validate and run the complete fingerprinting demonstrator and summarize its result through the normal application. | Passed; all 280 local Windows/MSVC tests and all supported GitHub CI jobs pass |
| UX-2 - Model and example discovery | List and describe available artifacts, parameters, evidence, and limitations. | Passed; five model families, ten examples, all 281 local Windows/MSVC tests, and all supported GitHub CI jobs pass |
| UX-3 - Human-readable and HTML result reporting | Provide concise terminal, tabular, and shareable HTML views over the complete machine-readable result. | Passed; six-file bundle, all 284 current tests, and all supported GitHub CI jobs pass |
| UX-4 - Derived experiments and campaigns | Create controlled variants, replicates, parameter sweeps, paired comparisons, and aggregate analyses. | Passed; six-run reference campaign, all 284 current tests, and all supported GitHub CI jobs pass |
| UX-5 - Python API and notebooks | Support common scientific analysis and plotting workflows without replacing the C++ implementation authority. | Passed; process API, readers, analysis, optional plots, two notebooks, all 284 current tests, and all supported GitHub CI jobs pass |
| UX-6.1 - Workbench product and technical foundation | Define users and workflows, select the interface architecture, establish safety/accessibility baselines, and prove read-only graphical discovery without duplicating scientific logic. | Passed and included in Workbench 1.0; five model families and ten examples load through the accepted process API |
| UX-6.2 - Guided scenario workspace | Open the complete FP9/lung starter, edit schema-derived scalar fields with their units/evidence/limits, retain the complete source, and validate/save/reopen without overwriting. | Passed and included in Workbench 1.0; a derivative round-trips with all 13 artifacts/evidence retained and unsafe writes fail closed |
| UX-6.3 - Validation and corrective feedback | Explain structural, semantic, and cross-file problems before save/run and make the decision shareable. | Passed and included in Workbench 1.0; accepted-CLI parity, stable located diagnostics, candidate identity, and closed invalid save/run gates |
| UX-6.4 - Run and campaign control | Confirm, start, monitor, cancel, and inspect an exact validated scenario or the curated six-run campaign without bypassing accepted APIs. | Passed and included in Workbench 1.0; bounded evidence, exact inputs/seeds, lifecycle, real cancellation, and protected artifacts |
| UX-6.5 - Result dashboard and comparison | Explain completed outcomes, cases, stages, campaign groups, and paired differences while retaining authoritative artifacts and excluding non-results. | Passed and included in Workbench 1.0; accepted-reader parity, guarded comparison/reporting, and explicit zero-observation failure policy |
| UX-6.6 - Provenance, evidence, and interpretation boundaries | Keep software/build identity, seeds, hashes, models, sources, licences, maturity, limitations, and the non-clinical scope attached to every displayed result. | Passed and included in Workbench 1.0; round-trip provenance, integrity/evidence states, and exact JSON audit export |
| UX-6.7 - Sensitivity, uncertainty, visualization, and export | Explain replicate variation, parameter effects, and same-seed differences without overstating the small curated campaign, then export the exact analysis. | Passed and included in Workbench 1.0; accepted-reader observations, descriptive views, exact-value table, and source-bound JSON/CSV/SVG exports |
| UX-6.8 - Usability, accessibility, packaging, and release acceptance | Package and test the workbench as a dependable entry point for novice and expert researchers and contributors. | Passed; version 1.0 wheel/command, isolated install, example workspace, semantic accessibility and responsive recovery gates, 286-test regression, synchronized documentation, and green supported CI for the exact published commit |

The Workbench is the recommended integrated entry point:

```text
mehlissa-workbench --repository-root . --check
mehlissa-workbench --repository-root .
```

The equivalent composable command-line route remains available for automation,
method inspection, and advanced workflows:

```text
mehlissa scenario list
mehlissa scenario validate --file examples/scenarios/fp9-lung-level-a-v1.json
mehlissa scenario run --file examples/scenarios/fp9-lung-level-a-v1.json --output results/fp9-reference
mehlissa result summarize --file results/fp9-reference/<printed-run-directory>/result.json
mehlissa result report --file results/fp9-reference/<printed-run-directory>/result.json --output reports/fp9-reference
mehlissa campaign run --file examples/campaigns/fp9-collector-count-v1.json --output results/fp9-campaign
```

UX-1 and UX-2 established the operational entry layer: one-command execution,
actionable errors, unique result directories, model and example discovery, and
fail-safe starter copying. UX-3 through UX-5 added the reproducible research
workflow: non-overwriting reports, retained campaign variants, deterministic
replicates and paired comparisons, version-guarded Python readers, optional
plots, and two licensed notebooks. Inputs, hashes, seeds, evidence, limitations,
and the non-clinical boundary remain attached throughout.

UX-6 delivered the graphical layer as MEHLISSA Research Workbench 1.0.0. Its
eight increments cover product and safety foundations, schema-derived scenario
editing, accepted-command validation, confirmed execution and cancellation,
reader-backed dashboards and comparisons, provenance and evidence audit,
descriptive campaign analysis and exact export, accessibility, packaging, and
release acceptance. The local browser host delegates scientific authority to
the matching C++ simulator, repository artifacts, and accepted Python process
API. It binds only to the local computer, serves no remote assets, records no
telemetry, and cannot convert failed or incomplete jobs into observations.

The complete published Workbench release passes 286 local Windows/MSVC tests
and GitHub Windows/MSVC, Linux/GCC, and Linux/Clang CI, including formatting,
clang-tidy, AddressSanitizer, and UndefinedBehaviorSanitizer. Detailed increment
contracts, acceptance evidence, and interpretation limits remain in `docs/ux`
and the User Guide.

The platform now also provides an accepted native macOS/Apple Clang source
build and the same Workbench workflow. Dedicated CMake presets and the pinned
macOS 15 ARM64 job passed the complete suite in GitHub CI run 33956456353.
This support does not create an application bundle, installer, signed or
notarized binary, or downloadable executable. Intel Macs remain an unqualified
source-compatibility expectation rather than an accepted CI claim.

## 9. Opportunities for collaborators

| Contributor profile | High-value starting point |
|---|---|
| C++ and simulation engineering | General scenario runner, multirate orchestration, model factories, profiling, and regression infrastructure. |
| Hemodynamics and pulmonary physiology | Review pulmonary cards, qualify cohorts, refine state dependence, and develop anatomical or pulsatile variants. |
| Renal physiology | Define and validate a kidney model family using the existing organ and capillary contracts. |
| Molecular communication | Calibrate local channels, implement assembly physics, and connect an external network simulator through the existing adapter. |
| Cell biology and pharmacology | Replace synthetic receptor, signaling, apoptosis, and delivery assumptions with evidence-qualified models. |
| Statistics and uncertainty quantification | Design independent validation, global sensitivity, uncertainty propagation, identifiability, and ensemble analysis. |
| Research UX and visualization | Evaluate Workbench 1.0 with diverse researchers and assistive technologies, then turn observed friction into bounded follow-up packages without weakening scientific authority. |
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
| `docs/ROADMAP.md` | Guiding principles, delivery history, scientific qualification sequence, medical scenarios, additional organs, personalization, scaling, and cross-cutting programs. |
| `docs/qualification/PULMONARY_CAPILLARY_QUALIFICATION_PROTOCOL.md`, `PCQ1_EVIDENCE_SOURCE_SCREEN.md`, `PCQ1_PRE_OUTCOME_AMENDMENT.md`, `PCQ1_DATA_INGRESS.md`, `PCQ1_UNCERTAINTY_IDENTIFIABILITY.md`, `PCQ1_REPOSITORY_FIRST_DATA_AUDIT.md`, and `PCQ1_DATA_REQUEST_PACKAGE.md` | Active PCQ-1 claim, frozen candidates and analysis, ranked sources, safe ingress, uncertainty and identifiability, repository availability and alternatives, governed request drafts, controls, and pre-outcome sequence. |
| `docs/qualification/BCQ1_BIOLOGICAL_CELL_MODEL_SELECTION.md`, `BCQ1_REPRODUCTION_PROTOCOL.md`, `BCQ1_REPRODUCTION_PROTOCOL_AMENDMENT_1.md`, `BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md`, `BCQ1_MEHLISSA_QUALIFICATION_PROTOCOL.md`, and `BCQ1_MEHLISSA_QUALIFICATION_RESULT.md` | Complete BCQ-1.1–1.7 selection, prospective protocols, retained replay amendment, COPASI archive, typed MEHLISSA implementation, cross-engine result, structure/population decisions, independent checks, and bounded claim. |
| `docs/qualification/DCCQ1_QUALIFICATION_PLAN.md`, `DCCQ1_EVIDENCE_SOURCE_SCREEN.md`, and `DCCQ1_QUALIFICATION_RESULT.md` | Dynamic Capillary-Tissue-Cell Qualification programme, source decision, prospective equations/SI/evaluation protocol, typed seven-owner implementation, 41-run computational and sensitivity archive, five passed and three bounded partial/blocked gates, independent checker, and exact permitted claim. |
| `docs/PROJECT_STATE.json` | Machine-readable shared release facts, Roadmap section mappings, selected traceability expectations, and ordered qualification packages. |
| `docs/requirements/SYSTEM_REQUIREMENTS.md` | Numbered requirements derived from the dissertation and project decisions. |
| `docs/requirements/TRACEABILITY_MATRIX.md` | Separate implementation and evidence status, achieved verification, and remaining gap for every tracked requirement. |
| `docs/m7/M7_GATE_REVIEW.md` | Formal decision and scientific limitations of the holistic fingerprinting demonstrator. |
| `docs/publication/EVIDENCE_AND_VALIDITY_BASELINE.md` | Six-family machine-readable evidence and validity baseline, source-role audit, and claim boundaries. |
| `docs/publication/PAPER1_TECHNICAL_EXPERIMENT_PROTOCOL_V2.md` | Locked Paper 1 technical questions, conditions, controls, metrics, failure rules, and archive contract. |
| `docs/publication/PAPER1_TECHNICAL_MEASUREMENTS.md` | Complete body-observation, M7 resource/replay, and access-parity results with explicit non-claims. |
| `publication/paper1/release-candidates/paper1-platform-methods-rc1-20260903/HANDOFF.md` | Exact source and artifact identity, reproduction route, claim limits, and reviewer checklist. |
| `docs/ux/README.md` | Index of all eight Workbench increments and their detailed product, validation, execution, reporting, audit, analysis, accessibility, and release contracts. |

- **Repository:** https://github.com/RegineWendt/MEHLISSA
- **Verified Workbench 1.0 CI run:** https://github.com/RegineWendt/MEHLISSA/actions/runs/33745263319
