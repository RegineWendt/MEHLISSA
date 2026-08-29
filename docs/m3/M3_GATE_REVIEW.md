<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 Gate Review – Body–Organ Coupling

**Review date:** 29 August 2026

**Reviewed baseline:** `29d6e3e` plus the versioned M3.19 artifacts in this
review increment

**Result:** passed — body-organ coupling, pulmonary reference evidence,
same-scenario resolution comparison, and historical FP9 timer baseline complete

**Post-review update, 28 August 2026:** M3.7 adds the first executable,
literature-parameterized pulmonary 0D reference candidate. This closes the
absence of source-scoped mean pressure, flow, resistance, compliance, total
transit, right/left perfusion, and parameter uncertainty at implementation
level. It does not change the gate result because its nominal values form a
composite calibration reference rather than a jointly measured subject set,
independent quantitative validation and anatomical refinement remain open,
and the FP9 scenario-layer baseline is still not executable. See
[Pulmonary 0D Reference Candidate](PULMONARY_0D_REFERENCE.md).
The post-review local Visual Studio Debug suite passed 91/91 tests; CI evidence
is recorded separately when the increment is pushed.

**Second post-review update, 28 August 2026:** M3.8 adds executable independent
aggregate validation against two previously unused healthy cohorts. A
scope-matched supine comparison and invasive semiupright rest/exercise
crosscheck pass all 6 required pressure, compliance, and resting-RC endpoints.
The diagnostic exercise RC endpoint fails strongly and records the need for
state-dependent vascular adaptation. This strengthens the scientific evidence
but does not close anatomical refinement, subject-level validation, or the FP9
scenario-layer baseline. See
[Independent Pulmonary 0D Validation](PULMONARY_0D_INDEPENDENT_VALIDATION.md).
The post-M3.8 local Visual Studio Debug suite passed 93/93 tests; CI evidence
is recorded with the pushed increment.

**Third post-review update, 28 August 2026:** M3.9 adds an immutable v2
candidate with bounded flow-dependent PVR and compliance. Claessen et al. is
used for calibration; Wright et al. is excluded because Bentley reused that
healthy-control cohort. The untouched Bentley stress test retains 6/6 required
passes and reduces the exercise RC diagnostic z-score from 18.571 to 3.005,
which remains a fail. State-dependent lumped adaptation is therefore
implemented, while subject-level uncertainty, regional redistribution,
anatomy, and FP9 remain open. See
[Bounded Pulmonary 0D Flow Adaptation](PULMONARY_0D_FLOW_ADAPTATION.md).
The post-M3.9 local MSVC suite passed 100/100 tests; CI evidence is recorded
with the pushed increment.

**Fourth post-review update, 28 August 2026:** M3.10 adds a schema-validated,
pseudonymous subject-level multipoint analysis path. It evaluates at least three
joint mPAP/PAWP/cardiac-output stages per healthy control against the immutable
v2 model and reports pressure-flow fits, stage PVR, optional compliance/RC,
residuals, and RMSE. Synthetic fixtures are rejected by the default evidence
loader. The independent UA iCPET healthy-control cohort is the preferred first
source, but its stage records require author approval and data-use terms.
Consequently, the analysis software is verified while measured subject-level
validation remains open. See
[Subject-Level Pulmonary Multipoint Validation](PULMONARY_0D_SUBJECT_MULTIPOINT_VALIDATION.md).
The post-M3.10 local MSVC build and 103/103 CTest suite passed; CI evidence is
recorded with the pushed increment.

**Fifth post-review update, 28 August 2026:** M3.11 evaluates the immutable v2
candidate against four directly published, calibration-disjoint population
series from Kovacs and Wolsk: 255 healthy volunteers and 18 aggregate stages.
The locked rules yield 10/18 agreeing stages. Kovacs passes 3/3 and the Wolsk
40–59-year stratum 5/5; the 20–39-year stratum passes 0/5 and the 60–80-year
stratum 2/5. These failures are retained as evidence that the composite,
age-independent v2 model needs an age-conditioned successor. Published
population data therefore remove the immediate controlled-access blocker but
do not claim participant-level validation. See
[Published-Population Pulmonary Multipoint Validation](PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md).
The post-M3.11 local MSVC build and 105/105 CTest suite passed; CI evidence is
recorded after the isolated increment is pushed.

**Sixth post-review update, 28 August 2026:** M3.12 adds an immutable v3
candidate with a separately calibrated age multiplier on PVR. Kane et al.
(2016) supplies resting PASP/CO age ratios and supported bounds of 18–85 years;
Kovacs and Wolsk remain untouched validation sources. Under the existing rules,
agreement improves from 10/18 to 14/18 stages. The older stratum improves from
2/5 to 5/5, the reference stratum remains 5/5, and the young stratum improves
only from 0/5 to 1/5. The remaining failure is retained and the gate stays open
for anatomical, continuous/regional physiological, participant-level, and FP9
evidence. See [Pulmonary 0D Age Conditioning](PULMONARY_0D_AGE_CONDITIONING.md).
The post-M3.12 local MSVC build and 108/108 CTest suite passed; CI evidence is
recorded after the isolated increment is pushed.

**Seventh post-review update, 28 August 2026:** M3.13 investigates the young
v3 residual instead of fitting it away. A published invasive reanalysis reports
0.8625 WU resting PVR for ages 24–50 and a young moderate-exercise PVR change
whose implied flow exponent closely matches the existing Claessen rule. The
immutable v4 candidate therefore changes only the young resistance level and
narrows its lower age bound to 24. All 15 stages in the calibration-disjoint
Wolsk cohort agree; pressure RMSE falls from 2.453 to 1.413 mmHg. Kovacs 2009
is not counted because Kovacs 2012 reanalysed part of the same historical
corpus. This strengthens population evidence but does not close anatomical,
participant-level, pulsatile/regional, or FP9 requirements. See
[Pulmonary 0D Young-Adult Resistance Qualification](PULMONARY_0D_YOUNG_RESISTANCE.md).
The post-M3.13 local MSVC build and 110/110 CTest suite passed; CI evidence is
recorded with the pushed increment.

**Eighth post-review update, 28 August 2026:** M3.14 introduces an immutable v5
candidate using the Linehan pressure-distensible vessel equation and the
Reeves healthy-population mean distensibility coefficient. The structural law
replaces the empirical flow adaptation and cannot be enabled with it. Derived
zero-pressure resistance preserves the previously qualified resting
equilibrium. Against the unchanged, calibration-disjoint Wolsk stages, v5
agrees at 11/15: both younger strata pass 5/5, but the 60–80-year stratum passes
only 1/5. This is retained as evidence that a fixed all-age distensibility is
insufficient; v5 therefore does not supersede the 15/15 empirical v4 reference.
See [Pulmonary 0D Pressure Distensibility](PULMONARY_0D_PRESSURE_DISTENSIBILITY.md).
The post-M3.14 local MSVC build and 115/115 CTest suite passed; CI evidence is
recorded with the pushed increment.

**Ninth post-review update, 28 August 2026:** M3.15 tests the smallest
independently supported response to v5's older-stratum failure. Reeves et al.
report `alpha = 0.015 ± 0.001 mmHg^-1` in an invasive 61–83-year group versus
approximately `0.020 mmHg^-1` in young adults. The immutable v6 candidate uses
the older value from age 60, records the published uncertainty as standard
error, and re-normalizes `R0` to preserve each age band's resting equilibrium.
Against the unchanged Wolsk protocol, older RMSE improves from 5.411 to
4.603 mmHg but agreement remains 1/5 older and 11/15 overall. The older source
is male-specific, so sex-general use is also explicitly limited. V6 narrows
the structural hypothesis but does not replace v4 or close the gate. See
[Pulmonary 0D Age-Conditioned Distensibility](PULMONARY_0D_AGE_DISTENSIBILITY.md).
The post-M3.15 local MSVC build and 119/119 CTest suite passed.

**Tenth post-review update, 28 August 2026:** M3.16 implements five parallel,
anatomically named lobar 0D beds. A transparent DE-CT proxy supplies the fixed
lobar fractions, while `R_i = R/f_i` and `C_i = f_i C` preserve the qualified
v4 aggregate response exactly. Individual entities follow one deterministic
lobe transit path. The new structure materially resolves the former serial
regional surrogate gap, but DE-CT PBV is not direct flow and no independent
lobar target has yet been evaluated. See
[Pulmonary Lobar Parallel Beds](PULMONARY_LOBAR_PARALLEL_BEDS.md).

**Eleventh post-review update, 29 August 2026:** M3.17 evaluates all five v7
bed fractions against Bourhis et al.'s independent normal V/Q SPECT/CT lobe
shares. The source is disjoint from v7's Lee DE-CT calibration by authors,
cohort, and modality. Both attenuation reconstructions pass the declared
3-percentage-point per-lobe, 2-percentage-point RMSE, and
3-percentage-point right-lung limits without refitting. Published rounding is
preserved and explicitly normalized; the uncorrected values total 100.1% as
reported. This qualifies the fixed healthy-adult supine distribution, while
dynamic posture/activity redistribution, disease, participant-level evidence,
and FP9 remain open. See
[Independent Pulmonary Lobar Perfusion Validation](PULMONARY_LOBAR_PERFUSION_VALIDATION.md).
The post-M3.17 local Visual Studio Debug build and 127/127 CTest suite passed;
cross-platform CI evidence is recorded with the pushed increment.

**Twelfth post-review update, 29 August 2026:** M3.18 adds a checked-in,
schema-validated end-to-end scenario that changes only the pulmonary model
card. The shared body graph, route, seed, 25 entity identities, population,
substance amount, volume flow, synchronization step, and acceptance rules are
identical for the effective compartment and five-lobe v7 model. Both return
25/25 entities and all three typed payloads exactly with closed ownership. The
2.0 s and 6.4 s transit results deliberately remain model-specific outputs.
Candidate-local route overrides are rejected by schema. This satisfies the
same-scenario Gate M3 criterion without adding resolution branches to the body,
coupler, or kernel. General CLI composition and FP9 remain open. See
[Coarse–Detailed Body–Lung Scenario Comparison](COARSE_DETAILED_SCENARIO_COMPARISON.md).
The post-M3.18 local Visual Studio Debug build and 129/129 CTest suite passed;
the complete cross-platform matrix passed in
[run 33242297287](https://github.com/RegineWendt/MEHLISSA/actions/runs/33242297287).

**Thirteenth post-review update, 29 August 2026:** M3.19 adds an executable,
schema-validated replay of the historical FP9 lung timer at the experiment
layer. Direct source review confirms that the 91 s value is the complete time
from injection through collection and wrist reporting, not a delay added after
the 25 s localization and 15.99 s assembly values. The executable chain is
therefore 0 s injection, 25 s first localization, 40.99 s active message, and
209/91 s external report for the published 1,000/10,000 collector cohorts. It
rejects acausal times and unknown cohorts and runs an altered non-lung target
through the same implementation. No fingerprint, tissue, or lung branch enters
the kernel, body, coupler, or organ model. See
[Historical FP9 Lung Timer Baseline](FP9_TIMER_BASELINE.md).
The post-M3.19 local Visual Studio Debug build and complete 133/133 CTest suite
passed, including the separately committed benchmark package now in the
reviewed baseline. Cross-platform CI evidence is recorded with the pushed
increment.

## 1. Review method

The review checked the Roadmap Gate M3 statements and the additional acceptance
criteria in ADR-0006 against executable evidence. Software verification was
kept separate from anatomical fidelity, physiological parameterization, and
independent validation. A second class was not treated as a “detailed organ
model” merely because it contained more regions than the coarse surrogate.

## 2. Roadmap gate criteria

| Gate M3 criterion | Status | Evidence and finding |
|---|---|---|
| an agent moves reproducibly from the body graph into an organ and back | satisfied | `BodyOrganCoupler`, explicit outside-body ownership ledger, stable ID and named-port validation, coarse/regional × 0.5/1.0 s matrix |
| flow, populations, and substance amounts are conserved across the layer boundary | satisfied for lossless transit | dimension-safe contracts, exact boundary ledger, both lung endpoints preserve transfer ID and typed payload; changing biochemical exchange is intentionally not claimed |
| the organ has an independent, interchangeable model implementation | satisfied for the scoped mean 0D reference | the factory selects a distinct five-bed anatomical 0D implementation behind the same contract; aggregate physiology inherits exact v4 equivalence and both independent normal-supine V/Q SPECT/CT reconstruction series pass the regional criteria |
| a coarse compartment and more detailed organ model use the same scenario | satisfied | M3.18 defines the body, route, injection, seed, step, conserved payload, and acceptance rules once; only the schema-selected organ card changes, and both candidates preserve exact scenario meaning while retaining model-specific timing |

## 3. Additional ADR-0006 criteria

| Criterion | Status | Finding |
|---|---|---|
| agent and substance flow complete body–lung–body traversal | satisfied for entity; organ traversal satisfied for conserved quantities | the complete entity round trip reaches the body return ledger; conserved population, substance, and flow traverse both lung endpoints without payload change |
| no agent or relevant amount is duplicated or lost | satisfied | positive and negative ownership, duplicate-ID, route, time, and exact-payload tests |
| both lung variants implement the same contract | satisfied | one `ModelComponent` interface, one `LungModelConfig`, schema-selected definitions, generated cross-variant tests |
| flow, pressure/transit time, and perfusion share have units, sources, uncertainty, and an independent comparison | satisfied for aggregate, multipoint population, and five-lobe normal-supine evidence; participant-level method also verified | the 0D candidates have executable SI pressure, flow, effective PVR/compliance, measured transit, evidence roles, source- and cohort-disjoint validation, frozen population comparisons, and a separate participant-level evaluator; M3.17 additionally compares all five executable shares with independent V/Q SPECT/CT while preserving source rounding |
| historical FP9 timer baseline runs without scenario-specific kernel logic | satisfied | M3.19 loads a strict versioned scenario and emits the published causal FP9 event chain for both collector cohorts from a reusable experiment-layer implementation; a synthetic non-lung case verifies the absence of lung-specific branching |

## 4. Verification evidence

- local Visual Studio 2026 Debug build: passed;
- post-M3.19 local CTest suite: 133/133 passed;
- local formatting and targeted Clang-Tidy/bugprone checks: passed;
- `git diff --check`: passed for the reviewed M3 changes;
- complete M3.5 GitHub matrix: [run 33081276396](https://github.com/RegineWendt/MEHLISSA/actions/runs/33081276396), passed on Windows/MSVC, Linux/GCC, and Linux/Clang with Clang-Tidy, ASan, and UBSan;
- final M3.6 orchestration matrix:
  [run 33081839637](https://github.com/RegineWendt/MEHLISSA/actions/runs/33081839637),
  passed on Windows/MSVC, Linux/GCC, and Linux/Clang with Clang-Tidy, ASan,
  and UBSan.

## 5. What is complete and reusable

- versioned entity, population, substance-amount, and volume-flow contracts;
- deterministic identity and quantity ownership at model boundaries;
- explicit body hand-off, organ ownership, and body return;
- coarse and regional pulmonary transit implementations behind one interface;
- schema-validated executable model cards with validity, evidence, sources,
  licenses, limitations, and optional external-data axes/units/provenance;
- one checked-in schema-validated scenario across coarse and five-lobe
  resolutions, plus compatible host-step regressions;
- one schema-validated historical FP9 timer baseline at the experiment layer,
  with stable causal event identifiers and no kernel-specific logic; and
- cross-platform build, analysis, sanitizer, and regression infrastructure.

These contracts are suitable for continued software integration. The qualified
reference candidates remain population-scoped research models, not
patient-specific or clinically usable simulations.

## 6. Blocking scientific closure package

M3 can pass only after the following evidence is added and reviewed:

1. ~~qualify and license a concrete pulmonary reference case or a sourced 0D/1D
   parameterization, including population and physiological state~~ — M3.7
   supplies the sourced 0D parameterization and M3.16–M3.17 add and independently
   evaluate the five-lobe distribution; geometry remains a later refinement;
2. ~~provide sourced pressure, resistance/flow, perfusion, and transit targets
   in SI units with uncertainty and explicit calibration/validation
   separation, followed by independent aggregate and multipoint population
   comparisons~~ — implemented by M3.7–M3.11; participant-level validation
   remains a higher-resolution follow-up rather than the immediate M3 blocker;
3. ~~implement, independently evaluate, and compare an anatomical/hemodynamic
   regional variant with the effective compartment~~ — M3.16 implements five
   parallel beds, M3.17 qualifies the fixed shares, and M3.18 runs both
   resolutions under one scenario; dynamic regional states remain a refinement;
4. ~~add an explicitly reviewed baseline lobar distribution~~ — M3.16 adds a
   fixed DE-CT PBV proxy with limitations; posture/exercise redistribution and
   regional recruitment remain open;
5. ~~implement the historical FP9 timing regression at the appropriate scenario
   layer~~ — M3.19 executes both published collector cohorts in the experiment
   layer, with direct source semantics and no kernel/organ special case; and
6. ~~rerun this gate review on the resulting immutable data/model baseline and
   complete CI matrix~~ — the post-M3.19 local 133-test suite and final
   cross-platform build, analysis, sanitizer, and test matrix complete the
   review evidence.

## 7. Exit decision

M3 passes. The software architecture, lossless coupling slice, independent
aggregate and population physiological comparisons, five-lobe implementation
and regional validation, same-scenario coarse/detailed comparison, and neutral
historical FP9 timing reference are executable and reviewed. The
subject-level multipoint path is software-verified but still awaits measured
participant records. Dynamic regional physiology, patient-specific anatomy,
transforming exchange, and the biological/gateway levels of fingerprinting
remain explicit later refinements rather than hidden failures. Downstream work
may use the verified interfaces provided it retains each model definition's
validity and evidence-class labels and does not cite an output beyond its
qualified physiological scope.
