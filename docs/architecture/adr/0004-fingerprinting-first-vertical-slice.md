<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0004: Fingerprinting as the First Vertical Demonstrator

- **Status:** Accepted
- **Date:** 26 August 2026
- **Applies to:** M0/M7; `SCN-001`

## Context

The roadmap needs a scenario that tests the architecture early against a
medically motivated end-to-end question. A pure body-transport test does not
exercise organ, capillary, cell, and nano-IoT interfaces. The metastasis
scenario covers all layers, but requires an especially large number of
biological models that are not yet established.

Proteome fingerprinting is described in the paper and dissertation and was
simulated in an abstracted historical MEHLISSA version. Configuration, device
classes, assembly times, and end-to-end reference values exist for nine tissues.

## Decision

Fingerprinting becomes the first vertical demonstrator. Development proceeds
through five acceptance levels:

1. historical timer baseline;
2. organ-specific concentration/binding-based detection;
3. assembly surrogate or NetTAS detailed model;
4. explicit wrist gateway and nano-IoT path;
5. sensitivity, uncertainty, and misclassification analysis.

The binding baseline is defined in the [scenario specification](../../requirements/FINGERPRINTING_SCENARIO.md).

## Consequences

Positive:

- Published values enable regression and comparison.
- The scenario enforces clean exchanges from the body to the gateway.
- Detail can be added incrementally without changing the domain objective.
- Gaps in proteomics, device, and gateway data become visible early.

Negative:

- The current baseline uses strong biological simplifications.
- A complete demonstrator is possible only after substantial parts of M3–M6.
- The two-gene fingerprint hypothesis requires independent biological and experimental review.

## Rejected alternatives

- **CAR-T first:** good performance baseline, but no complete nano-IoT/layer coupling and biologically extreme cell counts.
- **Liquid biopsy first:** existing figures, but weaker organ/message connection.
- **Metastasis prevention first:** the best long-term capstone, but too many simultaneous research uncertainties for the start.
