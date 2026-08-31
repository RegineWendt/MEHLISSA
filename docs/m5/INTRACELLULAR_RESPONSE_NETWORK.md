<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Intracellular Response Network (M5.5)

## Purpose

M5.5 is the first cell model whose output is an intracellular state rather than
receptor occupancy. A prescribed bound-receptor fraction activates a conserved
messenger pool; active messenger activates a conserved effector pool. Both can
deactivate, and the first upward effector-threshold crossing is retained as the
cell response event.

The two implementations share one network contract:

- deterministic fractions integrated with bounded fixed-step RK4;
- finite messenger and effector molecule counts simulated with exact direct SSA;
- identical rates, receptor trajectory, initial state, threshold, and outputs;
- bounded traces and explicit integration/reaction budgets; and
- named per-cell SSA streams for exact replay and future parallel evaluation.

## Checked reference

The profile and schema are:

```text
examples/cell-models/synthetic-intracellular-response-v1.json
data/schemas/intracellular-response-profile/1.0.0.schema.json
```

The ten-second case starts with inactive pools and holds receptor occupancy at
`0.75`. With synthetic activation/deactivation rates, the ODE reference gives:

| Output | Checked value |
|---|---:|
| final active messenger fraction | `0.7499953918` |
| final active effector fraction | `0.7494434333` |
| first effector crossing of `0.5` | `2.202913832 s` |

The SSA uses 200 messenger and 200 effector molecules in each of 1,000 cells.
Its fixed-seed population gives:

| Output | Seeded SSA result |
|---|---:|
| mean final messenger fraction | `0.75017` |
| messenger-fraction variance | `0.000847521` |
| mean final effector fraction | `0.74938` |
| effector-fraction variance | `0.000931816` |
| cells reaching the response threshold | `1000 / 1000` |

Both means lie inside their predeclared ODE-comparison gates. Exact replay,
pool bounds, a silent zero-input network, malformed inputs, event-budget
exhaustion, and provenance rejection are also tested.

For the same constant `0.75` receptor input, the independent steady-state
balances are
`m* = (1.2 × 0.75) / (1.2 × 0.75 + 0.3) = 0.75` and
`e* = (0.8 × m*) / (0.8 × m* + 0.2) = 0.75`. A separate 60-second test verifies
that both RK4 states converge to these analytical equilibria within `1e-12`.
This supplies an analytical reaction-network reference in addition to the
ODE/SSA cross-implementation comparison.

## Interpretation boundary

This is a deliberately generic two-stage software surrogate. It establishes
reaction-network contracts, solver interchangeability, finite-count noise, and
an internal response event. It does not represent a named receptor, kinase,
transcription factor, cell type, drug, disease, or patient population. It has
no spatial localization, transcription, metabolism, competing pathway,
biological heterogeneity, feedback to receptor binding, or calibrated assay.

M5.6 consumes the effector event through a separate conservative activation and
release boundary. It does not reinterpret these synthetic kinetics as biological
evidence.

## Verification

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure `
  -R "intracellular|ODE|SSA"
```

See [ADR-0038](../architecture/adr/0038-shared-intracellular-ode-ssa-network.md)
and [M5 implementation evidence](README.md).
