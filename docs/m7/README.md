<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M7 Fingerprinting Vertical Slice

## Objective

M7 turns the independently verified M2-M6 layers into the first complete
MEHLISSA multilayer demonstrator. The reference scenario is the dissertation's
FP9 lung fingerprinting workflow. It must remain reproducible, replaceable by
layer, and free of fingerprint- or lung-specific kernel branches.

Gate M7 **passed on 2 September 2026 for the reproducible research-software
demonstrator**. This is not a clinical or physiologically validated FP9 assay.
M7.1-M7.7 cover composition, typed runtime initialization, deterministic
identity flow, artifact-hashed results, receptor binding, explicit tiles,
executed communication, and misclassification analysis.

In the names below, an `M7.x` label identifies one implementation increment.
The `Level A` through `Level E` labels identify increasing functional realism
inside the same fingerprinting workflow: historical timing, mechanistic
detection, tile assembly, executed communication, and classification analysis.
They are not names for separate runs.

## Implemented increment

### M7.1 - versioned Level-A composition

- independent `MEHLISSA::fingerprinting_scenario` library above the M2-M6
  libraries;
- strict scenario-profile schema `1.0.0` with scenario, run, seed, target,
  artifact, acceptance, source, and limitation fields;
- exactly one selected definition and schema for each of thirteen required
  physiological, cellular, device, communication, gateway, BAN, and timer
  roles;
- repository-relative path safety, file existence, JSON parsing, and schema
  validation for every selected artifact;
- exact ten-stage causal contract from injection to external report;
- semantic agreement between the scenario target and the typed FP9 timer
  baseline, including the selected 1,000-collector cohort;
- deterministic repeated composition tests; and
- explicit non-claim that schema-valid component selection is already an
  end-to-end simulation.

See [M7.1 Level-A Composition Contract](LEVEL_A_COMPOSITION_CONTRACT.md) and
[ADR-0049](../architecture/adr/0049-scenario-owned-fingerprinting-composition.md).

### M7.2-M7.4 - runtime, result, and Level-B detection

- typed loading of all selected components and actual initialization/advancement
  of the body, body-state, lung, and capillary stack;
- exact ten-stage causal trace with continuous identities and an explicit
  evidence basis per stage;
- strict result schema with paths and SHA-256 hashes for all selected data and
  schema artifacts;
- byte-stable repeated result generation;
- concentration-driven analytical receptor binding and conversion to the
  existing molecular-detection event contract; and
- a below-threshold negative control that emits no event.

See [Runtime, Result, and Level-B Detection Contracts](RUNTIME_RESULT_AND_LEVEL_B_DETECTION.md).

### M7.5-M7.7 - Levels C-E and holistic result

- nine explicit FP9 tile identities and an all-required assembly rule;
- incomplete-assembly negative control;
- executed locator-to-collector and collector-to-gateway links;
- active-gateway measurement, BAN frame, and external-station reception;
- separate local and BAN timing and energy metrics;
- labelled sensitivity/misclassification cases with Wilson intervals; and
- strict holistic result schema 2.0.0 with byte-stable deterministic output.

See [Levels C-E and the Holistic Fingerprinting Run](LEVELS_C_TO_E_AND_HOLISTIC_RUN.md).

## Gate status

The formal criterion-by-criterion decision, retained limitations, and User
Guide review are recorded in the [M7 Gate Review](M7_GATE_REVIEW.md).
