<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Level-A Fingerprinting Composition Contract (M7.1)

## Purpose

M7.1 defines one machine-checkable answer to “which concrete models form the
first fingerprinting run?” It separates component selection from component
execution. This prevents hidden file choices and interface gaps from being
buried in an application while avoiding the false claim that loading twelve
profiles already simulates the complete workflow.

## Contract and reference files

```text
data/schemas/fingerprinting-scenario-profile/1.0.0.schema.json
examples/scenarios/fp9-lung-level-a-v1.json
examples/scenarios/fp9-lung-level-a-cluster-v1.json
```

The scenario profile fixes:

- stable scenario and run identities;
- acceptance level `A`, master seed, and collector cohort;
- fingerprint, tissue, and region identity;
- one repository-relative definition/schema pair for every required role;
- the complete causal stage order;
- deterministic-replay and clinical-non-claim flags; and
- sources and limitations.

The selected roles are body model, body state, organ model, capillary model,
capillary-to-cell signal, receptor model, locator, collector, communication
cluster, active gateway, BAN station, and historical timer baseline. The
scenario-specific cluster is configuration data using the general M6.3 schema;
it joins the existing locator, uplink collector, and wrist-gateway identities
without adding behavior to the IoT library.

## Composer behavior

`load_scenario_profile()` performs structural and semantic profile validation.
`compose_level_a_plan()` then resolves every path below an explicit repository
root, rejects missing/non-file paths, validates every definition against its
selected schema, loads the timer through its typed M3.19 API, and checks exact
target and collector-cohort compatibility.

The public result is a typed `LevelAPlan` containing the decoded profile,
resolved artifacts, typed timer baseline, and deterministic timer run. The
composer never mutates the repository and does not select files implicitly.

## Causal stage contract

The required order is:

1. injection;
2. body transport;
3. organ transfer;
4. capillary localization;
5. molecular recognition;
6. fingerprint assembly;
7. local collection;
8. collector return;
9. gateway measurement; and
10. external report.

Changing, omitting, or duplicating a stage is invalid. M7.2 must emit traceable
events in this order; future detailed models may add sub-events but may not
silently reorder the scenario semantics.

## Verification

The focused test target checks:

- all twelve artifact definitions and schemas exist and validate;
- the FP9/lung identity and 1,000-collector historical timer resolve;
- the ten-stage order is stable;
- repeated composition produces identical paths and timer events; and
- unsafe paths, duplicate roles, altered stage order, and target mismatch fail.

Run it after a Debug build with:

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure -R "M7.1"
```

## Validity boundary

M7.1 is software composition evidence. Only the historical Level-A timer is
executed. The other artifacts are schema-valid selections whose runtime
interfaces will be joined in M7.2. Cellular, communication, gateway, and BAN
parameters remain synthetic surrogates, and the result is neither a clinical
prediction nor biological validation of FP9.
