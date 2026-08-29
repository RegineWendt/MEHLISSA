<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Historical FP9 Lung Timer Baseline

## 1. Decision and scope

M3.19 makes the dissertation's historical FP9 lung timing chain executable at
the experiment/scenario layer. It is a deliberately neutral Level A replay of
published MEHLISSA outputs. It does not add fingerprint, gene, collector, or
gateway branches to the simulation kernel, body model, coupler, or lung model.

The checked-in baseline is
`examples/scenarios/fp9-lung-historical-timer-v1.json`; schema
`data/schemas/fingerprint-timer-baseline/1.0.0.schema.json` makes the target,
population, timing semantics, sources, acceptance claim, and limitations
explicit. The reusable implementation is
`experiment/fingerprint_timer_baseline`: a different target, fingerprint, and
timing chain can run through the same code without a lung-specific branch.

This closes the additional FP9 criterion in ADR-0006. It does not implement the
complete fingerprinting demonstrator planned for M7.

## 2. Source reconstruction

The values were checked directly against the dissertation rather than copied
only from legacy CSV data:

- Table 6.3, page 184: FP9 assembly duration `15.99 s` from the NetTAS result;
- Table 6.4, page 186: FP9 maps lung tissue to historical organ index `61`;
- Section 6.5.1, pages 186-187: locators release once in their target tissue;
  an active message exists only after the assembly duration;
- Table 6.5 and Section 6.5.2, pages 188-189: left-arm-vein injection at
  historical segment 64, 1,000 locators across nine targets, first lung
  localization at `25 s`, and external-report times of `209 s` and `91 s` for
  1,000 and 10,000 collectors respectively; and
- page 190: all 111 lung-targeted locators have released by `29 s`, a useful
  later population-level regression that is not silently inferred by M3.19's
  first-event baseline.

The NANOCOM 2023 paper supports the two-gene proteome-fingerprint and conditional
message-assembly concept. It does not provide the historical MEHLISSA transport
times and is therefore marked `concept-only` in the executable baseline.

## 3. Timing semantics

The dissertation's *Erfassungszeit* is an end-to-end time: injection through
message collection and subsequent reporting at the wrist. It is not an
additional delay after localization and assembly.

| Event | FP9 time from injection | Meaning |
|---|---:|---|
| injection | 0.00 s | locators and collectors enter at the left arm vein |
| first localization/tile release | 25.00 s | first matching locator reaches lung tissue |
| first active message | 40.99 s | `25.00 + 15.99 s` fixed assembly timer |
| external report, 1,000 collectors | 209.00 s | historical end-to-end output |
| external report, 10,000 collectors | 91.00 s | historical end-to-end output |

Consequently, the executable result reports derived post-assembly
collection-and-return spans of `168.01 s` and `50.01 s`. These differences are
derived bookkeeping quantities, not separately measured or published model
parameters. The historical implementation omitted wrist-readout latency as an
assumed constant; M3.19 records that omission and does not relabel it as zero
measured latency.

## 4. Executable contract

Loading a baseline performs JSON Schema validation followed by semantic checks:

- localization cannot precede injection;
- assembly duration is positive;
- an external report cannot precede message activation;
- locator and target allocations are positive and bounded;
- collector cohort sizes are unique and positive; and
- requesting an unreported collector cohort fails rather than interpolating a
  new scientific result.

Execution emits the stable event sequence
`injection -> first_localization -> message_active -> external_report`. The
event records retain fingerprint and target-region IDs. Automated tests verify
both published collector cohorts, causal failure cases, stable event names, and
the same implementation with a synthetic non-lung target.

## 5. Architectural boundary

M3.19 belongs to `experiment/` because it composes historical observations and
a timer into a scenario result. It does not claim that the current pulmonary
0D transit model predicts the dissertation's 25 s localization or 91 s report.
Those values arose from the earlier complete-body fingerprint abstraction.

Later milestones replace inputs while preserving the event vocabulary:

- M4 can provide capillary arrival and tissue residence;
- M5 can derive detection and binding rather than assuming localization success;
- M6 can separate collection, wrist passage, readout, and communication; and
- M7 can run population distributions, stochastic replicates, errors, and
  sensitivity while retaining this historical timer as a regression.

## 6. Scientific limitations

This baseline is software and historical-model verification, not physiological
or clinical validation. It does not model gene-product concentration, binding,
tile diffusion, message decay, collector encounter probability, or radio
communication. The two collector cases must not be used as a fitted scaling law
for other counts. FP9 gene identities and biological robustness remain future
data work and do not need to be resolved to verify the historical timer
abstraction.
