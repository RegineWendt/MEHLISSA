<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Einheitensystem des MEHLISSA-Kerns

MEHLISSA Next rechnet intern in SI und unterscheidet physikalische Dimensionen
bereits im C++-Typsystem. Ein Wert ohne sichtbare Einheit soll nur an einer
eng begrenzten Serialisierungs- oder Mathematikgrenze vorkommen.

## Typen und kanonische Einheiten

| C++-Typ | Dimension | interner SI-Wert | benannte Eingaben |
|---|---:|---|---|
| `Length` | L | Meter | `meters`, `millimeters`, `micrometers` |
| `Time` | T | Sekunde | `seconds`, `milliseconds`, `minutes` |
| `Area` | L² | Quadratmeter | Ableitung aus `Length * Length`, `square_meters` |
| `Volume` | L³ | Kubikmeter | `cubic_meters`, `liters`, `milliliters` |
| `Speed` | L/T | Meter pro Sekunde | Ableitung aus `Length / Time`, `meters_per_second`, `millimeters_per_second` |
| `Amount` | N | Mol | `moles`, `millimoles`, `micromoles` |
| `Concentration` | N/L³ | Mol pro Kubikmeter | Ableitung aus `Amount / Volume`, `moles_per_cubic_meter`, `millimoles_per_liter` |

`1 mmol/l` entspricht exakt `1 mol/m³`. Die Ganzzahl-Nanosekunden der
`SimulationClock` bleiben davon getrennt, damit Ereignisse ohne Rundungsdrift
geordnet werden.

## Verbindliche Regeln

- Keine implizite Konvertierung von oder zu `double`.
- Addition und Subtraktion nur bei gleicher Dimension.
- Multiplikation und Division leiten die Dimension des Ergebnisses ab.
- Öffentliche Modell-APIs verwenden Größen-Typen, keine Einheitensuffixe an
  nackten Zahlen.
- JSON- und Datenformate nennen Einheit und Wert getrennt und konvertieren beim
  Dekodieren genau einmal in SI.
- Modelle prüfen Endlichkeit, Vorzeichen und fachliche Wertebereiche an ihrer
  Eingabegrenze; Dimensionssicherheit ersetzt diese Validierung nicht.

## Nachweis

`tests/quantity_tests.cpp` enthält Laufzeit- und Compile-Zeit-Tests für alle
M1-Größen. Die Geometrietests verwenden die migrierte `Position3D`-API und der
abhängigkeitenfreie Smoke-Test prüft sie zusätzlich.
