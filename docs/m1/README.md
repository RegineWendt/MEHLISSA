<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M1 – Trustworthy Kernel

**Status:** complete

**Period:** 26–27 August 2026

M1 provides a small, reproducible, and tested simulation kernel. Medical models
are built only on top of this foundation.

## Implementation status

| Increment | Status | Verification |
|---|---|---|
| M1.0 C++20/CMake/vcpkg and platform CI | complete | MSVC, GCC, and Clang jobs; CTest |
| M1.0 monotonic time, 3D geometry, named RNG streams | complete | kernel unit and smoke tests |
| M1.1 versioned experiment manifest | complete | schema `1.0.0`, parser, validator, CLI and negative tests; platform CI |
| M1.2 provenance manifest | complete | schema `1.0.0`, automatic generation, SHA-256 and contract tests; platform CI |
| M1.3 dimension-safe unit system | complete | SI contract, migrated 3D geometry, compile-time and unit tests; platform CI |
| M1.4 `SimulationContext` and component lifecycle | complete | context-bound clock/RNGs, unique ownership, lifecycle and error-path tests; platform CI |
| M1.5 structured errors, logging, and checkpoint contract | complete | stable error codes, JSONL schema, checkpoint schema, and tamper tests; platform CI |
| M1.6 cross-platform determinism verification | complete | byte-identical golden-reference run on MSVC, GCC, and Clang; platform CI |

The binding quantity and conversion catalog for M1.3 is in
[`UNITS.md`](UNITS.md). The M1.4 contract is documented in
[`COMPONENT_LIFECYCLE.md`](COMPONENT_LIFECYCLE.md). Error identifiers, the run
log, and the checkpoint format are described in
[`ERRORS_LOGS_CHECKPOINTS.md`](ERRORS_LOGS_CHECKPOINTS.md). The byte-identical
M1.6 reference run is defined in [`DETERMINISM.md`](DETERMINISM.md).

## Gate result

The “Trustworthy Kernel” gate is satisfied:

- the complete build and all 40 tests run on Windows/MSVC, Linux/GCC, and
  Linux/Clang;
- Clang additionally checks formatting, static analysis, ASan, and UBSan;
- kernel invariants for time, units, geometry, RNG, lifecycle, errors,
  manifests, logs, and checkpoints are automatically tested and documented;
- the M1.6 reference run is byte-identical on all three toolchains;
- `core/` contains no medical scenario class.

Completing M1 does not yet claim domain validity for a body, lung, capillary,
or cell model. That verification begins with M2.

## Using M1.1

From the repository root:

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe validate `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json

build/windows-msvc/Debug/apps/mehlissa.exe run `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json `
  --checkpoint-schema data/schemas/checkpoint/1.0.0.schema.json
```

Without `--schema`, the CLI uses
`data/schemas/experiment/1.0.0.schema.json` relative to the current working
directory.

The current minimal experiment does not yet contain medical models. The `run`
command validates the contract, advances the simulation clock deterministically
to the configured duration, and then writes
`<outputs.directory>/provenance.json`. Among other fields, the provenance
manifest contains the SHA-256 hash of the experiment file, seed, Git and build
state, compiler, platform, timestamp, and reached simulation time. Its contract
is defined in `data/schemas/provenance/1.0.0.schema.json`.

The run additionally creates `run.log.jsonl` and `checkpoint-000000.json`. The
minimal experiment checkpoint does not yet contain domain component snapshots,
but it already verifies experiment binding, time, seed, and random-stream
counters against the versioned contract.
