<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0009: Maschinenlesbare Laufprovenienz

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M1; `DATA-003`, `SYS-002`, `SYS-007`, `UX-001`

## Kontext

Ein identischer Seed allein macht einen Simulationslauf nicht reproduzierbar.
Experimentdatei, Softwarestand, nicht eingecheckte Änderungen, Build,
Compiler, Plattform und erreichte Simulationszeit können das Ergebnis ebenso
beeinflussen. Diese Angaben dürfen nicht von manueller Dokumentation abhängen
und müssen später automatisiert verglichen und archiviert werden können.

## Entscheidung

1. Jeder erfolgreich abgeschlossene CLI-Lauf schreibt automatisch
   `<outputs.directory>/provenance.json`.
2. Das Dokument folgt einem strikten, semantisch versionierten JSON Schema
   nach Draft 2020-12. Die erste Version ist `1.0.0`.
3. Die Provenienz identifiziert Experiment und Schemaversion und speichert den
   Pfad sowie den SHA-256-Hash der tatsächlich eingelesenen Manifestdatei.
4. Sie erfasst MEHLISSA-Version, Git-Commit, Dirty-Status, Buildtyp,
   Compiler-ID und -Version sowie Betriebssystem und Architektur.
5. Der Master-Seed, UTC-Start- und Endzeit, Laufstatus und erreichte
   Simulationszeit in ganzzahligen Nanosekunden werden aufgezeichnet.
6. Git- und Buildinformationen werden beim Konfigurieren beziehungsweise
   Kompilieren eingebettet. Ein Build außerhalb eines Git-Worktrees verwendet
   ausdrücklich `unknown` statt erfundener Werte.
7. PicoSHA2 berechnet die Prüfsumme als kleine, geprüfte Header-only-Abhängigkeit
   über den fixierten vcpkg-Baseline-Stand.
8. Versions- und Prüfsummen realer Modelle und Datensätze erweitern denselben
   Vertrag, sobald diese in M2 eingebunden werden.

## Folgen

Positiv:

- Ein Ergebnis kann automatisiert seinem Eingabemanifest und Softwarestand
  zugeordnet werden.
- Dirty-Builds werden sichtbar und nicht still als veröffentlichter Commit
  ausgegeben.
- Das versionierte Schema bildet einen stabilen Vertrag für spätere
  Ergebnisarchive, Vergleichswerkzeuge und die Python-API.
- Standardvektor- und Schematests prüfen Hashberechnung und Dokumentstruktur.

Negativ:

- Der konfigurierte Git-Zustand kann veralten, wenn Quellen verändert werden,
  ohne CMake erneut auszuführen; die normalen Builds führen die
  CMake-Prüfung bei geänderten Eingaben erneut aus.
- UTC-Zeitstempel sind Beobachtungsdaten und daher zwischen ansonsten
  identischen Läufen verschieden.
- Die erste Version deckt noch keine externen Daten- oder Modellkataloge ab.

## Alternativen

- **Nur menschenlesbares Log:** abgelehnt, weil Struktur, Pflichtfelder und
  automatische Validierung fehlen.
- **Hash allein:** abgelehnt, weil er Build- und Plattformunterschiede nicht
  erklärt.
- **Git-Commit ohne Dirty-Status:** abgelehnt, weil ein lokaler Build dann
  fälschlich als exakt reproduzierbarer Commit erscheinen kann.
- **Plattformspezifische Kryptografie-API:** abgelehnt, weil sie den
  portablen Build und identische Prüfsummenpfade erschwert.
- **Provenienz erst beim Export:** abgelehnt, weil Laufkontext bis dahin
  verloren gehen oder falsch zugeordnet werden kann.
