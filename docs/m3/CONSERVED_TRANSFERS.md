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

## Organ endpoint integration

`ModelComponent` accepts and emits a `ConservedTransfer` variant in addition to
individual entities. Both lung implementations validate the target model,
entry port, synchronization time, contract version, and unique transfer ID.
They own accepted transfers during transit, then preserve the typed payload and
stable transfer ID while replacing the route with the configured pulmonary
venous return and the actual emission time.

The shared generated regression sends all three transfer categories through
both lung variants. It verifies unchanged population count, molar amount, flow
rate, interval, and integrated volume. It also verifies that duplicate IDs,
wrong routes, and wrong synchronization times are rejected.

This is a conservative transport endpoint, not a biochemical or gas-exchange
model. Consumption, production, reaction, or barrier exchange will require an
explicitly balanced transformation contract in M4/M5.

## Verification

Contract tests cover:

- a population of 10,000 nanodevices;
- 2.5 mmol of a named substance;
- 0.0001 m³/s over two seconds, integrating to 0.0002 m³;
- duplicate sent IDs; and
- a received population changed from 100 to 99.

The endpoint regression complements the ledger tests by exercising ownership
and transit through the coarse and regional pulmonary implementations.
