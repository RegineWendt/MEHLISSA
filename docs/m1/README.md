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
| M1.1 versioniertes Experimentmanifest | abgeschlossen | Schema `1.0.0`, Parser, Validator, CLI- und Negativtests; Plattform-CI |
| M1.2 Provenienzmanifest | abgeschlossen | Schema `1.0.0`, automatische Erzeugung, SHA-256- und Vertragstests; Plattform-CI |
| M1.3 dimensionssicheres Einheitensystem | abgeschlossen | SI-Vertrag, migrierte 3D-Geometrie, Compile- und Unit-Tests; Plattform-CI |
| M1.4 `SimulationContext` und Komponentenlebenszyklus | abgeschlossen | kontextgebundene Uhr/RNGs, eindeutiges Ownership, Lifecycle- und Fehlerpfadtests; Plattform-CI |
| M1.5 strukturierte Fehler, Logging und Checkpointvertrag | umgesetzt, CI-Prüfung ausstehend | stabile Fehlercodes, JSONL-Schema, Checkpoint-Schema und Manipulationstests |
| M1.6 plattformübergreifender Determinismusnachweis | offen | identischer Referenzlauf auf Windows/Linux |

Der verbindliche Größen- und Konversionskatalog für M1.3 steht in
[`UNITS.md`](UNITS.md).
Der M1.4-Vertrag ist in [`COMPONENT_LIFECYCLE.md`](COMPONENT_LIFECYCLE.md)
dokumentiert.
Fehlerkennungen, Laufprotokoll und Checkpointformat beschreibt
[`ERRORS_LOGS_CHECKPOINTS.md`](ERRORS_LOGS_CHECKPOINTS.md).

## M1.1 benutzen

Vom Repository-Root:

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe validate `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json

build/windows-msvc/Debug/apps/mehlissa.exe run `
  --experiment examples/experiments/minimal.json `
  --schema data/schemas/experiment/1.0.0.schema.json `
  --checkpoint-schema data/schemas/checkpoint/1.0.0.schema.json
```

Ohne `--schema` verwendet die CLI relativ zum aktuellen Arbeitsverzeichnis
`data/schemas/experiment/1.0.0.schema.json`.

Das aktuelle Minimalexperiment enthält noch keine medizinischen Modelle. Der
`run`-Befehl validiert den Vertrag, führt die Simulationsuhr deterministisch bis
zur konfigurierten Dauer und schreibt anschließend
`<outputs.directory>/provenance.json`. Das Provenienzmanifest enthält unter
anderem den SHA-256-Hash der Experimentdatei, Seed, Git- und Buildzustand,
Compiler, Plattform, Zeitstempel und erreichte Simulationszeit. Sein Vertrag
liegt unter `data/schemas/provenance/1.0.0.schema.json`.

Zusätzlich entstehen `run.log.jsonl` und `checkpoint-000000.json`. Der
Checkpoint des Minimalversuchs enthält noch keine fachlichen
Komponenten-Snapshots, prüft aber bereits Experimentbindung, Zeit, Seed und
Zufallsstromzähler nach dem versionierten Vertrag.
