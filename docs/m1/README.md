<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M1 – Trustworthy Kernel

**Status:** in Arbeit  
**Beginn:** 26. August 2026

M1 schafft einen kleinen, reproduzierbaren und getesteten Simulationskern. Die
medizinischen Modelle beginnen erst auf diesem Fundament.

## Arbeitsstand

| Inkrement | Status | Nachweis |
|---|---|---|
| M1.0 C++20/CMake/vcpkg und Plattform-CI | abgeschlossen | MSVC-, GCC- und Clang-Jobs; CTest |
| M1.0 monotone Zeit, 3D-Geometrie, benannte RNG-Ströme | abgeschlossen | Core-Unit- und Smoke-Tests |
| M1.1 versioniertes Experimentmanifest | umgesetzt, CI-Prüfung ausstehend | Schema `1.0.0`, Parser, Validator, CLI- und Negativtests |
| M1.2 Provenienzmanifest | offen | `provenance.json` pro Lauf |
| M1.3 vollständiges Einheitensystem | offen | Compile- und Unit-Tests |
| M1.4 `SimulationContext` und Komponentenlebenszyklus | offen | Lifecycle- und Invariantentests |
| M1.5 strukturierte Fehler, Logging und Checkpointvertrag | offen | Fehler-/Snapshot-Tests |
| M1.6 plattformübergreifender Determinismusnachweis | offen | identischer Referenzlauf auf Windows/Linux |

## M1.1 benutzen

Vom Repository-Root:

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe validate `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json

build/windows-msvc/Debug/apps/mehlissa.exe run `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json
```

Ohne `--schema` verwendet die CLI relativ zum aktuellen Arbeitsverzeichnis
`data/schemas/experiment/1.0.0.schema.json`.

Das aktuelle Minimalexperiment enthält noch keine medizinischen Modelle. Der
`run`-Befehl validiert den Vertrag und führt die Simulationsuhr deterministisch
bis zur konfigurierten Dauer. Provenienz und reale Komponenten folgen in den
nächsten Inkrementen.
