<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M5 Evidence Qualification

## Decision

M5 has sufficient **software-verification evidence** to pass its Roadmap gate.
It does not have biological, pharmacodynamic, therapeutic, or clinical
validation. Every checked M5 profile therefore remains classified
`software_test_surrogate`, and broad `CELL-*` research requirements remain
partial or research where their physiological scope exceeds the technical gate.

## Evidence inventory

| Increment | Verification reference | What it establishes | What it does not establish |
|---|---|---|---|
| M5.1 | closed-form reversible one-to-one binding | equation, units, receptor conservation, threshold time | biological ligand/receptor kinetics |
| M5.2 | exact amount/volume concentration and non-consuming inventory | neutral M4-to-M5 identity, time, units, ownership | dynamic tissue field or depletion |
| M5.3 | closed-form constant and segment-wise pulse solutions plus step refinement | ODE implementation and bounded trajectory semantics | physiological exposure history |
| M5.4 | analytical binomial equilibrium moments and fixed-seed gates | finite-receptor SSA, replay, distribution accounting | clinical sensitivity/specificity or cell heterogeneity |
| M5.5 | analytical constant-input equilibria, fixed ODE values, conservation, ODE/SSA topology equality, population comparison | shared generic reaction-network implementation | independently measured pathway kinetics |
| M5.6 | analytical first-order release/uptake solution and amount balance | activation gating and conservative ownership | pharmacokinetics, diffusion, efficacy, or toxicity |
| M5.7 | exact Hill half-max and synthetic chain reference | bounded arithmetic, commitment gate, neutral event | mechanistic or experimentally observed apoptosis |
| M5.8 | exact weighted aggregation, count-scale invariant, parameter sensitivities | scalable cohort arithmetic and explicit validity scope | biological population distribution or treatment response |

The third Gate M5 statement permits analytical **or** external reference data.
M5 satisfies it through independent analytical references for receptor binding
and constant-input reaction-network equilibria, supplemented by a shared
ODE/SSA comparison. This is a technical
acceptance criterion, not a substitute for external biological validation.

## External comparison candidate—not current evidence

A reviewed next-step candidate is BioModels
[`BIOMD0000000525`](https://www.ebi.ac.uk/biomodels/services/download/get-files/MODEL1403050002/3/BIOMD0000000525.pdf),
the Kallenberger et al. CD95/caspase-8 apoptosis model. Its BioModels report
describes a large ensemble of single-cell models fitted to single-cell and
population data and states that the encoded model is dedicated to the public
domain under CC0. It is attractive because it directly exposes the
single-cell/population distinction that M5.8 now makes explicit.

The earlier Eissing et al. receptor-induced caspase model is another compact
mechanistic candidate: *Bistability analyses of a caspase activation model for
receptor-induced apoptosis*, Journal of Biological Chemistry 279 (2004),
36892–36897, [DOI 10.1074/jbc.M404893200](https://doi.org/10.1074/jbc.M404893200),
[PMID 15208304](https://pubmed.ncbi.nlm.nih.gov/15208304/). It links fast
single-cell caspase activation with slower population-level behavior.

Neither candidate has been imported, mapped, reproduced, licensed for bundled
redistribution, or compared numerically with MEHLISSA in M5. Therefore neither
is counted as gate evidence. A later qualification package must freeze a model
version and checksum; review units, compartments, stimulus, cell line, and
license; reproduce published outputs in an independent solver; define a typed
adapter; and predeclare comparison metrics before mapping it to a MEHLISSA
scenario.

## Evidence gaps retained after M5

- a capillary-generated dynamic biomarker field rather than prescribed exposure;
- named ligand, receptor, cell type, device, drug, target, and disease context;
- jointly compatible kinetic, dose-response, population, and uncertainty data;
- external SBML/CellML simulator conformance and independently reproduced
  apoptosis trajectories;
- calibrated inter-cell heterogeneity and spatial correlation;
- pharmacokinetics, pharmacodynamics, metabolism, toxicity, resistance, and
  clearance; and
- prospective or retrospective biological/clinical validation.

These gaps are visible constraints on use. They are not failures of the
software gate and must not be erased by relabeling synthetic references as
physiological evidence.
