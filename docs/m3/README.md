<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 – Body–Organ Coupling

**Status:** technical coupling, literature-parameterized flow-, age-, and
invasive-young-resistance-conditioned 0D references, independent aggregate and
published-population multipoint validation, and subject-level analysis path complete;
anatomical/FP9 scientific gate open

**Started:** 27 August 2026

M3 creates the first real boundary between the validated whole-body transport
and an independently replaceable regional model. The reference organ is the
lung and the initial scope is pulmonary circulation—not ventilation,
respiratory mechanics, or gas exchange.

## Planned increments

| Increment | Status | Exit evidence |
|---|---|---|
| M3.1 model-component and entity-transfer contract | verified | versioned exchange object, named ports, strict temporal/route/identity validation, coarse `LungCompartment`, and deterministic contract tests |
| M3.2 body–lung–body entity round trip | verified | body transport hand-off, explicit orchestrator ownership, complete circulation test, no entity loss or duplication |
| M3.3 conservative population and substance exchange | verified for lossless transit | dimension-safe population, amount, and volume-flow transfers; both lung endpoints; positive and negative balance tests |
| M3.4 structured pulmonary-circulation variant | software-verified serial regional surrogate; anatomical refinement open | second implementation behind the same contract with artery, regional capillary surrogate, and venous return |
| M3.5 physiological parameters and external-data pipeline | definition schema, loader, qualification gate, and first sourced resting 0D parameter set verified; external anatomy and exercise data open | versioned model cards, rest/exercise parameters, uncertainty, reproducible axes/units/provenance, qualified SimVascular/VMR reference |
| M3.6 orchestration and gate regression | external lung definition and fixed-step matrix verified; FP9 baseline open | deterministic synchronization, coarse/detailed scenario switch, historical FP9 timing baseline, formal M3 gate review |
| M3.7 healthy-adult resting pulmonary 0D reference | implemented and software-verified; independent validation/anatomical refinement open | source-scoped pressure, flow, resistance, compliance, transit, right/left perfusion, uncertainty, analytical RC dynamics, and lossless coupling |
| M3.8 independent pulmonary 0D validation | qualified aggregate pass; subject-level/anatomical validation open | executable source-separation guard; supine healthy-rest comparison; invasive rest/exercise crosscheck; 6/6 required endpoints pass; exercise RC limitation exposed |
| M3.9 bounded rest-to-exercise 0D adaptation | implemented, independently stress-tested; subject-level/anatomical refinement open | immutable v1 baseline; source-disjoint Claessen calibration; bounded effective PVR/compliance; Bentley 6/6 required pass; exercise RC z reduced from 18.571 to 3.005 but remains diagnostic fail |
| M3.10 subject-level multipoint validation path | software-verified; measured data access pending | pseudonymous three-stage schema; immutable-model mPAP/PAWP pressure-flow fits; stage PVR/compliance/RC diagnostics; synthetic-evidence guard; UA iCPET request draft |
| M3.11 published-population multipoint validation | qualified partial result; 10/18 stages agree | independent Kovacs and Wolsk series; 255 healthy volunteers; exact mean/SD and mean/95% CI semantics; age-stratified failure pattern exposed without refitting |
| M3.12 independent age conditioning | qualified improvement; 14/18 stages agree | separate Kane calibration; bounded 18–85-year three-band PVR multiplier; immutable Kovacs/Wolsk validation; older stratum improves 2/5 to 5/5 while young-stratum limitation remains |
| M3.13 invasive young-adult resistance qualification | qualified Wolsk population pass; participant-level/anatomical validation open | invasive Kovacs 2012 young-PVR calibration; retained independently supported flow exponent; overlap-aware exclusion of Kovacs 2009; 15/15 disjoint Wolsk stages agree without refitting |

## M3.1 result

The first increment establishes three dependency layers:

```text
core (medical-neutral lifecycle and time)
  └─ models/coupling (versioned cross-model contracts)
       └─ models/organ (lung implementations)
```

`EntityTransfer` carries stable identity, type, complete source and target
route, and simulation time. `ModelComponent` defines how an orchestrator hands
entities to a named entry and collects outbound transfers. The coarse lung
component owns accepted entities until their configured transit time is
reached, then emits the same identities through a configured venous return.

Contract semantics are documented in [Entity Exchange Contract](ENTITY_EXCHANGE.md).
The scientific scope and limitations of the first model are documented in the
[Lung Compartment Model Card](LUNG_COMPARTMENT.md). The binding architecture
decision is [ADR-0020](../architecture/adr/0020-model-component-and-entity-exchange.md).

## M3.2 result

`BodyOrganCoupler` now performs the first complete ownership round trip. The
body transport explicitly places an ID in its outside-body ledger, the organ
owns it during transit, and the coupler returns it only through the configured
venous route and at the declared synchronization time. Failed organ acceptance
restores the body state. Outbound transfers remain owned by the coupler until
their complete route and time have been validated.

The executable regression starts entity 1 in synthetic body segment
`artery-10`, transfers it through `LungCompartment`, and returns the same ID to
`vein-90`. Details are in [Body–Organ Round Trip](BODY_ORGAN_ROUND_TRIP.md).

## What the current implementation deliberately does not claim

- Conserved transfers currently provide lossless transit; they do not yet model
  gas exchange, reaction, production, consumption, or barrier transport.
- `LungCompartment` is a software-verified surrogate, not a physiologically
  calibrated or independently validated pulmonary model.
- The serial-region implementation remains structural. The new 0D
  implementation has independent aggregate validation, but remains a
  composite-population reference candidate rather than a patient model.
- The CLI and experiment manifest do not yet compose model components.

These are milestone tasks, not hidden assumptions. The M3 gate remains open.

## Conserved-transfer result

M3.3 provides typed population, substance-amount, and volume-flow transfers,
an exact balance ledger, and lossless endpoints in both lung variants. The same
generated regression verifies unchanged payloads and transfer IDs after organ
transit. See [Conserved Transfers](CONSERVED_TRANSFERS.md).

M3.4 also provides `PulmonaryCirculation`, a second `ModelComponent` that
separates pulmonary artery, regional capillary surrogate, and pulmonary vein.
It passes the same port, identity, timing, and return contract as
`LungCompartment`; see its [Model Card](PULMONARY_CIRCULATION.md). It is more
structural, but not yet an anatomical 0D/1D or imported vascular model.

The `make_lung_model` composition boundary now selects any implementation
from one typed scenario configuration. The same body–lung–body regression runs
against both variants without coupler branches; see
[Lung Model Selection](LUNG_MODEL_SELECTION.md). External experiment-manifest
composition remains open.

The complete body–organ regression loads the standalone surrogate definitions
at 0.5 s and 1.0 s host steps and the pulmonary 0D card at compatible 0.1 s and
0.2 s steps. Entity and conserved-quantity contracts agree across variants; see
[Orchestration Regression](ORCHESTRATION_REGRESSION.md). General experiment-
manifest and asynchronous multirate scheduling remain separate extensions.

## Next implementation step

M3.5 now has a schema-validated executable model-card format and an explicit
external-data qualification gate; see
[Versioned Lung Model Definitions](LUNG_MODEL_DEFINITIONS.md) and the
[Qualification Checklist](EXTERNAL_PULMONARY_DATA_QUALIFICATION.md). The next
scientific step is to review and parameterize an actual pulmonary reference.
No external data set or physiological parameter values have been silently
introduced.

The formal [M3 Gate Review](M3_GATE_REVIEW.md) records the technical evidence
and the scientific closure package. M3 remains open until that package and the
historical FP9 executable reference are complete.

M3.7 adds a [healthy-adult resting pulmonary 0D reference candidate](PULMONARY_0D_REFERENCE.md)
with executable pressure, resistance, compliance, flow partition, transit,
uncertainty, and evidence roles. It supplies a physiological downstream
closure independent of SimVascular and leaves every calibration choice and
limitation visible in the versioned model definition.

M3.8 adds an executable
[independent aggregate validation](PULMONARY_0D_INDEPENDENT_VALIDATION.md).
All six required pressure, compliance, and resting-RC endpoints pass the
locked one-standard-deviation criterion. The diagnostic exercise RC endpoint
fails and therefore defines the next physiological model change rather than
being hidden by refitting.

M3.9 adds a
[bounded flow-adaptive v2 candidate](PULMONARY_0D_FLOW_ADAPTATION.md). The
adaptation is calibrated from the independent Claessen healthy-control cohort;
Wright et al. was rejected because Bentley reused that Toronto cohort. Bentley
remains untouched as the post-calibration stress test. Pressure and compliance
remain required-pass endpoints, while exercise RC improves materially but
still fails its deliberately narrow diagnostic criterion.

M3.10 adds the
[subject-level multipoint validation path](PULMONARY_0D_SUBJECT_MULTIPOINT_VALIDATION.md).
It computes observed and predicted pressure-flow trajectories, per-stage PVR,
optional compliance/RC, mean pressure error, and RMSE without refitting v2.
Synthetic test data are rejected by the default evidence loader. The preferred
UA iCPET healthy-control records require an approved data request, so this
increment verifies the method but does not claim measured subject-level
validation. A [send-ready request draft](UA_ICPET_DATA_REQUEST.md) records the
minimal pseudonymized fields and reuse questions.

M3.11 adds the immediately accessible
[published-population multipoint validation](PULMONARY_0D_POPULATION_MULTIPOINT_VALIDATION.md).
The locked v2 model is evaluated against four independent series spanning 255
healthy volunteers and 18 stages. Kovacs passes 3/3 stages and the Wolsk
40–59-year stratum passes 5/5, while the younger and older strata expose a
systematic missing age dimension. Overall agreement is 10/18 stages under the
predeclared rules. This is a reproducible partial result, not a relabelled pass.
Subject-level data remain a valuable higher-resolution follow-up but no longer
block use of published population evidence.

M3.12 adds the
[age-conditioned v3 candidate](PULMONARY_0D_AGE_CONDITIONING.md). Relative
resting PASP/CO age effects from the separate Kane cohort modify PVR only;
Kovacs and Wolsk remain validation-only. Without refitting, agreement improves
from 10/18 to 14/18 stages. All five older-stratum stages now agree and the
middle stratum remains 5/5, while only one of five young-stratum stages agrees.
The residual young high-flow discrepancy remains an explicit open scientific
question rather than a reason to tune against the validation data.

M3.13 resolves that question with the
[invasive young-adult resistance candidate](PULMONARY_0D_YOUNG_RESISTANCE.md).
Kovacs et al. (2012) shows that the retained flow exponent is already
appropriate but that the young resting PVR level is too high. The immutable v4
candidate therefore changes only that level. All 15 stages in the disjoint
Wolsk age-stratified cohort agree; the overlapping Kovacs 2009 literature
summary is deliberately excluded from the counted v4 result.

An initial [SimVascular Healthy Pulmonary Candidate Review](SIMVASCULAR_PULMONARY_CANDIDATE_REVIEW.md)
finds the official case useful for an imported pulmonary-artery technical
variant, but insufficient as a complete or normative lung reference. Data
access, artifact licensing, state ambiguity, units, venous return, uncertainty,
and independent validation remain explicit qualification blockers.
