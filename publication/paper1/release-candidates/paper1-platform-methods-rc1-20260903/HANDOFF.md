<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Paper 1 reviewer and collaborator handoff

## Proposed paper position

Paper 1 should be a platform and methods paper.  Its defensible contribution is
the versioned multiscale architecture, explicit coupling and ownership
contracts, reproducible experiment machinery, evidence/validity bookkeeping,
bounded observation, supported research interfaces, and an integrated FP9/lung
software demonstrator.  It should not be positioned as an already validated
clinical fingerprinting system.

## Exact candidate identity

- source commit: `30415e1c6997cae9744115d502604b7df0fe234c`;
- development branch: `mehlissa-next-generation`;
- suggested release-candidate tag: `paper1-platform-methods-rc1`;
- protocol: Paper 1 Technical Experiment Protocol v2.0.0;
- evidence matrix: version 1.0.0, six executable model families;
- release manifest: `release-candidate.json`;
- byte manifest: `SHA256SUMS.json`.

The tag is a suggestion only.  No tag or DOI is created by this package.

## Reproduce the technical checks

From the repository root, install the publication validator and run:

```powershell
python -m pip install -e ".[publication]"
python scripts/check_evidence_validity_matrix.py
python scripts/check_paper1_protocol.py
python scripts/check_paper1_release_candidate.py
python -m unittest tests.test_evidence_validity_matrix -v
python -m unittest tests.test_paper1_protocol -v
python -m unittest tests.test_paper1_measurement_tools -v
python -m unittest tests.test_paper1_release_candidate -v
```

Build and run the complete supported test suite using the Development Guide.
The GitHub candidate-commit CI must pass Windows/MSVC, Linux/GCC,
Linux/Clang, formatting, clang-tidy, AddressSanitizer, and
UndefinedBehaviorSanitizer before the candidate is accepted for manuscript use.

The raw measurement ZIPs are complete retained runner outputs.  Expand them to
a new directory; do not edit them in place.  The body campaign can be rerun
with `benchmarks/run_rq4_campaign.py`, the M7 study with
`publication/paper1/run_m7_resource_study.py`, and the access comparison with
`publication/paper1/check_access_parity.py`.  Use the locked protocol and the
retained machine plan; do not silently change seeds, repetitions, conditions,
or failure rules.

## Results that may be reported with scope

- all 112 official body-observation attempts completed and their core state,
  population, RNG, and detailed observation invariants passed;
- bounded policies retained far less output than the deliberately detailed O3
  condition on the measured Windows/MSVC host;
- two small M7 collector-count conditions replayed deterministically across
  three repetitions and stayed within the reported local resource envelope;
- CLI, Python, and Workbench exposed the same authoritative M7 summary and the
  Workbench detected a tampered result;
- all six current executable families have schema-validated evidence roles,
  sources, licences, uncertainty/range metadata, validity scope, limitations,
  blockers, and linked artifacts.

Every statement above must cite its exact artifact and retain its stated test
scope.  The claim registry is the operational source of truth.

## Claims that remain prohibited

Do not claim biological validation of the FP9 receptor/cell/assembly chain,
clinical sensitivity or specificity, patient-specific prediction, treatment
efficacy, general HPC scaling, physiological correctness of synthetic IoT
parameters, or end-to-end external validation.  Historical regression,
analytical verification, literature parameterization, and software tests have
different evidence roles and must remain distinct.

## Reviewer checklist

- verify all hashes and schemas;
- inspect the invalid setup report and confirm it contributes no result;
- inspect negative controls rather than reporting positive paths alone;
- check that manuscript claims match the claim registry verbatim in scope;
- review calibration and validation source separation family by family;
- confirm raw archives can be expanded and machine reports parsed;
- confirm the complete local suite and exact candidate-commit CI pass;
- record any protocol amendment as a new version without editing v2.0.0;
- create a tag or DOI only after explicit release approval.
