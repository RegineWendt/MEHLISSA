<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 Gate Review – Body–Organ Coupling

**Review date:** 27 August 2026

**Reviewed baseline:** `ea2b2932f9e6268e70f0494507a3ab7ceae9ea41`

**Result:** not passed — technical coupling candidate complete; scientific gate
remains open

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
| the organ has an independent, interchangeable model implementation | partially satisfied | two independent `ModelComponent` implementations and one factory/definition path exist; the regional implementation remains a structural test surrogate rather than an independently qualified physiological model |
| a coarse compartment and more detailed organ model use the same scenario | partially satisfied | the same externally selected body–organ regression runs both implementations without kernel/coupler branches; “more detailed” is structural only, not anatomical 0D/1D or vascular geometry |

## 3. Additional ADR-0006 criteria

| Criterion | Status | Finding |
|---|---|---|
| agent and substance flow complete body–lung–body traversal | satisfied for entity; organ traversal satisfied for conserved quantities | the complete entity round trip reaches the body return ledger; conserved population, substance, and flow traverse both lung endpoints without payload change |
| no agent or relevant amount is duplicated or lost | satisfied | positive and negative ownership, duplicate-ID, route, time, and exact-payload tests |
| both lung variants implement the same contract | satisfied | one `ModelComponent` interface, one `LungModelConfig`, schema-selected definitions, generated cross-variant tests |
| flow, pressure/transit time, and perfusion share have units, sources, uncertainty, and an independent comparison | satisfied for aggregate and published multipoint population evidence; participant-level method also verified | the 0D candidates have executable SI pressure, flow, effective PVR/compliance, empirical and fixed/age-conditioned pressure-distensible alternatives, measured total transit, right/left perfusion, uncertainty/evidence roles, source- and cohort-disjoint validation, explicit stress-test diagnostics, frozen population comparisons, and a separate pseudonymous participant-level trajectory evaluator; v5 exposes the fixed-distensibility limitation and v6 shows that the independently sourced older coefficient improves error without resolving failed stages |
| historical FP9 timer baseline runs without scenario-specific kernel logic | not satisfied | the dissertation baseline is specified and traced, but fingerprint detection/assembly/collection is not executable; implementing it prematurely in the organ kernel would violate the architecture |

## 4. Verification evidence

- local Visual Studio 2026 Debug build: passed;
- local CTest suite: 85/85 passed;
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
- one externally selected scenario across both resolutions and compatible host
  steps; and
- cross-platform build, analysis, sanitizer, and regression infrastructure.

These contracts are suitable for continued software integration. They do not
make the current lung models physiological reference models.

## 6. Blocking scientific closure package

M3 can pass only after the following evidence is added and reviewed:

1. qualify and license a concrete pulmonary reference case or a sourced 0D/1D
   parameterization, including population and physiological state; the initial
   [SimVascular candidate review](SIMVASCULAR_PULMONARY_CANDIDATE_REVIEW.md)
   narrows the open questions but does not qualify the archive;
2. ~~provide sourced pressure, resistance/flow, perfusion, and transit targets
   in SI units with uncertainty and explicit calibration/validation
   separation, followed by independent aggregate and multipoint population
   comparisons~~ — implemented by M3.7–M3.11; participant-level validation
   remains a higher-resolution follow-up rather than the immediate M3 blocker;
3. implement the anatomical/hemodynamic regional variant and compare it with
   both independent targets and the effective compartment;
4. extend the implemented lumped rest/exercise flow adaptation with reviewed
   right/left or lobar redistribution if it belongs in the M3 reference set;
5. implement the historical FP9 timing regression at the appropriate scenario
   layer, or formally revise the ADR/M3 boundary if that executable baseline is
   deferred to M7; and
6. rerun this gate review on the resulting immutable data/model baseline and
   complete CI matrix.

## 7. Exit decision

The M3 software architecture, lossless coupling slice, first independent
aggregate physiological comparison, and subject-level multipoint analysis path
are verified, and published population multipoint validation has produced a
qualified partial result, but the milestone is not closed. The gate remains
open on anatomical/state- and age-dependent refinement and the FP9 executable
reference, not on a hidden software failure. Downstream
prototyping may use the verified interfaces provided it retains the explicit
`software_test_surrogate` validity label and does not cite the current outputs
as pulmonary physiology.
