<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Conservative Nanodevice Drug Delivery (M5.6)

## Purpose

M5.6 connects the M5.5 response event to a deliberately separate actuation and
delivery contract. A consistent intracellular threshold response can emit a
versioned signal for one nanodevice and payload. If no threshold is reached,
there is no activation signal and the complete payload remains in the device.

After activation, one analytical model transfers amount through three owners:

```text
device payload -> extracellular drug -> intracellular drug
```

For loaded amount `D0`, release rate `kr`, uptake rate `ku`, and time `t` after
activation:

```text
D(t) = D0 exp(-kr t)
E(t) = D0 kr/(ku-kr) [exp(-kr t) - exp(-ku t)]
I(t) = D0 - D(t) - E(t)
```

When `kr = ku = k`, the stable limit is `E(t) = D0 k t exp(-k t)`. The model
checks non-negativity and reports the balance
`D0 - D(t) - E(t) - I(t)` explicitly.

## Checked reference

The strict profile and schema are:

```text
examples/cell-models/synthetic-conservative-drug-delivery-v1.json
data/schemas/drug-delivery-profile/1.0.0.schema.json
```

The actual M5.5 ODE reference crosses its effector threshold at
`2.202913832 s`; that event activates a synthetic device carrying `100 nmol`.
With `kr = 0.4 s^-1`, `ku = 0.2 s^-1`, and a ten-second observation after
activation, the analytical result is:

| Owner or quantity | Checked amount |
|---|---:|
| device payload | `1.83156388887342 nmol` |
| extracellular drug | `23.4039288695757 nmol` |
| intracellular drug | `74.7645072415509 nmol` |
| total released from device | `98.1684361111266 nmol` |
| conserved total | `100.0000000000000 nmol` |

Tests cover the complete M5.5-to-M5.6 path, the no-threshold sealed-payload
case, the equal-rate analytical limit, amount conservation, strict identity,
schema/provenance rejection, and inconsistent event rejection.

## Interpretation boundary

This is a synthetic software surrogate, not a dose or treatment model. The
device, drug, amount, rates, compartments, and trigger are not biologically or
clinically qualified. The use of a cell-network threshold as a device command
is an abstract test connection and does not claim a physical signaling path.

Release and uptake are homogeneous, irreversible first-order processes. There
is no diffusion distance, membrane or receptor binding, saturation, metabolism,
elimination, toxicity, efficacy, feedback, or apoptosis. Intracellular amount
is an inventory only. Therefore M5.6 advances but does not complete `CELL-003`.

## Verification

```powershell
ctest --test-dir build/windows-msvc -C Debug --output-on-failure `
  -R "drug-delivery|activation|conservation"
```

See [ADR-0039](../architecture/adr/0039-conservative-nanodevice-release-and-uptake.md)
and [M5 implementation evidence](README.md).
