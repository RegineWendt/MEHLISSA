<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# BCQ-1.4–1.7 Prospective MEHLISSA Qualification Protocol

This document freezes the second half of BCQ-1 before its first authoritative
MEHLISSA qualification archive is generated. The machine-readable authority is
`data/qualification/biological-cell-model-integration-protocol-v1.json`.

The protocol covers four increments:

1. **BCQ-1.4:** a typed M5 adapter for the two selected minimal average-cell
   cases;
2. **BCQ-1.5:** a no-refit comparison against the independently generated
   COPASI archive, numerical convergence, and a bounded 525/526 structural
   audit;
3. **BCQ-1.6:** an explicit decision not to invent a population distribution,
   plus deterministic local parameter sensitivities;
4. **BCQ-1.7:** a runner-independent archive and claim review, including every
   blocked limitation.

## Pre-protocol development disclosure

The adapter was compiled and exercised during test development before this
protocol was frozen. Those feasibility trajectories are temporary development
outputs, are not retained as qualification evidence, and cannot pass a BCQ
gate. The first authoritative archive must be created from the committed
protocol and implementation hashes.

## Typed boundary and equation separation

`QualifiedCd95ApoptosisAdapter` is the M5 boundary. It accepts only the frozen
source case, `CD95L=16.6`, an explicit `unresolved-model-native` unit tag, and
the declared time grid. It rejects invented SI units, stimulus changes, and
parameter changes. All 18 source species are returned, with four named primary
observables.

`KallenbergerMinimalMechanism` separately implements the 13 SBML reaction
rates and the `CD95act` assignment rule. This separation lets reviewers compare
the code to the source equations without confusing input mapping with pathway
mathematics. Qualification-only perturbations use the mechanism directly and
do not widen the public adapter contract.

## Frozen numerical evaluation

The primary MEHLISSA calculation uses classical fixed-step RK4 with maximum
internal step `0.01`; the tightened run uses `0.005`. Both emit 961 points on
the unchanged model-native `0`–`240` grid at interval `0.25`. The primary run
must reproduce every state in the COPASI reference within
`1e-7 + 1e-7 * max(1, |reference|)`. Primary and tightened MEHLISSA runs must
agree within `1e-9 + 1e-8 * max(1, |tightened|)`. Initial states, nine
invariants, nonnegativity, deterministic replay, identities, and file hashes
are independently checked.

## Structural and population boundaries

The 525/526 models are from the same publication and are not independent
validation data. Their role is limited to a source-identity and equation-graph
audit: each retains 18 species but has 19 rather than 13 reactions and 15
rather than 12 global parameters. They are not silently added as runtime
variants and no trajectory outcome from them is used to select a threshold.

The public package does not establish a reusable row-level distribution of
initial protein values and correlations. BCQ-1.6 therefore retains the two
average-cell cases. It calculates local central sensitivities at 1% and 0.5%
steps only as model-behavior diagnostics. These sensitivities are not a
population, probability distribution, confidence interval, or patient model.

## Review and permissible conclusion

A separate checker must reconstruct every numerical metric from the archived
CSV files and verify source, licence, code-to-equation, structural, population,
documentation, and claim boundaries. External human reviewer attestation is
not currently available. BCQ-1 may therefore close with explicit blocked
findings, but the variant cannot be called biologically qualified.

If all unblocked gates pass, the allowed statement is:

> MEHLISSA contains a typed, no-refit reimplementation of the selected
> Kallenberger minimal average-cell SBML mechanism whose trajectories reproduce
> the independently generated COPASI reference within frozen numerical limits.

Publication-curve reproduction, ensemble reproduction, individual-cell
prediction, endothelial realism, organ realism, clinical validity, independent
experimental validation, and biological qualification remain forbidden claims.
