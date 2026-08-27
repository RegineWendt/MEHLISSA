<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M1.6 Determinism Verification

## Purpose

The `mehlissa_cross_platform_determinism` test demonstrates that the
deterministic M1 kernel primitives produce the same byte-identical result with
MSVC, GCC, and Clang. All platforms compare against the same golden file:

`tests/data/determinism/reference-v1.json`

Its SHA-256 checksum is:

`49d405197ff73691289321aabfaecbe73451df2da14a6f44a63fc6c52961bef2`

## Fixed run

| Item | Contract |
|---|---|
| Reference | `core-rng-clock-v1` |
| Master-Seed | `0x6a09e667f3bcc909` / `7640891576956012809` |
| Lifecycle | `ComponentHost`: initialize, 16 advances, finalize |
| Step size | 62,500,000 ns |
| End time | 1,000,000,000 ns |
| `circulation` stream | two raw draws per step, 32 total |
| `sensor-noise` stream | one raw draw per step, 16 total |
| Signature | FNV-1a-64 over big-endian bytes of the raw values |
| File comparison | SHA-256 over a canonical, binary-written JSON file |

The test deliberately uses raw integer values from the generator rather than a
floating-point distribution. Standard-library distributions may use different
algorithms across implementations. Domain models must therefore later use
either their own deterministic transformations or explicit statistical
tolerance contracts.

## Running the verification

The verification is part of every normal CTest run:

```powershell
ctest --preset windows-msvc-debug -R mehlissa_cross_platform_determinism -V
```

The generated temporary file is removed after comparison. Changing the golden
file changes the contract and must be reviewed together with a new reference
version and a rationale.
