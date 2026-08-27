<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Conserved Population, Substance, and Flow Transfers

## Contract categories

M3.3 deliberately separates three meanings:

- `PopulationTransfer`: an integer count of a named population;
- `SubstanceAmountTransfer`: a dimension-safe molar amount of a named substance;
- `VolumeFlowTransfer`: a dimension-safe volume rate over an explicit duration.

Every transfer carries contract version `1.0.0`, a stable transfer ID, complete
source and target route, and emission time. Positive finite values are required.
For a flow transfer, integrated volume is derived from rate × interval with the
kernel unit system.

## Conservation ledger

`ConservationLedger` records sent and received objects independently. A ledger
is balanced only when every transfer ID occurs exactly once on both sides and
the complete typed payload is identical. It rejects duplicate IDs and reports
changed counts, amounts, flow rates, intervals, routes, or times as unbalanced.

This establishes exact software conservation at synchronization boundaries.
It does not claim physiological conservation inside an organ model until the
corresponding endpoints consume and emit these contracts.

## Verification

Contract tests cover:

- a population of 10,000 nanodevices;
- 2.5 mmol of a named substance;
- 0.0001 m³/s over two seconds, integrating to 0.0002 m³;
- duplicate sent IDs; and
- a received population changed from 100 to 99.

The next slice connects the ledger to the coarse and detailed lung endpoints
and tests simultaneous entity, population, substance, and flow balances.
