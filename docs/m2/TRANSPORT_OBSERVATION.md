<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2.5 – Transport Output, Extraction, and Measurement Sites

**Status:** complete

**Report schema:** `data/schemas/transport-observation-report/1.0.0.schema.json`

## Purpose

M2.5 makes the identity-preserving M2.3 transport observable without forcing
large experiments to emit unbounded individual-object output. Four result types
remain explicitly separate:

1. scheduled extraction events;
2. aggregate counters at sample and gateway measurement sites;
3. temporal segment populations;
4. optional individual trajectories and observations.

The first three result types preserve scientifically relevant totals.
Individual records, in contrast, are deliberately bounded.

## Extractions

An `ExtractionEvent` contains a time, segment ID, and optional count.

- With a count, up to that many active entities are extracted. If fewer are present, the actually available count is extracted.
- Without a count, all entities available in the segment at that time are extracted.
- Selection is deterministic: active entities are extracted in ascending ID order.
- The conservation invariant is then `active + extracted = injected`.

As with M2.3 injections, an event is processed at the first closed simulation
step whose end reaches the scheduled time. At that step end, due transitions
occur first, followed by extraction. The result record therefore contains both
`scheduled_time_ns` and `processed_time_ns`; it does not claim finer time
precision than the outer coupling step supplies.

## Measurement sites

A measurement site has a stable ID, segment ID, and kind:

| Kind | Meaning in M2.5 |
|---|---|
| `sample` | passive sampling/observation point |
| `gateway` | passive gateway measurement site without a communication protocol |

Every injection into or entry into the segment increments the total counter of
all measurement sites defined there. Optionally, the first individual passages
are stored with time and particle ID. The total remains exact after the detail
limit has been reached.

The M2.5 gateway has no range, contact probability, packets, channel errors, or
BAN connection yet. This active communication semantics belongs to M6. M2.5
provides only a stable spatial measurement and coupling point.

## Bounded output

`TransportObservationConfig` controls independently:

- `trajectory_selection`: `none`, `all`, or deterministic `first_n`;
- maximum number of stored trajectory events;
- maximum number of stored individual observations;
- aggregate interval and maximum number of population snapshots;
- any number of passive measurement sites, each with an unbounded 64-bit total counter.

Trajectories contain the actions `injected`, `entered_segment`, and `extracted`.
`first_n` selects the lowest particle IDs and is therefore stable across
platforms and seeds as a sample definition. `all` means “all selected entities”
but remains bounded by the mandatory record limit.

When a limit is reached:

- the corresponding `truncation` value becomes `true`;
- existing records are not overwritten;
- simulation, population conservation, and measurement-site totals continue;
- no reservoir or random sampling occurs.

Population snapshots are recorded at time zero and then at the first actual
step end that reaches the next interval. Skipped intermediate times are not
interpolated.

## Structured result format

`write_transport_observation_report` writes deterministic JSON containing:

- model ID and version;
- injected, active, and extracted population;
- transition count;
- complete observation configuration;
- truncation indicators;
- measurement-site totals and bounded individual observations;
- bounded trajectories;
- bounded segment-population snapshots;
- all executed extraction events.

The report is validated against the versioned JSON Schema before writing.
Visualization and later experiment orchestration can therefore read the format
without knowing internal C++ objects.

## Automated verification

`compartment_transport_tests` verifies:

- deterministic extraction of the lowest IDs;
- complete extraction when no count is provided;
- conservation of `active + extracted = injected`;
- exact sample and gateway totals;
- hard limits and set truncation indicators;
- aggregates remain correct after detail truncation;
- schema-validated JSON output.

## Limitations and next integration

The typed configuration will be integrated into the experiment/co-simulation
path in M3. M2.5 deliberately keeps observation in `models/body` so that body
transport does not import visualization, network, or scenario logic. Compressed
binary trajectories, streaming output, and gateway communication are later
interchangeable output sinks or models.
