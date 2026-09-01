<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Levels C-E and the Holistic Fingerprinting Run (M7.5-M7.7)

## M7.5 - explicit tiles and assembly

A positive Level-B event releases explicit, uniquely identified FP9 tiles from
the selected locator payload. The reference surrogate requires all nine tile
identities before it marks the message complete. With the selected historical
cohort, nine of 111 target locators supply the required tiles. An eight-locator
negative control remains incomplete and cannot continue the positive workflow.

The `15.99 s` assembly duration remains the dissertation's historical FP9
NetTAS result. MEHLISSA executes an all-required-tiles rule but does not claim
to execute NetTAS, diffusion, collision, degradation, or sequence chemistry.

## M7.6 - executed communication path

The complete positive path now uses the existing M6 implementations:

```text
locator.synthetic.1
  -> collector.synthetic.uplink.1
  -> gateway.synthetic.wrist
  -> scheduled BAN adapter
  -> station.synthetic.analysis-control
```

The locator sends the assembled fingerprint over the selected 10 ms link. The
collector clears the received message, creates a trace-preserving measurement,
and uses the selected 20 ms gateway link. The active gateway publishes a typed
measurement; the selected 10 ms BAN transport delivers its frame to the
external station. The result separates local transmitter, receiver, and link
energy from BAN transmitter, receiver, and link energy.

The gateway endpoint is now the thirteenth explicit scenario artifact. It is
validated and hashed like every other dependency; M7 no longer relies on an
implicit filename to construct the active gateway.

All communication latencies, delivery schedules, sizes, capacities, and energy
costs are synthetic software-contract values. Collector return remains aligned
with the historical FP9 aggregate return interval because no independent
anatomical return-transport observation is available.

## M7.7 - sensitivity and misclassification

Level E runs labelled concentration/exposure cases through exactly the same
Level-B detector. Every case is reported as true positive, true negative,
false positive, or false negative. Sensitivity, specificity, false-positive
rate, and false-negative rate include two-sided 95% Wilson intervals.

The default four-case campaign intentionally creates one of each classification:

| Analyst label | Concentration | Detection | Classification |
|---|---:|---:|---|
| present | reference | yes | true positive |
| present | below threshold | no | false negative |
| absent | reference | yes | false positive |
| absent | below threshold | no | true negative |

The resulting 0.5 sensitivity and 0.5 specificity are demonstrations of metric
semantics, not performance estimates. Labels are scenario inputs rather than
clinical diagnoses, and four cases cannot characterize an assay.

## Holistic result 2.0.0

`run_holistic_fingerprinting_scenario` executes Levels A-E in order and writes
one strict `fingerprinting-result/2.0.0` document. It contains:

- run identity, master seed, target, all thirteen definition/schema paths and
  SHA-256 digests;
- the ten-stage qualified identity trace;
- concentration, occupancy, threshold, and detection-event results;
- every released tile and the assembly decision;
- both local routes, gateway measurement, BAN frame, external report, timing,
  delivery counts, and separated communication energy; and
- every Level-E case, classification, metric interval, varied parameter, and
  validity limitation.

Repeated execution with the same manifest and data produces a byte-identical
report digest.

