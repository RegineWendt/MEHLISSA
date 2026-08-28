<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary Lobar Parallel Beds (M3.16)

## Result

M3.16 replaces the single spatially aggregated pulmonary path with five
anatomically named parallel 0D beds:

```text
                         ┌─ right upper lobe ─┐
                         ├─ right middle lobe ┤
pulmonary arterial inlet ├─ right lower lobe ┤ pulmonary venous outlet
                         ├─ left upper lobe  ─┤
                         └─ left lower lobe  ─┘
```

All beds see the same mean pulmonary arterial pressure and left-atrial outlet
pressure. For aggregate resistance `R`, compliance `C`, total flow `Q`, and a
normalized bed fraction `f_i`, the executable decomposition is:

```text
Q_i = f_i Q       R_i = R / f_i       C_i = f_i C
V_i = Q_i T_i
```

Consequently, `sum(Q_i) = Q`, `sum(C_i) = C`, and
`1 / sum(1 / R_i) = R`. The v7 network therefore reproduces v4 aggregate
pressure-flow behavior exactly while adding an explicit anatomical state.

## Lobar evidence and derivation

Lee et al. report median lobe volume and normalized dual-energy CT perfused
blood volume (PBV) for 103 suspected-pulmonary-embolism examinations that were
negative for embolism and had no cardiopulmonary comorbidity. M3.16 computes a
declared aggregate proxy from the product of the two separately reported
medians and normalizes it over all five lobes:

| Bed | Median volume (mL) | Median normalized PBV (%) | v7 fraction |
|---|---:|---:|---:|
| right upper lobe | 649 | 15.8 | 0.222550 |
| right middle lobe | 355 | 11.4 | 0.087833 |
| right lower lobe | 788 | 15.0 | 0.256533 |
| left upper lobe | 789 | 13.8 | 0.236310 |
| left lower lobe | 657 | 13.8 | 0.196775 |

The implied right/left split is 0.566916/0.433084. This is not called measured
lobar flow: normalized DE-CT PBV is a perfusion surrogate, and a product of
separately reported medians is not the median of subject-level iodine content.
The distinction is stored in the model card rather than hidden in code.

All five beds initially use the qualified aggregate 6.4 s first-pass transit
from Swift et al. No reliable lobe-specific transit series was identified, so
inventing regional delays would create false anatomical precision.

## Software behavior

`PulmonaryParallelBedsModel` is selected by the existing lung factory whenever
a pulmonary 0D definition contains `parallel_beds`. Thus v1–v6 remain unchanged
and use `PulmonaryZeroDimensionalModel`, while v7 uses the new implementation
behind the same `ModelComponent` contract.

- Individual entities are mapped reproducibly to exactly one bed by a stable
  integer mixing function and the cumulative perfusion fractions.
- Each entity traverses that bed's own transit component and returns once
  through the common pulmonary venous port.
- Flow, resistance, compliance, transit, and blood volume are observable for
  every bed.
- Conserved transfers remain one aggregate boundary object because the current
  coupling ledger is intentionally one-to-one. The regional state nevertheless
  partitions the amount-conserving total flow exactly.

The schema loader requires every executable bed fraction and transit time to
carry evidence metadata and rejects disconnected model/evidence values.

## Qualification status

The aggregate v7 hemodynamics are not refitted. They are identical to the v4
candidate that agreed at all 15 frozen, calibration-disjoint Wolsk stages.
Unit tests verify the parallel-network identities and the exact aggregate
equivalence. This transfers the v4 aggregate qualification to v7's aggregate
outputs; it does not validate the five lobar outputs independently.

The lobar structure is therefore an evidence-qualified anatomical candidate,
not a clinically validated regional perfusion model. Independent lobar flow or
perfusion observations, posture/gravity response, exercise redistribution,
regional recruitment, and disease-specific changes remain open.

## Sources

1. Lee HJ, et al. Lobar pulmonary perfusion quantification with dual-energy CT
   angiography. *Eur J Radiol Open*. 2022;9:100428.
   <https://doi.org/10.1016/j.ejro.2022.100428>
2. Swift AJ, et al. Pulmonary arterial hypertension: MR imaging-derived
   first-pass bolus kinetic parameters. *Radiology*. 2012;263:678–687.
   <https://doi.org/10.1148/radiol.12111049>
3. Wendt R. Dissertation simulation chapters, Chapter 4: MEHLISSA.
   Repository copy: `literature/Diss_WENDT_Simulationchapters.pdf`.
