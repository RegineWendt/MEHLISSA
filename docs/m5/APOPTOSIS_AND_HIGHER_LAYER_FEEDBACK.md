<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Apoptosis and Higher-Layer Feedback (M5.7)

## Purpose

M5.7 turns the intracellular drug inventory produced by M5.6 into the first
complete, externally observable cell response. It deliberately separates three
concerns:

```text
M5.6 intracellular amount
    -> M5.7 cell-specific dose-response surrogate
    -> neutral cell-state event for a higher layer
```

The cell model returns a measurable `effect_fraction` and either `viable` or
the irreversible state `apoptosis_committed`. The co-simulation adapter emits a
versioned higher-layer event only for the committed state. A viable response is
observable locally but remains silent at the event boundary.

## Response equation and checked reference

For intracellular amount `A`, half-maximal amount `A50`, and Hill coefficient
`n`, the synthetic effect is

```text
effect(A) = A^n / (A50^n + A^n)
```

The implementation evaluates the equivalent log-ratio form to remain stable at
very small and very large finite amounts. Commitment occurs when the effect is
at least the profile threshold. The strict profile and schema are:

```text
examples/cell-models/synthetic-apoptosis-response-v1.json
data/schemas/apoptosis-response-profile/1.0.0.schema.json
```

The reference consumes the actual M5.6 result of
`74.7645072415509 nmol`. With `A50 = 50 nmol` and `n = 2`, it produces:

| Quantity | Checked value |
|---|---:|
| synthetic effect fraction | `0.6909662593017674` |
| commitment threshold | `0.65` |
| cell state | `apoptosis_committed` |
| reported event time | `12.202913832 s` |

The event time is the M5.5 activation offset `2.202913832 s` plus the M5.6
ten-second observation interval. It is the time at which the state is observed,
not an inferred biological time of apoptosis commitment.

## Higher-layer boundary

The neutral `CellStateEvent` contract carries:

- contract, event, and event-type identity;
- source model, source cell, and source request;
- target model and target port;
- observation time; and
- a named finite measurement value.

The adapter verifies the configured source identity before emitting
`cell.apoptosis_committed`. The target in the synthetic profile is a scenario
port, but the contract does not depend on a concrete M6 or M7 implementation.

## Interpretation boundary

This is a software-verification surrogate, not a pharmacodynamic or clinical
model. It uses final intracellular amount, not concentration or exposure
history. Drug identity, cell identity, `A50`, Hill coefficient, commitment
threshold, and target scenario are synthetic. There is no target binding,
metabolism, repair, resistance, toxicity competition, pathway dynamics,
caspase cascade, morphology, execution delay, clearance, or tissue feedback.

Accordingly, `apoptosis_committed` means only that the declared synthetic
software rule crossed its threshold. It must not be interpreted as a predicted
therapeutic response, dose, efficacy, toxicity, safety outcome, or patient
result. Biological qualification, concentration-based kinetics, uncertainty,
and population heterogeneity remain for M5.8 and later work.

## Verification

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure `
  -R "apoptosis|feedback"
```

Tests cover the checked Hill value, exact half-maximal response, stable large
finite input, inactive and subthreshold viable cases, strict identity and
provenance rejection, and the complete M5.5-to-M5.7 higher-layer event path.

See [ADR-0040](../architecture/adr/0040-synthetic-apoptosis-and-higher-layer-feedback.md),
[M5 implementation evidence](README.md), and
[M5.6 conservative delivery](CONSERVATIVE_DRUG_DELIVERY.md).
