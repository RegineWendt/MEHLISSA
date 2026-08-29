<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Balanced Capillary Substance Exchange

## Purpose

M4.5 introduces an explicit, mass-conserving exchange boundary among blood,
endothelium, interstitium, and cell compartments. The lossless M4.1 control
path remains the default. A substance changes only when the capillary bed is
constructed with a compatible, versioned exchange profile.

The first executable profile is a transparent staged partition for software
verification. It establishes units, accounting, provenance, and component
semantics before diffusion, permeability, metabolism, or organ-specific
physiology are added.

## Versioned profile

The strict schema and example are:

```text
data/schemas/capillary-exchange-profile/1.0.0.schema.json
examples/capillary-models/synthetic-oxygen-exchange-v1.json
```

Every profile identifies one compatible capillary model and declares one or
more substance rules. A rule contains three dimensionless fractions:

1. blood to endothelium;
2. endothelium to interstitium; and
3. interstitium to cell.

Fractions are applied when the transfer crosses from the capillary region into
the venule. Unknown substances follow the required `pass_through` policy and
remain unchanged. The blood-to-endothelium fraction must be less than one so a
positive blood-side transfer can complete the existing organ round trip.

## Balance equations

For incoming amount `A` and fractions `f_BE`, `f_EI`, and `f_IC`, MEHLISSA
calculates:

```text
E_received = A * f_BE
B_out      = A - E_received

I_received = E_received * f_EI
E_stored   = E_received - I_received

C_stored   = I_received * f_IC
I_stored   = I_received - C_stored
```

The binding invariant is:

```text
A = B_out + E_stored + I_stored + C_stored
```

All amounts are kernel `Amount` quantities in SI moles. The runtime verifies
the balance within a narrow relative floating-point tolerance before changing
the outbound transfer. A violation is an invariant error, not an accepted
approximation.

The synthetic example partitions 2.5 mmol of oxygen as follows:

| Compartment | Amount |
|---|---:|
| outgoing blood | 1.500 mmol |
| endothelium inventory | 0.500 mmol |
| interstitium inventory | 0.375 mmol |
| cell inventory | 0.125 mmol |
| **accounted total** | **2.500 mmol** |

These fractions are chosen for readable arithmetic and have no physiological
meaning.

## Runtime contract

An exchange-capable `CapillaryBed`:

- preserves the transfer ID, substance ID, route, and synchronization
  semantics;
- replaces only the outbound blood-side substance amount;
- accumulates endothelium, interstitium, and cell tissue inventories by
  substance;
- emits one exchange record containing profile ID, reporting boundary, input,
  and all four accounted terms;
- leaves population and volume-flow transfers unchanged; and
- leaves unmatched substances unchanged and unrecorded.

Records are drainable observations, while tissue inventories persist and
accumulate across transfers. The reporting time is the host synchronization
boundary at which capillary-region completion is observed; it is not claimed
to be a sub-step molecular event time.

Recruitment and exchange profiles can be combined. Recruitment changes the
transport state; the M4.5 fractions remain prescribed and do not yet scale with
surface area, transit, concentration, or flow. That coupling requires a later
physiological exchange model rather than an undocumented implicit rule.

## Verification

Automated tests verify:

- strict schema loading and evidence metadata;
- exact four-compartment accounting for the synthetic fixture;
- unchanged pass-through for an unmatched substance;
- unchanged population and volume-flow payloads;
- accumulation of repeated tissue deposits;
- rejection of duplicate substances, invalid fractions, complete blood
  removal, and incompatible models; and
- a complete organ-capillary-organ transfer with reduced blood amount,
  balanced tissue inventory, and closed ownership ledger.

## Scientific boundary and next work

M4.5 verifies an exchange contract, not oxygen physiology. It has no
concentration field, compartment volume, diffusion time, permeability-surface
product, binding, consumption, reverse flux, metabolism, clearance, or
uncertainty model. Physiological use requires new evidence-scoped rules or a
different interchangeable exchange implementation.

M4.6 adds residence-sensitive retention, adhesion, and extravasation
likelihood observations without conflating those nanodevice outcomes with
substance mass exchange. It deliberately leaves entity state and ownership
unchanged; see [Capillary Nanodevice Residence and Interaction Observations](CAPILLARY_ENTITY_OBSERVATION.md).

See [ADR-0025](../architecture/adr/0025-balanced-capillary-exchange.md) for the
binding decision.
