<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# External Pulmonary Data Qualification Checklist

Before a pulmonary geometry or hemodynamic data set becomes an executable
MEHLISSA reference, record and review all of the following:

- canonical source URL, release/case ID, retrieval date, and immutable checksum;
- license and redistribution status for original and derived artifacts;
- subject population, health/pathology, physiological state, and acquisition or
  simulation conditions;
- anatomical scope, omitted branches, inlet/outlet definitions, and boundary
  conditions;
- native coordinate system, handedness, origin, axes, and every transformation;
- native units and explicit conversion to MEHLISSA SI units;
- mesh/network quality checks, connectivity, duplicate IDs, and conservation at
  every junction;
- parameter derivation, assumptions, uncertainty, and sensitivity plan;
- calibration targets separated from independent validation targets; and
- a comparison plan against the effective compartment and historical FP9
  scenario without tuning on the validation result.

The definition schema stores the portable subset of this information. The
review record and source-specific conversion procedure must accompany the data
manifest. Passing the schema is necessary but not sufficient for scientific
qualification.
