<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0013: Cross-Platform Determinism Verification

- **Status:** Accepted
- **Date:** 27 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M1; `SYS-001`, `SYS-002`, `SYS-003`, `QUA-001`, `QUA-002`

## Context

Successful tests on several platforms do not establish that the same simulation
run produces the same result on each. Comparing complete provenance, log, or
checkpoint files would also be unsuitable because they intentionally contain
timestamps, compiler details, and platform details.

The M1 kernel therefore needs a small reference run containing only
deterministic state quantities and testing every supported toolchain against the
same result.

## Decision

1. `mehlissa_determinism_reference` runs through the regular `ComponentHost` lifecycle, not privileged test access.
2. The run uses a fixed master seed, 16 steps of 62,500,000 ns each, and the named streams `circulation` and `sensor-noise`.
3. Each step draws two and one raw `std::mt19937_64` values, respectively, in a defined order. Floating-point distributions are not part of this contract.
4. Each 64-bit value is fed to FNV-1a-64 as a big-endian byte sequence. Individual and overall digests are compact regression signatures, not cryptographic integrity protection.
5. The result is written in binary mode as a canonical UTF-8/ASCII JSON file. This avoids divergent CRLF bytes on Windows.
6. CTest compares the SHA-256 of the generated file byte for byte with `tests/data/determinism/reference-v1.json`.
7. The same test runs in Windows/MSVC, Linux/GCC, and Linux/Clang/ASan/UBSan CI. Equality with the same reference transitively verifies platform equality.
8. Any intentional reference change requires a new format or reference version and a documented rationale; the golden file must not be updated silently.

## Consequences

Positive:

- Clock, seed derivation, stream names, draw order, and component path are jointly verified against a byte-identical reference.
- Platform deviations appear as normal CTest failures and block CI.
- Observational metadata remains separate from the deterministic result.

Negative and limitations:

- The verification covers M1 kernel primitives, not yet medical models.
- FNV-1a compresses the sequence and is not collision-resistant; the outer SHA-256 protects the complete reference file but does not replace domain invariants.
- Platform-identical floating-point and distribution algorithms are not claimed. They require their own contracts and tolerance classes before use.
- Replicate planning and statistical reproducibility remain later parts of `SYS-002`.

## Alternatives

- **Test only known individual values:** rejected because clock, lifecycle, and draw order would not be covered together.
- **Compare provenance or log files:** rejected because observational metadata intentionally varies.
- **Check only hash values in source code:** rejected because a separate reference file is easier to inspect, archive, and version.
- **Include floating-point distributions immediately:** deferred until rounding, the mathematics library, and permitted tolerances are explicitly defined.
