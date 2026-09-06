<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BCQ-1.3 External-Solver Reproduction Report

**Run:** `20260906T071114Z-5dd09984d838`  
**Decision:** PASS for the nine computational source-reproduction gates; BLOCKED for quantitative publication-curve alignment.  
**Solver:** COPASI 4.46 Build 300, LSODA, unchanged source numbers.  
**Models:** `BIOMD0000000523` and `BIOMD0000000524`, one average cell each.  
**Units:** `unresolved-model-native`; COPASI import defaults are recorded but are not treated as source evidence.  

All six frozen primary, replay, and tightened trajectories contain 961 points.
Source identity, import structure, initial state, finite output, deterministic
replay, numerical convergence, conservation, nonnegativity, reporter direction,
and all ten negative controls passed. Full numeric metrics and every run log are
retained beside this report.

The allowed claim is: The frozen public average-cell SBML artifacts were independently executed in COPASI with reproducible, numerically converged trajectories and source-derived invariants.

This is not quantitative reproduction of a publication figure, the complete
held-out population analysis, biological qualification, endothelial or organ
realism, patient prediction, or clinical evidence. M5 therefore remains
`software_test_surrogate`. The next increment is BCQ-1.4 typed MEHLISSA adapter.
