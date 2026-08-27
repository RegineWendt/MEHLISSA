<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 – Body–Organ Coupling

**Status:** technical coupling candidate complete; scientific gate open

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
| M3.5 physiological parameters and external-data pipeline | definition schema, loader, and qualification gate verified; sourced parameters/data open | versioned model cards, rest/exercise parameters, uncertainty, reproducible axes/units/provenance, qualified SimVascular/VMR reference |
| M3.6 orchestration and gate regression | external lung definition and fixed-step matrix verified; FP9 baseline open | deterministic synchronization, coarse/detailed scenario switch, historical FP9 timing baseline, formal M3 gate review |

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
- The second implementation is structurally more detailed, but is not yet a
  sourced anatomical or physiological pulmonary model.
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

The `make_lung_model` composition boundary now selects either implementation
from one typed scenario configuration. The same body–lung–body regression runs
against both variants without coupler branches; see
[Lung Model Selection](LUNG_MODEL_SELECTION.md). External experiment-manifest
composition remains open.

The complete body–organ regression now loads those standalone definitions and
runs both resolutions at 0.5 s and 1.0 s host steps. Entity and conserved-
quantity results agree across all four combinations; see
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
