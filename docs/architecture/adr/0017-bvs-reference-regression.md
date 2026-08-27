<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0017: Separate BVS Dynamics and Perfusion Regression

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M2.4; `BODY-006`, `BODY-010`, `ORG-002`

## Context

BloodVoyagerS reports distribution experiments using 94 vessels. The
dissertation adds a 95th vessel and calibrates organ and regional flows much
more finely. A single supposedly exact reproduction value would conflate these
two model generations. In addition, the calculation called “standard
deviation” in the paper is methodologically described as a mean absolute vessel
deviation.

## Decision

1. BVS equilibrium, injection location, and population scaling are evaluated as comparative dynamic claims, not as identical reproduction of the 94-vessel code.
2. The dissertation's 23 perfusion targets are checked in a separate stationary gate and explicitly described as calibration regression, not independent physiological validation.
3. The dynamic metric is named normalized mean absolute deviation. Population scaling uses total variation.
4. All thresholds are fixed before the first complete run and stored in the machine-readable report.
5. The run is event-driven, integrates residence times exactly, uses a named deterministic random stream, and verifies exact population conservation.
6. A versioned JSON Schema and byte-identical golden reference make method, inputs, results, and gates automatically verifiable.

## Consequences

Positive:

- Published claims are operationalized reproducibly.
- Differences between BVS 2018 and the dissertation profile remain visible.
- Subsequent movement of tolerances is exposed by the report and golden reference.
- Long runs remain executable without an artificial global integration time step.

Limitations:

- The reference runner tests the same transition and transit-time model, but is a specialized event-driven measurement implementation rather than the general M2.3 time-step host.
- Passing calibration regression does not establish clinical or patient-specific validity.
- The future normative profile requires independent data and its own gates.

## Alternatives

- **Port the historical code unchanged:** rejected as the primary gate because it would preserve the obsolete 94-vessel contract and its implicit assumptions. It remains a comparison source.
- **Enforce paper figures as exact targets:** rejected because topology, transitions, and velocity model differ.
- **Check only the stationary flow graph:** rejected because this would cover neither transient transport nor injection-location and population effects.

## Verification

- `docs/m2/BVS_REFERENCE_REGRESSION.md`
- `data/reference-results/bvs95-dissertation-rest-m2.4.json`
- `bvs_reference_tests`
