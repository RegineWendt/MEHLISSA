<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Runtime, Result, and Level-B Detection Contracts (M7.2-M7.4)

## Purpose

M7.2-M7.4 turn the M7.1 selection boundary into an executable, auditable
scenario increment. They deliberately preserve three different evidence bases
instead of presenting all stage times as outputs of one physiological model:

- `model_execution` means a selected implementation was actually evaluated;
- `historical_timer` means a value is replayed or partitioned from the
  dissertation's FP9 regression timeline; and
- `software_surrogate` means a synthetic contract value exercises an interface.

None of these labels implies clinical validation.

## M7.2 scenario coordinator

`run_level_a_runtime` loads every selected M2-M6 profile through its typed
loader. The body graph and body-state profile, five-lobe lung, and pulmonary
capillary bed are instantiated in one `ComponentHost` and advanced with the
declared master seed. A representative particle verifies initialization of the
selected historical injection segment. The cellular and IoT definitions are
typed, instantiated where their current dependency boundary permits it, and
their topology is resolved.

The coordinator emits exactly ten ordered `StageTraceEntry` records. Every
record carries an input identity and output identity; adjacent records must
join exactly. The trace explains which intermediate Level-A timestamps are
partitions of the historical first-localization or external-report interval.
It therefore does not claim that the one-millisecond component probe generated
the published 209-second FP9 result.

## M7.3 result contract

The strict `fingerprinting-result/1.0.0` schema records:

- scenario, run, seed, collector cohort, and target identity;
- repository-relative paths and SHA-256 hashes for all thirteen selected
  definitions and their schemas;
- instantiated component identities and whether each component was advanced;
- the full ten-stage trace with evidence basis and qualification; and
- the no-clinical-validation declaration and scenario limitations.

Writing the same report twice from the same manifest, data, and seed must yield
the same SHA-256 digest. The report currently covers the Level-A runtime; later
M7 increments extend the run outcome while retaining this reproducibility core.

## M7.4 concentration and receptor binding

`run_level_b_detection` replaces the Level-A assumption of immediate molecular
recognition. It evaluates the selected ligand concentration, exposure, and
initial occupancy with the selected analytical 1:1 receptor model. If the
binding threshold is crossed, the existing M5-to-M6 adapter creates a traceable
detection event for the selected locator. Its relative crossing time is added
to the FP9 localization time and replaces the recognition stage in the runtime
trace.

The reference concentration of `0.0003 mol/m3` reaches a final bound fraction
of approximately `0.736263` and crosses the configured `0.5` threshold. A
`1e-9 mol/m3` negative control remains below threshold and creates no detection
event. This is mechanistic software execution, but its receptor, ligand,
concentration, and kinetic parameters are still synthetic and are not an
FP9-specific proteomic calibration.

## Verification

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure -R "M7\.[234]"
```

The focused suite checks deterministic coordination, causal identity flow,
strict result validation, byte-stable report reproduction, positive
concentration-driven binding, and the below-threshold negative control.
