<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Receptor-Ligand Baseline

## Purpose

M5.1 establishes the first executable cell-layer boundary. It answers a narrow
verification question: for reversible one-to-one receptor binding in a constant
ligand reservoir, does MEHLISSA reproduce the exact receptor occupancy and the
time at which an occupancy threshold is first reached?

The profile and schema are:

```text
examples/cell-models/synthetic-receptor-ligand-v1.json
data/schemas/receptor-ligand-profile/1.0.0.schema.json
```

The implementation is selected through an abstract receptor-ligand interface in
`models/cell/`. It does not depend on the body, organ, capillary, or scenario
libraries.

## Mathematical reference

For free receptor `R`, ligand `L`, and bound complex `RL`, the model uses:

```text
R + L <-> RL
```

The extracellular ligand concentration `L` is constant. If `f` is the fraction
of receptors in `RL`, `kon` is the association rate, and `koff` is the
dissociation rate:

```text
df/dt = kon L (1 - f) - koff f
lambda = kon L + koff
f_eq = kon L / lambda
f(t) = f_eq + (f(0) - f_eq) exp(-lambda t)
```

For an initially sub-threshold state and a threshold below equilibrium, the
first crossing is:

```text
t_threshold = -ln((f_threshold - f_eq) / (f(0) - f_eq)) / lambda
```

If the initial occupancy is already at or above the threshold, the crossing
time is zero. If the threshold cannot be reached within the observation
interval, the response contains no crossing time.

## Contract and units

The request names the ligand and compartment and supplies:

- ligand concentration in `mol/m3`;
- observation duration on the simulation clock; and
- initial bound-receptor fraction.

The selected model supplies receptor identity, cell volume, total receptor
concentration, `kon` in `m3/(mol s)`, `koff` in `s-1`, and the threshold. The
response preserves all identities and reports total, free, and bound receptor
amounts in moles as well as occupancy and threshold information.

The implementation enforces:

```text
total receptor amount = free receptor amount + bound receptor amount
```

Cell volume currently affects this amount conversion only; it does not create a
spatial cell geometry.

## Checked reference case

The synthetic profile uses:

| Parameter | Value |
|---|---:|
| cell volume | `1e-15 m3` |
| total receptor concentration | `1e-6 mol/m3` |
| total receptor amount | `1e-21 mol` |
| association rate | `1000 m3/(mol s)` |
| dissociation rate | `0.1 s-1` |
| ligand concentration | `0.0003 mol/m3` |
| initial bound fraction | `0` |
| observation duration | `10 s` |
| detection threshold | `0.5` |

The exact outputs are:

- equilibrium bound fraction: `0.75`;
- bound fraction after ten seconds: `0.7362632708334493`; and
- first threshold crossing: `2.7465307216702746 s`.

A second test checks the zero-ligand dissociation limit. Profile and contract
tests reject wrong identities, negative concentrations, impossible thresholds,
duplicate evidence sources, and other nonphysical or incomplete data.

Run the focused verification after a Debug build:

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure `
  -R "receptor|Receptor|binding|dissociation"
```

## Evidence and interpretation

The equations are a standard mass-action derivation and are recorded here as
the analytical oracle. The local dissertation identifies binding, dissociation,
threshold detection, and cell-layer coupling as conceptual requirements. The
checked numerical parameters are deliberately synthetic; their source role in
the executable profile is `software_verification`.

A passing result means that the cell-layer contract, units, exact transient,
threshold calculation, and receptor accounting agree for this mathematical
case. It does not validate any biological parameter or predictive endpoint.

## Explicit limitations and next hand-off

The model has no ligand depletion, transport feedback, competitive binding,
cooperativity, receptor synthesis or degradation, internalization, spatial
gradient, stochasticity, cell-to-cell variation, intracellular signaling, drug
release, apoptosis, or clinical decision threshold.

M5.2 must define how a time-scoped extracellular signal produced by the M4
capillary/tissue boundary becomes an M5 request. That step must make units,
compartment identity, temporal aggregation, conservation where applicable, and
ownership of state explicit. The binding library must remain independently
testable after that adapter is added.
