<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Organ-Capillary Round Trip

## Purpose

M4.2 connects the independent organ and capillary layers without making either
one aware of the other's implementation. It provides the conservative
ownership bridge needed for a nanodevice or aggregate payload to leave an
organ, traverse a capillary component, and return.

The current executable route is deliberately synthetic. It demonstrates that
the four-layer architecture can represent this journey; it does not claim that
one second of transit, eight paths, or four perfused paths describe a human
lung or another organ. M4.3 preserves that timing through internally consistent
v2 geometry and continuity rather than prescribed regional transit values.

## Four-boundary route

```text
organ departure -> capillary entry -> capillary exit -> organ return
```

Each arrow endpoint is a named port. The coupler validates all four port names,
both model IDs, the transfer contract, and the synchronization timestamp. Port
roles are configured explicitly rather than inferred from organ type or model
name.

## Ownership lifecycle

1. The organ owns an outbound transfer until the coupler collects it.
2. The coupler retains the transfer in a departure queue until the capillary
   accepts it.
3. Its identifier then appears in the coupler's outstanding ledger while the
   capillary owns and transports it.
4. The coupler collects the capillary return and retains it until the organ
   accepts it.
5. Successful organ acceptance closes the ledger entry and increments the
   completed-round-trip counter.

If validation or target acceptance fails, the transfer remains pending. This
makes retry possible and prevents a routing or synchronization error from
becoming silent loss. Unknown returns and duplicate outstanding departures are
rejected.

## Conserved meaning

The same journey is verified for:

- an individual entity with stable ID and type;
- an integer population;
- a typed substance amount; and
- a volume flow with unchanged rate, interval, and integrated volume.

M4.2 does not transform these values. This lossless path is the control against
which M4 exchange, retention, and reaction behavior must later balance its
explicit source, sink, and storage terms.

## Synchronization

The coupler does not advance either model. A host or future co-simulation
orchestrator advances the endpoints and invokes transfer only at a shared
synchronization boundary. Tests cover 100 ms, 250 ms, and 500 ms host steps for
the one-second synthetic transit, as well as a rejected early return followed
by a successful retry at the correct time.

## Verification

After building, run the focused tests with:

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure -R "organ capillary|coupler"
```

They verify the complete entity and conserved-payload route, ownership counts,
step-size stability, invalid endpoint configuration, rejected routes, and
retryable synchronization failures.

## Scientific boundary and next work

M4.2 satisfies the first Gate M4 criterion only at the synthetic
software-contract level, and M4.3 supplies its dimension-safe geometry and
continuity calculation. Still required are physiological organ-specific cards,
physiologically qualified recruitment, blood/tissue exchange, retention and extravasation,
molecular channels, and comparison of detailed and surrogate models against
common references.

See [ADR-0022](../architecture/adr/0022-organ-capillary-round-trip-coupling.md)
for the binding architectural decision and
[Capillary Transit Bed](CAPILLARY_TRANSIT_BED.md) for the transported component.
