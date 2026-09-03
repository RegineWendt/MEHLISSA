<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.5 Result Dashboard and Comparison

**Status:** locally accepted on 3 September 2026; publication and supported CI
remain pending an explicit push.

## 1. Purpose and boundary

UX-6.5 makes completed MEHLISSA results understandable without requiring a
researcher to inspect JSON or CSV first. It adds a result workspace to the local
graphical workbench while preserving the scientific source of truth:

- scenario dashboards are projected by `mehlissa.load_result`;
- campaign dashboards are projected by `mehlissa.load_campaign_result`;
- the browser performs presentation only and contains no result-schema or
  simulation semantics;
- retained result JSON, campaign CSV, and the established UX-3 HTML report
  remain directly inspectable; and
- values remain descriptive software outputs. They are not clinical or
  patient-specific performance claims.

## 2. Individual-run dashboard

A completed individual scenario exposes detection and fingerprint-assembly
outcomes; sensitivity and specificity estimates; collector count and seed;
every cumulative runtime stage in milliseconds; and every Level-E analysis case
with truth label, observed detection, classification, and final bound fraction.
Missing values remain visibly missing.

The UX-3 report is generated after the simulator has produced a valid scenario
result. It is retained in the unique workbench job directory and opened in a
sandboxed frame after an authenticated local fetch. The authoritative result
JSON remains available beside it.

## 3. Side-by-side comparison

The comparison selector admits exactly two different, completed individual
scenario jobs from the current session. It compares collector count, detection,
assembly, sensitivity, and specificity. Numeric differences are `right - left`;
Boolean outcomes are placed side by side without a fabricated numeric delta.
If either value is absent, the row states that it is missing and excluded.

## 4. Campaign dashboard

A completed campaign is grouped using the accepted campaign reader. The view
retains the design distinction between independent reproducibility replicates,
a collector-count sweep, and the shared-seed baseline/comparison pair. Each row
shows run identity, role, replicate, seed, collector count, and four supported
outcomes. Paired differences come from
`CampaignResult.paired_differences()`. A missing baseline or comparison yields
`null` and is marked excluded, never coerced to zero. The authoritative campaign
JSON and CSV remain directly available.

## 5. Missing and failed-run policy

Only a `completed` job with a readable versioned result contributes observations.
For every other state the response contains `available: false`, the exact job
status, `observation_count: 0`, and an exclusion reason. It contains no summary,
case, stage, group, or metric arrays. A failed, cancelled, partial, or running job
therefore cannot silently change a denominator or look like a negative result.

## 6. Private workbench interfaces

These process-private adapters are not public MEHLISSA APIs:

| Interface | Meaning |
|---|---|
| `GET /api/run/dashboard?id=<job-id>` | Return a reader-backed dashboard or an explicit zero-observation exclusion record. |
| `POST /api/run/compare` | Compare `left_id` and `right_id` when both identify completed scenario jobs. |
| `GET /api/run/artifact` | Fetch an allowlisted retained JSON, CSV, text, or HTML artifact inside the selected job directory. |

They retain the ephemeral session header, loopback Host validation, non-caching
responses, safe text insertion, sandboxed HTML preview, and repository-bounded
artifact resolution.

## 7. Local acceptance evidence

UX-6.5 is locally accepted because:

1. scenario dashboard values are checked against `load_result`;
2. campaign groups and paired sensitivity differences are checked against
   `load_campaign_result`;
3. a 1,000-versus-10,000-collector comparison produces a 9,000 difference;
4. cancelled and synthetic failed jobs expose zero observations and cannot enter
   a comparison;
5. each completed scenario retains authoritative and UX-3 report output;
6. the complete Windows/MSVC suite passes; and
7. desktop and narrow-viewport browser review covers dashboards, tables,
   comparison, report drill-through, console state, and horizontal overflow.

UX-6.6 next adds complete provenance, evidence, licensing, maturity, hash, and
interpretation-boundary panels to these reader-backed views.
