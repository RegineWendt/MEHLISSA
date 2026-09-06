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

## Computationally qualified published average-cell mechanism

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

BCQ-1.1 did not by itself change the M5 evidence classification. The completed
[BCQ-1.2 prospective protocol](../qualification/BCQ1_REPRODUCTION_PROTOCOL.md)
it freezes COPASI 4.46 Build 300/LSODA, both source cases, an unresolved-unit
guard, a six-run primary/replay/tightened matrix, the four direct observables,
source-derived invariants, numeric gates, ten negative controls, and the result
archive before execution. The subsequent
[BCQ-1.3 external-solver result](../qualification/BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md)
executes all six frozen trajectories in COPASI, passes nine unblocked
computational gates and ten negative controls, and retains the original exact-
zero replay failure plus the pre-run versioned amendment. The result establishes
numerically stable execution of the unchanged public equations.

The completed
[BCQ-1.4–1.7 qualification](../qualification/BCQ1_MEHLISSA_QUALIFICATION_RESULT.md)
now adds a typed, no-refit MEHLISSA adapter and a separate implementation of
the 13 source reactions. Both 18-state, 961-point MEHLISSA trajectories agree
with the independent COPASI archive inside prospective limits, replay exactly,
converge under step halving, and preserve the source invariants. The 525/526
pair is verified only as a same-publication structural companion. All local
parameter sensitivity step checks pass.

This promotes only the named Kallenberger minimal variant to
`computationally-qualified published average-cell mechanism`. It does not
promote the configurable synthetic M5 fixtures, and it does not establish
biological qualification. Publication-curve alignment, a reusable population
ensemble, and external human reviewer attestation remain blocked. Article and
experimental-data rights remain separate from the CC0 model licence, and each
selected source artifact still represents one average cell.

## Evidence gaps retained after M5

- a capillary-generated dynamic biomarker field rather than prescribed exposure;
- named ligand, receptor, cell type, device, drug, target, and disease context;
- jointly compatible kinetic, dose-response, population, and uncertainty data;
- external experimental or publication-series validation beyond the completed
  MEHLISSA/COPASI numerical conformance of the selected average-cell mechanism;
- calibrated inter-cell heterogeneity and spatial correlation;
- pharmacokinetics, pharmacodynamics, metabolism, toxicity, resistance, and
  clearance; and
- prospective or retrospective biological/clinical validation.

These gaps are visible constraints on use. They are not failures of the
software gate and must not be erased by relabeling synthetic references as
physiological evidence.
