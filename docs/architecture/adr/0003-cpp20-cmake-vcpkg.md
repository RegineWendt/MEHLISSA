<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0003: C++20, CMake und vcpkg als technisches Fundament

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Betrifft:** M0/M1; `QUA-001`, `QUA-002`, `QUA-005`

## Kontext

Ganzkörper- und Mehrskalensimulationen erfordern kontrollierbare Laufzeit, Speicherlayout und Parallelisierung. Historische MEHLISSA-Versionen sind in C++ implementiert. Für Experimente, Datenanalyse und wissenschaftliche Workflows ist Python langfristig attraktiv, als alleiniger Kern für große Agentenzahlen aber nicht ohne vorherigen Leistungsvergleich begründbar.

Die lokale Entwicklungsumgebung und CI unterstützen MSVC, GCC und Clang. Abhängigkeiten müssen reproduzierbar fixiert werden.

## Entscheidung

- Der Simulationskern und performancekritische Modelle werden in standardkonformem C++20 entwickelt.
- CMake ist das alleinige Buildsystem; Builds erfolgen out of source über Presets.
- Direkte C++-Abhängigkeiten werden in `vcpkg.json` mit festem Baseline-Commit deklariert.
- CTest orchestriert Tests; Catch2 ist die Unit-Testbibliothek.
- MSVC, GCC und Clang werden in CI unterstützt; der Analysejob verwendet clang-tidy sowie Address-/UndefinedBehaviorSanitizer.
- Python wird später als versionierte API für Experimenterstellung, Ensembles und Analyse ergänzt. Die Bindung darf das C++-Domänenmodell nicht duplizieren.
- Neue Abhängigkeiten benötigen einen konkreten Anwendungsfall, Lizenzprüfung und begründeten ADR-Eintrag, sofern sie die Architektur prägen.

## Folgen

Positiv:

- Performancekritische Pfade und Speicherlayout bleiben kontrollierbar.
- Bestehende C++-Kenntnisse und ausgewählte Legacy-Algorithmen sind nutzbar.
- Der Build ist bereits auf drei Compilern automatisiert geprüft.
- Python kann eine nutzerfreundliche Oberfläche liefern, ohne Kernkorrektheit zu ersetzen.

Negativ:

- C++ erhöht die Anforderungen an Ownership-, Einheiten- und API-Disziplin.
- Sprachübergreifende Bindungen und Paketierung kommen als zusätzlicher Aufwand hinzu.
- vcpkg benötigt erreichbare Paketquellen; der Offline-Smoke-Test deckt nur den Kern ab.

## Neubewertung

Die Entscheidung wird nach M2 anhand gemessener Profile, API-Erfahrungen und des Bedarfs externer Modellierer überprüft. Ein alternativer Kern ist nur sinnvoll, wenn ein repräsentativer Prototyp Korrektheit, Reproduzierbarkeit und Performance nachweislich verbessert.
