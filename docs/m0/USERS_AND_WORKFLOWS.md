<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Target Users and Prioritized Workflows

**As of:** 26 August 2026

**Status:** M0 baseline

## 1. Product mission

MEHLISSA Next is primarily a scientific modeling and experimentation platform.
It should let multiple disciplines collaborate on a reproducible multiscale
simulation without requiring every user group to develop the C++ kernel.

Clinical decision support, real-time operation on patients, and regulated
product functions are outside the initial product scope.

## 2. User roles

### P1 – Simulation and platform developer

**Goal:** Reliably advance the kernel, coupling, data formats, and performance.

**Needs:** C++ API, CMake/CI, debugging, invariants, benchmarks, ADRs.

**Success:** Changes are tested and reproducible and affect reference runs only with an explanation.

### P1 – Nanonetwork/communication researcher

**Goal:** Compare device, channel, gateway, and protocol variants under realistic mobility.

**Needs:** Scenario configuration, interchangeable communication models, network metrics, ensembles.

**Success:** A communication model can be replaced without changing physiological models.

### P1 – Biomedical model developer

**Goal:** Contribute and validate organ, capillary, cell, or reaction models.

**Needs:** Documented model contracts, units, Python/file adapters, model cards, reference cases.

**Success:** A model can be tested independently and compared with coarse/detailed variants.

### P1 – Experimental or clinical research partner

**Goal:** Test assumptions, parameter ranges, and simulation results against real measurements.

**Needs:** Understandable model cards, data dictionary, sensitivity, uncertainty, exportable reports.

**Success:** Measurement and simulation quantities are mapped unambiguously; calibration and validation data remain separate.

### P2 – Scenario author and scientific user

**Goal:** Configure, replicate, compare, and publish experiments.

**Needs:** Validated manifest, CLI/Python API, templates, result summary, provenance.

**Success:** A new experiment does not require a simulation-kernel change.

### P2 – Students and educators

**Goal:** Understand models, investigate small variants, and conduct reproducible exercises.

**Needs:** Installable releases, tutorials, small data sets, visualization, short runtimes.

**Success:** A reference experiment runs locally with an understandable result.

### P2 – Project leadership, reviewers, and publication readers

**Goal:** Assess scientific claims, evidence, and reproducibility.

**Needs:** Traceability, model cards, commit/data version, comparison and validation reports.

**Success:** Every published figure can be traced to an experiment, model, and source.

## 3. Prioritized workflows

| Rank | Workflow | Primary role | Milestone acceptance |
|---:|---|---|---|
| 1 | reproduce an existing reference run from a manifest | all technical users | M1/M2 |
| 2 | vary parameters/seed and compare replicates | scenario author | M1 |
| 3 | load and validate the 1995 body model and reproduce the BVS distribution | platform developer | M2 |
| 4 | add a new model variant through a stable contract | biomedical model developer | M3–M5 |
| 5 | execute fingerprinting from injection to wrist | interdisciplinary team | M7 |
| 6 | import an external data set and verify provenance | data/model developer | M2/M3 |
| 7 | report calibration, independent validation, and uncertainty | experimental partner | M3–M7 |
| 8 | start an ensemble/sensitivity run locally or in batch/HPC | scientific user | M7/M10 |
| 9 | visualize and compare stored runs | students/researchers | M7 |
| 10 | export a reproducible publication package | project leadership/author | M7 |

## 4. User interfaces by maturity

1. **M1–M2:** C++ CLI, validated manifest file, structured results.
2. **M2–M4:** Python API for experiments, ensembles, and analysis.
3. **M3–M7:** standardized adapters for external models and data.
4. **M7:** decoupled interactive visualization and run comparison.
5. **M8+:** controlled interfaces for patient-specific research data.

A graphical interface does not replace an archived manifest. Every interactively
created run must export to the same reproducible representation.

## 5. Definition of a usable release

A release is usable for a role when:

- installation and the minimal experiment are documented;
- at least one typical workflow completes without source-code changes;
- input errors are reported understandably before simulation starts;
- results contain units, provenance, and validity limitations;
- runtime on the target hardware is documented;
- a citable version and archived reference experiment exist.

## 6. Outside the near-term scope

- diagnosis or treatment decisions for individual patients;
- automatic processing of identifiable health data;
- real-time control of physical nanodevices;
- certification as a medical device;
- universal biological accuracy without scenario-specific validation.
