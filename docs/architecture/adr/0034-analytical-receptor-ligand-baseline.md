<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0034: Analytical receptor-ligand cell baseline

## Status

Accepted for M5.1.

## Context

M4 provides molecular-channel concentrations and conservative tissue hand-offs,
but MEHLISSA has no executable cell layer. The first M5 increment must introduce
that layer without prematurely coupling it to one capillary implementation or
claiming biological validity for unqualified kinetics.

Receptor binding is a useful first boundary because it converts an extracellular
ligand concentration into receptor occupancy and a measurable detection event.
For constant ligand concentration, reversible one-to-one binding also has an
exact analytical solution. That makes it suitable as an independent software
reference before ODE, stochastic, population, and intracellular models are
added.

## Decision

Create an independent `MEHLISSA::cell_model` library with an implementation-
neutral receptor-ligand request and response. The first implementation uses a
homogeneous cell compartment and the reversible reaction
`R + L <-> RL` under a constant extracellular ligand reservoir:

```text
df/dt = kon L (1 - f) - koff f
f_eq = kon L / (kon L + koff)
f(t) = f_eq + (f(0) - f_eq) exp(-(kon L + koff)t)
```

Here `f` is the bound-receptor fraction. The response reports total, free, and
bound receptor amounts, equilibrium and final occupancy, and the first
threshold-crossing time when it occurs within the observation interval.

Define the second-order association rate as a dimension-safe SI quantity. Load
all executable parameters, evidence scope, sources, limitations, and the
analytical reference case through a strict versioned JSON schema. Keep the
checked-in profile synthetic and classify it as a software-test surrogate.

Do not connect M4 output in this increment. M5.2 will define the explicit,
conservative capillary/tissue-to-cell signal hand-off against this stable cell
contract.

## Consequences

The cell layer now has an independently testable boundary and exact reference
answer. Alternative deterministic, stochastic, or external implementations can
reuse the request and response without changing callers. Receptor conservation,
units, threshold semantics, and profile provenance are explicit.

The model assumes a constant ligand reservoir, no ligand depletion, one-to-one
binding, a homogeneous deterministic cell, and fixed receptor abundance. It has
no cooperativity, internalization, receptor turnover, spatial gradients,
intracellular reaction network, population variability, noise, false-positive
or false-negative classification, or feedback to another layer. Its threshold
is a software event, not a validated diagnostic decision.

## Alternatives considered

- **Couple directly to the M4.13 wall sink:** rejected because a cross-section-
  averaged loss term is not a receptor-occupancy contract and would entangle
  independent layers.
- **Start with a full intracellular signaling network:** deferred because it
  would remove the simple analytical oracle needed to verify the new boundary.
- **Start with stochastic simulation only:** deferred until deterministic
  semantics and conservation have a verified reference.
- **Embed binding in the capillary library:** rejected because receptor and cell
  state belong to the independently replaceable cell layer.
- **Use literature-valued kinetics immediately:** deferred until ligand,
  receptor, cell type, state, parameter provenance, and validation endpoints can
  be qualified together.

## Affected requirements and gates

- CELL-002 advances from research-only to a partial executable binding,
  dissociation, and detection-threshold baseline.
- The analytical part of the M5 receptor-binding gate is established; an
  intracellular reaction-network reference remains open.
- CELL-001 and CELL-003 remain open for time-dependent biomarker fields and the
  M4-to-M5 signal path.
- CELL-004 and CELL-005 remain open for intracellular and apoptosis responses.
- The required M5 User Guide maintenance begins with this developer-level
  experiment and its explicit non-claims.
