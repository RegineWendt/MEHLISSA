<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Paper 1 Evidence and Validity Baseline

## Purpose

This baseline makes the evidence status of the executable M2-M7 platform
reviewable before Paper 1 claims are written. The authoritative record is the
schema-validated
`data/evidence/evidence-validity-matrix-v1.json`; this document is its concise
human-readable guide. The matrix separates model structure, parameterization,
calibration, independent validation, analytical/numerical verification,
historical regression, assumptions, synthetic fixtures, and sensitivity-only
evidence.

The baseline supports a platform-and-methods paper. It does **not** establish
biological feasibility, device feasibility, patient prediction, diagnostic
performance, treatment utility, or clinical validity. In particular, the M7
FP9/lung workflow is software integration evidence plus historical regression,
not a biological or clinical feasibility result.

## Current evidence maturity

| Executable family | Strongest present evidence | Correct interpretation | Principal open blocker |
|---|---|---|---|
| Body/systemic transport | historical regression; numerical integrity | faithful, deterministic use of the released schematic BVS95 reference and state-neutral observation | independent modern systemic-flow/transport validation |
| Organ/lung | literature calibration plus calibration-disjoint aggregate and lobar comparisons | partial healthy-adult aggregate/lobar qualification inside declared protocols | participant-level trajectories, dynamic lobar transit and joint uncertainty |
| Capillary/molecular transport | measured/literature-derived equivalent geometry; analytical and numerical verification | conservative interchangeable methods on an evidence-bounded geometry | named biomarker/tissue kinetics and external biological validation |
| Cell/intracellular response | analytical and numerical verification with synthetic fixtures | verified generic binding, ODE/SSA, delivery and population contracts | named biological system, frozen external model and independent reproduction |
| Nano-IoT/gateway/BAN/station | deterministic positive and fail-closed software tests | interface, causal-identity, accounting and governance semantics | calibrated physical channels, hardware and security qualification |
| M7 FP9/lung scenario | deterministic integration, artifact provenance and historical timer regression | complete platform vertical slice and software/integration proof | resolved cross-layer biology, communication physics and external medical reference case |

## Calibration and validation separation

The lung family is the only current family with substantive independent
physiological comparisons. The matrix records calibration and validation source
IDs separately:

- ESC/ERS, Swift, Claessen, Kane, Kovacs 2012 and Lee inform model parameters;
- Forton, Bentley, Wolsk and Bourhis are retained for no-refit comparisons;
- the two Bourhis reconstructions share a cohort and are therefore not counted
  as independent cohorts; and
- the body reference and FP9 timeline reuse their historical sources as
  regression targets and are never labelled independent validation.

The pulmonary capillary card combines functional and morphometric evidence from
different small cohorts. This is a transparent numerical closure, not a joint
cohort validation. Cell and communication parameters remain synthetic even
when their implementations are analytically or numerically verified.

## Machine checks

Install the publication validation extra and execute:

```console
python -m pip install -e ".[publication]"
python scripts/check_evidence_validity_matrix.py
python -m unittest tests/test_evidence_validity_matrix.py
```

The checker validates the JSON Schema, exact six-family coverage, stable and
unique identifiers, source references, calibration/validation overlap
disclosures, file existence, concrete external-source roles and the global
no-clinical-validity declaration. Negative tests demonstrate rejection of an
unknown evidence role, an invented source, a missing artifact and an incomplete
family record.

## Bibliography export

`docs/publication/paper1-evidence-sources.bib` provides a reproducible starter
export for Paper 1. Its presence does not imply that every source will be cited
in the manuscript. The machine-readable matrix remains authoritative for the
role and licensing status of each source.

## Maintenance rule

Any new executable model family, material parameter group, scientific output,
or planned platform claim must update the matrix and pass its checks before it
is presented as Paper 1 evidence. Missing evidence must remain explicit; it may
not be converted into a literature-derived or validated status by inference.
