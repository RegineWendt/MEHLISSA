<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary 0D Reference Candidate

## 1. Decision and scope

M3.7 introduces the first literature-parameterized pulmonary model behind the
same `ModelComponent` contract as the two software surrogates. Its explicit
reference state is a healthy adult at rest in the supine position. It models
mean pulmonary hemodynamics from the pulmonary-arterial entry to a fixed
left-atrial pressure sink. It does not model ventilation, gas exchange,
respiratory mechanics, pulsatile valve/ventricular dynamics, disease, or a
patient-specific subject.

The executable definition is
`data/lung-models/healthy-adult-rest-supine-0d-v1.json`. It validates against
schema `lung-model-definition/1.1.0`. The data file is CC-BY-4.0; cited source
articles are not redistributed and retain their own terms.

## 2. Governing model

The model is a two-element mean pulmonary Windkessel:

```text
                         R_right
Q_in -> C_PA -> P_PA ----/\/\/\/----+
                    |                +-> P_LA
                    +----/\/\/\/----+
                         R_left
```

The total outflow and pulmonary-arterial pressure obey

```text
Q_out = (P_PA - P_LA) / R_PVR
C_PA dP_PA/dt = Q_in - Q_out
P_eq = P_LA + R_PVR Q_in
```

For a constant inflow over a host step, the implementation uses the analytical
solution

```text
P_PA(t + dt) = P_eq + (P_PA(t) - P_eq) exp(-dt / (R_PVR C_PA)).
```

This avoids integration-step error for the linear RC state. The configured
time constant is 0.36 s. A `VolumeFlowTransfer` prescribes the inflow at the
next synchronization step; otherwise the last flow persists. Conflicting flow
prescriptions at one synchronization point are rejected.

Right and left mean flows use the measured supine split `f_R`:

```text
Q_R = f_R Q_out                 R_R = R_PVR / f_R
Q_L = (1 - f_R) Q_out           R_L = R_PVR / (1 - f_R).
```

The whole-circulation entity and conserved-transfer delay is currently one
measured pulmonary transit-time quantity. The model intentionally does not
invent unsupported arterial/capillary/venous timing fractions.

## 3. Reference quantities

All executable values are stored in SI units together with the source,
evidence role, uncertainty form, and derivation. Familiar clinical units below
are display units only.

| Quantity | Executable value | Evidence interval | Role and rationale |
|---|---:|---:|---|
| cardiac output | 6.0 L/min | 4-8 L/min normal interval | calibration choice inside the ESC/ERS reference interval and aligned with the dissertation rest baseline |
| left-atrial pressure sink | 8 mmHg | PAWP upper reference limit 15 mmHg | calibration choice; PAWP is an acknowledged surrogate |
| total PVR | 1.2 WU | 0.3-2.0 WU normal interval | calibration choice inside the ESC/ERS reference interval |
| pulmonary arterial compliance | 5.0 mL/mmHg | normal lower limit 2.3 mL/mmHg | calibration choice above the ESC/ERS limit |
| right-lung flow fraction | 0.5563 | propagated SD 0.0534 | derived from supine RPA 1996 ± 274 and LPA 1592 ± 266 mL/min/m² in 24 healthy volunteers |
| pulmonary transit time | 6.4 s | IQR 5.7-7.1 s | measured right-to-left ventricular first-pass PTT in 18 matched healthy controls |
| predicted mean PA pressure | 15.2 mmHg | 8-20 mmHg normal interval | derived prediction compared with a published normal-reference interval |

The principal hemodynamic intervals come from Table 11 and the pressure-
resistance definition in the 2022 ESC/ERS pulmonary-hypertension guideline.
The model card marks those numerical selections as calibration rather than
validation. The 15.2 mmHg equilibrium is a model output; it is compared with the
published normal interval but is not itself validation. Independent aggregate
validation is reported separately in
[Independent Validation of the Pulmonary 0D Candidate](PULMONARY_0D_INDEPENDENT_VALIDATION.md).

The right/left split is derived from Wong et al.'s phase-contrast MRI study.
The standard deviation shown for the ratio is a MEHLISSA first-order
propagation from the two reported flow SDs and assumes independence; the source
did not publish that ratio uncertainty directly. Swift et al.'s PTT uses a
specific contrast-bolus measurement definition, which must not be mixed with
pulmonary-capillary or other transit-time definitions.

## 4. Sources

1. Humbert M, Kovacs G, Hoeper MM, et al. 2022 ESC/ERS Guidelines for the
   diagnosis and treatment of pulmonary hypertension. *European Heart
   Journal*. 2022;43:3618-3731. <https://doi.org/10.1093/eurheartj/ehac237>.
2. Wong DTH, Lee KJ, Yoo SJ, Tomlinson G, Grosse-Wortmann L. Changes in
   systemic and pulmonary blood flow distribution in normal adult volunteers
   in response to posture and exercise. *Journal of Physiological Sciences*.
   2014;64:105-112. <https://doi.org/10.1007/s12576-013-0298-z>.
3. Swift AJ, Telfer A, Rajaram S, et al. Pulmonary arterial hypertension: MR
   imaging-derived first-pass bolus kinetic parameters are biomarkers for
   pulmonary hemodynamics, cardiac function, and ventricular remodeling.
   *Radiology*. 2012;263:678-687. <https://doi.org/10.1148/radiol.12111049>.
4. Wendt R. Dissertation simulation chapters, Chapter 4, included in
   `literature/Diss_WENDT_Simulationchapters.pdf`.

## 5. Verification evidence

The automated tests verify:

- dimension-safe pressure, resistance, compliance, and RC time constants;
- the 15.2 mmHg baseline equilibrium and exact right/left partition;
- analytical RC response to a flow step;
- unchanged return of the incoming flow transfer after 6.4 s;
- the shared named-port and route contract;
- schema decoding of evidence roles, uncertainty, sources, and SI units; and
- rejection of nonphysical parameters and mixed variant fields.

This is stronger than the old timed-region surrogate because pressure, flow,
resistance, compliance, state, evidence, and uncertainty are executable. It is
still a **reference candidate**, not a clinically validated lung model.

## 6. Remaining scientific work

M3 is not closed by this increment alone. The remaining work is:

1. explicitly retain the composite population calibration reference until a
   jointly measured alternative is selected;
2. retain the completed aggregate and published-population multipoint
   validation, and later extend it with participant-level, jointly measured
   supine data under a prospectively prespecified analysis;
3. add lobar/gravity-dependent or anatomical pulmonary structure and compare
   it with the aggregate right/left target;
4. extend the implemented bounded PVR/compliance flow adaptation with a
   separately calibrated age dimension, participant-level uncertainty,
   pressure-dependent recruitment, and reviewed right/left or lobar exercise
   redistribution;
5. qualify the SimVascular archive and combine its arterial geometry with this
   downstream 0D closure, if its data rights and state are resolved; and
6. implement or formally defer the neutral historical FP9 timing/event
   baseline at the scenario layer before rerunning the M3 gate review.
