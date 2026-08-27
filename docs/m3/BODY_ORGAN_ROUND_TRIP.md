<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Body–Organ Entity Round Trip

## Purpose

M3.2 turns the M3.1 boundary contract into genuine co-simulation. It proves
that one stable entity can leave the body graph, be owned by an independent
organ component, and return without duplication or identity loss.

## Ownership sequence

1. `CompartmentTransport::handoff_particle` verifies the ID and departure
   segment, removes the particle from active body transport, and records it in
   the outside-body ledger.
2. `BodyOrganCoupler` creates a version-1 entity transfer and gives it to the
   configured organ entry port. If acceptance fails, it restores the particle
   to the departure segment.
3. The organ owns the entity while resident and until its outbound transfer is
   collected.
4. The coupler owns collected outbound transfers while validating source,
   exit, target, return port, identity ledger, and synchronization time.
5. The body accepts the original ID at the configured return segment and
   removes it from the outside-body ledger.

The body invariant is therefore:

```text
active in body + outside body + extracted = injected
```

The coupler additionally requires every outside entity it sent to appear in
its in-flight set until the return is complete.

## Executable reference

The contract test uses the synthetic four-segment graph and a two-second lung
transit:

```text
body artery-10
  → pulmonary-arterial-departure
  → lung pulmonary-arterial-entry
  → fixed transit
  → lung pulmonary-venous-exit
  → pulmonary-venous-return
  → body vein-90
```

The returned particle has ID 1, the active body count returns to one, and all
outside/resident/in-flight queues return to zero.

## Remaining M3 work

This regression conserves individual identity and count. It does not yet
transfer an aggregated population, volume flow, or substance amount. M3.3 adds
those typed contracts and their dimension-safe balance tests. Scenario/CLI
composition and a second detailed pulmonary implementation also remain open.
