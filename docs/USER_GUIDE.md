<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Next User Guide

**Guide status:** living document

**Covered software:** M0 through the current M3 body–organ coupling slice

**Last updated:** 29 August 2026

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

The repository also contains one schema-validated comparison scenario that
runs unchanged with the coarse lung compartment and the five-lobe pulmonary
model. This is currently an executable regression rather than a general CLI
composition feature.

The software does not yet contain a geometry-resolved or participant-level
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
data/lung-models/healthy-adult-rest-exercise-age-0d-v3.json
data/lung-models/healthy-adult-rest-exercise-age-invasive-0d-v4.json
data/lung-models/healthy-adult-pressure-distensible-age-0d-v5.json
data/lung-models/healthy-adult-pressure-distensible-age-0d-v6.json
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

The age-conditioned v3 card validates against schema 1.3.0 and preserves v2's
flow rules. Set `hemodynamics.age_conditioning.default_age_years` to the age
represented by the scenario: 18–<40 applies a PVR multiplier of 0.92335,
40–<60 uses 1.0, and 60–85 applies 1.12246. Values outside 18–85 are rejected.
The age factor changes PVR only; it does not imply age-conditioned compliance,
PAWP, anatomy, or patient physiology. See the
[age-conditioning model card](m3/PULMONARY_0D_AGE_CONDITIONING.md) before use.

The v4 card also uses schema 1.3.0. For ages 24–under-40 it replaces the v3
PASP/CO proxy with an invasive resting-PVR multiplier of 0.71875. The middle
and older bands and the bounded flow law remain unchanged. Ages below 24 are
outside v4's source support and are rejected. Read the
[young-adult resistance model card](m3/PULMONARY_0D_YOUNG_RESISTANCE.md) for
the derivation and evidence-overlap rule.

The v5 card validates against schema 1.4.0. It retains v4's resting and age
calibration but replaces empirical flow adaptation with a Linehan
pressure-distensible relationship. The schema and runtime reject enabling both
laws together. A fixed `0.020 mmHg^-1` healthy coefficient is converted to SI,
and zero-pressure resistance is derived so that the qualified resting state is
unchanged. Read the
[pressure-distensibility model card](m3/PULMONARY_0D_PRESSURE_DISTENSIBILITY.md)
before using v5; its frozen population comparison is weaker than v4.

The v6 card validates against schema 1.5.0. It keeps `alpha = 0.020 mmHg^-1`
below age 60 and uses the independently reported older value
`0.015 mmHg^-1` from age 60. The older calibration is directly supported in
men, so using v6 as sex-neutral is an explicit extrapolation. Read the
[age-distensibility model card](m3/PULMONARY_0D_AGE_DISTENSIBILITY.md) before
interpreting v6.

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
  source-disjoint exercise adaptation and a 10/18 published-population result.
  v3 adds a separate Kane-calibrated PVR age factor and improves the unchanged
  population comparison to 14/18 stages, but still agrees with only 1/5 young
  stages. v4 uses invasive young-adult PVR calibration and agrees with all
  15 stages in the disjoint Wolsk cohort, while the overlapping Kovacs 2009
  series is not counted. v5 provides a structurally distinct pressure-
  distensible experiment and agrees with 11/15 frozen Wolsk stages; its 1/5
  older-stratum result is a declared limitation, so it does not replace v4.
  V6 uses an independent older coefficient and reduces older pressure RMSE,
  but stage agreement remains 11/15. A
  software-verified subject-level multipoint analysis
  path exists;
  measured participant-level data have not yet been licensed or evaluated,
  and no variant is geometry-resolved or fully pulsatile,
  patient-specific, or clinically usable. V7 adds five anatomical lobe beds;
  its fixed healthy-adult supine shares pass an independent V/Q SPECT/CT
  comparison, but dynamic redistribution is not yet modeled;
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

The immediately usable multipoint cases contain four non-overlapping healthy
population series from Kovacs and Wolsk. They apply reported mean PAWP and flow
as stage boundaries and never refit v2 or v3 parameters. The v4 case contains
only the three Wolsk series because its Kovacs 2012 calibration overlaps the
historical corpus underlying Kovacs 2009.

Run its checks with:

```powershell
& build/windows-msvc/tests/Debug/mehlissa_organ_model_tests.exe `
  "[population-multipoint-validation]"
```

The v2 evidence file is
`data/validation/pulmonary-zero-dimensional/healthy-population-multipoint-v1.json`.
The v3 evaluation is locked in `healthy-population-multipoint-v2.json`, whose
Wolsk series add predeclared representative ages of 29.5, 49.5, and 70 years;
it uses population schema 1.1.0. The v2 case remains on immutable schema 1.0.0.
The v4 evaluation is `healthy-population-multipoint-v3.json` and also uses
population schema 1.1.0.
The v5 structural evaluation is
`healthy-pressure-distensible-population-v1.json` and uses the same schema and
the same three Wolsk series without fitting.
The v6 age-distensibility evaluation is
`healthy-pressure-distensible-population-v2.json`; its Wolsk values and
acceptance protocol are unchanged from v5.
Reported mmHg and L/min values are converted to SI by the loader. Wolsk cardiac
index is converted to mean absolute flow using the explicitly recorded mean
BSA of 1.9 m2.

Both results are deliberately partial. In v2, Kovacs passes 3/3 stages and the
Wolsk 20–39, 40–59, and 60–80 groups pass 0/5, 5/5, and 2/5. With the separate
Kane age calibration, v3 yields 3/3, 1/5, 5/5, and 5/5, or 14/18 overall. A
v4 comparison counts only the disjoint Wolsk groups and yields 5/5, 5/5, and
5/5, or 15/15, without refitting. The v5 structural comparison yields 5/5,
5/5, and 1/5, or 11/15, and is deliberately not tuned after that result. V6
retains those counts while reducing older RMSE from 5.411 to 4.603 mmHg. A
reported 95% CI is an interval for the
population mean, not an individual
tolerance range. Read the complete
[population multipoint report](m3/PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md)
and the [v3 age-conditioning](m3/PULMONARY_0D_AGE_CONDITIONING.md) and
[v4 young-resistance](m3/PULMONARY_0D_YOUNG_RESISTANCE.md) model cards before
interpreting a stage result. For v5, also read the
[pressure-distensibility model card](m3/PULMONARY_0D_PRESSURE_DISTENSIBILITY.md).
For v6, read the
[age-distensibility model card](m3/PULMONARY_0D_AGE_DISTENSIBILITY.md).

### 8.2.1 Use the anatomical five-lobe pulmonary model

The v7 definition is
`data/lung-models/healthy-adult-lobar-parallel-0d-v7.json`. Loading it through
the normal lung-definition path automatically selects the parallel-bed
implementation; no scenario or coupler branch is required. Its observable
state contains flow, resistance, compliance, transit time, and blood volume
for the right upper, right middle, right lower, left upper, and left lower
lobes.

Use v7 when an experiment needs a reproducible anatomical lobe assignment or
regional state. Do not interpret the fixed fractions as patient-specific flow:
they are a documented DE-CT perfused-blood-volume proxy and currently remain
fixed across posture and exercise. Read the
[parallel-beds model card](m3/PULMONARY_LOBAR_PARALLEL_BEDS.md) before reporting
regional results.

Run the independent regional validation with:

```powershell
ctest --test-dir build/windows-msvc -C Debug `
  -R "SPECT|Lobar validation rejects" --output-on-failure
```

The versioned source observations are in
`data/validation/pulmonary-lobar-perfusion/healthy-normal-spect-v1.json` and
their strict schema is
`data/schemas/pulmonary-lobar-perfusion-validation/1.0.0.schema.json`.
The uncorrected published fractions total 100.1% because of table rounding;
the loader preserves them and the evaluator explicitly normalizes the series.
Both source reconstructions pass without refitting v7. Read the
[lobar validation report](m3/PULMONARY_LOBAR_PERFUSION_VALIDATION.md) for the
metrics, acceptance rules, and scope limitations.

### 8.2.2 Compare coarse and detailed lung resolution in one scenario

The scenario file
`examples/scenarios/body-lung-resolution-comparison-v1.json` defines the body,
route, injection, random seed, synchronization step, conserved payload, and
acceptance rules once. It then names the coarse compartment and five-lobe v7
model as interchangeable candidates.

Run its regression with:

```powershell
& build/windows-msvc/tests/Debug/mehlissa_organ_model_tests.exe `
  "[resolution-comparison]"
```

Both candidates must return all 25 identities, the population, substance
amount, and volume flow exactly. Their completion times are expected to differ:
2.0 seconds is the coarse software-test value, while 6.4 seconds is the v7
literature-derived pulmonary transit. Read the
[coarse–detailed comparison](m3/COARSE_DETAILED_SCENARIO_COMPARISON.md) before
interpreting this as a physiological model comparison.

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

### 8.4 Reproduce the historical FP9 lung timer

The Level A FP9 baseline replays the dissertation's published event semantics
without pretending that they are new physiological predictions. Its versioned
scenario is `examples/scenarios/fp9-lung-historical-timer-v1.json`.

Run the focused regression with:

```powershell
& build/windows-msvc/tests/Debug/mehlissa_experiment_tests.exe "[fp9]"
```

The expected event chain is injection at 0 s, first lung localization at 25 s,
message activation after the 15.99 s assembly timer at 40.99 s, and external
reporting at 209 s or 91 s for the published 1,000- and 10,000-collector cases.
The report times are totals from injection, not extra delays added after
assembly. Read the [FP9 timer baseline](m3/FP9_TIMER_BASELINE.md) before using
these historical outputs; the replay does not model binding, diffusion,
collector encounter probability, or gateway communication.

## 9. M4 developer preview: capillary transit and layer coupling

M4.1 adds an independent capillary-layer component with three explicit serial
regions: arteriole, capillary, and venule. Its first card is intentionally a
synthetic software contract, not a physiological microcirculation model.

The current executable definition and schema are:

```text
examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json
data/schemas/capillary-bed-definition/2.0.0.schema.json
```

The card declares eight parallel paths, four perfused paths, and a one-second
total transit. M4.3 supplies length, diameter, parallel-vessel count, and total
flow; MEHLISSA derives area, velocity, and transit through volume-flow
continuity. The former v1 schema prescribed transit directly and remains only
as historical M4.1 evidence. The v2 values make region boundaries and equations
easy to verify; they must not be interpreted as human anatomy or physiology.
The runtime preserves nanodevice identity, population count, substance amount,
flow rate, interval, and integrated volume while routing transfers from the
arteriole entry to the configured organ return port.

M4.2 connects a compatible organ to this component through four explicit
boundaries: organ departure, capillary entry, capillary exit, and organ return.
The generic coupler records which entity and conserved-transfer IDs are outside
the organ, retains failed deliveries for inspection or retry, and only closes a
round trip after the organ accepts the return. The current test organ is
scripted and synthetic; this proves layer composition and conservation without
claiming pulmonary physiology.

To run only this developer test group after building:

```powershell
ctest --test-dir build -C Debug --output-on-failure -R capillary
ctest --test-dir build -C Debug --output-on-failure -R "organ capillary|coupler"
```

Read [Capillary Transit Bed](m4/CAPILLARY_TRANSIT_BED.md) before using the
component and [Organ-Capillary Round Trip](m4/ORGAN_CAPILLARY_ROUND_TRIP.md)
before composing layers. The equations and schema migration are documented in
[Capillary Geometry and Continuity](m4/CAPILLARY_GEOMETRY_AND_CONTINUITY.md).
Physiological organ-capillary gateways and geometry, dynamic recruitment,
barrier exchange, retention, and molecular channels are subsequent M4 work.

## 10. Troubleshooting

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

## 11. Where this guide goes next

The highest-priority documentation package is a new non-expert Part I. It will
introduce what MEHLISSA is useful for, explain its virtual body/entities/models
mental model, and describe the kinds of experiments a researcher can perform
before presenting installation or command-line details. It will include guided
examples for circulation and injection, passive observation, physiological
state comparisons, coarse-versus-detailed organ models, individual-versus-
population transport, and validation or sensitivity studies. Each example will
state its research question, inputs, outputs, interpretation, and limitations.
The current technical material will remain as Part II rather than being lost.
This work package is tracked explicitly in the Roadmap's immediate to-do list.

The guide will be extended with:

- M3 general CLI composition, qualified pulmonary data, and gate closure;
- M4 capillary models and transport hand-offs;
- M5 cellular and molecular interaction models;
- M6 active gateway and Nano-IoT configuration;
- end-to-end fingerprinting scenarios;
- result analysis and visualization workflows.

For development details, use [MEHLISSA Next Development](DEVELOPMENT.md). For
scientific scope and milestone gates, use the [Roadmap](ROADMAP.md).
