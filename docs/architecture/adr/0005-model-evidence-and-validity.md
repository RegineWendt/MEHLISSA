<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0005: Treat Model Evidence and Validity Explicitly

- **Status:** Accepted
- **Date:** 26 August 2026
- **Applies to:** all gates; `DATA-005` through `DATA-008`

## Context

MEHLISSA combines literature values, anatomical data sets, historical
simulation results, external models, and assumptions that have not yet been
confirmed. Earlier prototypes assigned values or adjusted results afterward
for practical reasons. Without clear labels, calibration could be interpreted
as validation, or a hypothetical mechanism as medically established.

The platform should be useful precisely where empirical data are missing.
Uncertainty must therefore not be hidden; it must be a modeled property.

## Decision

Every published model variant and every material parameter set receives a model
card containing at least:

- domain claim and validity scope;
- evidence class: `published-observation`, `independently-validated`, `calibrated`, `derived`, `expert-assumption`, or `research-hypothesis`;
- sources and data versions;
- units and population context;
- calibration method and separate validation data;
- uncertainties, sensitivity, and known counterexamples;
- responsible person and review date.

Result reports inherit this information through provenance. A model variant
fitted to target values must not be described as independently validated with
the same data. Historical MEHLISSA results are classified as reproduction
baselines, not automatically as physiological truth.

## Consequences

Positive:

- Scientific claims remain reviewable and differentiated.
- Deviations between models can be treated substantively rather than cosmetically.
- Partners can see specifically where wet-lab, clinical, or physiological data are missing.
- Scenarios can be used exploratively despite uncertainty.

Negative:

- Data and model maintenance takes additional time.
- Results become more complex than individual point values.
- Some parameters in current use will become visibly insufficiently supported.

## Minimum publication rule

Every publication derived from MEHLISSA Next must reference the software commit,
data-set versions, experiment manifest, seeds/replicates, separation of
calibration and validation, and the relevant model cards.
