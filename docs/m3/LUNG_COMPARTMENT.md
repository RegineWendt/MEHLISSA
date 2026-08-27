<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Lung Compartment Model Card

## Identity and purpose

- **Implementation:** `mehlissa::models::organ::LungCompartment`
- **Model class:** effective pulmonary-circulation transit compartment
- **Evidence class:** architectural surrogate / hypothesis model
- **Current assurance:** software verification only
- **Primary purpose:** validate body–organ ownership, timing, routing, and interchangeability contracts before anatomical detail is introduced

## Scale and state

The component represents the complete pulmonary circulation as one
well-defined transit region. Its state consists of resident stable entity IDs,
their elapsed residence time, and an outbound queue. It has one named arterial
entry and one named venous exit. Transit time and the return route are explicit
configuration values using the kernel's nanosecond duration type.

The model has no geometry, regional perfusion, pressure, resistance,
concentration, or tissue state. It uses no random stream. Equal accepted input
and synchronization schedule produce identical output.

## Verification

The M3.1 contract tests verify:

- identity and type preservation;
- correct rewriting of source and target route at exit;
- rejection before initialization;
- rejection of an unsupported route or synchronization time;
- rejection of a duplicate resident/outbound ID;
- exact resident/outbound/accepted/completed counts; and
- equal output for compatible 0.5-second and 1-second step subdivisions.

## Validity and limitations

This model is valid only as a coarse software surrogate for coupling tests and
fast reference experiments after parameters have been supplied by a scenario.
No default transit time is claimed to be physiological. A configured value
must not be presented as evidence unless its source, population, state,
uncertainty, and comparison method are recorded separately.

The component does not model:

- pulmonary arterial or venous anatomy;
- regional lung perfusion or gravity;
- capillary recruitment and transit-time distributions;
- ventilation, respiratory mechanics, or gas exchange;
- alveolar tissue, biomarkers, cells, or molecular communication;
- disease, exercise response, or patient-specific state.

The detailed M3 variant will add pulmonary artery, regional distribution,
capillary surrogate, and pulmonary veins behind the same `ModelComponent`
boundary. Respiratory and alveolar biology remain separate later variants in
accordance with ADR-0006.

The first structured alternative is now documented in the
[Pulmonary Circulation Model Card](PULMONARY_CIRCULATION.md).
