<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Third-party notices

This file records dependencies and external material known to the MEHLISSA
project. Their own license and attribution terms remain authoritative.

| Component/material | Role | License/status |
|---|---|---|
| Catch2 | Next unit-test dependency obtained through vcpkg | BSL-1.0 |
| jsoncons | JSON parsing and JSON Schema 2020-12 validation, obtained through vcpkg | BSL-1.0 |
| PicoSHA2 | Streaming SHA-256 checksums for provenance, obtained through vcpkg | MIT |
| vcpkg | Dependency manager; not vendored | MIT |
| ns-3 | Optional legacy and future communications integration | GPL-2.0-only |
| SimVascular | Potential external modeling tool/adapter | BSD-3-Clause |
| `literature/` publications | Scientific reference material | Publisher/publication terms; not covered by repository licenses |
| External biological and anatomical datasets | Model inputs | License of each specific source and version; record in a data manifest before use |

Generated release archives must exclude material whose redistribution rights
have not been confirmed. Dependency license files shipped in binary packages
must be retained.
