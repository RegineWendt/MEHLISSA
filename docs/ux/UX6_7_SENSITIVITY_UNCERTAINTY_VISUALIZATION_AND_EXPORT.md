<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UX-6.7 Sensitivity, Uncertainty, Visualization, and Export

**Status:** locally accepted on 3 September 2026; publication and supported CI
remain pending an explicit push.

## Purpose and boundary

UX-6.7 makes the existing six-run campaign visually inspectable without
turning a very small software experiment into statistical or clinical
evidence. The workbench therefore uses three deliberately different views:

1. replicate observations show seed-to-seed variation under one fixed
   configuration;
2. sweep observations show deterministic parameter contrasts;
3. paired differences show comparison minus baseline within a shared seed.

The package does not add a physiological parameter law, an uncertainty model,
population sampling, confidence intervals, credible intervals, or a clinical
performance claim. A single scenario is explicitly rejected as an uncertainty
sample.

## Analysis authority

`GET /api/run/analysis?id=<job-id>` is a capability-protected, read-only local
endpoint. `RunWorkspace.analysis()` accepts only a server-owned completed
campaign job, resolves its allowlisted retained result, and loads it through
`mehlissa.load_campaign_result`. The browser receives a presentation contract;
it does not parse the authoritative campaign JSON or recalculate scientific
results.

The versioned `1.0.0` response contains:

- the accepted reader name, result schema version, retained path, and SHA-256;
- all four declared response metrics: sensitivity, specificity, detected, and
  assembled;
- exact observations grouped by campaign design, with run identity, role,
  replicate index, seed, parameter, parameter value, response, and inclusion;
- descriptive sample count, missing count, mean, sample standard deviation,
  and observed minimum-to-maximum range;
- accepted within-seed paired differences;
- the original declared sensitivity hooks and their qualification;
- explicit interpretation text and the non-clinical boundary;
- deterministic JSON, CSV, and SVG export names and CSV content.

Missing values remain missing and are marked excluded. They are never imputed
or converted into zero observations.

## Statistical interpretation

| Displayed quantity | Meaning | Meaning it does not have |
|---|---|---|
| replicate points | retained outcomes from different declared seeds at one fixed configuration | independent human or biological samples |
| mean and sample standard deviation | descriptive summaries of the available retained replicate values | estimates of population parameters when the design does not support that claim |
| observed range | minimum and maximum among retained values | confidence interval, credible interval, tolerance interval, or physiological bound |
| sweep plot | response at each explicitly executed collector count | global sensitivity, interpolation law, or response outside the executed values |
| paired difference | comparison minus baseline for one shared seed | causal effect or population treatment effect |

Every chart states its response unit and sample count. The analysis boundary
states that inferential uncertainty is unavailable. The current campaign has
only two replicate observations, one run at each sweep value, and one paired
comparison. Consequently, the views are useful for software reproducibility
and design inspection, not for uncertainty quantification.

## Current reference result

The accepted FP9 collector-count campaign currently produces sensitivity and
specificity of `0.5` and positive detected/assembled outcomes in all six runs.
The 1,000-to-10,000 collector contrast is therefore flat for these four
responses, and the paired differences are zero. UX-6.7 displays that result
without adding jitter, smoothing, fabricated error bars, or a fitted trend.

This flat response is an important limitation, not a visualization defect. A
future scientifically qualified campaign must introduce additional bounded
parameters and response metrics before the workbench can demonstrate richer
sensitivity behavior.

## Visual contract

For the selected response metric, the workbench renders:

- an accessible replicate dot plot;
- a collector-count sweep plot with log-positioned x values;
- a paired-difference plot centered on the explicit zero reference;
- an exact data table containing all plotted values and exclusions.

The SVG figures have programmatic titles and descriptions, visible values,
axis labels, response units, sample counts, high-contrast colors, and a text
table fallback. Color is not the only carrier of meaning. Layout reflows on a
narrow viewport without creating global horizontal overflow.

## Reproducible export

The analysis panel provides three export forms:

- **analysis JSON** is the complete server response, including source identity,
  values, summaries, hooks, boundaries, and deterministic export metadata;
- **analysis CSV** is generated server-side from those same accepted values and
  contains one row per metric observation and paired difference, including the
  source-result SHA-256 on every row;
- **figure SVG** is the exact currently displayed plot with embedded metadata:
  API version, source-result SHA-256, metric, unit, view, and sample count.

No external plotting service, remote font, telemetry endpoint, or JavaScript
chart dependency is used. Exports stay local and can be matched back to the
retained campaign result through its SHA-256.

## Security and failure behavior

- all analysis requests require the ephemeral workbench capability;
- only completed server-owned campaign jobs are eligible;
- unknown, running, failed, cancelled, and individual-scenario jobs return no
  analysis observations;
- the retained campaign result remains the authority and is never overwritten;
- browser code receives only bounded, versioned values from the server;
- export filenames are server-provided and bound to the opaque local job ID.

## Local acceptance evidence

UX-6.7 is locally accepted because:

- the analysis response matches the accepted `CampaignResult` groups, values,
  sensitivity hooks, and paired-difference calculation;
- all six retained runs and all four declared metrics are represented;
- the two replicate sensitivity values produce `n=2`, mean `0.5`, sample
  standard deviation `0.0`, and observed range `[0.5, 0.5]` with no
  inferential interval;
- the sweep points match the retained 1,000 and 10,000 collector runs exactly;
- the deterministic CSV contains 24 observation rows plus four paired-
  difference rows and binds every row to the result SHA-256;
- a single scenario and a cancelled campaign expose zero analysis
  observations;
- desktop and mobile browser checks cover metric switching, chart labels,
  source identity, exports, persistent interpretation boundaries, console
  errors, and horizontal overflow;
- the complete Windows/MSVC test suite remains green.

## Next increment

UX-6.8 performs task-based usability and assistive-technology review, packages
the workbench for supported clean installations, and closes the graphical
workbench release gate.
