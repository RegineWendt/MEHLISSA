<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0048: Fail-Closed Synthetic Resilience Scope

- Status: Accepted
- Date: 2026-09-01
- Decision owners: MEHLISSA Next maintainers

## Context

Gate M6 requires failure and security scenarios, but the current communication
plane has no identity provider, cryptographic protocol, clinical controller, or
qualified physical channel. Describing the M6.5 allow-list as security would
overstate the implementation. Treating every prescribed loss as an exception
would also erase valid communication metrics.

## Decision

MEHLISSA will distinguish two failure classes:

1. prescribed transport loss, corruption, or expiry is a typed communication
   result and remains included in attempt, byte, outcome, latency, and energy
   accounting; and
2. malformed, misrouted, disallowed, replayed, identity-mismatched, or excess
   input is rejected at the owning boundary before downstream state changes.

M6.7 defines a strict twelve-case synthetic resilience profile. Every case
declares its injection, owning boundary, expected disposition, accounting
expectation, and unchanged-state requirement. The profile also carries an
explicit threat model, protected properties, excluded claims, sources, and
limitations.

The term “security scenario” at M6 means only deterministic boundary misuse
under this declared scope. It does not mean authentication, encryption,
cryptographic authorization, clinical safety, or resistance to a real attacker.

## Consequences

- Failure behavior is executable, data-driven, and reviewable.
- Communication metrics do not silently become biological outcomes.
- Denied or invalid inputs cannot mutate the station, gateway, external client,
  or nanodevice beyond explicitly documented attempt accounting.
- Future cybersecurity work needs a new ADR, threat model, contracts, and
  validation; it cannot claim that M6.7 already supplies those controls.
- Gate M6 can close at the synthetic software-contract level while broader
  physical-channel and security research requirements remain visible.
