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
| M3.2 body–lung–body entity round trip | planned | body transport adapter, explicit ownership hand-off, complete circulation test, no entity loss or duplication |
| M3.3 conservative population and substance exchange | planned | dimension-safe typed contracts, balance ledger, positive and negative conservation tests |
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

## What M3.1 deliberately does not claim

- The existing body transport does not yet transfer an entity into the lung;
  that adapter is M3.2.
- No population or substance amount crosses the boundary yet.
- `LungCompartment` is a software-verified surrogate, not a physiologically
  calibrated or independently validated pulmonary model.
- There is not yet a second detailed implementation proving scenario-level
  interchangeability.
- The CLI and experiment manifest do not yet compose model components.

These are milestone tasks, not hidden assumptions. The M3 gate remains open.

## Next implementation step

M3.2 adds two explicit exchange sites to `CompartmentTransport`: a pulmonary
arterial departure and a pulmonary venous return. A coupling orchestrator then
executes body → lung → body ownership hand-off at synchronization points and
verifies the complete identity ledger. This is the smallest next step that
turns the architectural boundary into genuine co-simulation.
