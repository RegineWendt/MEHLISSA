<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0012: Strukturierte Fehler, Laufprotokolle und Checkpoints

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M1; `SYS-002`, `SYS-007`, `DATA-001`, `DATA-003`, `QUA-005`

## Kontext

Freie Fehlermeldungen und wechselnde Exit-Codes sind für automatisierte
Experimente, CI und spätere Python-Werkzeuge nicht zuverlässig auswertbar.
Gleichzeitig müssen lange Simulationsläufe ihren Verlauf maschinenlesbar
dokumentieren und einen definierten Wiederaufnahmepunkt hinterlassen können.

Ein Checkpoint ist nur belastbar, wenn er eindeutig an Experiment, Zeit, Seed,
Zufallszustand und versionierte Komponentenstände gebunden ist. Ein bloßes
Speicherabbild wäre plattformabhängig und könnte Änderungen unbemerkt als
kompatibel behandeln.

## Entscheidung

1. Kontrollierte Fehler erben von `MehlissaError` und tragen einen stabilen
   numerischen `ErrorCode` sowie eine Kennung `MEHLISSA-Edddd`.
2. Vergebene Kennungen werden nicht umgedeutet. Neue Ursachen erhalten eine
   neue Kennung; die freie Diagnose darf präzisiert werden.
3. Die CLI schreibt die Kennung nach `stderr` und ordnet Fehlergruppen stabilen
   Exit-Statuswerten zu.
4. Jeder Lauf schreibt `run.log.jsonl`. Jede Zeile ist ein unabhängiges
   JSON-Dokument nach dem versionierten Schema `1.0.0` und enthält Sequenz,
   UTC-Zeit, Simulationszeit, Level, Quelle, Ereignis und Nachricht.
5. Fehlerdatensätze enthalten zusätzlich numerischen Code und Kennung. Falls
   das Schreiben einer Fehlermeldung selbst scheitert, bleibt der ursprüngliche
   Fehler maßgeblich.
6. Checkpoints verwenden ein versioniertes JSON-Manifest statt eines rohen
   Speicherabbilds. Version `1.0.0` bindet Experiment-Hash, Softwareversion,
   Sequenz, Simulationszeit, Master-Seed und Ziehungsstände benannter
   Zufallsströme.
7. Komponentenstände liegen in eigenen, relativ zum Checkpointverzeichnis
   adressierten Dateien. Jede Referenz nennt Komponentenname,
   Zustandsschemaversion und SHA-256-Prüfsumme.
8. Schreiben und Laden validieren das Manifest gegen JSON Schema, verbieten
   ausbrechende Komponentenpfade, prüfen eindeutige Namen und verifizieren die
   referenzierten Prüfsummen.
9. Der Minimalversuch schreibt nach erfolgreichem Lauf einen finalen
   `checkpoint-000000.json`. Die eigentliche Wiederaufnahme folgt, sobald
   zustandsbehaftete M2-Komponenten einen Snapshotvertrag implementieren.

## Folgen

Positiv:

- Skripte können Ursache und Exit-Status unabhängig vom Wortlaut auswerten.
- JSONL bleibt auch bei großen Protokollen zeilenweise verarbeitbar und nach
  jedem Datensatz gespült.
- Checkpoints sind selbstbeschreibend, versionierbar und gegen vertauschte oder
  nachträglich veränderte Komponentenstände geschützt.
- RNG-Ziehungsstände erlauben eine algorithmisch definierte Rekonstruktion,
  ohne plattformspezifisches Speicherlayout zu serialisieren.

Negativ:

- UTC-Zeitstempel unterscheiden sich zwischen ansonsten identischen Läufen;
  deterministische Vergleiche müssen Beobachtungsfelder ausnehmen.
- Das Flushen jeder Logzeile kostet I/O-Leistung. Eine gepufferte Variante darf
  später nur mit expliziter Verlusttoleranz ergänzt werden.
- M1 spezifiziert und prüft den Checkpoint, führt aber noch keine vollständige
  Wiederaufnahme fachlicher Komponenten aus.
- Eine Rekonstruktion sehr weit fortgeschrittener RNG-Ströme allein über den
  Zähler kann teuer werden; ein späteres portables Engine-State-Format benötigt
  eine eigene Version und Vergleichstests.

## Alternativen

- **Nur Textmeldungen:** abgelehnt, weil Kennung und Felder nicht stabil
  maschinenlesbar sind.
- **Ein großes JSON-Log:** abgelehnt, weil ein Abbruch das Gesamtdokument
  ungültig hinterlassen kann und Streaming erschwert.
- **Binäres Speicherabbild:** abgelehnt, weil Layout, Endianness und
  Softwarekompatibilität implizit blieben.
- **Komponentenstatus direkt im Manifest:** abgelehnt, weil große oder binäre
  Zustände das Kontrollmanifest aufblähen und nicht unabhängig versionierbar
  wären.
- **Absolute Snapshotpfade:** abgelehnt, weil Checkpoints dann nicht portabel
  verschoben oder archiviert werden könnten.
