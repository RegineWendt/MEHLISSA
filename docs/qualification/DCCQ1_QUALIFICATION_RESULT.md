<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DCCQ-1 Dynamic Coupling Qualification Result

## Decision

DCCQ-1.3 through DCCQ-1.6 are complete. DCCQ-1.7 is complete to the
machine-verifiable close-out boundary and remains partial only because no
external human-review attestation exists. The frozen MEHLISSA candidate is a
typed, SI-explicit, independently written reduced coupling for human
VEGF-A165a homodimer transport, VEGFR2 binding, internalization and degradation
in a one-primary-HUVEC reference context. It is not a port of the source
publication's 281-state model.

The exact permitted conclusion is:

> The frozen MEHLISSA candidate is an independently written, SI-explicit,
> amount-conserving reduced VEGF-A165a/VEGFR2 HUVEC-informed dynamic coupling
> suitable for software, mathematical, numerical, and local-sensitivity
> qualification.

This conclusion is computational and literature-informed. It is not
condition-matched independent experimental validation, pulmonary physiology,
an in-vivo result, an individual-cell or patient prediction, or clinical
evidence.

## What was implemented

`DynamicCapillaryTissueCellModel` advances one ligand through seven mutually
exclusive owners:

```text
blood free <-> endothelium free <-> interstitium free
                                      <-> receptor bound
                                           -> internalized
                                           -> cleared/degraded
blood free -> outlet
```

The open-system invariant is recalculated at every output:

```text
initial + cumulative inlet
  = blood free + endothelium free + interstitium free
  + receptor bound + internalized + cleared/degraded + outlet
```

Association removes VEGF-A165a from the free interstitial owner, dissociation
returns it, and internalization moves it to a separate intracellular owner.
Clearance, degradation and outlet transit are explicit terminal ledgers. The
fixed receptor capacity prevents more surface-bound ligand than available
VEGFR2 sites.

The reference NRP1 decision is deliberately conservative. Human NRP1 identity
and site eligibility are explicit, but the reference introduces no unsupported
kinetic multiplier. NRP1 exclusion and a clearly labelled 1.5-fold
facilitation assumption are retained as structural sensitivity cases. They are
not fitted biology.

Cell-derived feedback is a bounded software challenge. Occupancy measured at
the end of one 60-second synchronization interval can reduce the next
interval's endothelial-to-interstitial transfer multiplier, never the interval
that generated the occupancy. This tests causal scheduling without claiming
that the assumed feedback gain is established HUVEC physiology.

## Prospective source and SI decisions

The DCCQ-1.3 protocol was frozen before the authoritative run. It defines nine
equations, 15 parameters, five numerical refinements, four coupling-interval
refinements, ten negative controls, and the failure/retention policy. Important
source conversions include:

| Quantity | Frozen value | Role |
|---|---:|---|
| VEGF-A165a convention | 44 kg/mol | 44-kDa homodimer treated as one ligand |
| extracellular volume | `1e-11 m3/cell` | source culture volume |
| VEGFR2 surface capacity | `8.136641429151849e-21 mol/cell` | 4,900 receptors divided by Avogadro's constant |
| stimulus amount | `1.1363371054780858e-17 mol/cell` | 6,843,182 homodimers |
| reduced association | `1e4 m3 mol^-1 s^-1` | S16 1:1 VEGF-A165a/VEGFR2 value converted from pM |
| dissociation | `1e-3 s^-1` | S16 |
| liganded internalization | `6.9e-4 s^-1` | S14 threefold VEGFR2 internalization |
| degradation | `2.3e-4 s^-1` | S14 |

The effective blood/endothelium/interstitium transfer, clearance, outlet and
feedback parameters are MEHLISSA qualification assumptions, not measurements
from the HUVEC publication. This is why literature parameterization remains
`PARTIAL` even though identity and units pass.

During this review, the Rab11 surface entry in the DCCQ-1.2 source screen was
corrected from 325 to the S12 value of 350 square micrometres per cell. The
source-screen record and the protocol bind the corrected value.

## Authoritative execution

The immutable archive is:

```text
results/dccq1/vegfa165a-vegfr2-huvec/20260906T120000Z-dccq1-v1
```

It contains 41 trajectories: baseline and exact replay; five internal-step
resolutions; four synchronization resolutions; zero-flux, zero-binding and
constant-reservoir limits; feedback and NRP1 structural cases; and 24
plus/minus-ten-percent local-sensitivity runs over 12 parameters. No raw
external experimental data or unlicensed repository code is bundled.

Headline results are:

| Check | Result |
|---|---:|
| maximum balance residual | `7.857794173099922e-32 mol` |
| minimum owner amount | `0 mol` |
| deterministic replay difference | exactly `0` |
| 1 s versus 0.5 s final relative L1 error | `1.0649136586993484e-14` |
| 30 s versus 15 s synchronization error | `1.2144341029339634e-5` |
| constant-reservoir occupancy error | `6.142120053898026e-6` |
| local-sensitivity comparisons | `24` |

The internal-step error decreases monotonically from 8 to 1 seconds against
the 0.5-second reference. The synchronization error decreases monotonically
from 120 to 30 seconds against the 15-second reference. The well-mixed
candidate has no spatial mesh, so a spatial-refinement claim is explicitly not
applicable rather than silently passed.

The output-resolution timing check finds blood departure at 60 seconds,
interstitial arrival and detectable binding at 120 seconds, feedback scheduling
at 240 seconds, and first feedback application at 300 seconds. Arrival and
binding share one reported timestamp; the model does not claim sub-output
ordering between them. Feedback nevertheless has a strict one-interval delay.

## Gate status

| Gate | Status | Meaning |
|---|---|---|
| DCCQ-G1 identity and units | **PASS** | one VEGF-A165a identity, exact SI mapping, one HUVEC, typed owners |
| DCCQ-G2 mass balance | **PASS** | every retained trajectory is nonnegative and conserves the complete ledger |
| DCCQ-G3 limiting cases | **PASS** | zero flux, zero binding, near-constant reservoir, pulse withdrawal and no feedback behave as frozen |
| DCCQ-G4 numerical convergence | **PASS** | RK4 step and independent coupling-interval refinements meet the predeclared limits |
| DCCQ-G5 causal timing | **PASS** | feedback is bounded and delayed to the next synchronization boundary |
| DCCQ-G6 uncertainty and sensitivity | **PARTIAL** | numerical, synchronization, 12-parameter local and structural effects reported; joint distributions and observational covariance absent |
| DCCQ-G7 independent reference | **BLOCKED** | no reusable condition-matched, source-disjoint primary-HUVEC dynamic series exists |
| DCCQ-G8 claim and review | **PARTIAL** | archive, independent checker, licences, documentation and claim reconciled; external human attestation absent |

The independent Peach kinetic work uses engineered HEK293T cells, while the
Zhao primary-HUVEC work supplies a directional 30-minute endpoint rather than
a condition-matched dynamic series. Neither is converted into a numeric
validation pass, and no validation refitting was performed.

## Reproduction

Build the qualification runner and execute a new, non-overwriting archive:

```powershell
cmake --build --preset windows-msvc-debug --target mehlissa_dccq1_qualification_runner
python scripts/run_dynamic_capillary_tissue_cell_qualification.py `
  --runner build/windows-msvc/models/cosimulation/Debug/mehlissa_dccq1_qualification_runner.exe
python scripts/check_dynamic_capillary_tissue_cell_protocol.py
python scripts/check_dynamic_capillary_tissue_cell_qualification_result.py
```

Linux and macOS use their corresponding preset and runner path. A new archive
does not replace the authoritative result automatically. Changed equations,
parameters, sources, tolerances, transformations or exclusions require a new
prospective protocol and candidate version.

## Remaining scientific work

The computational block is closed. The two external exits are intentionally
not manufactured inside the repository:

1. obtain a rights-compatible, condition-matched, source-disjoint dynamic
   primary-HUVEC series; freeze its values, transformations, uncertainty and
   no-refit acceptance rules before comparing this candidate; and
2. obtain an external scientific reviewer attestation over source mapping,
   equations, code, archive, evidence roles and claim language.

Those inputs can advance DCCQ-G7 and DCCQ-G8. They may also fail or narrow the
candidate; that outcome must be retained rather than used to rewrite this
evaluated version.
