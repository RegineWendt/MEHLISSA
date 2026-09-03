<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Paper 1 publication measurements

This directory retains the locked technical measurements for the proposed
MEHLISSA platform/methods paper.  The measurements are evidence about software
execution, reproducibility, observation overhead, bounded artifacts, and
access-path parity.  They are not biological or clinical validation evidence.

Except for third-party material already carrying a more specific declaration,
the generated measurement data and this documentation are licensed under
CC-BY-4.0; see `LICENSES/CC-BY-4.0.txt` at the repository root.

## Contents

| Experiment | Readable entry point | Complete raw archive |
|---|---|---|
| `P1-E1-BODY-OBSERVATION` | `P1-E1-campaign-report.json` and `P1-E1-machine-plan.json` | `P1-E1-BODY-OBSERVATION.zip` |
| `P1-E2-M7-RESOURCE` | `P1-E2-summary.json` | `P1-E2-M7-RESOURCE.zip` |
| `P1-E3-ACCESS-PARITY` | `P1-E3-access-parity-report.json` | `P1-E3-ACCESS-PARITY.zip` |

The ZIP files retain the original directory hierarchy and every official
individual measurement, log, result, provenance record, negative control, and
analysis artifact emitted by the respective runner.  Their hashes are recorded
in the release-candidate manifest and `SHA256SUMS.json`.

## Execution identity

- protocol: `technical-experiment-protocol-v2.0.0.json`;
- measured C++ source/build commit: `55322f8b5bc7177d90514e2fcdc66ed106014a78`;
- body benchmark executable SHA-256:
  `6cc69ff8cfc2975690804f668b9f51759c0a1628a2f6dbd3955ba2ccf48c3cfb`;
- M7 executable SHA-256:
  `070c0c36646ad3d4cdecece1737cce83344fab5e6b9905a2cbeb9100aef074ff`;
- host class: Windows 11, AMD64, eight logical CPUs, 33,895,956,480 bytes
  physical memory, MSVC Release build;
- official measurement interval: 3 September 2026, UTC times retained in the
  machine-readable reports.

The access-parity harness received a line-ending portability correction in
commit `96169ad` before the accepted parity run.  That correction changes only
the comparison of canonical JSON candidates on Windows and does not alter the
simulator executable or the measured result.

## Retained setup deviation

`P1-E1-invalid-setup-report.json` is the report from an invalid first attempt.
The temporary machine plan accidentally copied O2's
`maximum_measurement_records = 100` into O1, while the frozen repository plan
requires `0`.  The driver rejected all 32 O1 attempts with error
`MEHLISSA-E2004`; the runner therefore marked the campaign unsuitable for
analysis.  No value from that attempt is used as an official result.

The official machine plan was then compared with the frozen repository plan
and matched after normalizing line endings and substituting only the required
non-placeholder machine label.  The complete official rerun finished with
112/112 completed attempts, no violations, and `suitable_for_analysis = true`.

