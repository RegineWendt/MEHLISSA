<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Next User Guide

**Guide status:** living document

**Covered software:** M0 through the current M3 body–organ coupling slice

**Last updated:** 28 August 2026

This guide is the main entry point for people who want to build, inspect, and
run MEHLISSA Next. It will grow with each milestone. Internal architecture
decisions and scientific derivations remain in the linked specialist
documents; this guide focuses on practical use and interpretation.

## 1. What you can do today

The current software can:

- validate versioned experiment manifests;
- create provenance, run-log, and checkpoint artifacts for a minimal run;
- validate arbitrary vascular graphs without recompiling;
- reproduce the canonical 95-segment dissertation graph from released legacy
  source data;
- run the deterministic M2.4 BloodVoyagerS distribution regression;
- transport identity-preserving particles through a vascular graph;
- schedule injection and extraction events;
- collect bounded trajectories, population snapshots, sample-site events, and
  passive gateway measurements;
- apply versioned rest, exercise, and posture profiles without rebuilding the
  executable;
- exchange identity-preserving entities, populations, substance amounts, and
  volume flows through a typed body–organ boundary;
- select a coarse, regional-surrogate, or literature-parameterized pulmonary
  0D implementation from a schema-validated
  executable model card; and
- run the same body–lung–body regression at two compatible host steps.

The software does not yet contain an anatomical or independently validated
pulmonary model, capillary/cellular exchange, or active Nano-IoT communication.

## 2. Repository orientation

| Path | Purpose |
|---|---|
| `apps/` | command-line application |
| `core/` | generic simulation clock, lifecycle, quantities, and random streams |
| `experiment/` | manifests, provenance, logs, and checkpoints |
| `models/body/` | validated vascular graphs, transport, state profiles, and body-level reports |
| `models/coupling/` | versioned cross-model entity and conserved-transfer contracts |
| `models/organ/` | interchangeable lung implementations and model-definition loader |
| `models/cosimulation/` | body–organ ownership and route adapter |
| `data/body-models/` | canonical executable vascular models |
| `data/body-states/` | state overlays for compatible body models |
| `data/lung-models/` | executable evidence-scoped pulmonary reference candidates |
| `data/schemas/` | versioned JSON Schemas |
| `data/reference-results/` | checked-in scientific Golden References |
| `examples/` | minimal manifests and synthetic models |
| `tests/` | software, invariant, schema, and regression tests |
| `docs/` | user, scientific, architecture, and development documentation |
| `mehlissa/`, `mehlissa2.0/` | historical implementations retained as references |

## 3. Prerequisites

On Windows, install:

- Visual Studio Community 2026;
- the **Desktop development with C++** workload;
- CMake 3.28 or newer;
- vcpkg, exposed through `VCPKG_ROOT`.

The recommended editor is Visual Studio Code with Microsoft's **C/C++** and
**CMake Tools** extensions. These extensions improve editing and project
control, but the Visual Studio C++ workload supplies the actual compiler.

Linux builds use a C++20 compiler, CMake, Ninja, and vcpkg. See the
[Development Guide](DEVELOPMENT.md) for the supported presets.

## 4. Configure, build, and test on Windows

Open **Developer PowerShell for VS 2026**, change to the repository root, and
run:

```powershell
$cmake = Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'

& $cmake --preset windows-msvc
& $cmake --build --preset windows-msvc-debug
& $ctest --preset windows-msvc-debug --output-on-failure
```

The debug CLI is then available at:

```text
build/windows-msvc/apps/Debug/mehlissa.exe
```

All paths in the following examples are relative to the repository root.

## 5. Command overview

Run the CLI without arguments to print its current syntax:

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe
```

The non-zero exit is intentional because a command is required. Stable exit
categories are documented in [Errors, Logs, and Checkpoints](m1/ERRORS_LOGS_CHECKPOINTS.md).

### 5.1 Validate an experiment manifest

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe validate `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json
```

Validation first checks the JSON Schema and then semantic constraints such as
duration, seed, model list, and output directory.

### 5.2 Run the minimal deterministic experiment

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe run `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json `
  --checkpoint-schema data/schemas/checkpoint/1.0.0.schema.json
```

The configured output directory receives:

- `provenance.json` with software, input hash, seed, and timing information;
- `checkpoint-000000.json` with clock and random-stream state;
- `run.log.jsonl` with structured lifecycle events.

This command currently demonstrates the reproducible M1 runtime contract. The
M3 body–organ path is tested as a typed developer API but is not yet composed by
this general CLI command.

### 5.3 Validate a vascular model

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe validate-body `
  --model data/body-models/bvs95-dissertation-rest-v1.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

Validation covers stable IDs, SI units, geometry, area, volume, velocity,
flow, transition probabilities, source references, strong connectivity, and
junction-flow conservation.

To prove that the loader is not hard-coded for BVS95, validate the synthetic
four-segment model with the same executable:

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe validate-body `
  --model examples/body-models/synthetic-branching-circuit.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

### 5.4 Recreate the canonical 95-segment model

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe migrate-legacy-95 `
  --vasculature mehlissa2.0/data/95_vasculature.csv `
  --transitions mehlissa2.0/data/95_transitions.csv `
  --output build/bvs95-recreated.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

The converter is deterministic. Tests require its result to match the
checked-in canonical graph byte for byte. It does not overwrite the historical
CSV files.

### 5.5 Run the M2.4 scientific regression

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe reference-bvs `
  --model data/body-models/bvs95-dissertation-rest-v1.json `
  --output build/bvs95-reference-report.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json `
  --report-schema data/schemas/bvs-reference-report/1.0.0.schema.json
```

This run checks:

- minute-7 versus minute-120 distribution equilibrium;
- aorta versus popliteal injection;
- 6,359 versus 63,590 particles;
- exact population conservation;
- the 23 dissertation perfusion targets.

The expected checked-in result is
`data/reference-results/bvs95-dissertation-rest-m2.4.json`. Scientific meaning
and limitations are explained in the [M2.4 report](m2/BVS_REFERENCE_REGRESSION.md).

### 5.6 Apply a body-state profile

The same executable can derive a state-specific graph from a compatible base
model. No source change or rebuild is required.

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe apply-body-state `
  --model data/body-models/bvs95-dissertation-rest-v1.json `
  --profile data/body-states/bvs95-supine-cycle-exercise-1.9x-v1.json `
  --output build/bvs95-exercise.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json `
  --profile-schema data/schemas/body-state-profile/1.0.0.schema.json
```

Available M2.6 profiles are:

| Profile | Cardiac-output effect | Interpretation |
|---|---:|---|
| `bvs95-rest-supine-v1.json` | ×1.0, 6.0 L/min | historical dissertation identity baseline |
| `bvs95-supine-cycle-exercise-1.9x-v1.json` | ×1.9, 11.4 L/min | global-flow exercise sensitivity |
| `bvs95-head-up-tilt-70deg-v1.json` | ×0.7833, 4.7 L/min | global-flow head-up-tilt sensitivity |

The profile is schema-validated and must name the exact compatible model and
version. Application then:

1. applies any topology-preserving transition overrides;
2. solves the complete stationary flow system;
3. anchors it to the requested cardiac-output multiplier;
4. recomputes mean velocity from flow and fixed cross-sectional area;
5. validates the resulting vascular graph again;
6. writes all profile sources and validity limits into the derived model.

The exercise and tilt files are sensitivity profiles, not clinically validated
physiological models. Read their `limitations` arrays and the
[M2.6 model note](m2/BODY_STATE_PROFILES.md) before interpreting results.

## 6. Transport observations and extraction

M2.5 transport observation is currently exposed as a typed C++ API. A
`CompartmentTransport` can be configured with:

- scheduled `InjectionEvent` and `ExtractionEvent` objects;
- passive `sample` and `gateway` measurement sites;
- trajectories selected as `none`, `all`, or deterministic `first_n`;
- hard record limits for trajectories and individual measurements;
- a population-snapshot interval and hard snapshot limit.

When a detail limit is reached, the report sets the corresponding truncation
flag. Aggregate measurement counts and the invariant
`active + extracted = injected` remain exact.

The output writer validates JSON against:

```text
data/schemas/transport-observation-report/1.0.0.schema.json
```

The full event-order and truncation contract is documented in
[Transport Output, Extraction, and Measurement Sites](m2/TRANSPORT_OBSERVATION.md).
A CLI/manifest connection for this body observation configuration remains
planned. Standalone lung definitions already provide schema-validated M3 organ
selection, but the general experiment manifest does not compose them yet.

## 7. M3 developer preview: the first lung boundary

M3 uses a typed C++ boundary rather than a scenario-specific body shortcut. A
`ModelComponent` exposes a stable model ID, named entry and exit ports, and
versioned entity and conserved-quantity transfers. Three implementations exist:
`LungCompartment`, a coarse transit surrogate, and `PulmonaryCirculation`, a
serial artery/regional-capillary/vein surrogate, plus
`PulmonaryZeroDimensionalModel`, a literature-parameterized mean hemodynamic
reference candidate.

The current contract verifies that:

- an incoming transfer names its source and target model and port;
- acceptance occurs at the declared synchronization time;
- an entity ID cannot exist twice inside the organ boundary;
- identity and entity type survive pulmonary transit;
- the outgoing transfer names the configured venous return route; and
- population count, molar amount, flow rate, interval, and integrated volume
  survive lossless transit; and
- compatible time-step subdivisions produce the same result: 0.5 s/1.0 s for
  both software surrogates and 0.1 s/0.2 s for the 6.4-second pulmonary 0D
  card.

The executable lung model cards are:

```text
examples/organ-models/lung-compartment-contract-v1.json
examples/organ-models/lung-regional-contract-v1.json
data/lung-models/healthy-adult-rest-supine-0d-v1.json
data/lung-models/healthy-adult-rest-exercise-0d-v2.json
```

The two contract cards validate against
`data/schemas/lung-model-definition/1.0.0.schema.json`. They are classified as
`software_test_surrogate`; their transit values are not physiological data.
The resting 0D card validates against schema 1.1.0 and records SI pressure, flow,
resistance, compliance, transit, right/left perfusion, uncertainty, evidence
roles, derivations, and limitations. At its resting reference state it predicts
15.2 mmHg mean pulmonary-arterial pressure from 6 L/min cardiac output, 8 mmHg
left-atrial pressure, and 1.2 Wood units PVR. It is a composite healthy-adult
reference candidate, not a patient or clinical model.

The flow-adaptive v2 card validates against schema 1.2.0. It preserves the v1
resting state and applies independently sourced, bounded PVR and compliance
multipliers between the resting reference and the calibrated peak/rest flow
ratio. It does not extrapolate below or above that interval.

The optional external-data section can preserve a source checksum, format,
coordinate system, units, and transformation history, but no qualified
pulmonary geometry is distributed yet.

This remains a developer API: the first explicit body → lung → body ownership
round trip is implemented, but the CLI cannot yet compose it automatically.
See the [M3 working plan](m3/README.md),
[model-definition guide](m3/LUNG_MODEL_DEFINITIONS.md), and the model cards
before interpreting results.

## 8. How to interpret model evidence

MEHLISSA distinguishes:

- **historical regression:** reproduces a previous implementation or paper;
- **calibration:** chooses parameters to match target values;
- **verification:** checks software and numerical invariants;
- **validation:** compares against independent evidence;
- **sensitivity scenario:** explores a transparent assumption without claiming
  complete physiology.

These labels are not interchangeable. In particular:

- the BVS95 geometry is schematic;
- its equivalent diameters are transport parameters, not measured anatomy;
- the M2.4 perfusion table is a calibration regression;
- the M2.6 exercise and posture profiles change global flow but do not yet
  model complete regional redistribution;
- the coarse and serial-region lung variants are verification surrogates;
- the pulmonary 0D variants have qualified independent aggregate validation
  for healthy pressure, compliance, and resting RC behavior; v2 also has a
  source-disjoint exercise adaptation, published population multipoint
  validation with 10/18 stages agreeing, and a software-verified subject-level
  multipoint analysis path. The population result exposes missing age
  dependence; measured participant-level data have not yet been licensed or
  evaluated, and neither variant is anatomical, fully pulsatile,
  patient-specific, or clinically usable;
- none of the current outputs is patient-specific or suitable for clinical
  decision-making.

### 8.1 Run the pulmonary 0D independent validation

After building the Debug preset, run only the independent validation checks:

```powershell
ctest --test-dir build/windows-msvc -C Debug `
  -R "Independent pulmonary|Pulmonary validation" --output-on-failure
```

The versioned observations are in
`data/validation/pulmonary-zero-dimensional/healthy-adult-independent-v1.json`
for the immutable resting baseline and `healthy-adult-independent-v2.json` for
the bounded flow-adaptive candidate.
The human-readable protocol, results, and limitations are in
`docs/m3/PULMONARY_0D_INDEPENDENT_VALIDATION.md`. The validator rejects a
validation source that is also present in the model evidence card. A passing
aggregate result must still be read together with diagnostic failures and the
declared validity scope.

The v2 result is not a fitted Bentley reproduction: Claessen et al. supplies
the adaptation calibration, while Bentley remains the untouched stress-test
cohort. The exercise RC discrepancy improves but remains a diagnostic failure.

### 8.2 Run the published-population multipoint validation

The immediately usable multipoint case contains four non-overlapping healthy
population series from Kovacs and Wolsk. It applies reported mean PAWP and flow
as stage boundaries and never refits the v2 parameters.

Run its checks with:

```powershell
& build/windows-msvc/tests/Debug/mehlissa_organ_model_tests.exe `
  "[population-multipoint-validation]"
```

The evidence file is
`data/validation/pulmonary-zero-dimensional/healthy-population-multipoint-v1.json`;
the schema is
`data/schemas/pulmonary-zero-dimensional-population-multipoint-validation/1.0.0.schema.json`.
Reported mmHg and L/min values are converted to SI by the loader. Wolsk cardiac
index is converted to mean absolute flow using the explicitly recorded mean
BSA of 1.9 m2.

The current result is deliberately partial: Kovacs passes 3/3 stages, Wolsk
ages 40–59 passes 5/5, ages 20–39 passes 0/5, and ages 60–80 passes 2/5. A
reported 95% CI is an interval for the population mean, not an individual
tolerance range. Read the complete
[population multipoint report](m3/PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md)
before interpreting a stage failure.

### 8.3 Verify the subject-level multipoint analysis path

The multipoint path accepts pseudonymous healthy-control records with at least
three jointly measured cardiac-output, mPAP, and PAWP stages. If heart rate and
systolic/diastolic PAP are supplied together, it also derives observed
compliance and RC time. The locked v2 parameters are never fitted to these
records.

Run the dedicated software checks with:

```powershell
& build/windows-msvc/tests/Debug/mehlissa_organ_model_tests.exe `
  "[multipoint-validation]"
```

The current fixture is synthetic and stored only under `tests/data`. Loading it
through the normal evidence path fails by design. A report produced from that
fixture explicitly states that it is not measured evidence.

The data schema is
`data/schemas/pulmonary-zero-dimensional-multipoint-validation/1.0.0.schema.json`.
See the
[subject-level validation plan](m3/PULMONARY_0D_SUBJECT_MULTIPOINT_VALIDATION.md)
for required fields, formulas, source qualification, and the current data-access
blocker. Do not add a clinical source file to the repository unless its data-use
terms explicitly permit redistribution.

## 9. Troubleshooting

### CMake selects the wrong Visual Studio version

Use **Developer PowerShell for VS 2026** and the CMake executable installed
inside Visual Studio, as shown in section 4. A globally installed older CMake
may not recognize the `Visual Studio 18 2026` generator.

### vcpkg dependencies cannot be downloaded

Check that `VCPKG_ROOT` points to a valid vcpkg installation. If network access
is temporarily unavailable, use the dependency-free smoke preset described in
the [Development Guide](DEVELOPMENT.md). It checks the core but does not replace
the complete suite.

### A model or profile is rejected

Read the stable error identifier and message. Common causes are:

- using the wrong schema version;
- applying a profile to a different model ID or version;
- transition probabilities that do not sum to one;
- topology-changing overrides;
- flow, velocity, or geometry values that violate SI invariants;
- an unknown data-source reference.

### The full test suite takes longer than expected

The M2.4 regression simulates both 6,359 and 63,590 particles and recreates its
Golden Reference. A complete debug suite therefore takes roughly one to two
minutes on the current Windows reference system.

## 10. Where this guide goes next

The guide will be extended with:

- M3 general CLI composition, qualified pulmonary data, and gate closure;
- M4 capillary models and transport hand-offs;
- M5 cellular and molecular interaction models;
- M6 active gateway and Nano-IoT configuration;
- end-to-end fingerprinting scenarios;
- result analysis and visualization workflows.

For development details, use [MEHLISSA Next Development](DEVELOPMENT.md). For
scientific scope and milestone gates, use the [Roadmap](ROADMAP.md).
