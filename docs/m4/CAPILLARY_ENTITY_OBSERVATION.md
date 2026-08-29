<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Capillary Nanodevice Residence and Interaction Observations

## Purpose

M4.6 makes a nanodevice's local capillary state observable without changing
its ownership or route. It answers three software-level questions:

1. Where is an entity inside the arteriole-capillary-venule path now?
2. How much simulation time did it actually spend in each region?
3. Given an explicit interaction-rate profile, what are the competing
   pass-through, retention, adhesion, and extravasation likelihoods?

This is deliberately an observation layer. A reported retention or
extravasation likelihood is not an executed event and cannot remove an entity
from circulation.

## Versioned input

The strict profile contract is:

```text
data/schemas/capillary-entity-observation-profile/1.0.0.schema.json
```

The executable software-test fixture is:

```text
examples/capillary-models/synthetic-nanodevice-observation-v1.json
```

Every profile identifies one compatible capillary model, declares a bounded
record buffer, gives evidence and limitations, and supplies one unique rule
per supported entity type. An unmatched entity follows the only currently
supported policy, `observe_residence_only`: its regional residence is reported
and its pass-through likelihood is one.

## Local position

`CapillaryBed::entity_positions()` returns a current snapshot for every
resident entity:

- stable entity ID and type;
- current region ID and kind;
- axial distance from the start of that region;
- dimensionless axial fraction from zero to one; and
- total residence accumulated across all regions so far.

The position is derived from the same distance state that drives transport.
Recruitment therefore changes future velocity without resetting position.

## Exact regional residence

The runtime accounts for the amount of each integration interval actually
consumed in the active region. When an entity crosses one or more boundaries
inside a host step, only the sub-interval spent in each region is added. When a
scheduled recruitment event splits a host step, residence before and after the
event is integrated with the corresponding velocity.

The synthetic fixed-flow reference route records:

| Region | Residence |
|---|---:|
| feeding arteriole | 0.2 s |
| exchange bed | 0.6 s |
| draining venule | 0.2 s |
| total | 1.0 s |

The completion record is reported at the host synchronization boundary. Its
regional residence values describe integrated model time; the reporting time
is not presented as a sub-step biological event timestamp.

## Competing interaction likelihoods

For one matched entity type, let capillary residence be `t` seconds and the
configured constant rates be `r_ret`, `r_adh`, and `r_ext`, all in `s^-1`.
With `R = r_ret + r_adh + r_ext`, M4.6 calculates:

```text
P_pass = exp(-R t)
P_ret  = (r_ret / R) * (1 - P_pass)
P_adh  = (r_adh / R) * (1 - P_pass)
P_ext  = (r_ext / R) * (1 - P_pass)
```

All rates must be finite and non-negative, and `R` must be positive. The four
likelihoods are required to be finite, bounded, and normalized to one.

For the synthetic rule `0.1`, `0.2`, and `0.3 s^-1` and a 0.6 s capillary
residence, `P_pass = exp(-0.36)`. The remaining interaction likelihood is
distributed in the ratio 1:2:3. These values were chosen for transparent
verification and have no physiological interpretation.

## Ownership and buffering

Observation never mutates the entity transfer. ID, type, target organ, return
port, and completion behavior remain identical to the lossless baseline. This
preserves the organ-capillary ownership ledger.

Completed observations are drainable. The profile limits the number of
records buffered at one time; excess records increment a cumulative drop
counter rather than causing unbounded memory growth. Current position
snapshots are calculated on request and are not retained as trajectories.

## Verification

Automated tests cover:

- strict schema loading, evidence, and limitation metadata;
- local axial position after a partial step;
- exact arteriole, capillary, venule, and total residence;
- normalized competing likelihoods;
- unchanged entity identity and organ-return route;
- residence-only handling of an unmatched type;
- bounded buffering and its drop counter; and
- rejection of incompatible models, duplicate entity types, negative rates,
  and zero combined rate.

## Scientific boundary and next work

M4.6 verifies observable state and a likelihood contract. It does not contain
physiological nanodevice rates, surface chemistry, margination, hematocrit,
wall shear, receptor kinetics, endothelial state, spatially varying hazards,
or uncertainty distributions. It also does not sample an outcome.

A later state-changing increment must define who owns a retained or
extravasated entity, how tissue entry is represented, whether adhesion is
reversible, and how an organ or cell model acknowledges termination. Such an
increment can use this observation record as a reference without silently
breaking conservation.

See [ADR-0026](../architecture/adr/0026-non-state-changing-capillary-entity-observation.md)
for the binding decision.
