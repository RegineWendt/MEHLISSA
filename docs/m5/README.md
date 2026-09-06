<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M5 Implementation and Evidence

## Objective

M5 adds an independently replaceable cell layer that turns local molecular
signals into binding, detection, intracellular state, and ultimately a
measurable cellular response. It must preserve the distinction between software
verification and biological validation.

The M5 gate passed its formal review on 31 August 2026. Its accepted statements
are:

- a molecular signal from the capillary model to trigger a cell reaction;
- a cell model to return a measurable event or state change to higher layers;
- receptor binding and the reaction network to be checked against analytical or
  external references;
- documented single-cell and population validity scopes; and
- a completed English User Guide gate-impact review.

This is a synthetic software-contract milestone, not biological validation.
See the [formal gate review](M5_GATE_REVIEW.md) and
[evidence qualification](M5_EVIDENCE_QUALIFICATION.md).

## Implemented increments

### M5.1 - analytical receptor-ligand cell baseline

- independent `MEHLISSA::cell_model` library;
- stable receptor-ligand request and response contracts;
- typed cell volume, concentrations, receptor amounts, second-order association
  rate, dissociation rate, and observation duration;
- exact reversible one-to-one binding for a constant ligand reservoir;
- final and equilibrium occupancy plus first threshold-crossing time;
- explicit total/free/bound receptor balance;
- strict profile schema `1.0.0` with model identity, evidence scope, sources,
  limitations, and an executable reference case;
- a synthetic, CC BY 4.0 profile whose analytical result is verified in tests;
- negative tests for incompatible ligand/compartment, nonphysical inputs,
  invalid thresholds, and incomplete provenance; and
- an initial M5.1 update to the English User Guide.

See [Receptor-Ligand Baseline](RECEPTOR_LIGAND_BASELINE.md) and
[ADR-0034](../architecture/adr/0034-analytical-receptor-ligand-baseline.md).

### M5.2 - capillary/tissue-to-cell signal hand-off

- neutral extracellular-signal contract in `MEHLISSA::model_coupling`;
- explicit signal, source model, source compartment, amount, represented volume,
  observation time, and validity duration;
- concentration derived dimension-safely from amount divided by volume;
- non-consuming uniform-inventory snapshot semantics;
- M4 capillary source for endothelium and interstitium inventories without a
  dependency on Cell;
- separate `MEHLISSA::cell_cosimulation` adapter without adding a Capillary
  dependency to the Cell library;
- strict mapping profile schema `1.0.0` with source/target identity, exposure,
  provenance, validity, limitations, and executable reference results;
- duplicate-sample prevention and retry after a rejected stale-time attempt;
- complete synthetic M4.5 exchange-to-M5.1 receptor response test; and
- proof that observation leaves all M4 tissue inventories unchanged.

See [Capillary-to-Cell Signal Hand-off](CAPILLARY_CELL_SIGNAL_HANDOFF.md) and
[ADR-0035](../architecture/adr/0035-non-consuming-capillary-cell-signal-handoff.md).

### M5.3 - time-varying receptor binding and detection

- separate time-varying request/response without changing the exact M5.1
  contract;
- prescribed piecewise-constant, left-closed ligand trajectories with strict
  identity, time, order, concentration, and origin checks;
- fixed-step classical fourth-order Runge-Kutta integration of reversible
  one-to-one receptor binding;
- exact step boundaries at every concentration knot;
- explicit rate-step safety criterion and maximum integration-step budget;
- bounded state samples plus final and peak occupancy, receptor amounts, and
  first upward threshold crossing;
- constant-input convergence against the independent M5.1 closed-form result;
- a segment-wise analytical pulse reference covering signal onset, detection,
  withdrawal, and dissociation;
- exact final total/free/bound receptor balance; and
- negative tests for malformed trajectories, unsafe solver configuration,
  exhausted step budgets, no-signal exposure, and invalid profile metadata.

See [Time-Varying Receptor Binding](TIME_VARYING_RECEPTOR_BINDING.md) and
[ADR-0036](../architecture/adr/0036-time-varying-receptor-ligand-ode.md).

### M5.4 - stochastic binding and population classification

- exact Gillespie direct SSA for finite free/bound receptor counts;
- caller-owned named random streams and one stable stream per cohort cell;
- bounded event traces and exact integer receptor conservation;
- population mean, variance, quantiles, event, and random-draw summaries;
- declared positive and negative cohorts with TP/FN/FP/TN accounting;
- analytical binomial means and variances as independent references; and
- fixed-seed results inside predeclared moment and FP/FN gates.

See [Stochastic Receptor Binding](STOCHASTIC_RECEPTOR_BINDING.md) and
[ADR-0037](../architecture/adr/0037-stochastic-receptor-binding-and-population-classification.md).

### M5.5 - intracellular response network

- shared receptor-to-messenger-to-effector activation/deactivation topology;
- conserved bounded messenger and effector pools;
- deterministic fixed-step RK4 and exact finite-count Gillespie SSA variants;
- common piecewise-constant receptor-occupancy input and effector response event;
- strict profile binding both solvers to identical kinetics and thresholds;
- checked ODE state and response-time references;
- 1,000-cell named-stream SSA population compared with the ODE means; and
- silent-input, replay, conservation, budget, schema, and provenance tests.

See [Intracellular Response Network](INTRACELLULAR_RESPONSE_NETWORK.md) and
[ADR-0038](../architecture/adr/0038-shared-intracellular-ode-ssa-network.md).

### M5.6 - conservative nanodevice drug delivery

- versioned activation signal derived from a consistent M5.5 ODE or SSA event;
- stable device, payload, source-network, source-request, and time identity;
- sealed payload when no intracellular threshold is reached;
- analytical first-order device release and cellular uptake;
- explicit device, extracellular, and intracellular amount ownership;
- exact balance and an analytical equal-rate limit;
- strict profile with synthetic reference values, evidence, and limitations; and
- end-to-end M5.5-to-M5.6, conservation, negative, and provenance tests.

See [Conservative Nanodevice Drug Delivery](CONSERVATIVE_DRUG_DELIVERY.md) and
[ADR-0039](../architecture/adr/0039-conservative-nanodevice-release-and-uptake.md).

### M5.7 - apoptosis and higher-layer feedback

- numerically stable, bounded Hill surrogate from intracellular drug amount to
  a measurable effect fraction;
- explicit `viable` and irreversible `apoptosis_committed` cell states;
- silent higher-layer boundary for inactive or subthreshold viable responses;
- versioned implementation-neutral cell-state event with source, target, time,
  type, and measurement identity;
- separate co-simulation adapter so cell biology does not depend on a scenario;
- strict profile binding the M5.6 reference, response rule, feedback route,
  evidence, and limitations; and
- complete M5.5-to-M5.7 reference plus analytical, gating, identity,
  provenance, and boundary tests.

See [Apoptosis and Higher-Layer Feedback](APOPTOSIS_AND_HIGHER_LAYER_FEEDBACK.md)
and [ADR-0040](../architecture/adr/0040-synthetic-apoptosis-and-higher-layer-feedback.md).

### M5.8 - population scale, evidence, and gate review

- cohort-compressed deterministic apoptosis population with exact weighted
  aggregate counts, fractions, and mean effect;
- work proportional to cohort count rather than represented cell count;
- caller-bounded cohort-detail retention while all cohorts contribute to
  aggregates;
- a checked one-trillion-cell/four-cohort reference and a scale-equivalent
  1,000-cell comparison;
- predeclared half-max and threshold sensitivities;
- one model-selection table separating homogeneous single-cell, explicit-cell
  stochastic, and compressed-population validity scopes;
- an evidence audit that keeps analytical verification separate from external
  biological validation;
- a completed mandatory English User Guide impact review; and
- formal acceptance of all five Gate M5 statements at the synthetic
  software-contract level.

See [Population Scale and Validity](POPULATION_SCALE_AND_VALIDITY.md),
[M5 Evidence Qualification](M5_EVIDENCE_QUALIFICATION.md),
[M5 Gate Review](M5_GATE_REVIEW.md), and
[ADR-0041](../architecture/adr/0041-cohort-compressed-apoptosis-population.md).

## M5.1 reference result

The checked-in profile uses synthetic values and asks how a receptor population
responds to a constant `0.0003 mol/m3` ligand concentration for ten seconds.
With `kon = 1000 m3/(mol s)`, `koff = 0.1 s-1`, and zero initial occupancy:

| Output | Expected value |
|---|---:|
| equilibrium bound fraction | `0.75` |
| bound fraction after 10 s | `0.7362632708334493` |
| first crossing of a `0.5` threshold | `2.7465307216702746 s` |
| receptor balance error | numerical zero within test tolerance |

These values verify the implementation against its closed-form solution. They
do not represent a particular ligand, receptor, cell type, disease, drug, or
human population.

## M5.3 reference results

The checked pulse is off for two seconds, exposes the receptor to
`0.0003 mol/m3` for five seconds, and returns to zero concentration for five
seconds. With the M5.1 kinetics and a `0.05 s` RK4 step:

| Output | Segment-wise analytical value |
|---|---:|
| peak bound fraction | `0.6484985375725405` |
| bound fraction after 12 s | `0.39333424581655096` |
| first crossing of a `0.5` threshold | `4.746530721670274 s` |

The constant reference separately reproduces the M5.1 ten-second result. Runs
at `0.5 s`, `0.25 s`, and `0.125 s` demonstrate decreasing numerical error
under step halving.

## M5 completion

M5.1 through M5.8 and the formal gate review are complete. Later biological
qualification is new versioned work, not an unrecorded extension of the
accepted synthetic references.

## Current scientific boundary

M5.1 through M5.8 are software verification only. The receptor-binding models
prescribe a non-depleting external ligand reservoir. The
M5.3 trajectory is not yet produced by a dynamic capillary tissue field, and
the M5.2 snapshot remains constant during its exposure. M5.4 adds finite-count
reaction noise but no biological cell heterogeneity. Receptor abundance does
not change, and no binding site is internalized. Its threshold classification
rates describe only the two synthetic cohorts and are not clinical sensitivity
or specificity. M5.5 creates messenger and effector states, but its generic
network and kinetics are synthetic and no higher-layer state is changed.
M5.6 transfers a synthetic payload conservatively from device to extracellular
and intracellular inventories after a response event. It does not model spatial
diffusion, binding, dose response, efficacy, toxicity, or apoptosis.
M5.7 adds an amount-based synthetic Hill response, irreversible commitment
label, and neutral higher-layer event. It does not establish pharmacodynamics,
biological apoptosis, efficacy, toxicity, safety, or clinical relevance.
M5.8 aggregates that deterministic response over prescribed homogeneous amount
cohorts. Its one-trillion-cell reference demonstrates computational scaling,
not population realism; it adds no biological heterogeneity, correlation,
lineage, spatial interaction, or evidence.

M5.2 satisfies the first M5 statement at the synthetic software-contract level:
an M4 extracellular tissue signal triggers receptor binding and threshold
detection. Physiological and dynamic-tissue qualification remains open. M5.3 and
M5.4 add analytical, convergence, stochastic-moment, and population evidence
for the receptor-binding part of the third statement. M5.4 establishes explicit
stochastic single-cell and population scopes; M5.8 adds a scalable compressed
population and the complete fourth-statement validity comparison. M5.5
adds the intracellular-network part of the third statement with shared ODE/SSA
software evidence. External or biological validation remains open.
M5.6 advances `CELL-003` with executable activation, release, and uptake, but
the release-diffusion-binding chain remains open. M5.7 advances `CELL-005` and
satisfies the measurable higher-layer-response gate statement at the synthetic
software-contract level. The evidence qualification and mandatory User Guide
review are complete. M5 is closed as a technical gate; the limitations above
remain binding.

Post-gate [BCQ-1.1](../qualification/BCQ1_BIOLOGICAL_CELL_MODEL_SELECTION.md)
has selected and licence-screened the minimal Kallenberger 2014
CD95L-CD95-caspase-8 BioModels pair `BIOMD0000000523`/`0524`. This is an
auditable next-model decision only. The
[BCQ-1.2 protocol](../qualification/BCQ1_REPRODUCTION_PROTOCOL.md) now freezes
the independent solver, source cases, missing-unit guard, runs, outputs,
numeric rules, controls, and archive before execution. The artifacts are still
not imported or executed, so no M5 evidence status changes until the BCQ-1.3
external-solver reproduction, typed adapter, cross-engine checks, population
decision, and independent review have been completed.
