<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Scientific qualification

This directory contains prospective, bounded qualification plans that build on
the milestone evidence without rewriting it after results are known.

| Protocol | Status | Purpose |
|---|---|---|
| [PCQ-1 pulmonary and capillary qualification](PULMONARY_CAPILLARY_QUALIFICATION_PROTOCOL.md) | design v0.1.0, [PCQ-1.2 source screen](PCQ1_EVIDENCE_SOURCE_SCREEN.md), [PCQ-1.3 amendment](PCQ1_PRE_OUTCOME_AMENDMENT.md) v0.2.0, [PCQ-1.4 rights-aware ingress](PCQ1_DATA_INGRESS.md), [PCQ-1.5 uncertainty/identifiability analysis](PCQ1_UNCERTAINTY_IDENTIFIABILITY.md), [PCQ-1.5a repository-first data audit](PCQ1_REPOSITORY_FIRST_DATA_AUDIT.md), and an unsent [request and waiting-period package](PCQ1_DATA_REQUEST_PACKAGE.md); no participant outcomes acquired | qualify participant-level pulmonary hemodynamics, regional perfusion, capillary volume and transit, and their joint coherence |
| [BCQ-1 biological cell-model qualification](BCQ1_BIOLOGICAL_CELL_MODEL_SELECTION.md) | BCQ-1.1 through BCQ-1.7 complete: selection, [COPASI protocol](BCQ1_REPRODUCTION_PROTOCOL.md), disclosed [replay amendment](BCQ1_REPRODUCTION_PROTOCOL_AMENDMENT_1.md), [external-solver reproduction](BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md), [typed MEHLISSA protocol](BCQ1_MEHLISSA_QUALIFICATION_PROTOCOL.md), and [bounded completion result](BCQ1_MEHLISSA_QUALIFICATION_RESULT.md) | one published average-cell mechanism is computationally qualified; publication-series, population, external-human-review, biological, patient, and clinical gates remain blocked |
| [DCCQ-1 dynamic capillary-tissue-cell qualification](DCCQ1_QUALIFICATION_PLAN.md) and [DCCQ-1.2 evidence source screen](DCCQ1_EVIDENCE_SOURCE_SCREEN.md) | DCCQ-1.1 prospective audit and DCCQ-1.2 target/source screen complete; DCCQ-1.3 equation and evaluation protocol is next | human VEGF-A165a/VEGFR2 HUVEC target selected with NRP1 explicit, SI conversion still to freeze, unlicensed repository code excluded from reuse, and independent validation still blocked |

BCQ means **Biological Cell-model Qualification**. Its BCQ-1.1 machine
authority is
`data/qualification/biological-cell-model-candidate-register-v1.json`. The
screen selects the minimal Kallenberger 2014 `BIOMD0000000523` and
`BIOMD0000000524` pair, freezes their public source commits and SBML hashes,
separates CC0 model rights from article and experimental-data rights, and
retains three ranked alternatives. It proves an auditable selection, not a
model reproduction or biological result. BCQ-1.2 supplied the required
no-refit protocol before either model was executed or mapped to MEHLISSA.

BCQ-1.2 now freezes that machine-checkable protocol in
`data/qualification/biological-cell-model-reproduction-protocol-v1.json`.
It binds COPASI command line 4.46 Build 300 and LSODA, the two source hashes and
complete initial vectors, the unchanged `CD95L = 16.6` stimulus, a six-run
primary/replay/tightened matrix, a 961-point model-native grid, four primary
observables, source-derived conservation sums, numeric gates, ten negative
controls, and a non-overwriting failure-retaining result archive. Because the
SBML files omit explicit time and substance units, the protocol forbids an SI,
seconds, or minutes interpretation until a citable mapping is frozen.
BCQ-1.3 executed all six primary, replay, and tightened trajectories in
COPASI. The first exact-zero replay protocol remains a disclosed failed result;
the committed version 1.1 amendment defines a much tighter-than-solver numerical
equivalence rule for a new run. The authoritative archive passes nine unblocked
computational gates and ten negative controls. Quantitative publication
alignment remains blocked without a rights-compatible numeric reference.

BCQ-1.4 through BCQ-1.7 are now complete under the prospectively frozen
`data/qualification/biological-cell-model-integration-protocol-v1.json`.
The typed no-refit adapter and separate 13-reaction mechanism reproduce both
18-state COPASI references, replay byte-identically, converge under RK4 step
halving, and preserve every invariant. The 525/526 pair remains a verified
same-publication structural comparison. The public package does not establish
a reusable joint protein distribution, so the result retains an explicit
average-cell boundary and reports stable local sensitivities as diagnostics
only. A runner-independent checker closes the archive and claim review.

The permitted outcome is a `computationally-qualified published average-cell
mechanism`. Publication-curve alignment, population-ensemble reproduction,
external human reviewer attestation, and biological qualification are blocked.
The synthetic M5 fixtures retain their separate `software_test_surrogate`
classification.

The machine-readable design authority for PCQ-1 is
`data/qualification/pulmonary-capillary-qualification-protocol-v1.json`. Its
schema, frozen-asset hashes, endpoint hierarchy, negative controls, and claim
boundary are checked in continuous integration. A checked protocol demonstrates
prospective study discipline; it is not itself physiological validation.

The PCQ-1.2 source-selection authority is
`data/qualification/pulmonary-capillary-evidence-candidate-register-v1.json`.
Its separate schema and semantic checker retain thirteen candidates, rankings,
access and rights boundaries, measurement jointness, source independence,
public outcome exposure, rejection reasons, and unsent next actions. The
coordinated [PCQ-1 data-request package](PCQ1_DATA_REQUEST_PACKAGE.md) indexes
the [D'Souza](DSOUZA_2025_DATA_REQUEST.md),
[University of Arizona](../m3/UA_ICPET_DATA_REQUEST.md),
[Bailey](BAILEY_2019_DATA_REQUEST.md), and
[Lassen](LASSEN_2023_FEASIBILITY_REQUEST.md) drafts and defines work that can
remain outcome blind while replies are pending. These are templates, not
records of external contact or granted data access. Lassen is deliberately a
feasibility inquiry rather than a request to transfer participant records.

The PCQ-1.3 analysis authority is
`data/qualification/pulmonary-capillary-preoutcome-amendment-v1.json`. It
freezes eight observation models, source activation guards, sample and
precision floors, six primary numeric gates, missingness and statistical
rules, and explicit inconclusive or blocked states. In particular, the current
capillary residence time cannot be compared directly with Lassen's pulmonary-
trunk-to-left-atrium transit measurement. A machine-checked amendment is
prospective analysis discipline, not a physiological qualification result.

The PCQ-1.4 ingress authority is
`data/qualification/pulmonary-capillary-data-ingress-policy-v1.json`. Its
manifest-first checker requires approved rights, privacy, confirmed cohort
independence, and an explicit outside-repository quarantine path before opening
measured data. Four strict family schemas and CC0 synthetic fixtures verify the
normalized adapters without using outcomes. Command output contains metadata,
never raw observations; whole-pulmonary transit remains blocked.

The PCQ-1.5 uncertainty authority is
`data/qualification/pulmonary-capillary-uncertainty-plan-v1.json`. It covers
all six protocol uncertainty classes and all nine endpoints, registers
distribution and correlation availability, compares seven pulmonary structures
without winner selection, verifies seven convergent local sensitivities, and
tests nine identifiability designs. Missing joint distributions block global
variance attribution, and the whole-pulmonary transit observation model stays
blocked. These are prospective design and software results, not participant-
level qualification evidence.

The PCQ-1.5a repository-availability authority is
`data/qualification/pulmonary-capillary-repository-audit-v1.json`. It binds the
unchanged PCQ-1.2 source register and records exact target-study findings, five
non-equivalent repository alternatives, access/licence constraints, an
outcome-blind file boundary, and the still-unsent contact queue. It found no
drop-in primary dataset and changes no frozen source role or numeric rule.

Each later protocol must preserve the separation between:

- software and numerical verification;
- literature parameterization;
- calibration;
- source-disjoint validation; and
- clinical evidence, which is outside the current MEHLISSA claim.
