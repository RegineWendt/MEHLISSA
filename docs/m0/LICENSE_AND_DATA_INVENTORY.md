<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# License and Data Inventory

**As of:** 26 August 2026

**Status:** M0 inventory and licensing model approved; 1995 source data set authorized since 27 August 2026; further rights reviews remain artifact-specific

**Note:** This document is a technical inventory, not legal advice.

## 1. Summary

The historical MEHLISSA code consistently contains **GNU GPL version 2**
notices and names Universität zu Lübeck and—for later contributions—Technische
Universität Berlin as rights holders. The repository therefore deliberately
uses a per-file and per-artifact licensing model.

The approved and implemented structure is:

1. release independently developed MEHLISSA Next code under `MPL-2.0`;
2. retain legacy code and direct ports under `GPL-2.0-only`;
3. license new original project documentation and approved original data under `CC-BY-4.0`;
4. provide full license texts under `LICENSES/` and the mapping in `LICENSE.md`;
5. document rights holders and third-party material in `NOTICE.md` and `THIRD_PARTY_NOTICES.md`, respectively;
6. exclude unresolved existing data and publication PDFs from public Next data or software packages.

For existing material, the decision follows the available license notices,
remains compatible with an optional ns-3 connection, and opens the independently
developed Next kernel for broader integration. It is binding through
[ADR-0007](../architecture/adr/0007-repository-license.md).

## 2. Source code and tools

| Component | Use | Identified license | Finding/action |
|---|---|---|---|
| `mehlissa/` | legacy MEHLISSA 1.x/ns-3 module | GPL-2.0-only according to existing headers | retain rights holders and authors per file; full text under `LICENSES/` |
| `mehlissa2.0/` | legacy MEHLISSA 2.0 | GPL-2.0-only according to existing headers | Universität zu Lübeck and TU Berlin named; full text under `LICENSES/` |
| `core/`, `apps/`, `tests/`, `cmake/` | independently developed MEHLISSA Next | MPL-2.0 | SPDX headers present; GPL ports must remain identified separately |
| Catch2 | unit tests, through vcpkg | BSL-1.0 | compatible test dependency; retain copyright file in build artifact |
| vcpkg | package manager | MIT | port libraries retain their own licenses |
| CMake/compiler | build tools | not vendored in MEHLISSA | no code adoption; record version in provenance |
| ns-3 | optional legacy/communication adapter | GPL-2.0-only | not a mandatory dependency of the Next kernel; retain license compatibility |
| SimVascular | future external adapter | BSD-3-Clause | do not vendor unless necessary; document version and copyright |

Official references: [Catch2](https://github.com/catchorg/Catch2), [vcpkg](https://github.com/microsoft/vcpkg), [ns-3](https://www.nsnam.org/), [SimVascular](https://simvascular.github.io/).

## 3. Repository data

Since 27 August 2026, the three files under `mehlissa2.0/data/95_*.csv` have
explicit `CC-BY-4.0` sidecars and the release manifest
`data/legacy/bvs95/release-v1.json`. The other CSV files still have no
individual license notices; their Git history does not replace an explicit data
license.

| File | Lines | SHA-256 | Origin/finding |
|---|---:|---|---|
| `fingerprint.csv` | 9 | `a6e42f40d6ff5f159224ab85e9d0a6c73e75f23217a922e3b49e854f259e467c` | Regine Wendt, commit `4757903`; identical to 2.0 copy; inconsistent trailing comma |
| `transitions95.csv` | 23 | `186650341da23f233ec483fcebfd212833b48071922345f297bc36a771709a8a` | Regine Wendt, commit `4757903`; identical to 2.0 copy; inconsistent trailing comma |
| `vasculature_transitions95.csv` | 95 | `8e04a2e004e42fd37a4cb1d225f4c0dd8b6bf27d05afec4e40a51fbb18c31588` | Regine Wendt, commit `4757903`; combined legacy format without header |
| `vasculature_transitions_endocrine_avs.csv` | 104 | `90e3429ab30e07606b507e3eb3c76cf0c4e9688658e81f6a888483ead99ba241` | Saswati Pal, commit `c14ad46`; AVS extension without data schema/unit metadata |
| `bodymodels/vasculature_transitions95female.csv` | 96 non-empty lines | `b3a3d30cdbe0d95ba2ffb15e605992095999ad965c67970325ea36e8cd950f23` | Regine Wendt, commit `a3f3090`; **known defect:** data record 51 is split across file lines 51 and 53 |
| `bodymodels/vasculature_transitions95male.csv` | 95 | `3fbc694159554624169cab0f4420d054037f5d1262b3f6df4e88feb2d4ab072e` | Regine Wendt, commit `a3f3090`; without header/unit metadata |
| `mehlissa2.0/data/95_fingerprints.csv` | 9 | `a6e42f40d6ff5f159224ab85e9d0a6c73e75f23217a922e3b49e854f259e467c` | Lisa Y. Debus, commit `59cccdf`; byte-identical to root file; **authorized under CC-BY-4.0** |
| `mehlissa2.0/data/95_transitions.csv` | 23 | `186650341da23f233ec483fcebfd212833b48071922345f297bc36a771709a8a` | Lisa Y. Debus, commit `59cccdf`; byte-identical to root file; **authorized under CC-BY-4.0** |
| `mehlissa2.0/data/95_vasculature.csv` | 95 | `de4b2b730bf3f88d3cb59213d67fcaf3d4764a3aca69be892e151109eb8c9db8` | Lisa Y. Debus, commit `59cccdf`; consistent ten fields; **authorized under CC-BY-4.0** |

### 3.1 Binding migration rule

Legacy files are not silently corrected or overwritten. Migration into `data/` must:

- record the source file and SHA-256;
- make field meaning, unit, and coordinate system explicit;
- execute transformations through a reproducible tool;
- detect known defects as validation errors;
- separate new canonical IDs from historical vessel numbers;
- record license, authorship, publication source, and validity scope in the data manifest.

## 4. Literature in the repository

Six publication PDFs reside under `literature/`. They are primary domain sources
but carry publisher or publication rights and do not grant repository-wide
redistribution permission.

Rule pending rights review:

- PDFs may be used locally for scientific traceability;
- automated release packages and containers exclude `literature/`;
- public mirroring or redistribution is not inferred from the MEHLISSA code license;
- DOI and bibliographic citation are the permanent public reference;
- author version, publisher permission, or open-access status is recorded per PDF.

## 5. Planned external data sources

| Source | Purpose | License/terms of use | Inclusion condition |
|---|---|---|---|
| [Human Protein Atlas](https://www.proteinatlas.org/about/licence) | fingerprint gene products and tissue expression | CC BY 4.0 for copyrightable data; handle third-party sources separately | record exact HPA version, genes/URLs, and citation |
| [BodyParts3D](https://lifesciencedb.jp/bp3d/info_en/license/index.html) | generic organ/body geometry | CC BY-SA 2.1 Japan | retain ShareAlike and required credit; document version/coordinate warnings |
| [Vascular Model Repository](https://www.vascularmodel.com/FAQs.html) | pulmonary and other vascular/boundary-condition models | research and development use with copyright/README and acknowledgment obligations | archive the specific model and its `README-COPYRIGHT`; do not label generically as open data |
| [SimVascular](https://simvascular.github.io/) | segmentation, 0D/1D/3D hemodynamics, conversion | BSD-3-Clause | record tool version and conversion workflow |
| [BioModels](https://www.ebi.ac.uk/biomodels/faq) | curated reaction/cell models | models under CC0 according to the FAQ | record model ID, revision, original publication, and curation status |
| [Physiome Model Repository](https://models.physiomeproject.org/) | CellML/circulation/multiscale models | publicly accessible content generally CC BY 3.0; individual entries may differ or be unclear | verify each model's license and record a persistent revision and citation |

## 6. Release gates

Before external data or models are included, the following fields must be complete:

```text
identifier
title
source_url
source_version_or_date
retrieved_at
sha256
license_spdx_or_text
required_attribution
original_citation
transformations
units
coordinate_system
population_and_validity
known_limitations
responsible_reviewer
```

## 7. Remaining artifact-specific rights review

The repository-wide licensing decision has been made and the three
`mehlissa2.0/data/95_*.csv` sources are authorized. The chain of rights for the
other existing CSV files and publication PDFs remains open. These artifacts
will be labeled as CC-BY-4.0 data or included in public Next packages only after
the responsible rights holders have confirmed authorization. New original data
uses a manifest and `CC-BY-4.0` from the outset.
