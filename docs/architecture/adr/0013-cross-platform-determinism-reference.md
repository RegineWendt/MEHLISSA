<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0013: Plattformübergreifender Determinismusnachweis

- **Status:** Accepted
- **Datum:** 27. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M1; `SYS-001`, `SYS-002`, `SYS-003`, `QUA-001`, `QUA-002`

## Kontext

Erfolgreiche Tests auf mehreren Plattformen belegen noch nicht, dass derselbe
Simulationslauf dort dasselbe Ergebnis erzeugt. Ein Vergleich von Provenienz,
Logs oder Checkpoints als vollständige Dateien wäre ebenfalls ungeeignet, weil
sie absichtlich Zeitstempel, Compiler- und Plattformangaben enthalten.

Der M1-Kern benötigt deshalb einen kleinen Referenzlauf, der ausschließlich
deterministische Zustandsgrößen abbildet und jede unterstützte Toolchain gegen
dasselbe Ergebnis prüft.

## Entscheidung

1. `mehlissa_determinism_reference` läuft über den regulären
   `ComponentHost`-Lebenszyklus, nicht über einen privilegierten Testzugriff.
2. Der Lauf verwendet einen fixierten Master-Seed, 16 Schritte zu je
   62.500.000 ns und die benannten Ströme `circulation` und `sensor-noise`.
3. Pro Schritt werden in definierter Reihenfolge zwei beziehungsweise ein rohe
   `std::mt19937_64`-Werte gezogen. Verteilungen für Fließkommawerte sind nicht
   Teil dieses Vertrags.
4. Jeder 64-Bit-Wert wird in Big-Endian-Bytefolge in FNV-1a-64 eingespeist. Die
   Einzel- und Gesamtdigests dienen als kompakte Regressionssignatur, nicht als
   kryptografischer Integritätsschutz.
5. Das Ergebnis wird in Binärmodus als kanonische UTF-8/ASCII-JSON-Datei
   geschrieben. Dadurch entstehen unter Windows keine abweichenden CRLF-Bytes.
6. CTest vergleicht den SHA-256 der erzeugten Datei bytegenau mit
   `tests/data/determinism/reference-v1.json`.
7. Derselbe Test läuft in der Windows-MSVC-, Linux-GCC- und
   Linux-Clang/ASan/UBSan-CI. Gleichheit mit derselben Referenz ist der
   transitive Nachweis der Plattformgleichheit.
8. Jede beabsichtigte Änderung der Referenz erfordert eine neue Format- oder
   Referenzversion und eine dokumentierte Begründung; die Golden-Datei darf
   nicht stillschweigend aktualisiert werden.

## Folgen

Positiv:

- Uhr, Seedableitung, Streamnamen, Ziehungsreihenfolge und Komponentenpfad
  werden gemeinsam gegen eine bytegenaue Referenz geprüft.
- Plattformabweichungen erscheinen als normale CTest-Fehler und blockieren CI.
- Beobachtungsmetadaten bleiben vom deterministischen Ergebnis getrennt.

Negativ und Grenzen:

- Der Nachweis umfasst die M1-Kernprimitiven, noch keine medizinischen Modelle.
- FNV-1a komprimiert den Verlauf und ist nicht kollisionssicher; der äußere
  SHA-256 schützt die vollständige Referenzdatei, ersetzt aber keine fachlichen
  Invarianten.
- Plattformidentische Fließkomma- und Verteilungsalgorithmen werden nicht
  behauptet. Sie benötigen vor ihrer Verwendung eigene Verträge und
  Toleranzklassen.
- Replikatplanung und statistische Reproduzierbarkeit bleiben spätere Teile von
  `SYS-002`.

## Alternativen

- **Nur bekannte Einzelwerte testen:** abgelehnt, weil Uhr, Lifecycle und
  Ziehungsreihenfolge nicht gemeinsam erfasst würden.
- **Provenienz oder Logdatei vergleichen:** abgelehnt, weil beobachtende
  Metadaten absichtlich variieren.
- **Nur Hashwerte im Quellcode prüfen:** abgelehnt, weil eine eigenständige
  Referenzdatei besser prüf-, archiv- und versionierbar ist.
- **Fließkommaverteilungen sofort aufnehmen:** vertagt, bis Rundung,
  Mathematikbibliothek und erlaubte Toleranzen ausdrücklich festgelegt sind.
