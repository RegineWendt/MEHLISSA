<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 – Body–Organ Coupling

**Status:** in progress

**Started:** 27 August 2026

M3 creates the first real boundary between the validated whole-body transport
and an independently replaceable regional model. The reference organ is the
lung and the initial scope is pulmonary circulation—not ventilation,
respiratory mechanics, or gas exchange.

## Planned increments

| Increment | Status | Exit evidence |
|---|---|---|
| M3.1 model-component and entity-transfer contract | implemented, under CI review | versioned exchange object, named ports, strict temporal/route/identity validation, coarse `LungCompartment`, and deterministic contract tests |
| M3.2 body–lung–body entity round trip | implemented, under CI review | body transport hand-off, explicit orchestrator ownership, complete circulation test, no entity loss or duplication |
| M3.3 conservative population and substance exchange | contract and ledger implemented; endpoint integration planned | dimension-safe population, amount, and volume-flow transfers; positive and negative balance tests |
| M3.4 detailed pulmonary-circulation variant | planned | second implementation behind the same scenario contract; artery, regional distribution, capillary surrogate, and venous return |
| M3.5 physiological parameters and external-data pipeline | research/planned | versioned model cards, rest/exercise parameters, uncertainty, reproducible axes/units/provenance, qualified SimVascular/VMR reference |
| M3.6 orchestration and gate regression | planned | deterministic synchronization, coarse/detailed scenario switch, historical FP9 timing baseline, formal M3 gate review |

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

- No population or substance amount crosses the boundary yet.
- `LungCompartment` is a software-verified surrogate, not a physiologically
  calibrated or independently validated pulmonary model.
- There is not yet a second detailed implementation proving scenario-level
  interchangeability.
- The CLI and experiment manifest do not yet compose model components.

These are milestone tasks, not hidden assumptions. The M3 gate remains open.

## Next implementation step

M3.3 now provides typed population, substance-amount, and volume-flow transfers
plus a conservative balance ledger. The next slice connects these contracts to
both lung variants. See [Conserved Transfers](CONSERVED_TRANSFERS.md).
