<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Entity Exchange Contract 1.0.0

## Purpose

The contract transfers ownership of one stable entity between independent
model components. It is a runtime C++ contract in M3.1. A versioned serialized
form will be added when checkpoint and external-adapter requirements are
defined for coupled models.

## Fields

| Field | Meaning | Validation |
|---|---|---|
| `contract_version` | exchange semantics | exactly `1.0.0` |
| `entity_id` | identity across all participating layers | non-zero and not already held by the receiver |
| `entity_type` | stable scenario/domain type | non-empty |
| `source_model_id` | component giving up ownership | non-empty |
| `source_port_id` | named exit boundary | non-empty |
| `target_model_id` | intended receiver | non-empty and equal to receiver model ID |
| `target_port_id` | intended entry boundary | non-empty and accepted by receiver |
| `emitted_at` | synchronization time in nanoseconds | non-negative and equal to receiver synchronization time in M3.1 |

## Ownership and conservation

After successful acceptance, exactly one receiver owns the entity. The sender
must remove it from its active population. A rejected transfer changes neither
side. The receiver retains the ID while it is resident and while an outbound
transfer is waiting to be collected, preventing duplicate re-entry.

M3.2 introduces an orchestrator ledger that makes the sender/receiver update
atomic at the co-simulation level and verifies:

```text
body active + organ resident + queued transfers + extracted = total injected
```

M3.1 verifies the receiver half of this rule but does not yet claim a complete
body–organ conservation proof.

## Time semantics

Transfers occur only at synchronization points. The first lung surrogate emits
at the first synchronization boundary at or after its fixed transit time.
Therefore a step that does not divide the transit time exactly may introduce a
bounded delay smaller than one synchronization interval. The future multirate
orchestrator will make this quantization and interpolation policy explicit for
each model pair.

## Versioning

Breaking semantic or field changes require a new contract version. Receivers
reject unsupported versions. Population, flow, physiological state, and event
exchanges are separate contracts rather than extensions with unrelated
optional fields.
