<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0039: Conservative nanodevice release and uptake

- Status: Accepted
- Date: 2026-08-31

## Context

M5.5 produces an internal effector-threshold event, but it deliberately has no
actuation or drug semantics. M5.6 must turn a decision into nanodevice
activation, release payload into an extracellular compartment, and transfer
drug into a cellular compartment without losing or duplicating substance.
Embedding those steps in the signaling solver would couple detection,
actuation, and delivery kinetics and obscure ownership.

## Decision

Introduce a versioned activation signal derived from a consistent M5.5 ODE or
SSA threshold response. The signal carries stable activation, device, payload,
source-request, source-network, and time identities. Absence of a threshold
produces no signal and leaves the payload sealed.

Implement release and uptake as a separate irreversible analytical chain:

```text
device payload --k_release--> extracellular drug --k_uptake--> intracellular drug
```

Every amount has exactly one owner. The initial device amount equals the sum of
device, extracellular, and intracellular amounts at every observation time.
The unequal-rate solution is evaluated in closed form; equal rates use their
analytical limiting expression instead of an unstable division.

Bind identities, SI amounts, rates, a checked reference, validity, sources, and
limitations in one strict versioned profile. The reference is a synthetic
software-verification case.

## Consequences

- Detection and delivery remain independently replaceable.
- A false or absent response cannot release payload.
- Device, extracellular, and intracellular ownership is explicit and exactly
  auditable through an amount balance.
- M5.7 can consume intracellular inventory without modifying delivery kinetics.
- The model does not yet include spatial diffusion, binding, saturation,
  metabolism, elimination, therapeutic response, or biological calibration.

## Alternatives considered

- Releasing directly inside the M5.5 network was rejected because it merges a
  detector with an actuator and prevents independent verification.
- A fixed released fraction was rejected because it provides no time-dependent
  uptake experiment or kinetic reference.
- A numerical ODE was deferred because this linear chain has an exact solution
  that is a stronger first software baseline.

## Affected requirements and gates

This implements Roadmap increment M5.6 and advances `CELL-003` from specification
to a partial executable contract. `CELL-003` remains open because spatial
diffusion and receptor binding are not yet connected end to end. The measurable
higher-layer response and apoptosis parts of Gate M5 remain for M5.7.
