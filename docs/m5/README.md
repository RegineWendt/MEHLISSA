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

The M5 gate remains open. Its acceptance statements require:

- a molecular signal from the capillary model to trigger a cell reaction;
- a cell model to return a measurable event or state change to higher layers;
- receptor binding and the reaction network to be checked against analytical or
  external references;
- documented single-cell and population validity scopes; and
- a completed English User Guide gate-impact review.

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

## Planned increments

1. **M5.6 - activation and drug release:** couple a nanodevice decision to a
   conservative release and uptake contract.
2. **M5.7 - apoptosis and higher-layer feedback:** implement the first complete
   response event and return it through an explicit boundary.
3. **M5.8 - population model and gate review:** provide a scalable population
   variant, document both validity scopes, complete sensitivity/evidence work,
   and perform the mandatory User Guide review.

The numbering is a working implementation sequence, not evidence that later
increments are already designed or accepted. Each increment may be split when
scientific qualification or an architecture decision requires it.

## Current scientific boundary

M5.1 through M5.5 are software verification only. The receptor-binding models
prescribe a non-depleting external ligand reservoir. The
M5.3 trajectory is not yet produced by a dynamic capillary tissue field, and
the M5.2 snapshot remains constant during its exposure. M5.4 adds finite-count
reaction noise but no biological cell heterogeneity. Receptor abundance does
not change, and no binding site is internalized. Its threshold classification
rates describe only the two synthetic cohorts and are not clinical sensitivity
or specificity. M5.5 creates messenger and effector states, but its generic
network and kinetics are synthetic and no higher-layer state is changed.

M5.2 satisfies the first M5 statement at the synthetic software-contract level:
an M4 extracellular tissue signal triggers receptor binding and threshold
detection. The other four gate statements remain open at full M5 scope, as do
physiological and dynamic-tissue qualification of the first statement. M5.3 and
M5.4 add
analytical, convergence, stochastic-moment, and population evidence for the
receptor-binding part of the third statement. M5.4 also establishes documented
synthetic single-cell and population scopes toward the fourth statement. M5.5
adds the intracellular-network part of the third statement with shared ODE/SSA
software evidence. External or biological validation remains open.
