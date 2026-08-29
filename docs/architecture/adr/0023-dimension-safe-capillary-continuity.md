<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0023: Dimension-Safe Capillary Geometry and Continuity

- **Status:** Accepted
- **Date:** 29 August 2026
- **Applies to:** M4; `SYS-004`, `ARC-002`, `CAP-001`, `CAP-002`, `CAP-003`

## Context

The M4.1 schema prescribed a transit time for each microvascular region. This
was sufficient to test transport and ownership but allowed length, diameter,
flow, velocity, and transit to become mutually inconsistent when geometry was
added. The dissertation explicitly requires volume-flow continuity in the
parallel capillary bed.

M4 needs one authoritative set of independent inputs and must derive dependent
hemodynamic quantities without reducing them to untyped `double` values inside
the model.

## Decision

Capillary-bed definition schema `2.0.0` is a deliberate breaking revision.
Each region supplies SI length, SI diameter, and a positive parallel-vessel
count. The network supplies one positive SI volume-flow rate. Mean velocity and
transit time are no longer accepted as independent input.

The runtime uses the kernel's dimension-safe quantities to derive:

```text
A_single = pi * (diameter / 2)^2
A_total  = parallel_vessel_count * A_single
velocity = volume_flow_rate / A_total
transit  = length / velocity
```

Thus `FlowRate / Area` produces `Speed`, and `Length / Speed` produces `Time`
at compile time. Transit is rounded to the nearest simulation-clock nanosecond
only at the final clock boundary.

The parallel-vessel count in the `capillary` region must equal the network's
currently perfused path count. Arteriole and venule counts remain explicit and
independent. A transported `VolumeFlowTransfer` must match the configured
continuity flow within a narrow relative floating-point tolerance.

All lengths, diameters, counts, flow, derived areas, velocities, and transit
times must be positive, finite, and representable. The original `1.0.0` schema
and v1 card remain in the repository as M4.1 historical evidence; current
execution and composition use the v2 card.

## Consequences

Positive:

- flow, area, velocity, and transit cannot silently contradict one another;
- recruitment has a defined geometric lever for the next increment;
- programmatic configuration and JSON input use the same runtime invariants;
- detailed and surrogate variants can compare derived quantities with common
  units and equations;
- a mismatched volume-flow hand-off fails at the capillary boundary.

Negative:

- schema `1.0.0` cards require migration before current execution;
- the current model assumes one steady total flow through all serial regions;
- every region uses one representative diameter and length;
- rounding to clock resolution introduces a bounded timing quantization;
- the v2 reference values are still synthetic and do not validate physiology.

## Rejected alternatives

- **Keep prescribed transit beside geometry:** duplicate authoritative inputs
  permit inconsistency and obscure which value should drive transport.
- **Store velocity in JSON:** velocity is a consequence of flow and total
  cross-section under the selected continuity model.
- **Use raw doubles throughout the runtime:** this would bypass the established
  dimensional type system at exactly the boundary where unit errors matter.
- **Modify schema `1.0.0` in place:** adding required geometry and removing
  prescribed transit is a breaking contract change and requires a new major
  version.
- **Call the synthetic v2 card physiological:** its values are selected to
  preserve deterministic software-test timing, not fitted to human data.
