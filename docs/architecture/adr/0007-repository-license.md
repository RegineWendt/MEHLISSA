<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0007: Repository Licensing Model

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M0/M1 release; all source and data artifacts

## Context

Source files in `mehlissa/` and `mehlissa2.0/` contain GNU GPL version 2
notices. Universität zu Lübeck and, in some cases, Technische Universität
Berlin are named as rights holders. No central license file exists and GitHub
does not detect a license. Newly created Next files do not yet have SPDX labels.

Without an unambiguous repository-wide declaration, users and contributors do
not know under what terms Next code, documentation, and data may be used. Code
and data licenses must not be conflated.

## Decision

1. Completely new and independently developed MEHLISSA Next source code, build scripts, and tests are released under `MPL-2.0`.
2. Historical code in `mehlissa/` and `mehlissa2.0/` remains `GPL-2.0-only`. Direct ports, modifications, and integration components derived from this code retain `GPL-2.0-only`.
3. New original project documentation and original data approved for publication are released under `CC-BY-4.0`.
4. Licensing applies per file or artifact. New files receive SPDX labels; formats that cannot contain comments receive a `.license` sidecar or an unambiguous data manifest.
5. Existing copyright and author notices are retained.
6. This decision does not retroactively relicense existing data sets. Their release under CC BY 4.0 requires a confirmed chain of rights; until then they remain outside public Next data packages.
7. Third-party data, publications, and software remain subject to their respective terms. `LICENSE.md`, `NOTICE.md`, `THIRD_PARTY_NOTICES.md`, and data manifests document the boundaries.

## Rationale

- GPL-2.0-only matches the license notices in the existing code and remains compatible with ns-3.
- MPL-2.0 keeps changes to the new kernel open at file level while making integration into larger research and industrial applications easier.
- Separate licensing avoids unauthorized relicensing of historical contributions.
- CC BY 4.0 is more suitable than a software license for documentation and citable research data.
- SPDX labels make the applicable license machine-readable.

## Consequences

Positive:

- Users and contributors receive unambiguous terms for each artifact.
- The new kernel remains open while being easier to reuse than under strong copyleft for the entire work.
- License checks can be automated in CI.

Negative:

- Reviews must consistently check the boundary between independent new development and GPL ports.
- A work derived from legacy code does not become MPL code merely by being placed in a Next directory.
- Existing data and publication PDFs still require a separate rights review.
- Release and packaging processes must represent the multiple licenses.

## Alternatives

- **GPL-2.0-only for the entire repository:** legally and operationally simpler, but creates higher integration barriers for independent Next components.
- **BSD-3-Clause/MIT for Next:** maximum reusability, but does not require disclosure of improvements to the affected files.
- **Apache-2.0 for Next:** unfavorable for the planned integration paths because of incompatibility with GPL-2.0-only.
- **Dual license:** flexible, but requires comprehensive consent from all rights holders and a contributor agreement.
- **No central license:** rejected; leaves fundamental use questions unanswered and prevents a clean public release.

## Implementation and review rule

The decision is implemented through `LICENSE.md`, full texts under `LICENSES/`,
notices, and SPDX labels. Every pull request containing ported legacy code or
new data must explicitly verify the applicable license and provenance. Before
an institutional public release, the legal or transfer office should confirm
the documented boundaries; this review does not retroactively change licenses
of third-party artifacts.
