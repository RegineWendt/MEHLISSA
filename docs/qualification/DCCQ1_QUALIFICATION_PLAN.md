<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Dynamic Capillary-Tissue-Cell Qualification Plan (DCCQ-1)

**Programme status:** DCCQ-1.1 through DCCQ-1.6 are complete. DCCQ-1.7 has
reached its machine-verifiable close-out and remains partial only because an
external human-review attestation is absent. The selected human
VEGF-A165a/VEGFR2 primary-HUVEC target is implemented as a typed, SI-explicit,
seven-owner reduced coupling with NRP1 explicit. DCCQ-G1 through DCCQ-G5 pass,
G6 and G8 are partial, and G7 remains blocked. See the
[qualification result](DCCQ1_QUALIFICATION_RESULT.md).

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

DCCQ-1.2 has now selected human VEGF-A165a/VEGFR2 trafficking in primary HUVECs,
with NRP1 as an explicit structural choice. This supplies a human endothelial
in-vitro context, not resting pulmonary capillary physiology: HUVECs are not
lung endothelial cells and the evidence is neither in vivo nor patient-specific.
No patient, diagnostic, treatment, or clinical claim is in scope. See the
[DCCQ-1.2 evidence source screen](DCCQ1_EVIDENCE_SOURCE_SCREEN.md).

## Compatibility audit of the current platform

| Existing capability | Entry value | Why it is not yet the DCCQ model | DCCQ role |
|---|---|---|---|
| M4 staged capillary exchange | Exact amount partition and cumulative tissue inventories | one-time synthetic fractions; no reverse flux, clearance, or executable cell owner | regression baseline |
| M5.2 capillary-to-cell snapshot | typed identity, amount/volume concentration, time scope, retry safety | non-consuming homogeneous snapshot; oxygen-to-synthetic-ligand mapping has no biochemical meaning | contract regression only |
| M5.3 time-varying receptor model | analytical constant and pulse references with RK4 convergence | trajectory is prescribed and binding does not deplete ligand | limiting-case reference |
| BCQ-1 Kallenberger CD95 mechanism | named published mechanism and all-state COPASI agreement | fixed `CD95L=16.6`, unresolved source-native units, average HeLa cell, no dynamic consumptive SI boundary | biological context only; blocked from direct dynamic reuse |
| Kuehn-Checa VEGF/VEGFR work | named endothelial ligand/receptors, exact Apache-2.0 `VEGFR1` source tags, and perturbation context | two-dimensional mouse embryoid-body sprouting, model-specific units, and platform-scale integration do not fit the first human endothelial SI coupling | retained DCCQ-1.2 licensed platform alternative |
| M4 axial and radial transport solvers | advection, diffusion, reaction, wall sink, balance, and refinement | synthetic wall sink without receptor-resolved tissue ledger or feedback | transport verification baseline |

The most important result of this audit is negative but productive: the
computationally qualified CD95 average-cell mechanism must not be connected to
SI capillary transport merely because both APIs exist. Its model-native units
and fixed stimulus first need a new prospective mapping and input protocol.
The earlier endothelial candidate has now been resolved into ranked alternatives.
The selected Sarabipour HUVEC publication family has auditable CC-BY Supporting
Information and convertible source units, but its linked GitHub repository has
no explicit licence. It may be inspected and hash-bound, not copied or bundled.

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

| Gate | Meaning | Current state after DCCQ-1.6 and machine close-out |
|---|---|---|
| DCCQ-G1 — identity and units | one ligand identity and compatible units across every layer | **PASS:** one VEGF-A165a homodimer identity, exact SI conversions, one represented HUVEC, and runtime identity/unit guards |
| DCCQ-G2 — mass balance | complete ledger recalculated at every output and over the run | **PASS:** all 41 retained trajectories remain nonnegative with maximum residual `7.857794173099922e-32 mol` |
| DCCQ-G3 — limiting cases | zero-flux, zero-binding, constant-reservoir, pulse-withdrawal, and no-feedback references | **PASS:** all predeclared cases meet their frozen computational limits |
| DCCQ-G4 — numerical convergence | independent time-step, synchronization, and spatial refinement | **PASS:** time-step and synchronization refinements converge; spatial refinement is inapplicable to this well-mixed candidate |
| DCCQ-G5 — causal timing | transport, exposure, response, and feedback remain ordered | **PASS:** feedback is bounded and first applied one synchronization interval after it is scheduled |
| DCCQ-G6 — uncertainty and sensitivity | numerical, parameter, structural, observational, and synchronization uncertainty | **PARTIAL:** numerical, synchronization, 24 local-parameter and NRP1/feedback structural comparisons exist; joint distributions, correlations, observational covariance, and global identifiability do not |
| DCCQ-G7 — independent reference | no-refit comparison with source-disjoint time-resolved observations | blocked: an independent HEK293T kinetic challenge and directional HUVEC check are identified, but no condition-matched independent HUVEC series or frozen numeric protocol exists |
| DCCQ-G8 — claim and review | independent reconciliation of code, evidence, archive, licences, documentation, and claim | **PARTIAL:** independent checker, archive, licence boundary, documentation and bounded claim agree; external human attestation remains absent |

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
| **DCCQ-1.2** | biological target, artifact, data, and licence screen | **complete**: VEGF-A165a/VEGFR2 in primary HUVECs selected, with NRP1 explicit; four candidates, ten artifacts, five evidence roles, unit bridge, and rights boundaries are machine checked |
| **DCCQ-1.3** | prospective equation and evaluation protocol | **complete:** nine equations, exact SI map, 15 parameters, NRP1 choice, synchronization, metrics, limits, controls, and failure rules were frozen before the authoritative output |
| **DCCQ-1.4** | typed dynamic tissue and consumptive cell coupling | **complete:** modular implementation owns every ligand amount exactly once and applies bounded feedback only at the next synchronization boundary |
| **DCCQ-1.5** | balance, limiting-case, convergence, and timing qualification | **complete:** 41 retained trajectories pass every unblocked computational gate |
| **DCCQ-1.6** | uncertainty, sensitivity, and source-disjoint comparison | **complete with explicit evidence limits:** local and structural effects are reported; G6 remains partial and G7 blocked rather than inventing distributions or fitting mismatched evidence |
| **DCCQ-1.7** | independent close-out and bounded claim | **partial:** runner-independent machine, archive, licence, documentation and claim review complete; external human attestation remains blocked |

The computational programme is therefore closed at its present evidence
boundary. The next DCCQ action is external: obtain a reusable,
condition-matched, source-disjoint primary-HUVEC dynamic series and an external
reviewer attestation. Both must be frozen prospectively and must not be used to
refit the evaluated candidate. The full 281-state source model remains a
reference, not an authorized port.

## Negative controls

The machine plan freezes ten required failure cases. They include mismatched
ligand identity, unresolved-to-SI conversion, double ownership, an omitted
sink, same-step feedback, post-outcome mutation, validation refitting,
overclaiming numerical checks, relabelling repeated snapshots, and invented
independent parameter distributions. Later protocols may add controls but may
not delete these safeguards silently.

## Running the DCCQ checks

With the publication dependencies installed:

```powershell
python scripts/check_dynamic_capillary_tissue_cell_qualification_plan.py
python -m unittest tests/test_dynamic_capillary_tissue_cell_qualification_plan.py -v
python scripts/check_dynamic_capillary_tissue_cell_evidence_candidates.py
python scripts/check_dynamic_capillary_tissue_cell_protocol.py
python scripts/check_dynamic_capillary_tissue_cell_qualification_result.py
```

The plan and source-screen checkers establish prospective integrity. The result
checker independently reconstructs the archive and computational claims. None
of them converts the blocked independent experimental or human-review gates
into biological or clinical validity.
