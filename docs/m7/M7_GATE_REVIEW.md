<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M7 Gate Review - Holistic Vertical Slice

**Review date:** 2 September 2026  
**Decision:** passed for the reproducible research-software demonstrator  
**Clinical/physiological validation decision:** not claimed

## Decision scope

Gate M7 asks whether MEHLISSA can compose its independently developed layers
into one reproducible fingerprinting workflow without adding fingerprint logic
to the kernel or reusable model libraries. It does not ask whether the FP9
biology, communication channel, or patient response is clinically validated.

The gate passes in that software scope. The result must continue to be called a
multilayer research demonstrator, not an FP9 assay model, diagnostic simulator,
or patient prediction.

## Acceptance review

| Criterion | Decision | Evidence |
|---|---|---|
| Workflow functions without scenario-specific changes to kernel or layers | satisfied | All orchestration lives in `scenarios/fingerprinting`; the kernel and M2-M6 model libraries are consumed through existing public APIs. The holistic test executes composition, component probe, binding, tile release/assembly, local collection, gateway publication, BAN delivery, station reception, and Level-E analysis. |
| Every layer can be replaced with a simpler or detailed variant | satisfied at contract boundary | Thirteen role-based artifact references, typed loaders/factories, and scenario-owned adapters isolate selection from the kernel. A replacement must satisfy the same profile/schema and identity contracts; M7 does not claim arbitrary plug compatibility without those contracts. |
| Complete run is reproducible with manifest, seeds, versions, and result | satisfied | Result schema 2.0.0 records scenario/run/cohort/target, seed, paths and SHA-256 for every definition and schema, component state, stages, and Levels B-E outcomes. Repeated serialization has an identical SHA-256 digest. |
| Uncertainty and validity limitations accompany the result | satisfied | Stage evidence basis distinguishes execution, historical timer, and software surrogate. The result contains scenario and Level-E limitations plus Wilson intervals for classification proportions and an explicit no-clinical-validation declaration. |
| English User Guide passed gate-impact review | satisfied | The guide now explains the purpose, runnable M7.1-M7.7 paths, interpretation, result contents, negative controls, and non-claims; the roadmap retains mandatory review after every future M-gate. |

## Executed evidence

- M7.1 strict composition and stage order;
- M7.2 typed initialization and deterministic causal trace;
- M7.3 artifact-hashed result contract;
- M7.4 concentration/receptor binding plus subthreshold control;
- M7.5 explicit nine-tile assembly plus incomplete control;
- M7.6 locator-to-station communication and separated metrics;
- M7.7 deterministic false-positive/false-negative campaign and intervals; and
- one holistic Levels A-E runner and strict result 2.0.0 serialization test.

## Limitations retained after the gate

1. The BVS95, five-lobe lung, and pulmonary capillary components are selected,
   initialized, and advanced in one host, but the published FP9 localization
   and collector-return timings remain historical regression values rather
   than predictions from anatomically resolved cross-layer transport.
2. The capillary-to-cell signal profile and receptor kinetics are synthetic;
   the exact FP9 gene-product combination and a qualified disease marker data
   set are not implemented.
3. Tile assembly uses explicit identities and an all-required rule, but its
   duration is the historical NetTAS result; NetTAS and molecular assembly
   physics are not executed.
4. Local and BAN channels are deterministic software surrogates, not calibrated
   molecular, intrabody, wearable, Bluetooth, or IEEE 802.15.6 models.
5. The four-case Level-E campaign verifies metric semantics only. It does not
   estimate empirical sensitivity, specificity, prevalence effects, or
   patient-level uncertainty.
6. Binding affinity, perfusion, injection site, device-count distributions,
   stochastic transport, and patient variability require broader post-M7
   campaigns before scientific conclusions are possible.

These are validity limits and future research work, not hidden gate failures:
the accepted artifact is the platform's first complete, reproducible software
vertical slice.

## Verification command

```powershell
cmake --build --preset windows-msvc-debug --target mehlissa_fingerprinting_scenario_tests
ctest --test-dir build/windows-msvc -C Debug --output-on-failure -R "M7|holistic"
```

