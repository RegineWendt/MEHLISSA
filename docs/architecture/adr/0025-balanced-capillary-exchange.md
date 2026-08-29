<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0025: Balanced Capillary Substance Exchange

- **Status:** Accepted
- **Date:** 29 August 2026
- **Applies to:** M4; `ARC-003`, `ARC-004`, `CAP-001`, `CAP-004`

## Context

The lossless capillary baseline proves ownership and transport but cannot
represent a substance leaving blood for tissue. Mutating the existing
`SubstanceAmountTransfer` without recording its destination would appear as
mass loss. Embedding synthetic exchange in every capillary bed would also
invalidate the lossless reference and obscure which results depend on an
exchange assumption.

The first exchange increment needs a complete four-compartment balance,
dimension-safe amounts, explicit data provenance, and a route that remains
compatible with the existing organ-capillary coupler.

## Decision

Capillary exchange is an optional, strict, versioned overlay for one compatible
capillary model. Without an overlay, M4.1–M4.4 behavior remains lossless.

Schema `1.0.0` defines substance-specific staged fractions from blood to
endothelium, endothelium to interstitium, and interstitium to cell. At the
capillary-to-venule transition, the runtime derives outgoing blood and retained
tissue terms by subtraction and requires their sum to equal the incoming
amount within a narrow relative tolerance.

The transfer ID, substance ID, route, and contract version remain unchanged.
Only the blood-side amount is reduced. Tissue terms accumulate in inventories,
and a drainable record stores the exchange profile ID, reporting boundary,
incoming amount, outgoing blood, and all three tissue deposits.

The initial contract supports only the explicit `pass_through` policy for
unmatched substances. Population and flow transfers never enter substance
exchange. The blood-to-endothelium fraction is less than one because the
current conserved-transfer contract requires a positive returned amount.

The staged partition is a software surrogate. Fractions are not interpreted as
rates, permeability, equilibrium coefficients, or physiological measurements.

## Consequences

Positive:

- every removed blood-side amount has an explicit compartment destination;
- the synthetic M4 gate balance is executable across the complete layer route;
- quantities retain their physical dimension and SI representation;
- the lossless control path remains available without configuration changes;
- profiles, records, and inventories make assumptions and outcomes inspectable;
- future kinetic exchange implementations can preserve the same high-level
  accounting terms.

Negative:

- exchange is instantaneous at a reported synchronization boundary;
- tissue inventories only accumulate and do not yet release or consume mass;
- staged fractions do not depend on concentration, area, flow, or transit;
- a completely extracted blood amount cannot use the current positive-only
  return transfer;
- the synthetic example is not physiological validation.

## Rejected alternatives

- **Silently reduce the transfer amount:** this creates unaccounted mass loss.
- **Change the lossless baseline globally:** control and transforming models
  must remain separately selectable and comparable.
- **Put exchange in the coupler:** the coupler owns routing, not capillary or
  tissue physiology.
- **Call fractions permeability or rates:** the required dimensions, geometry,
  and time integration are absent from this surrogate.
- **Create four external model components immediately:** this would enlarge
  lifecycle and routing complexity before the balance contract is proven.
- **Drop unmatched substances:** an unsupported substance must remain visible
  and conserved rather than disappear.
