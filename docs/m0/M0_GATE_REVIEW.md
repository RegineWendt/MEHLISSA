<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M0 Gate Review – Project Charter and Architecture Decisions

**Review date:** 26 August 2026

**Result:** passed

## 1. Acceptance criteria

| Criterion | Status | Artifact/verification |
|---|---|---|
| dissertation translated into traceable requirements | satisfied | [system requirements](../requirements/SYSTEM_REQUIREMENTS.md), [traceability matrix](../requirements/TRACEABILITY_MATRIX.md) |
| vision, existing baseline, derivation, and new extension separated | satisfied | origin codes and ADR-0005 |
| four layers and responsibilities made binding | satisfied | ADR-0002, `ARC-001` through `ARC-007` |
| legacy adoption principle decided | satisfied | ADR-0001 |
| technology decision documented | satisfied | ADR-0003; C++20/CMake/vcpkg bootstrap |
| first vertical demonstrator prioritized | satisfied | ADR-0004 and fingerprinting specification |
| target users and workflows defined | satisfied | [target users and workflows](USERS_AND_WORKFLOWS.md) |
| data inventory and external sources cataloged | satisfied | [license and data inventory](LICENSE_AND_DATA_INVENTORY.md) |
| first reference organ selected | satisfied | ADR-0006: lung |
| research questions, data gaps, and partner roles named | satisfied | [data gaps and validation partners](VALIDATION_AND_DATA_PARTNERS.md) |
| legacy revisions archived | satisfied | tag `legacy-baseline-2026-08-26` |
| model validity and release quality defined | satisfied | ADR-0005, system requirements, and acceptance rules |
| repository-wide licensing model formally approved | satisfied | ADR-0007 `Accepted`; `LICENSE.md`, `LICENSES/`, notices, and SPDX labels |

## 2. Binding M0 decisions

- MEHLISSA Next uses a new kernel and qualifies legacy code before every port.
- The body, organ, capillary, and cell layers are independent co-simulation components.
- C++20/CMake/vcpkg form the technical foundation; Python follows as an experiment API.
- Fingerprinting is the first vertical demonstrator.
- The lung is the first organ model; work starts with pulmonary circulation, not complete respiratory mechanics.
- Reproducibility, units, provenance, evidence class, and uncertainty are product features.
- Public data and third-party models are included only with versioned license and transformation provenance.
- Independent Next code is MPL-2.0, legacy code and direct ports are GPL-2.0-only, and new original documentation and approved original data are CC-BY-4.0.

## 3. Entry into M1

Technical work on M1 may begin. The license and notice gate is satisfied.
Existing data with an unresolved chain of rights and publication PDFs remain
outside public Next packages until artifact-specific authorization is obtained.

The first M1 work packages are:

1. versioned experiment manifest and schema;
2. provenance manifest for every run;
3. complete unit system;
4. `SimulationContext` and component lifecycle;
5. structured errors and configuration validation;
6. subsequent migration of the 1995 vascular model in preparation for M2.

## 4. Final project-leadership decision

Project leadership confirmed the multiple licensing documented in ADR-0007. M0
is therefore complete. Outstanding rights reviews for individual existing
artifacts are tracked as release gates for the respective data packages and do
not block the start of M1.
