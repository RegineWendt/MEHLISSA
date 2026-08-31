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

## Planned increments

1. **M5.2 - capillary/tissue-to-cell signal hand-off:** define signal ownership,
   compartment mapping, temporal concentration semantics, and a conservative
   M4-to-M5 reference route.
2. **M5.3 - time-dependent binding and detection:** add a numerical ODE adapter,
   pulses or trajectories, and comparison with the M5.1 analytical limit.
3. **M5.4 - stochastic single-cell binding:** add an SSA variant, named random
   streams, population distributions, and declared false-positive/false-negative
   experiments.
4. **M5.5 - intracellular response network:** convert detection into a
   schema-defined signaling state and compare ODE/SSA or an external solver
   against a shared reference.
5. **M5.6 - activation and drug release:** couple a nanodevice decision to a
   conservative release and uptake contract.
6. **M5.7 - apoptosis and higher-layer feedback:** implement the first complete
   response event and return it through an explicit boundary.
7. **M5.8 - population model and gate review:** provide a scalable population
   variant, document both validity scopes, complete sensitivity/evidence work,
   and perform the mandatory User Guide review.

The numbering is a working implementation sequence, not evidence that later
increments are already designed or accepted. Each increment may be split when
scientific qualification or an architecture decision requires it.

## Current scientific boundary

M5.1 is software verification only. The constant reservoir excludes depletion
and transport feedback. A homogeneous deterministic occupancy fraction cannot
represent cell-to-cell variability or molecule-count noise. Receptor abundance
does not change, and no binding site is internalized. The threshold is not a
classifier with measured sensitivity or specificity. No capillary output is yet
wired to the request, and no intracellular or higher-layer state is changed.

Consequently, none of the five M5 gate statements is closed by M5.1 alone.
