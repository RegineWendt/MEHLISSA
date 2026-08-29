<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Capillary Transit Bed

## Purpose

M4.1 establishes the capillary layer as an independent `ModelComponent`. It is
the lossless reference against which later recruitment, exchange, retention,
and communication behavior can be checked.

The model follows the architecture in dissertation section 4.4: arterial flow
reaches arterioles, crosses a highly parallel capillary bed, and drains through
venules. Substance exchange and molecular communication belong at this layer,
but are deliberately not inferred from the initial software-test card.

## Contract

Each definition has named entry and exit ports and exactly three ordered
regions:

1. `arteriole`;
2. `capillary`;
3. `venule`.

Every region has a unique ID. M4.1 schema `1.0.0` prescribed positive transit
times. Current schema `2.0.0` instead requires SI length, SI diameter, and a
positive parallel-vessel count for each region plus one network volume flow.
Area, velocity, and transit are derived through continuity. The capillary
region's vessel count must equal the currently perfused path count.

The loader validates the JSON document against:

```text
data/schemas/capillary-bed-definition/2.0.0.schema.json
```

The executable reference is:

```text
examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json
```

The v1 schema and card remain unchanged as historical M4.1 evidence. They are
not the current executable contract. See
[Capillary Geometry and Continuity](CAPILLARY_GEOMETRY_AND_CONTINUITY.md) for
the v2 derivation and migration boundary.

## Runtime semantics

- a transfer must target the configured arteriole entry;
- its emission time must equal the component synchronization time;
- duplicate entity and conserved-transfer IDs are rejected;
- compatible time steps may cross more than one region in one advance;
- completion is reported at the synchronization boundary at which the venule
  transit finishes;
- outbound transfers name the capillary model and venule exit as their source
  and the configured organ return port as their target.

M4.2 connects these boundaries to any compatible organ component through the
generic four-port co-simulation coupler. Its pending queues and outstanding-ID
ledgers make temporary ownership explicit from organ departure through organ
return. See [Organ-Capillary Round Trip](ORGAN_CAPILLARY_ROUND_TRIP.md).

Population count, substance amount, flow rate, flow interval, and integrated
volume remain unchanged. This is an intentionally lossless control case. Later
exchange must report balanced blood, endothelium, interstitium, and cell terms
through a new typed contract; it must not mutate this baseline silently.

## Synthetic reference

The distributed v2 card contains eight total capillary paths, four marked
perfused, and geometry and flow that derive serial transit times of 0.2 s,
0.6 s, and 0.2 s. These coordinated values were selected for deterministic
tests. They do not describe human microcirculation and must not be used for
biological conclusions.

## Verification

The `[m4][capillary]` tests verify:

- region order and movement at exact boundaries;
- identity-preserving nanodevice transit;
- lossless population, substance, and volume-flow payloads;
- integrated-volume conservation;
- schema-to-runtime connection;
- dimension-safe cross-section, continuity velocity, and transit derivation;
- equality of configured and transferred volume flow;
- invalid recruitment counts and region order;
- wrong routes, wrong synchronization time, and duplicate ownership.

The `[m4][cosimulation][round-trip]` tests additionally verify complete organ
departure and return, conservation across both boundaries, compatible host
steps, pending-delivery retention, and invalid endpoint rejection.

## Explicitly deferred

- physiologically qualified diameter, length, density, velocity, pressure, and
  hematocrit;
- physiologically parameterized sphincter anatomy, feedback, and heterogeneous
  path conductance (aggregate scheduled recruitment is implemented in M4.4);
- spatial paths and stochastic transit distributions;
- physiologically parameterized barrier kinetics, reaction, retention,
  adhesion, and extravasation (a balanced staged exchange surrogate is
  implemented in M4.5; exact residence and non-state-changing competing
  interaction likelihoods are implemented in M4.6);
- molecular channel, clustering, reachability, and multi-hop behavior;
- a physiologically parameterized organ-capillary gateway and multirate
  orchestrator.
