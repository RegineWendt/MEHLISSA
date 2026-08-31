<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0035: Non-consuming capillary-to-cell signal hand-off

## Status

Accepted for M5.2.

## Context

M4.5 accumulates conserved substance amounts in endothelium, interstitium, and
cell-labelled tissue inventories. M5.1 independently evaluates reversible
receptor binding from a ligand concentration. Connecting them requires an
explicit conversion from amount to concentration, compatible identifiers, and a
time window.

The M5.1 constant-reservoir model does not consume ligand. Moving or subtracting
an amount during this coupling step would therefore invent unsupported mass
kinetics. Directly including a Capillary type in the Cell library, or a Cell
type in the Capillary library, would also violate the four-layer replacement
principle.

## Decision

Define a versioned `ExtracellularSignalSample` in the neutral model-coupling
library. A sample contains signal, source model and compartment, represented
amount and volume, observation time, validity duration, and the fixed semantics
`non_consuming_uniform_inventory_snapshot`. Concentration is derived as amount
divided by represented volume rather than stored redundantly.

Make the capillary bed implement the neutral signal-source interface for its
extracellular `endothelium` and `interstitium` inventories. Sampling is allowed
only at the current synchronization time and does not modify any inventory.

Create a separate `MEHLISSA::cell_cosimulation` adapter. Its strict profile maps
one source model, compartment, and signal to one cell model, ligand, and
compartment, plus represented volume, exposure duration, and initial receptor
occupancy. The adapter validates both sides, rejects duplicate sample IDs, and
passes the derived concentration to the implementation-neutral M5.1 interface.

Keep the base co-simulation library independent of the Cell library. The new
adapter is built as a separate target only when cell models are enabled.

## Consequences

An M4 tissue signal can now trigger an M5 receptor-binding evaluation without
either model layer depending on the other's implementation. Signal identity,
amount-to-concentration conversion, compartment, observation time, exposure
window, and duplicate processing are auditable. The unchanged M4 inventories
make the non-consuming semantics testable.

This is not a physical transfer or ligand-balance model. It assumes that the
complete accumulated inventory is homogeneous in a declared volume and that its
concentration remains constant for the exposure. It has no depletion, reverse
flux, clearance, spatial gradient, temporal interpolation, or feedback. The
cell response is returned to the caller but is not yet persisted or delivered
to a higher layer.

## Alternatives considered

- **Subtract bound ligand from the M4 inventory:** rejected because the current
  receptor model is a constant reservoir and reports receptor state, not a
  conservative ligand-consumption amount.
- **Pass a bare concentration:** rejected because its originating amount,
  represented volume, compartment, and time scope would be lost.
- **Use the M4.13 receiver concentration directly:** deferred because M4.13 is a
  separate synthetic channel case and does not yet deposit its result in a
  versioned tissue compartment.
- **Let Cell depend on Capillary:** rejected because model layers must remain
  independently replaceable.
- **Treat the cumulative tissue inventory as a dynamic concentration field:**
  rejected; uniformity and constant exposure are explicit surrogate assumptions.

## Affected requirements and gates

- CELL-002 now includes an executable, schema-defined M4-to-M5 concentration
  mapping in addition to the M5.1 analytical binding baseline.
- The first M5 gate statement passes at the declared synthetic software-contract
  level: an M4 tissue signal triggers receptor binding and threshold detection.
- CELL-001 and CELL-003 remain open for time-dependent biomarker fields and a
  release-diffusion-binding chain.
- Higher-layer feedback, intracellular reaction networks, stochastic cells, and
  population variants remain open M5 work.
