<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Body-transport observation benchmark

`mehlissa_body_transport_benchmark` is the dedicated single-run driver for the
Paper 1 RQ4 experiment. It executes the public `CompartmentTransport` API and
writes two schema-validated artifacts:

- the ordinary transport observation report; and
- a benchmark report containing timings, peak resident memory, transition
  throughput, output size, build and machine metadata, input hashes, RNG draw
  counts, and a deterministic final-population hash.

The manifest schema is
`data/schemas/body-transport-benchmark/1.0.0.schema.json`; the result schema is
`data/schemas/body-transport-benchmark-report/1.0.0.schema.json`. Relative model
and state-profile paths are resolved from the manifest directory. Output paths
are supplied on the command line so repeated blocks can use isolated files.

The checked-in `examples/benchmarks/synthetic-o2-smoke.json` is deliberately
small and tests the bounded-detail O2 policy. It is a software smoke test, not a
reported scientific measurement. The paper experiment orchestration is
responsible for warm-up, randomized O0--O3 block order, repetitions, resource
limits, and equality checks across policies.

## RQ4 campaign runner

`run_rq4_campaign.py` turns the frozen
`examples/benchmarks/rq4-primary-campaign.json` plan into fresh-process runs. It
creates one warm-up per condition, independently randomizes the ten primary
conditions and the long-duration anchor inside each of seven measured blocks,
monitors the 30-minute and
80%-of-RAM limits, and persists a ledger after every attempt. Failed and
interrupted attempts remain in that ledger.

The runner computes all seven raw observations plus median, inclusive
quartiles, IQR, and range. It checks transition counts, population totals,
final-state hashes, RNG draw counts, passive measurement totals, common
snapshots, and policy-specific truncation states. The conditional million-entity
phase is scheduled only after the measured 100,000-entity O0 medians pass both
frozen eligibility thresholds.

The checked-in O3 limits were frozen after the unreported pilot summarized in
`benchmarks/pilots/o3-record-limit-pilot-2026-08-27.json`. Before measurement,
replace the placeholder machine label. Measured execution rejects altered pilot
limits and anything other than a clean Release build. A safe schedule inspection
is available via:

```console
python benchmarks/run_rq4_campaign.py \
  --plan examples/benchmarks/rq4-primary-campaign.json \
  --output-directory rq4-plan-preview \
  --dry-run
```
