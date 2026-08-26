<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Fehler-, Log- und Checkpointvertrag

## Fehlerkatalog 1.0

| Kennung | Enum | Bedeutung | CLI-Status |
|---|---|---|---:|
| `MEHLISSA-E1001` | `command_line_invalid` | ungültiger CLI-Aufruf | 2 |
| `MEHLISSA-E2001` | `input_unreadable` | Eingabe fehlt oder ist nicht lesbar | 3 |
| `MEHLISSA-E2002` | `json_invalid` | ungültiges JSON oder JSONL | 3 |
| `MEHLISSA-E2003` | `schema_invalid` | das Prüfschema selbst ist ungültig | 3 |
| `MEHLISSA-E2004` | `manifest_invalid` | Experiment verletzt seinen Vertrag | 3 |
| `MEHLISSA-E3001` | `output_unwritable` | Ausgabe kann nicht geschrieben werden | 4 |
| `MEHLISSA-E3002` | `provenance_invalid` | Provenienzdokument verletzt seinen Vertrag | 4 |
| `MEHLISSA-E4001` | `lifecycle_invalid` | unzulässiger Komponenten-/Laufzustand | 5 |
| `MEHLISSA-E4002` | `invariant_violated` | Kern- oder Modellinvariante verletzt | 5 |
| `MEHLISSA-E4003` | `numeric_overflow` | numerischer Zähler oder Zeitbereich überschritten | 5 |
| `MEHLISSA-E5001` | `checkpoint_invalid` | Checkpoint oder referenzierter Zustand ungültig | 6 |
| `MEHLISSA-E5002` | `checkpoint_incompatible` | gültiger, aber nicht kompatibler Checkpoint | 6 |
| `MEHLISSA-E9001` | `internal_failure` | nicht näher klassifizierter interner Fehler | 1 |

Vergebene Kennungen sind öffentliche Maschinenverträge. Der Diagnosetext darf
mehr Kontext erhalten, die Bedeutung einer Kennung jedoch nicht wechseln.

## `run.log.jsonl`

Jede Zeile erfüllt `data/schemas/log-record/1.0.0.schema.json`. Sequenzen
beginnen bei null, sind lückenlos und geben die Schreibreihenfolge wieder.
Simulationszeit wird in ganzzahligen Nanosekunden gespeichert; der UTC-Zeitwert
ist nur Beobachtungsmetadatum.

Der Runner erzeugt mindestens:

1. `run_started` bei Simulationszeit null;
2. `run_completed` nach allen erfolgreichen Ausgaben; oder
3. bestmöglich `run_failed` mit Fehlerkennung.

## `checkpoint-000000.json`

Das Manifest erfüllt `data/schemas/checkpoint/1.0.0.schema.json`. Es enthält
keine großen Modellzustände selbst, sondern referenziert sie relativ und mit
eigener Schemaversion sowie SHA-256-Prüfsumme. Dadurch können Komponenten
unabhängige Formate verwenden, ohne die gemeinsame Hülle zu umgehen.

M1 erzeugt einen finalen Checkpoint des noch komponentenlosen
Minimalexperiments. M2 ergänzt zustandsbehaftete Komponenten-Snapshots und den
Wiederaufnahmebefehl. Bis dahin belegt der Roundtrip-Test bereits Schema,
Pfadgrenze, Namenseindeutigkeit und Manipulationserkennung.
