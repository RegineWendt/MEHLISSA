<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DCCQ-1.2 Biological Target, Artifact, Data, and Licence Screen

## Decision

**DCCQ** means **Dynamic Capillary-Tissue-Cell Qualification**. DCCQ-1.2 is
complete. The first dynamic coupling target is:

> Human VEGF-A165a binding to and trafficking with VEGFR2 in primary human
> umbilical vein endothelial cells (HUVECs), with NRP1 retained as an explicit
> included-or-excluded structural choice.

The 2025 Sarabipour and colleagues HUVEC model and experiments are the selected
source family. This decision names the biology and establishes exact source
identity, convertible units, data roles, and reuse boundaries. It does **not**
select the reduced MEHLISSA equations, copy the external model, implement a
dynamic coupling, pass a qualification gate, or establish biological,
pulmonary, patient, or clinical validity.

The machine-readable authority is:

```text
data/qualification/dynamic-capillary-tissue-cell-evidence-candidate-register-v1.json
```

It is checked against the DCCQ-1.1 parent-plan hash by:

```text
data/schemas/dynamic-capillary-tissue-cell-evidence-candidate-register/1.0.0.schema.json
scripts/check_dynamic_capillary_tissue_cell_evidence_candidates.py
tests/test_dynamic_capillary_tissue_cell_evidence_candidates.py
```

## Why this target was selected

The selected work studies primary human endothelial cells, names the VEGF-A
isoforms and VEGFR1, VEGFR2, and NRP1 receptors, and measures ligand-responsive
surface, intracellular, and whole-cell receptor behavior over time. Its model
represents extracellular medium, the cell surface, Rab4/5 and Rab11 endosomes,
and degradation. The paper reports that ligand-bound VEGFR2 requires increased
internalization to explain the HUVEC observations, while the other trafficking
processes remain parsimoniously unchanged.

This makes the family substantially better suited to the DCCQ amount ledger
than the existing synthetic snapshot or the fixed-input CD95 average-cell
model. The source expresses time in seconds, receptor and ligand inventories in
molecules per represented cell, compartment volumes in femtolitres per cell,
and surfaces in square micrometres per cell. Association and first-order rates
are explicitly related to these local geometries. Those quantities are
convertible to MEHLISSA's SI contracts, although that conversion is not yet
frozen.

The decision is intentionally not a lung claim. HUVECs are human endothelial
cells from an umbilical vein, not pulmonary capillary endothelial cells. The
experiments are confluent, serum-starved in-vitro cultures exposed to exogenous
ligand. They do not establish resting human pulmonary physiology, in-vivo
behavior, person-specific behavior, or clinical use.

## Ranked candidates

Each dimension was scored from 0 to 3. A score summarizes screening fitness; it
does not turn a limitation into evidence.

| Rank | Candidate | Score | Decision | Principal reason |
|---:|---|---:|---|---|
| 1 | Sarabipour 2025 VEGF-A165a/VEGFR2/NRP1 HUVEC trafficking | 19/24 | selected target and evidence family | strongest combination of human endothelial fit, exact sources, convertible units, CC-BY Supporting Information, and time-resolved observations |
| 2 | Clegg and Mac Gabhann 2015 VEGFR2 trafficking and phosphorylation | 16/24 | mechanistic structural alternative | detailed CC-BY reaction and parameter reference, but no exact licensed executable artifact was established and the model is larger than needed |
| 3 | Kuehn and Checa 2019 AngioABM VEGFR1/VEGFR2/Dll4/Notch model | 11/24 | licensed platform alternative | exact Apache-2.0 tags exist, but mouse embryoid-body sprouting, model-specific units, and a complete agent platform are a poor first SI coupling fit |
| 4 | Kallenberger 2014 CD95 model already qualified by BCQ-1 | 10/24 | cell-solver regression only | exact and computationally qualified, but the wrong ligand, receptor, and cell context, a fixed input, and unresolved source-native units block dynamic SI reuse |

The eight dimensions were biological fit, artifact identity, unit bridge,
time-resolved data, reuse rights, source-disjoint evidence, pulmonary context,
and bounded integration cost.

## Exact artifact and rights audit

The selected publication is Sarabipour et al., *Impact of ligand binding on
VEGFR1, VEGFR2, and NRP1 localization in human endothelial cells*, PLOS
Computational Biology 21(7), e1013254, DOI
[`10.1371/journal.pcbi.1013254`](https://doi.org/10.1371/journal.pcbi.1013254).
The article and its Supporting Information are CC-BY-4.0. The article states
that all manuscript data are in the Supporting Information and links the code
repository.

The linked repository was audited at immutable commit:

```text
https://github.com/SSarabipour/VEGFR-Trafficking-Projects
bed7b23b3f6edbed6376e818b6307eccd65ea38e
```

Three classes of rights must remain separate:

| Material | Exactness | Rights decision |
|---|---|---|
| PLOS article and Supporting Information, including parameter and data tables | DOI and numbered supplements | CC-BY-4.0; may be used with attribution |
| GitHub BioNetGen, generated MATLAB, and analysis code | commit and selected file SHA-256 hashes frozen in the machine register | repository has no `LICENSE`, `COPYING`, or `NOTICE`; inspect and hash only, do not copy, modify, bundle, or redistribute under an assumed licence |
| GitHub CSV copies of Figure 2 data | commit, path, and SHA-256 frozen | standalone repository licence is unresolved; obtain reusable observations from the CC-BY Supporting Information or secure an explicit licence |

This distinction matters. Public visibility and a paper's CC-BY licence do not
automatically license separately hosted source code. DCCQ-1.3 will therefore
specify independent MEHLISSA-native equations from the rights-compatible
publication record, not port the repository implementation.

The full source model contains 281 coupled ordinary differential equations.
That model is the mechanistic reference, not the preselected implementation
scope. Importing or reconstructing all 281 states is not authorized by this
screen.

## Unit bridge to freeze in DCCQ-1.3

The selected source records enough information to design an explicit SI bridge:

| Quantity | Source representation | DCCQ target |
|---|---|---|
| ligand and receptor inventory | molecules per represented cell | mol, with represented cell count explicit |
| time | s | s |
| extracellular volume | `1e7 fL/cell` | m3 for the represented tissue/cell population |
| Rab4/5 volume | `11.25 fL/cell` | m3 |
| Rab11 volume | `3.75 fL/cell` | m3 |
| cell surface | `1000 um2/cell` | m2 |
| Rab4/5 and Rab11 surfaces | `950` and `325 um2/cell` | m2 |
| external stimulus | `50 ng/mL` VEGF-A165a; encoded as `6843182 molecules/cell` for the source volume | mol/m3 with molecular-mass and dimer convention declared |
| kinetic rates | first-order `s^-1`; association rates adjusted by local amount, volume, or area | dimensionally typed SI rates derived from the frozen conversion |

DCCQ-1.3 must freeze the Avogadro constant, femtolitre and square-micrometre
conversions, represented cell count, extracellular volume, VEGF-A165a
molecular mass, monomer-versus-dimer convention, and derivation of every
adjusted association rate. Until then, DCCQ-G1 is `PARTIAL`, not passed.

## Evidence roles and independence

The screen separates five evidence entries:

| Evidence | Context and schedule | Permitted role | Independence |
|---|---|---|---|
| Sarabipour 2025 Figure 2C family | primary HUVEC; 50 ng/mL VEGF-A165a; 0, 15, 30, 60, 120, 240 min | mechanism selection and calibration | same selected family; not validation |
| Sarabipour 2025 Figure 2G/H family | primary HUVEC; surface, internal, and total measurements at 1 h and 4 h | within-study structural check | same selected family; not validation |
| Sarabipour 2024 unligated trafficking | primary HUVEC baseline localization and turnover | baseline parameterization and context | same laboratory and model lineage; not validation |
| Peach et al. 2019 NanoBRET | engineered HEK293T cells expressing tagged VEGFR2; binding/endocytosis through 90 min | source-disjoint no-refit kinetic challenge | separately authored and CC-BY, but cell type, receptor construct, and assay do not match HUVEC |
| Zhao et al. 2010 | primary HUVEC; 50 ng/mL VEGF-A165 for 30 min | source-disjoint directional check | separately authored and CC-BY, but a single qualitative endpoint is not a dynamic series |

Peach et al., DOI [`10.1111/bph.14755`](https://doi.org/10.1111/bph.14755),
provides the best identified source-disjoint time-resolved kinetic challenge.
Its HEK293T overexpression and fluorescent/tagged-receptor context prevents it
from being treated as a condition-matched HUVEC validation set. Zhao et al.,
DOI [`10.1371/journal.pone.0012563`](https://doi.org/10.1371/journal.pone.0012563),
supports the direction of VEGF-A165-induced VEGFR2 internalization in HUVECs,
but not a quantitative trajectory.

Consequently DCCQ-G7 remains `BLOCKED`. A condition-matched, source-disjoint
HUVEC time series has not been frozen, and even the two eligible challenge
sources need a prospective observation model and numeric rules before use.

## What DCCQ-1.3 may now specify

DCCQ-1.3 is authorized to freeze a reduced MEHLISSA-native model that covers:

1. free extracellular VEGF-A165a;
2. surface VEGFR2 and ligand-bound VEGFR2;
3. ligand association and dissociation with amount-conserving depletion and
   return;
4. internalization of bound and unbound receptor;
5. a bounded intracellular/recycling state and explicit degradation or
   clearance ledger; and
6. NRP1 as a prospectively decided included mechanism or declared structural
   alternative.

The protocol must be committed before dynamic results are inspected. It must
freeze equations, state ownership, SI conversions, parameter roles,
synchronization, initial and boundary conditions, observation transformations,
calibration/validation separation, analytical cases, convergence and balance
limits, uncertainty handling, negative controls, and failure rules.

## Current gate implications

| Gate | DCCQ-1.2 state | Reason |
|---|---|---|
| DCCQ-G1 identity and units | `PARTIAL` | ligand and source-unit system selected; exact SI and represented-population mapping not yet frozen |
| DCCQ-G6 uncertainty and sensitivity | `PARTIAL` | variability and parameter sources exist; distributions, correlations, and identifiability rules are not frozen |
| DCCQ-G7 independent reference | `BLOCKED` | independent challenges identified but context-matched HUVEC dynamic validation and a numeric no-refit protocol remain absent |
| DCCQ-G2, G3, G4, G5, G8 | `BLOCKED` | no reduced equation protocol, implementation, computational result, or close-out exists |

The allowed claim is therefore narrow: DCCQ-1.2 has selected and
machine-checked the target and evidence family. Dynamic coupling is not yet
implemented or qualified.

## Running the DCCQ-1.2 checks

With the publication validation dependencies installed:

```powershell
python scripts/check_dynamic_capillary_tissue_cell_evidence_candidates.py
python -m unittest tests/test_dynamic_capillary_tissue_cell_evidence_candidates.py -v
```

The negative tests reject source-hash drift, inconsistent scoring, silent reuse
of unlicensed repository material, same-family data relabelled as independent,
erased context mismatches, a prematurely passed DCCQ-G7, and a source unit
system relabelled as already SI-frozen.
