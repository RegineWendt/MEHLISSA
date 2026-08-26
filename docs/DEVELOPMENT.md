<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Next entwickeln

Dieses Dokument beschreibt den reproduzierbaren Build der neuen MEHLISSA-Generation. Die historischen Implementierungen in `mehlissa/` und `mehlissa2.0/` werden davon nicht verändert.

## Voraussetzungen

- CMake 3.28 oder neuer
- C++20-Compiler
- vcpkg mit gesetztem `VCPKG_ROOT`
- unter Linux: Ninja

Unter Windows ist Visual Studio Community 2026 mit dem Workload „Desktopentwicklung mit C++“ die Referenzumgebung. Die Visual-Studio-Developer-PowerShell setzt `VCPKG_ROOT` und die MSVC-Umgebung automatisch.

## Windows mit MSVC

In „Developer PowerShell for VS 2026“ aus dem Repository-Root:

```powershell
$cmake = Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'

& $cmake --preset windows-msvc
& $cmake --build --preset windows-msvc-debug
& $ctest --preset windows-msvc-debug
```

Der Release-Build verwendet entsprechend die Presets `windows-msvc-release`.

Falls vcpkg beziehungsweise das Paketregister vorübergehend nicht erreichbar ist, steht ein vollständig offline ausführbarer Smoke-Test zur Verfügung:

```powershell
& $cmake --preset windows-msvc-smoke
& $cmake --build --preset windows-msvc-smoke
& $ctest --preset windows-msvc-smoke
```

Dieser Preset prüft den Kern ohne externe Testbibliothek. Er ersetzt nicht die vollständige Catch2-Suite.

## Linux mit GCC

```bash
cmake --preset linux-gcc
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

## Analyse-Build mit Clang

Der Preset `linux-clang-analysis` aktiviert clang-tidy, AddressSanitizer und UndefinedBehaviorSanitizer:

```bash
cmake --preset linux-clang-analysis
cmake --build --preset linux-clang-analysis
ctest --preset linux-clang-analysis
```

## Abhängigkeiten

Alle direkten C++-Abhängigkeiten stehen in `vcpkg.json`. Der dort eingetragene `builtin-baseline` fixiert den getesteten vcpkg-Portstand. Abhängigkeiten werden nicht global und nicht manuell in das Repository kopiert.

Der Bootstrap verwendet Catch2 für Unit-Tests. Ein kleiner frameworkfreier CTest-Smoke-Test hält die Kerninvarianten zusätzlich offline prüfbar. Weitere Bibliotheken werden erst nach einer begründeten Architekturentscheidung ergänzt.

`jsoncons` validiert Experimentdateien gegen das versionierte JSON Schema. Die
Abhängigkeit ist bewusst nicht Teil des Offline-Smoke-Presets; dieser baut nur
den allgemeinen Kern ohne Apps und Experiment-I/O.

`PicoSHA2` berechnet die SHA-256-Prüfsumme des eingelesenen
Experimentmanifests für die Laufprovenienz. Die kleine Header-only-Abhängigkeit
ist wie `jsoncons` über den fixierten vcpkg-Baseline-Stand reproduzierbar.

## Qualitätsregeln

- Builds erfolgen ausschließlich außerhalb der Quellverzeichnisse unter `build/`.
- Der Build verwendet C++20 ohne Compilererweiterungen.
- Die CI behandelt Compilerwarnungen als Fehler.
- `clang-format` definiert die Formatierung; `clang-tidy` ergänzt statische Analyse.
- Tests werden über CTest ausgeführt.
- Gleicher Experiment-Seed und gleicher Streamname müssen dieselbe rohe Zufallsfolge erzeugen.
- Simulationszeit wird ganzzahlig in Nanosekunden geführt und darf nur streng monoton fortschreiten.
- Modellgrößen verwenden die dimensionssicheren SI-Typen und benannte
  Konvertierungen aus `core/quantity.hpp`; nackte `double`-Werte sind keine
  öffentliche Einheiten-API.
- Neue unabhängig entwickelte Dateien tragen `SPDX-License-Identifier: MPL-2.0`.
- Direkte Legacy-Portierungen bleiben `GPL-2.0-only` und werden in getrennten
  Dateien mit erhaltener Urheber- und Provenienzangabe umgesetzt.
- Neue Projektdokumentation und freigegebene eigene Daten verwenden
  `CC-BY-4.0`; Daten benötigen zusätzlich ein Provenienzmanifest.

## Aktueller Bootstrap

Der aktuelle Kern enthält bewusst nur überprüfbare technische Grundlagen:

- monotone Simulationszeit mit Nanosekundenauflösung und Überlaufschutz;
- dimensionssichere SI-Größen und dreidimensionale euklidische Geometrie;
- benannte, reproduzierbare Zufallsströme;
- versionierte Experimentvalidierung und automatische Laufprovenienz.

Medizinische Szenarien und Legacy-Zustände gehören nicht in diesen Kern.
