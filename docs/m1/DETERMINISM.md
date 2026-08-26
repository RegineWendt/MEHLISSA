<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Determinismusnachweis M1.6

## Zweck

Der Test `mehlissa_cross_platform_determinism` belegt, dass die
deterministischen M1-Kernprimitiven unter MSVC, GCC und Clang dasselbe
bytegenaue Ergebnis erzeugen. Alle Plattformen vergleichen sich mit derselben
Golden-Datei:

`tests/data/determinism/reference-v1.json`

Ihr SHA-256 lautet:

`49d405197ff73691289321aabfaecbe73451df2da14a6f44a63fc6c52961bef2`

## Fixierter Lauf

| Größe | Vertrag |
|---|---|
| Referenz | `core-rng-clock-v1` |
| Master-Seed | `0x6a09e667f3bcc909` / `7640891576956012809` |
| Lifecycle | `ComponentHost`: initialize, 16 advances, finalize |
| Schrittweite | 62.500.000 ns |
| Endzeit | 1.000.000.000 ns |
| Stream `circulation` | zwei rohe Ziehungen je Schritt, insgesamt 32 |
| Stream `sensor-noise` | eine rohe Ziehung je Schritt, insgesamt 16 |
| Signatur | FNV-1a-64 über Big-Endian-Bytes der Rohwerte |
| Dateivergleich | SHA-256 über kanonische, binär geschriebene JSON-Datei |

Der Test verwendet bewusst rohe Ganzzahlwerte des Generators und keine
Fließkommaverteilung. Standardbibliotheks-Verteilungen können zwischen
Implementierungen unterschiedliche Algorithmen verwenden. Fachmodelle müssen
deshalb später entweder eigene deterministische Transformationen oder explizite
statistische Toleranzverträge erhalten.

## Ausführen

Der Nachweis ist Teil jedes normalen CTest-Laufs:

```powershell
ctest --preset windows-msvc-debug -R mehlissa_cross_platform_determinism -V
```

Die erzeugte temporäre Datei wird nach dem Vergleich entfernt. Eine Änderung
der Golden-Datei ist eine Vertragsänderung und muss zusammen mit einer neuen
Referenzversion und Begründung geprüft werden.
