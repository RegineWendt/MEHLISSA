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

Gate M7 is **not yet accepted**. M7.1 establishes the composition contract and
selects a complete set of versioned artifacts. M7.2 will instantiate and
advance those components through the causal stage sequence; later increments
add biological detection, tile assembly, communication execution, uncertainty,
and robustness analysis.

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

## Next increment

M7.2 will add the scenario-owned runtime coordinator. It must instantiate the
selected typed components, preserve experiment/entity/event identities at
every hand-off, and produce one deterministic execution trace. It may add
adapters in the scenario or co-simulation packages, but it must not add FP9
branches to the kernel or model libraries.

## Gate status

Open items include:

- actual M2-M6 component advancement in one run;
- concentration- and binding-driven FP9 detection;
- tile release and assembly;
- executed collector, gateway, and BAN hand-offs;
- a versioned result report with uncertainty and validity limits;
- sensitivity, false-positive, and false-negative analysis; and
- the mandatory Gate M7 User Guide review.
