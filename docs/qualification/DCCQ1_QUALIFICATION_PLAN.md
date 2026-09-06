<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Dynamic Capillary-Tissue-Cell Qualification Plan (DCCQ-1)

## Decision at DCCQ-1.1

**DCCQ** means **Dynamic Capillary-Tissue-Cell Qualification**. DCCQ-1.1 is
complete: it freezes the bounded intended use, audits the compatibility of the
existing MEHLISSA components, defines the minimum cross-layer state and evidence
rules, and divides the work into seven reviewable increments.

DCCQ-1.1 is a prospective design result. It does **not** implement or qualify
the dynamic coupling. All eight execution and evidence gates therefore remain
`BLOCKED`, with a named reason, until later increments supply their inputs.
The machine-readable authority is:

```text
data/qualification/dynamic-capillary-tissue-cell-qualification-plan-v1.json
```

Its schema, semantic checker, frozen baseline hashes, and negative tests are:

```text
data/schemas/dynamic-capillary-tissue-cell-qualification-plan/1.0.0.schema.json
scripts/check_dynamic_capillary_tissue_cell_qualification_plan.py
tests/test_dynamic_capillary_tissue_cell_qualification_plan.py
```

## Why a new coupling is needed

The current M4-to-M5 path is intentionally narrow. A capillary transit can
partition one substance amount among blood, endothelium, interstitium, and a
cell-labelled inventory. M5.2 can observe endothelium or interstitium at one
synchronization time, divide amount by a declared volume, and give that
constant concentration to a receptor model. The observation is explicitly
non-consuming, homogeneous, and constant throughout the exposure.

That implementation proves useful contracts, but repeating its snapshots
would not create a dynamic biological model. In particular, it cannot account
for ligand depletion by binding, dissociation back into tissue, internalization,
clearance, reverse exchange, or feedback from cell state.

DCCQ-1 will replace that approximation with an explicit time-resolved amount
ledger. The existing snapshot remains supported as an M5 regression and simple
experiment tool; it will not be silently reinterpreted.

## Bounded intended use

The target claim is deliberately conditional:

> For one explicitly named ligand, receptor, cell type, capillary context, and
> physiological state, a frozen MEHLISSA candidate generates cellular exposure
> from a time-resolved capillary and tissue amount ledger and reproduces
> predeclared balance, limiting-case, convergence, timing, uncertainty, and
> evidence-compatibility criteria.

DCCQ-1.1 does not yet select the biological target or population. Resting
pulmonary capillary physiology is preferred because it builds on the strongest
current organ and capillary work, but DCCQ-1.2 must show that the selected
ligand-receptor evidence really supports that context. No patient, diagnostic,
treatment, or clinical claim is in scope.

## Compatibility audit of the current platform

| Existing capability | Entry value | Why it is not yet the DCCQ model | DCCQ role |
|---|---|---|---|
| M4 staged capillary exchange | Exact amount partition and cumulative tissue inventories | one-time synthetic fractions; no reverse flux, clearance, or executable cell owner | regression baseline |
| M5.2 capillary-to-cell snapshot | typed identity, amount/volume concentration, time scope, retry safety | non-consuming homogeneous snapshot; oxygen-to-synthetic-ligand mapping has no biochemical meaning | contract regression only |
| M5.3 time-varying receptor model | analytical constant and pulse references with RK4 convergence | trajectory is prescribed and binding does not deplete ligand | limiting-case reference |
| BCQ-1 Kallenberger CD95 mechanism | named published mechanism and all-state COPASI agreement | fixed `CD95L=16.6`, unresolved source-native units, average HeLa cell, no dynamic consumptive SI boundary | biological context only; blocked from direct dynamic reuse |
| Kuehn-Checa VEGF/VEGFR work | named endothelial ligand/receptors and perturbation context | exact released artifact, dependencies, data roles, and reuse rights are not frozen; biological context is not normal human pulmonary capillary physiology | candidate for DCCQ-1.2, not selected |
| M4 axial and radial transport solvers | advection, diffusion, reaction, wall sink, balance, and refinement | synthetic wall sink without receptor-resolved tissue ledger or feedback | transport verification baseline |

The most important result of this audit is negative but productive: the
computationally qualified CD95 average-cell mechanism must not be connected to
SI capillary transport merely because both APIs exist. Its model-native units
and fixed stimulus first need a new prospective mapping and input protocol.
Likewise, the endothelial VEGF candidate is biologically appealing but is not
yet an audited reusable model package.

## Minimum dynamic state contract

One ligand amount must have exactly one owner at each reported time:

```text
blood free
  <-> endothelium free
  <-> interstitium free
  <-> receptor bound
   -> internalized
   -> cleared or degraded

plus cumulative inlet and outlet ledgers
```

The open-system balance is:

```text
initial amount + cumulative inlet
  = blood free
  + endothelium free
  + interstitium free
  + receptor bound
  + internalized
  + cleared or degraded
  + cumulative outlet
```

The later implementation may represent a compartment or finite-volume field,
but every representation must expose this same balance. Receptor association
removes ligand from the free interstitial ledger; dissociation returns it.
Internalization, degradation, clearance, and outlet transport are explicit
sinks or terminal ledgers, never unexplained disappearance.

The same stable ligand identifier must refer to the same chemical or biological
entity in every layer. All amounts, volumes, concentrations, surfaces, flows,
times, and rates require compatible explicit units. An
`unresolved-model-native` state cannot cross into an SI model until a cited and
prospectively frozen conversion exists.

Cell-to-tissue feedback must be delayed, bounded, and applied only at a declared
synchronization boundary. A cell event cannot change the upstream parameter
that generated it inside the same step.

## Eight qualification gates

| Gate | Meaning | DCCQ-1.1 state |
|---|---|---|
| DCCQ-G1 — identity and units | one ligand identity and compatible units across every layer | blocked pending DCCQ-1.2 source selection |
| DCCQ-G2 — mass balance | complete ledger recalculated at every output and over the run | blocked pending implementation |
| DCCQ-G3 — limiting cases | zero-flux, zero-binding, constant-reservoir, pulse-withdrawal, and no-feedback references | blocked pending frozen equations and tolerances |
| DCCQ-G4 — numerical convergence | independent time-step, synchronization, and spatial refinement | blocked pending method selection |
| DCCQ-G5 — causal timing | transport, exposure, response, and feedback remain ordered | blocked pending scheduler contract |
| DCCQ-G6 — uncertainty and sensitivity | numerical, parameter, structural, observational, and synchronization uncertainty | blocked pending evidence-backed distributions |
| DCCQ-G7 — independent reference | no-refit comparison with source-disjoint time-resolved observations | blocked pending reusable independent data |
| DCCQ-G8 — claim and review | independent reconciliation of code, evidence, archive, licences, documentation, and claim | blocked until close-out |

`BLOCKED` here does not mean that the programme failed. It prevents an
unexecuted future gate from appearing as evidence merely because its plan is
well specified.

## Evidence ladder

DCCQ reports five evidence levels separately:

1. software contracts: schemas, identity, ownership, lifecycle, replay, and
   negative controls;
2. mathematical and numerical evidence: limiting cases, balance, convergence,
   and independent computational comparison;
3. literature parameterization: named biology, compatible conditions and
   units, provenance, uncertainty, and rights;
4. source-disjoint experimental qualification: a prospectively locked no-refit
   comparison with time-resolved observations; and
5. participant or clinical research: separate governance, privacy, study
   design, review, and claim-specific validation.

Passing an earlier level does not imply a later one.

## Seven increments

| Increment | Work | Exit |
|---|---|---|
| **DCCQ-1.1** | bounded intended use and baseline compatibility audit | **complete**: current capabilities and blockers are hash-bound; invalid shortcuts fail closed |
| **DCCQ-1.2** | biological target, artifact, data, and licence screen | one named candidate and alternatives ranked by exact artifact, units, data roles, rights, and independent observations |
| DCCQ-1.3 | prospective equation and evaluation protocol | equations, parameters, synchronization, metrics, tolerances, controls, and failure rules frozen before dynamic output |
| DCCQ-1.4 | typed dynamic tissue and consumptive cell coupling | modular implementation owns every ligand amount exactly once |
| DCCQ-1.5 | balance, limiting-case, convergence, and timing qualification | computational gates executed with all partial and failed results retained |
| DCCQ-1.6 | uncertainty, sensitivity, and source-disjoint comparison | influential uncertainty reported and eligible independent data evaluated without refitting, or evidence remains blocked |
| DCCQ-1.7 | independent close-out and bounded claim | runner-independent and human review reconcile implementation, evidence, archive, licences, and documentation |

The next step is therefore DCCQ-1.2. It begins with research and source
screening, not implementation. The previously noted VEGF-A/VEGFR endothelial
model will be reassessed alongside alternatives; it is not preselected.

## Negative controls

The machine plan freezes ten required failure cases. They include mismatched
ligand identity, unresolved-to-SI conversion, double ownership, an omitted
sink, same-step feedback, post-outcome mutation, validation refitting,
overclaiming numerical checks, relabelling repeated snapshots, and invented
independent parameter distributions. Later protocols may add controls but may
not delete these safeguards silently.

## Running the DCCQ-1.1 checks

With the publication dependencies installed:

```powershell
python scripts/check_dynamic_capillary_tissue_cell_qualification_plan.py
python -m unittest tests/test_dynamic_capillary_tissue_cell_qualification_plan.py -v
```

A passing result establishes the integrity and internal consistency of the
prospective programme only. It is not a dynamic simulation result.
