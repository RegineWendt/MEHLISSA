<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary Circulation Model Card

## Identity and purpose

- **Implementation:** `mehlissa::models::organ::PulmonaryCirculation`
- **Model class:** ordered multi-region pulmonary transit surrogate
- **Evidence class:** architectural/structural surrogate
- **Current assurance:** software verification only
- **Purpose:** prove that a more structured organ implementation is interchangeable with `LungCompartment`

## Structure

The checked contract test configures three serial regions:

1. pulmonary artery;
2. regional capillary surrogate;
3. pulmonary vein.

Every region has a stable ID and positive duration. Entities advance through
the ordered regions and retain identity and type. Entry, exit, body return
route, and synchronization semantics are identical to the coarse compartment.
The implementation supports any positive number of serial regions without
scenario logic in the kernel.

## Verification

The reference configuration uses 0.5 s arterial, 1.0 s regional/capillary, and
0.5 s venous transit. Entity 17 exits after 2.0 s through the same pulmonary
venous contract used by the coarse model. Duplicate region IDs, empty
configuration, non-positive duration, route mismatch, and duplicate entity IDs
are rejected by construction or the shared transfer contract.

## Scientific limits

The regions are not yet anatomical vessels or calibrated hemodynamic
compartments. They carry no pressure, resistance, branching, geometry,
gravity-dependent perfusion, uncertainty, or physiological-state parameters.
The current durations are contract-test values and must not be interpreted as
human pulmonary transit evidence.

The separate [pulmonary 0D reference candidate](PULMONARY_0D_REFERENCE.md) now
provides sourced mean hemodynamics and measured total transit without silently
relabeling these three synthetic timings. Anatomical refinement must next
qualify a SimVascular/VMR reference or another vascular data set and compare it
with independent pulmonary targets.
