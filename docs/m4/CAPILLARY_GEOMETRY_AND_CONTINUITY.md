<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Capillary Geometry and Continuity

## Purpose

M4.3 replaces prescribed regional transit times with a small, explicit
hemodynamic model. Geometry and total flow are inputs; cross-sectional area,
mean velocity, and transit time are derived. This prevents a model card from
claiming mutually incompatible flow, geometry, speed, and travel time.

## Definition contracts 2.0.0 and 3.0.0

The geometry contract began with the schema and executable software-test card:

```text
data/schemas/capillary-bed-definition/2.0.0.schema.json
examples/capillary-models/synthetic-arteriole-capillary-venule-v2.json
```

The network provides:

- total and currently perfused capillary-path counts;
- one steady total `volume_flow_rate_m3_s`; and
- exactly one arteriole, capillary, and venule region.

Every region provides `length_m`, `diameter_m`, and
`parallel_vessel_count`. Values are converted immediately into `Length` and
`FlowRate` quantities. Unknown fields and missing units are rejected by the
strict JSON Schema.

Version `2.0.0` is intentionally incompatible with the M4.1 `1.0.0` contract,
which prescribed `transit_time_s`. The v1 files remain historical evidence but
are not the current executable contract. M4.7 schema `3.0.0` retains the same
runtime geometry while adding required parameter-level evidence and consistency
metadata for literature-parameterized cards:

```text
data/schemas/capillary-bed-definition/3.0.0.schema.json
examples/capillary-models/pulmonary-healthy-adult-rest-supine-v1.json
```

The loader continues to accept v2 synthetic definitions and requires the new
qualification block for v3.

## Continuity derivation

For a region with diameter `d`, parallel-vessel count `n`, length `L`, and
network flow `Q`, MEHLISSA calculates:

```text
A_single = pi * (d / 2)^2
A_total  = n * A_single
V_region = A_total * L
v_mean   = Q / A_total
t_region = L / v_mean
```

Regional blood volume is exposed with the other derived metrics. The operations
are dimension checked by the C++ quantity system. Only the
final `Time` value is converted to a nanosecond simulation duration. The
capillary region's `n` must equal `perfused_path_count`, making the current
recruitment state part of the actual continuity calculation rather than unused
metadata.

The same total flow crosses all three serial regions. Therefore changes in
total cross-section produce the corresponding inverse change in velocity.
M4.3 remains a steady representative-tube model; it does not yet model
pressure, pulsatility, viscosity, hematocrit, or flow sharing among unequal
individual vessels.

## Synthetic reference result

The v2 card deliberately reproduces the previous one-second software-test
transit through geometry:

| Region | Parallel vessels | Derived mean velocity | Derived transit |
|---|---:|---:|---:|
| arteriole | 1 | 0.004 m/s | 0.2 s |
| capillary | 4 | 0.001 m/s | 0.6 s |
| venule | 1 | 0.004 m/s | 0.2 s |

Its 10 micrometre representative diameter, lengths, path counts, and total flow
form one coordinated arithmetic fixture. Their combination has not been
qualified as human pulmonary physiology and must not support biological
conclusions.

## Runtime validation

The model rejects:

- zero, negative, non-finite, or unrepresentable geometry and flow;
- zero region vessel counts;
- a capillary vessel count different from the perfused-path count;
- region order or identifiers that violate the established bed contract;
- a `VolumeFlowTransfer` inconsistent with the configured continuity flow; and
- geometry-derived transit below clock resolution or above its range.

## Verification

After building, run:

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure -R "geometry derives"
ctest --test-dir build/windows-msvc -C Debug --output-on-failure -R "capillary|coupler"
```

The tests prove the area, velocity, continuity, transit, schema, conservation,
and complete organ-capillary-organ path for the synthetic v2 reference.

## Next work

M4.4 now makes perfused-path count dynamic through a versioned recruitment
overlay and aggregate precapillary sphincter groups. Each profile explicitly
selects fixed total flow or a simplified fixed-pressure-drop conductance
surrogate when paths open or close. See
[Capillary Recruitment and Precapillary Sphincter Groups](CAPILLARY_RECRUITMENT_AND_SPHINCTERS.md).
M4.7 now supplies the first pulmonary parameter card, but independent
physiological comparison and further organ cards remain separate from that
mechanism. See
[Pulmonary Capillary Reference Candidate](PULMONARY_CAPILLARY_QUALIFICATION.md).

See [ADR-0023](../architecture/adr/0023-dimension-safe-capillary-continuity.md)
for the binding decision.
