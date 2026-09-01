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

Gate M7 is **not yet accepted**. M7.1 establishes the composition contract;
M7.2-M7.4 add typed runtime initialization, a deterministic identity-preserving
trace, a manifest-complete result contract, and concentration-driven receptor
binding. Tile assembly, executed communication, uncertainty, and robustness
analysis remain open.

## Implemented increment

### M7.1 - versioned Level-A composition

- independent `MEHLISSA::fingerprinting_scenario` library above the M2-M6
  libraries;
- strict scenario-profile schema `1.0.0` with scenario, run, seed, target,
  artifact, acceptance, source, and limitation fields;
- exactly one selected definition and schema for each of twelve required
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

## Next increment

M7.5 adds explicit tile release and an assembly surrogate. M7.6 then executes
locator-to-collector-to-gateway-to-station communication instead of retaining
the Level-A communication placeholders.

## Gate status

Open items include:

- tile release and assembly;
- executed collector, gateway, and BAN hand-offs;
- extension of the versioned result report with Levels B-E outcomes and
  quantitative uncertainty;
- sensitivity, false-positive, and false-negative analysis; and
- the mandatory Gate M7 User Guide review.
