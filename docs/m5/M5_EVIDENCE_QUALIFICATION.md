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

## Selected external comparison—not current evidence

[BCQ-1.1](../qualification/BCQ1_BIOLOGICAL_CELL_MODEL_SELECTION.md) has now
completed the candidate and licence screen. The selected family is the minimal
Kallenberger et al. CD95L-CD95-caspase-8 model: BioModels
`BIOMD0000000523` for CD95-overexpressing HeLa and `BIOMD0000000524` for
wild-type HeLa. Both are compact SBML Level 2 Version 4 average-cell artifacts
under CC0 1.0. Their exact public Git commits, file hashes, model dimensions,
rights boundary, and alternatives are machine checked.

The earlier note identified `BIOMD0000000525`. The evidence-led screen instead
selected 523/524 because the associated paper found the minimal cis/trans
mechanism sufficient. The larger 525/526 cis/trans-cis/trans pair remains a
same-publication structural-sensitivity companion, not independent evidence.
Rehm 2006 is the public CC0 fallback; the Eissing model remains conceptual only
because no stable, licensed machine artifact was established. A VEGF-A/VEGFR
endothelial model is retained for the later dynamic capillary-tissue-cell
program rather than used as this first, deliberately compact qualification.

BCQ-1.1 does not change the M5 evidence classification. The selected models
have not been imported, executed, mapped, or compared numerically with
MEHLISSA. Article and experimental-data rights are separate from the CC0 model
licence, and the public SBML artifacts each encode one average cell rather than
the reported population ensemble. BCQ-1.2 must predeclare the external solver,
observables, numerical tolerances, controls, failure handling, and no-refit
rules before reproduction begins.

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
