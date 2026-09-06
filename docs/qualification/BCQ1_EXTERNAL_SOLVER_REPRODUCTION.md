<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BCQ-1.3 Independent External-Solver Reproduction

## Outcome

BCQ-1.3 is complete. COPASI 4.46 Build 300 independently executed the two
unchanged, content-hashed BioModels SBML artifacts selected in BCQ-1.1. The
authoritative run is `20260906T071228Z-5dd09984d838` under
`results/bcq1/kallenberger-minimal/`.

Nine unblocked computational gates and all ten negative controls pass.
Quantitative alignment to a publication curve remains **blocked** because no
rights-compatible, machine-readable reference series has been frozen. This
result establishes source-artifact execution and numerical stability; it does
not establish biological qualification. M5 therefore remains
`software_test_surrogate`.

## What was independently reproduced

| Case | Public artifact | Meaning in the source | Frozen stimulus |
|---|---|---|---:|
| calibration-like case | `BIOMD0000000523` | one average CD95-overexpressing HeLa cell | unchanged source number `CD95L = 16.6` |
| publication prediction-like case | `BIOMD0000000524` | one average wild-type HeLa cell | unchanged source number `CD95L = 16.6` |

Each artifact has one compartment, 18 species, 13 reactions, 12 global
parameters, and one assignment rule. No value was fitted, optimized, converted,
or substituted. COPASI is external to MEHLISSA and is not a Workbench or runtime
dependency, so this exercise checks the public source equations before they are
implemented through a MEHLISSA adapter.

The source SBML omits explicit model time, substance, compartment, and species
units. Consequently all axes and state values remain labelled
`unresolved-model-native`. COPASI's displayed import defaults are retained as
solver provenance only and are not treated as evidence that the source time is
seconds or that its states are SI concentrations.

## Protocol lineage and disclosed failure

The original version 1.0 protocol required bit-exact equality between separate
COPASI processes. Its completed run `20260906T070728Z-7ea0d2273f31` failed that
gate: `BIOMD0000000523` differed by at most `2.637534635141492e-11` absolute,
or `2.0191528122055834e-13` after scale normalization, while
`BIOMD0000000524` was bit-identical. That run remains failed.

[Amendment BCQ-1.2-A1](BCQ1_REPRODUCTION_PROTOCOL_AMENDMENT_1.md) was then
committed before any version 1.1 output was generated. It replaces exact-zero
comparison with the following numerical-equivalence rule:

```text
abs(primary - replay) <= 1e-12 + 1e-12 * max(1, abs(replay))
```

This is at least one thousand times tighter in relative scale than the primary
LSODA relative tolerance of `1e-9`. It also adds a 120-second per-process
timeout after a retained setup attempt stalled. No scientific input, model,
solver setting, grid, observable, or other acceptance rule changed.

## Execution and results

For each artifact, COPASI ran a primary trajectory, an independent primary-
settings replay, and a tightened trajectory. Every run contains the same 961
points from `0` to `240` inclusive at intervals of `0.25` model-time units and
reports all 18 species. Primary LSODA tolerances were `1e-9` relative and
`1e-12` absolute; tightened tolerances were `1e-11` and `1e-13`.

| Check | Worst observed value | Acceptance | Result |
|---|---:|---:|---|
| replay equivalence | 0.308362 of permitted difference | at most 1.0 | PASS |
| tightened-solver convergence over four primary observables | 0.00338727 of permitted difference | at most 1.0 | PASS |
| source-derived conservation residual | `1.000444171950221e-11` | invariant-specific scale-aware limits | PASS |
| minimum state value | `0.0` | no material negative state | PASS |
| reporter-direction increase | `0.0` | non-increasing within numerical margin | PASS |
| negative controls | 10 of 10 rejected the invalid case | 10 of 10 | PASS |

The replay comparison was bit-identical for `BIOMD0000000523`. For
`BIOMD0000000524`, its maximum absolute replay difference was
`6.070877134334296e-11`, maximum scale-normalized difference
`3.6184268964088887e-13`, and maximum fraction of the prospective limit
`0.3083619554792158`. This is correctly described as numerically equivalent,
not bit-identical.

The tightened-run comparison covered `PrER_mGFP`, `PrNES_mCherry`, `p43`, and
`p18`. The worst normalized result was `0.003387266814670928`, leaving a large
margin to the frozen convergence threshold. All nine source-derived invariants,
both reporter-direction checks, exact initial-state checks, finite-value checks,
and grid checks passed for every applicable run.

## Negative controls

The runner demonstrated failure for:

1. a changed source byte or hash;
2. an unselected BioModels accession;
3. swapped 523/524 case semantics;
4. a source-value or rule modification;
5. a changed solver identity;
6. an invented seconds, minutes, or SI label;
7. a truncated or malformed output grid;
8. a non-finite or numerically invalid result;
9. enabled parameter fitting; and
10. a biological or held-out-population overclaim.

The controls use disposable files or in-memory copies and do not modify the
frozen public artifacts.

## Retained attempt history

| Run | Status | Interpretation |
|---|---|---|
| `20260906T065959Z-7ea0d2273f31` | failed output configuration | parser rejected a delimiter-free report |
| `20260906T070045Z-7ea0d2273f31` | failed output configuration | first separator correction remained invalid |
| `20260906T070125Z-7ea0d2273f31` | interrupted stalled process | partial output retained; no result claim |
| `20260906T070728Z-7ea0d2273f31` | failed v1 gate | all six runs completed but exact-zero replay failed |
| `20260906T071114Z-5dd09984d838` | superseded archive layout | computations passed, mandatory `protocol.json` alias absent |
| `20260906T071228Z-5dd09984d838` | authoritative partial/pass result | nine computational gates PASS; publication alignment BLOCKED |

These failures are evidence of fail-closed behavior, not discarded trials or
retroactively successful results.

## Reproduction and independent verification

Acquire the two source repositories at the exact commits named in the
[BCQ-1.2 protocol](BCQ1_REPRODUCTION_PROTOCOL.md), verify the frozen SBML
hashes, and obtain CopasiSE from the official COPASI 4.46 Build 300 release.
Then run:

```powershell
python scripts/run_biological_cell_model_reproduction.py `
  --copasi "C:\path\to\CopasiSE.exe" `
  --source-523 "C:\path\to\BIOMD0000000523.xml" `
  --source-524 "C:\path\to\BIOMD0000000524.xml"
```

The source artifacts and COPASI binary are deliberately not vendored. The
checked-in result can be verified without COPASI or network access:

```powershell
python scripts/check_biological_cell_model_reproduction_result.py
python -m unittest tests/test_biological_cell_model_reproduction_result.py
```

The verifier checks schema, protocol lineage, archive membership and hashes,
solver/source provenance, every raw CSV grid and initial state, replay and
convergence limits, conservation, nonnegativity, reporter direction, result
metrics, negative-control records, attempt retention, and claim boundaries.

## Claim boundary and next step

The supported claim is:

> The two selected unchanged BioModels average-cell artifacts were
> independently executed in COPASI with numerically equivalent replay and
> stable primary-observable results under tightened tolerances on the frozen
> model-time grid.

This is not quantitative reproduction of a publication curve, the complete
held-out population analysis, endothelial-cell or organ realism, patient
prediction, treatment evidence, or clinical validation. BCQ-1.4 next defines a
typed MEHLISSA adapter for only the declared stimulus, states, observables, and
still-unresolved unit/time semantics. Cross-engine comparison begins only in
BCQ-1.5.

## Sources

- [COPASI 4.46 Build 300 release](https://github.com/copasi/COPASI/releases/tag/Build-300)
- [BioModels `BIOMD0000000523`](https://github.com/biomodels/BIOMD0000000523)
- [BioModels `BIOMD0000000524`](https://github.com/biomodels/BIOMD0000000524)
- [Kallenberger et al. 2014](https://pmc.ncbi.nlm.nih.gov/articles/PMC4208692/)
