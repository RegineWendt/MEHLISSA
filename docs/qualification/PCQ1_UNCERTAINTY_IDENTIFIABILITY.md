<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1.5 uncertainty, sensitivity, convergence, and identifiability

## 1. Outcome and claim boundary

PCQ-1.5 completes the outcome-blind uncertainty design required before the
locked pulmonary and capillary evaluation. Its machine authority is
`data/qualification/pulmonary-capillary-uncertainty-plan-v1.json`; the schema
and executable checker bind it to the PCQ-1.4 ingress policy and to the exact
pulmonary and capillary model files.

The increment covers all nine frozen endpoints and all six uncertainty classes:

1. observational uncertainty and covariance;
2. parameter ranges, distributions, and correlations;
3. structural spread;
4. numerical error and convergence;
5. sensitivity; and
6. identifiability.

No candidate participant-level outcome was requested, received, opened, or
inspected. The calculations below use accepted model definitions, method
uncertainty floors, and an artificial flow-age grid. They are design and
software evidence, not physiological validation, parameter calibration, or a
qualification decision.

## 2. What is now operational

| Class | Executable result | Remaining limitation and consequence |
|---|---|---|
| observational | endpoint-specific floors; PVR and `Vc/Q` covariance envelopes; log-scale `Vc`; compositional lobe rule | source-native covariance is not available; measured inference must show the full correlation envelope or remain inconclusive |
| parameter | every influential lung, capillary, and observation input has an owner, unit, evidence representation, distribution state, correlation state, and endpoint mapping | most inputs provide intervals or point estimates, not joint probability distributions; probabilistic output intervals are not yet justified |
| structural | seven immutable pulmonary definitions are evaluated on the same three-flow, three-age grid; v4 and v7 remain one aggregate-equivalence group | this is an unweighted hypothesis set, not a probability distribution over structures |
| numerical | equilibrium, PVR, RC time, volume-flow residence, and propagation are closed-form; finite-difference sensitivities converge over three step sizes | cluster-bootstrap replicate convergence awaits an authorized decision-size cohort |
| sensitivity | seven signed local elasticities are evaluated with a fixed design point and perturbation-convergence gate | global variance-based indices remain blocked until joint distributions and correlations are evidence-backed |
| identifiability | nine design-matrix rank analyses expose distinguishable and confounded parameter combinations before calibration | structural/local rank does not guarantee practical identifiability in a small noisy cohort |

`Unavailable` and `blocked` are deliberate scientific results. The checker
forbids replacing missing distributions or covariance by an undocumented
normality or independence assumption.

## 3. Observational uncertainty by endpoint

- **PCQ-H1:** each mPAP value uses the larger of its supplied standard
  uncertainty and the locked 2 mmHg floor.
- **PCQ-H2:** PVR is `(mPAP - PAWP) / Q`. First-order propagation uses at
  least 2 mmHg for mPAP, 3 mmHg for PAWP, and 10% for flow. If the joint
  covariance is absent, the report retains the attainable all-input
  correlation envelope. For the arbitrary 15/8 mmHg and 5.5 L/min fixture,
  that envelope is approximately 0.055-1.036 WU; a single independence-based
  number is intentionally not substituted.
- **PCQ-H3:** at least three complete stages and a 2 L/min flow span remain
  mandatory. Inferential slope precision requires the within-person stage
  covariance or a declared covariance envelope.
- **PCQ-H4:** RC analysis requires a complete, stage-matched systolic PAP,
  diastolic PAP, heart-rate, resistance, and compliance tuple. Missing H4 data
  do not remove otherwise eligible H1-H3 observations.
- **PCQ-R1:** five lobe fractions live on a simplex. Treating five errors as
  independent is invalid because the fractions sum to one. Without source
  covariance, only the locked point-error gates and bounded compositional
  perturbations are decision-supporting.
- **PCQ-C1:** positive functional capillary volume is represented on the log
  scale. The 8% method CV corresponds to a log standard uncertainty of about
  0.07987. A source-supplied larger value takes precedence.
- **PCQ-C2:** no interval is calculated. Whole-pulmonary transit remains
  blocked until independent pre-capillary, post-capillary, and method-mixing
  delay terms exist.
- **PCQ-C3:** hematocrit or hemoglobin uncertainty is carried exactly once.
  An undocumented second correction is rejected.
- **PCQ-J1:** same-stage `Vc/Q` uncertainty requires their covariance. With
  artificial relative uncertainties of 8% and 10%, the full correlation
  envelope is 2%-18%. The algebraic closure is exact but supplies no new
  independent physiological observation.

## 4. Structural ensemble

The design evaluates the fixed-resistance v1, flow-adaptive v2, two age-
conditioned empirical variants v3/v4, two pressure-distensibility variants
v5/v6, and the v7 lobar decomposition. The grid contains flows of 6, 10, and
14 L/min at ages 30, 50, and 70 years. It is fixed independently of new
outcomes.

| Age | Flow | Minimum predicted mPAP | Maximum predicted mPAP | Structural spread |
|---:|---:|---:|---:|---:|
| 30 years | 6 L/min | 13.175 mmHg | 15.200 mmHg | 2.025 mmHg |
| 30 years | 10 L/min | 15.779 mmHg | 20.000 mmHg | 4.221 mmHg |
| 30 years | 14 L/min | 18.174 mmHg | 24.800 mmHg | 6.626 mmHg |
| 50 years | 6 L/min | 15.200 mmHg | 15.200 mmHg | 0.000 mmHg |
| 50 years | 10 L/min | 18.663 mmHg | 20.000 mmHg | 1.337 mmHg |
| 50 years | 14 L/min | 21.542 mmHg | 24.800 mmHg | 3.258 mmHg |
| 70 years | 6 L/min | 15.200 mmHg | 16.082 mmHg | 0.882 mmHg |
| 70 years | 10 L/min | 18.823 mmHg | 20.148 mmHg | 1.325 mmHg |
| 70 years | 14 L/min | 22.155 mmHg | 24.800 mmHg | 2.645 mmHg |

The spread is not an uncertainty interval and the extrema are not confidence
bounds. It demonstrates that structural choice becomes material away from the
common reference state. The checker also proves that v4 and v7 are identical
for aggregate mPAP, PVR, compliance, and RC time at every grid point, so the
regional v7 decomposition cannot be double-counted as an independent
hemodynamic hypothesis.

## 5. Local sensitivity screen

Central relative perturbations of 1%, 0.1%, and 0.01% are used only on the
frozen equations. All seven estimates converge within the predeclared 0.2%
relative tolerance.

| Input | Output | Finest signed elasticity | Interpretation |
|---|---|---:|---|
| pulmonary vascular resistance | mPAP | +0.493 | higher resistance raises pressure, while fixed PAWP reduces the normalized elasticity below one |
| pulmonary vascular resistance | PVR | +1.000 | direct proportionality |
| flow-resistance exponent magnitude | PVR | -0.103 | making the negative exponent more negative lowers high-flow PVR |
| pulmonary arterial compliance | RC time | +1.000 | direct proportionality |
| flow-compliance exponent magnitude | RC time | -0.342 | stronger negative adaptation shortens high-flow RC time |
| functional capillary volume | capillary residence | +1.000 | `tau = Vc/Q` |
| cardiac output | capillary residence | -1.000 | `tau = Vc/Q` |

These elasticities show local direction and scale at the artificial design
point. They are not Sobol indices, population effects, or evidence-derived
parameter rankings. Global variance decomposition remains blocked because a
plausible marginal range is not a substitute for a joint distribution.

## 6. Identifiability findings

| Analysis | Executable rank | Finding | Required action |
|---|---:|---|---|
| one-stage resistance/exponent | 1 of 2 | non-identifiable | never estimate both from one flow stage |
| three-stage resistance/exponent | 2 of 2 | locally identifiable under the design | retain the three-stage and flow-span rules; still assess practical precision |
| equilibrium R/C model without H4 | 2 of 4 | non-identifiable | equilibrium H1-H3 cannot identify compliance or its flow exponent |
| multipoint R/C model with H4 RC information | 4 of 4 | locally identifiable under the design | require complete independent H4 information |
| resistance plus one age-band multiplier | 1 of 2 | confounded | keep both independently fixed rather than fitting the validation cohort |
| five normalized lobe fractions | 4 of 5 | compositional rank four | report four degrees of freedom; normalized fractions do not identify total flow |
| `N*pi*(d/2)^2*L` capillary geometry | 1 of 3 | non-identifiable | retain path count as a numerical choice and do not relabel derived length as anatomy |
| four whole-pulmonary delay terms | 1 of 4 | non-identifiable and blocked | obtain independent delay evidence before PCQ-C2 activation |
| `Vc`, flow, and derived residence | 2 of 3 | diagnostic, no new information | use J1 for coherence only, not independent validation |

The rank tests are performed before any calibration. Even a full local rank
does not guarantee a narrow estimate in the available sample; profile or
posterior practical-identifiability diagnostics would belong to a future
calibration set, never the locked validation set.

## 7. Reproduction and controls

Install the repository's `publication` optional dependency and run:

```powershell
python scripts/check_pulmonary_capillary_uncertainty.py
python -m unittest tests.test_pulmonary_capillary_uncertainty -v
```

The negative tests reject changed parent or model hashes, altered rank claims,
endpoint gaps, changed structural-equivalence semantics, non-convergent local
sensitivities, a global analysis without distributions, and activation of the
blocked transit endpoint. The command prints counts and readiness states only.

## 8. What PCQ-1.5 does and does not establish

PCQ-1.5 establishes that the eventual no-refit analysis has an explicit and
testable treatment of uncertainty, structural alternatives, sensitivity, and
identifiability. It prevents common overclaims such as reporting one fitted
solution for a rank-deficient problem or presenting a local range as a global
probability distribution.

It does not add participant evidence, validate a physiological prediction,
select a winning lung model, enable whole-pulmonary transit, authorize data
processing, or make MEHLISSA clinically usable. PCQ-1.6 is next: after rights-
authorized, source-disjoint data become available, execute the already frozen
endpoint analyses without refitting and retain all eligible, partial, blocked,
and failed results.
