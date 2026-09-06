<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# MRSQ-1 medical reference scenario result

**Date:** 6 September 2026  
**State:** computational path complete; measured cohort evaluation and external human review blocked  
**Clinical use:** prohibited

## What has been achieved

Medical Reference Scenario Qualification (MRSQ-1) now has an executable,
source-disjoint path from intravenous fluorine-18 fluorodeoxyglucose ([18F]FDG)
administration through a vascular input, lung/liver/kidney exchange and trapping,
renal transfer, fluorine-18 decay convention, and dynamic PET-frame observation.
The implementation, its parameters, assumptions, code hashes and an eight-frame
software reference are frozen before any HedyPET participant outcome was opened.

This is a computational qualification result, not a physiological validation.
The HedyPET comparison has deliberately not run because the measured-data
boundary is not yet releasable.

## Result by increment

| Increment | Result | Evidence boundary |
| --- | --- | --- |
| MRSQ-1.1 scenario selection | PASS | Five scenarios and eight sources screened before participant outcomes. |
| MRSQ-1.2 prospective protocol | PASS | Five cohort endpoints, source separation, uncertainty and no-refit rules frozen. |
| MRSQ-1.3 selective ingress | synthetic PASS; measured BLOCKED | Manifest-first authorization, exact paths/sizes/hashes, direct-identifier rejection, opaque linkage and metadata-only reporting are exercised on arbitrary data. No participant CSV was opened. |
| MRSQ-1.4 FDG/PET modules | PASS | Typed public C++ API and deterministic implementation cover the complete candidate path. |
| MRSQ-1.5 candidate freeze | PASS | Source-disjoint parameters, assumptions, seven uncertainty classes, three source hashes, replay, convergence, decay and bounds checks, and an eight-frame reference are frozen. |
| MRSQ-1.6 cohort evaluation | software PASS; external endpoints BLOCKED | Participant-first duration-weighted normalized root-mean-square error, area-under-curve ratio, deterministic bootstrap and PASS/PARTIAL/FAIL/BLOCKED states are tested with arbitrary arrays. No HedyPET value was evaluated. |
| MRSQ-1.7 close-out | machine PASS; human BLOCKED | A runner-independent checker verifies archive, hashes, claims and all seven increment states. No external reviewer has attested the result. |

## Candidate construction

The four-exponential population blood impulse response is taken from the open
[population input-function study](https://doi.org/10.1186/s40658-022-00490-y)
and convolved with the recorded injection duration. Lung and liver reference
coefficients come from a separate
[normal-tissue simulation study](https://doi.org/10.1155/2014/807167).
Kidney coefficients use the normal-tissue table in a separate
[total-body PET study](https://doi.org/10.1007/s00259-024-06879-4).
The 109.77-minute physical half-life is bound to the
[NIST fluorine-18 standard](https://doi.org/10.6028/jres.119.013).
None of these construction sources is HedyPET.

The reference candidate deliberately retains several engineering assumptions:
a normalized population rather than individual arterial input, allometric
cardiac output, reference organ volumes, liver vascular fraction, irreversible
kidney trapping, a fixed renal-transfer rate, and a one-way bladder accumulator
without voiding or ureter delay. These assumptions are visible in
`data/qualification/medical-reference-scenario-candidate-freeze-v1.json`; they
must not be described as measured healthy-adult physiology.

## Public interfaces and reproducibility

The public API is
`scenarios/fdg_pet/include/mehlissa/scenarios/fdg_pet/fdg_pet_model.hpp`.
It exposes typed `Administration`, `TissueKinetics`, `CandidateParameters`,
`Frame`, and `FramePrediction` records, an explicit `DecayReference`, the
`source_disjoint_reference_candidate` factory, and `simulate`. Tracer behavior
is isolated in a scenario package; no VEGF, CD95 or FP9 kernel branch was
relabelled.

Build and execute the inspectable software reference on Windows with:

```powershell
cmake --build --preset windows-msvc-debug --target mehlissa_fdg_pet_reference_runner mehlissa_fdg_pet_scenario_tests
build/windows-msvc/tests/Debug/mehlissa_fdg_pet_scenario_tests.exe
build/windows-msvc/scenarios/fdg_pet/Debug/mehlissa_fdg_pet_reference_runner.exe
python scripts/check_medical_reference_scenario_closeout.py
python -m unittest tests/test_mrsq_cohort_evaluator.py tests/test_medical_reference_scenario_closeout.py
```

The committed CSV is a deterministic software oracle only. It contains no
participant observation and is not a calibration or validation result.

## Why measured ingress remains blocked

Four independent gates remain open:

1. a named local role must record the institutional determination and approve
   purpose, access, quarantine and retention outside the repository;
2. the four authorized participant assets require raw-byte SHA-256 values
   before parsing;
3. the selected combined HedyPET input-function and tissue-curve tables publish
   frame midpoints but not the exact frame durations required by the frozen
   frame-integration rule; an authoritative mapping or source companion is
   required;
4. after release, all five predeclared endpoints need at least 60 complete
   participants, and a genuinely external reviewer must attest the archive.

No interpolation from outcome curves, undocumented frame schedule, refit, or
post-outcome threshold change is permitted. A prospective MRSQ-1.3 amendment
must record the resolved governance, frame authority and exact content hashes
before the first participant CSV is opened.

## Permitted conclusion

MRSQ-1 is computationally complete through a frozen, source-disjoint FDG/PET
candidate and a synthetic-qualified evaluation path. External healthy-adult
cohort agreement has not been tested. The result establishes neither validated
physiology nor individual, diagnostic, therapeutic, safety or clinical use.
