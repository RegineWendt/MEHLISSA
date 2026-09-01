<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M6 Gate Review – End-to-End Nano-IoT

**Review date:** 1 September 2026

**Reviewed implementation baseline:** the M6.7 commit containing this review

**Result:** passed — all five Roadmap Gate M6 statements are executable or
documented at the declared synthetic software-contract level

## 1. Review method

The review required versioned device, message, gateway, BAN, station, external
simulator, and resilience profiles; positive and negative automated tests;
dependency inspection; separate communication metrics; explicit scientific and
security non-claims; and the mandatory English User Guide impact review. It
distinguished gate acceptance from physical-channel qualification,
cybersecurity, clinical control, and biological actuation.

## 2. Roadmap gate criteria

| Gate M6 criterion | Status | Evidence and finding |
|---|---|---|
| a simulated molecular detection produces a traceable external measurement | satisfied at synthetic contract level | M6.2 maps a successful M5 receptor response to a neutral detection event and local message; M6.3 routes it, M6.4 publishes a gateway measurement, and M6.5 wraps and delivers the measurement to the external station while preserving source event, correlation, device, gateway, time, size, and content identity |
| an external control command can reach a nanodevice or drug release | satisfied for nanodevice delivery | M6.5 approves one causally bound command, returns it through the BAN and gateway, and M6.3 routes it through a relay to an actuator nanodevice; receipt is explicit, while drug release and physiological effect are deliberately not executed |
| communication and physiology models can be exchanged independently | satisfied | body, organ, capillary, and cell targets do not depend on the IoT target; M6.2 uses a separate cosimulation adapter; local link and BAN transports are replaceable; M6.6 adds typed and JSON external-simulator clients without a physiological dependency |
| communication metrics are reported separately from biological results | satisfied | local, multihop, and BAN sessions retain attempts, deliveries, bytes, loss, corruption, expiry, delivered latency, and transmitter/receiver/link energy in communication-only snapshots; M6.7 confirms non-delivery accounting without station or biological mutation |
| English User Guide passed gate-impact review | satisfied | metadata, conceptual introduction, capability/non-claim lists, experiment catalog, decision aid, technical M6.1–M6.7 workflows, resilience interpretation, evidence links, architecture links, and next-milestone text were reviewed and updated |

## 3. Resilience and boundary-misuse findings

The strict M6.7 profile contains twelve cases covering availability, integrity,
miscontrol, traceability, capacity, and local resources. Six focused test cases
with 99 assertions verify the catalog and execute every injected class against
the production M6 boundaries.

Loss, corruption, and expiry remain typed communication outcomes with exact
attempt and byte accounting. Wrong targets, disallowed content, correlation
mismatch, duplicates, replay, response-identity mismatch, and exhausted
capacities/resources fail at the owning boundary. No rejected case creates an
extra station measurement, command approval, gateway acceptance, external
client call, transmission count, or energy debit.

## 4. Verification evidence

- MSVC Debug build with warnings as errors: passed;
- focused M6.7 resilience suite: 99 assertions in 6 test cases, passed;
- complete local CTest regression: 264/264 passed;
- clang-format and clang-tidy on the M6.7 implementation: passed; and
- GitHub Linux/GCC, Windows/MSVC, and Linux/Clang analysis/sanitizer jobs are
  required to pass before the reviewed commit is accepted.

## 5. User Guide impact review

The mandatory review found user-visible impact and updated:

- covered-software metadata from open M6.1–M6.6 increments to accepted M6;
- the Nano-IoT conceptual introduction and current capability list;
- a non-expert resilience/boundary-misuse experiment with interpretation and
  limitations;
- the first-experiment decision aid;
- a technical M6.7 section with files, expected behavior, focused checks,
  threat scope, and non-claims; and
- maintenance and next-milestone text so M7 is now the open gate.

No CLI command was invented. M6 remains a component/developer reference
workflow, and that access level stays explicit.

## 6. Requirements disposition

| Requirement | Review disposition |
|---|---|
| `IOT-001` | `DONE` retained: versioned device types, composable capabilities, payloads, targets, resources, lifecycle, and strict profiles are executable |
| `IOT-002` | `DONE` retained: the in-body network, gateway, BAN, external station, governed return, and actuator-nanodevice receipt compose |
| `IOT-003` | `PART` retained: an active logical gateway exists, but anatomical placement, range, hardware, and wearable qualification remain open |
| `IOT-004` | `PART` retained: counts, bytes, latency, prescribed outcome fractions, energy, and bounded capacities are executable; calibrated noise, interference, throughput, queueing, and channel statistics remain open |
| `IOT-005` | `DONE` retained: the external network-simulator boundary is versioned, payload-free, and physiologically independent; a concrete validated simulator model remains optional |

The incomplete physical and security scope does not contradict the five
narrower software gate statements.

## 7. Accepted limitations and exit decision

M6 does not establish anatomical radio placement, propagation, range,
interference, noise physics, throughput, queueing, retransmission, a concrete
BAN protocol, validated hardware, authentication, encryption, cryptographic
integrity, clinical authorization, fail-safe control, or command-driven drug
release and physiology. All reference capacities, outcomes, timings, and energy
values are synthetic.

Gate M6 passes as a technical milestone. M7 may compose the accepted M2–M6
contracts into the first complete fingerprinting demonstrator. Any later
physical-network, cybersecurity, or biological-actuation work must add a new
versioned model and evidence scope rather than silently strengthening the M6
claims.
