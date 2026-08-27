<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0011: Explicit Simulation Context and Component Lifecycle

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M1/M2–M6; `SYS-002`, `SYS-003`, `SYS-005`, `SYS-007`

## Context

The historical implementations tightly couple the global clock, randomness,
model state, scenario logic, and output. Global or static state impedes parallel
experiments, reproducible tests, and unambiguous object lifetimes. Later body,
organ, cell, and communication components need a shared run context without
requiring the kernel to know medical scenario classes.

Initialization failures and early termination must not leave components partly
registered or finalized more than once. A failed component update must also not
advance the shared clock erroneously.

## Decision

1. Every run has exactly one non-copyable and non-movable `SimulationContext`.
2. The context owns the monotonic simulation clock, master seed, and persistent random streams created by name. This state is not global.
3. Components implement `SimulationComponent` with a stable name and the phases `initialize`, `advance`, and `finalize`.
4. `ComponentHost` owns components exclusively through `std::unique_ptr`. Null pointers, empty names, duplicate names, and late registration are rejected.
5. Initialization follows registration order. On failure, only previously successful components are finalized in reverse order.
6. Normal finalization also proceeds in reverse order and exactly once. `finalize` is `noexcept` so that the host destructor can safely fulfill the contract.
7. During `advance`, all components see the same interval start and explicit step size. The shared clock is committed only after all components succeed.
8. The host is a lifecycle mechanism, not yet a medical `ModelComponent` or event scheduler. Domain layer contracts are defined on this neutral foundation in M2 through M6.

## Consequences

Positive:

- Multiple experiments can have independent clocks and random states.
- Ownership and finalization order are unambiguous and verified by tests.
- Failed initialization does not leave initialized components unfinished.
- A failed update does not advance shared time.
- The kernel remains free of medical scenario types.

Negative:

- Components may use the supplied context only as a non-owning reference; the type cannot completely prevent storing a raw pointer.
- Domain-state changes already made by earlier components are not yet rolled back when `advance` fails. Checkpoints and transactional model boundaries follow in M1.5 and later layers.
- Finalization must neither throw nor perform work whose failure could only be reported by an exception.
- Order-dependent couplings later require an explicit scheduler or exchange contract instead of implicit registration order.

## Alternatives

- **Global singletons:** rejected because tests and parallel runs would share state.
- **Shared component ownership:** rejected because cycles and unclear destruction times could result.
- **Finalization only on normal success:** rejected because error paths would leave resources and incomplete state behind.
- **Update clock before components:** rejected because a component failure would then expose simulation progress that never completed.
- **Complete event scheduler in M1.4:** deferred until M2 provides concrete transport and exchange events.
