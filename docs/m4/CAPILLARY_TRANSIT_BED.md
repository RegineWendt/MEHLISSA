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

Every region has a unique ID and positive transit time. The definition also
declares total and currently perfused parallel paths. At least one path must be
perfused and the count cannot exceed the total. In M4.1 these counts express a
validated recruitment state; they do not yet change flow or transit.

The loader validates the JSON document against:

```text
data/schemas/capillary-bed-definition/1.0.0.schema.json
```

The executable reference is:

```text
examples/capillary-models/synthetic-arteriole-capillary-venule-v1.json
```

## Runtime semantics

- a transfer must target the configured arteriole entry;
- its emission time must equal the component synchronization time;
- duplicate entity and conserved-transfer IDs are rejected;
- compatible time steps may cross more than one region in one advance;
- completion is reported at the synchronization boundary at which the venule
  transit finishes;
- outbound transfers name the capillary model and venule exit as their source
  and the configured organ return port as their target.

Population count, substance amount, flow rate, flow interval, and integrated
volume remain unchanged. This is an intentionally lossless control case. Later
exchange must report balanced blood, endothelium, interstitium, and cell terms
through a new typed contract; it must not mutate this baseline silently.

## Synthetic reference

The distributed card contains eight parallel paths, four marked perfused, and
serial transit times of 0.2 s, 0.6 s, and 0.2 s. These values were selected for
deterministic tests. They do not describe human microcirculation and must not be
used for biological conclusions.

## Verification

The `[m4][capillary]` tests verify:

- region order and movement at exact boundaries;
- identity-preserving nanodevice transit;
- lossless population, substance, and volume-flow payloads;
- integrated-volume conservation;
- schema-to-runtime connection;
- invalid recruitment counts and region order;
- wrong routes, wrong synchronization time, and duplicate ownership.

## Explicitly deferred

- physiological diameter, length, density, velocity, pressure, and hematocrit;
- flow redistribution and dynamic sphincters;
- spatial paths and stochastic transit distributions;
- barrier exchange, reaction, retention, adhesion, and extravasation;
- molecular channel, clustering, reachability, and multi-hop behavior;
- a complete organ -> capillary -> organ co-simulation path.
