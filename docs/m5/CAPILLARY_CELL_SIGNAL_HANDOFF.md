<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Capillary-to-Cell Signal Hand-off

## Purpose

M5.2 connects the independently verified M4.5 and M5.1 boundaries. It answers a
software-contract question: can a substance accumulated in an extracellular
capillary tissue inventory be observed with explicit amount, volume,
compartment, and time semantics and trigger the matching receptor response
without silently consuming or duplicating state?

The mapping profile and schema are:

```text
examples/cosimulation/synthetic-capillary-cell-signal-v1.json
data/schemas/capillary-cell-signal-profile/1.0.0.schema.json
```

## Boundary architecture

The hand-off has three deliberately separate parts:

1. `MEHLISSA::model_coupling` defines the neutral extracellular-signal request,
   sample, validation, concentration derivation, and source interface.
2. The M4 capillary bed exposes endothelium or interstitium inventory through
   that interface; it contains no Cell types.
3. `MEHLISSA::cell_cosimulation` maps the sample to the stable M5.1
   receptor-ligand request; the Cell library contains no Capillary types.

```text
M4 conserved exchange
  -> extracellular tissue inventory
  -> non-consuming amount/volume snapshot
  -> profile-defined signal-to-ligand mapping
  -> M5 receptor occupancy and threshold response
```

The signal sample records:

- contract and sample ID;
- signal, source model, and source compartment;
- represented amount and volume;
- non-consuming uniform-snapshot semantics;
- observation time and validity duration.

Concentration is always derived:

```text
ligand concentration = represented amount / represented volume
```

## Checked reference route

The M4 synthetic oxygen exchange fractions remain unchanged. The reference
injects `2e-18 mol` into the capillary at time zero. After the one-second
capillary transit, the staged M4.5 exchange accounts for:

| Destination | Amount |
|---|---:|
| outgoing blood | `1.2e-18 mol` |
| retained endothelium | `4e-19 mol` |
| retained interstitium | `3e-19 mol` |
| cell-labelled inventory | `3e-19 mol` |
| total | `2e-18 mol` |

At synchronization time `1 s`, M5.2 observes the retained interstitium amount
over a declared `1e-15 m3` represented volume:

```text
3e-19 mol / 1e-15 m3 = 0.0003 mol/m3
```

The profile maps source signal `oxygen` to the deliberately synthetic M5.1
ligand for a ten-second exposure. The independent analytical cell model then
returns:

| Output | Expected value |
|---|---:|
| sample validity interval | `[1 s, 11 s]` |
| equilibrium bound fraction | `0.75` |
| bound fraction after exposure | `0.7362632708334493` |
| first crossing of threshold `0.5` | `2.7465307216702746 s` after exposure starts |

Every endothelium, interstitium, and cell-labelled inventory is identical
before and after the sample. The observation therefore preserves M4 amount
accounting; it does not claim that ligand binding itself has been mass-balanced.

## Executable safeguards

Tests verify that:

- the profile and all provenance fields satisfy a strict schema;
- source and target model identities match the mapping;
- only endothelium and interstitium are exposed as extracellular sources;
- sampling occurs at the capillary synchronization time;
- amount divided by volume reproduces the expected typed concentration;
- observation and exposure windows remain identical across the adapter;
- the M5.1 analytical result and threshold time are preserved;
- all M4 inventories remain unchanged;
- duplicate sample IDs are rejected; and
- a failed stale-time attempt does not prevent a corrected retry.

Run the focused checks after a Debug build:

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure `
  -R "capillary-cell|M4 tissue|Capillary-cell"
```

## Evidence and limitations

The M4 balance and M5 analytical response are previously verified internal
references. The cross-layer numerical parameters are synthetic. The mapping
from oxygen to `synthetic-ligand` exists solely to verify identifiers, units,
time, and adapter behavior; it has no biochemical interpretation.

The complete cumulative inventory is assumed homogeneous inside the represented
volume. Its concentration is held constant throughout the exposure. There is no
depletion, temporal concentration trajectory, permeability, reverse flux,
clearance, spatial gradient, competitive binding, cell state persistence, or
higher-layer feedback. A future time-dependent adapter must not silently reuse
these snapshot assumptions.
