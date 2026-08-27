<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Simulation Context and Component Lifecycle

`SimulationContext` groups the state that belongs to exactly one MEHLISSA run:

- monotonic `SimulationClock`;
- immutable master seed;
- persistent, named `RandomStream` instances.

The context is neither global nor copyable. Within a run, the same stream name
returns the same continuing RNG instance. A new context with the same master
seed reproduces the sequence.

## Host state machine

```text
building --initialize()--> initialized --finalize()--> finalized
    |                           |
    | initialization failure    | destructor
    +---------------------------+----------------------> finalized
```

- Components may be registered only in `building`.
- Names are unique within a host.
- `initialize` runs forward; rollback finalizes already successful components
  in reverse order.
- `advance(delta)` is permitted only in `initialized`. All components see the
  old clock value; only overall success commits the prevalidated new value.
- `finalize` runs in reverse order, is repeatable without duplicate effects,
  and is also called by the destructor.

## Ownership rules

The host owns each component exclusively. Components receive the context only
for the duration of a callback and must not infer ownership from it. Component
dependencies will be modeled through explicit exchange objects or services,
not mutual `shared_ptr` references.

`finalize` is guaranteed not to throw. Persistent output must therefore be
completed beforehand or handled by a separate, error-reporting flush step.
M1.5 adds structured errors and the checkpoint contract.

## Verification

`simulation_context_tests.cpp` verifies seed and stream isolation.
`component_host_tests.cpp` verifies state transitions, ordering, rollback on
failure, absence of clock progress after failure, and exactly-once finalization.
