<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Audit des 95er-Legacy-Gefäßdatensatzes

**Stand:** 27. August 2026
**Quellen im Repository:** `mehlissa2.0/data/95_vasculature.csv`,
`95_transitions.csv`, `95_fingerprints.csv` und der zugehörige
`BloodCircuit`-Parser

## Reproduzierbare Strukturprüfung

| Merkmal | Befund |
|---|---:|
| Datensätze | 95 |
| eindeutige IDs / Bereich | 95 / 1–95 |
| Arterien / Venen / Organübergänge | 36 / 34 / 25 |
| gerichtete Kanten | 119 |
| Verzweigungen mit zwei Nachfolgern | 24 |
| Sackgassen / Knoten mit mehr als zwei Nachfolgern | 0 / 0 |
| von Gefäß 1 erreichbar | 95 |
| Gefäße, die Gefäß 1 wieder erreichen | 95 |
| explizite Wahrscheinlichkeitszeilen | 23 |
| maximale Abweichung der Wahrscheinlichkeitssumme von 1 | 0 |
| Fingerprint-Zeitdatensätze | 9 |

Der koordinatenbasierte Legacy-Algorithmus erzeugt damit einen stark
zusammenhängenden gerichteten Graphen. Die 24 Verzweigungs-IDs sind:

`1, 2, 3, 4, 5, 8, 9, 10, 12, 13, 14, 16, 19, 22, 26, 31, 34, 38, 41, 42, 44, 46, 48, 50`.

## Bestätigte Auffälligkeiten

1. Für Gefäß 9 fehlen Übergangswahrscheinlichkeiten, obwohl die
   Koordinatenlogik zwei Nachfolger (`81` und `83`) erzeugt. Der Legacy-Code
   verwendet dadurch implizit seinen Standard `1/0`; die physiologisch
   richtige Aufteilung ist ungeklärt und darf nicht erfunden werden.
2. Alle Start- und Endpunkte liegen auf nur zwei Ebenen, `z = -2` und `z = +2`.
   Das ist eine schematische Topologie, keine anatomische 3D-Geometrie.
3. Jede Gefäßzeile endet mit einem Trennzeichen. Das ergibt beim strikten Split
   zehn Felder; die neunte, vom Parser noch gelesene Spalte ist immer `0`, das
   zehnte Feld leer. Beide haben keinen dokumentierten fachlichen Gehalt.
4. Der Parser nutzt `std::stoi` für sämtliche Koordinaten und würde
   Dezimalstellen zukünftiger Dateien verlieren beziehungsweise ablehnen.
5. Sein `errorflag` wird nach einer unvollständigen Zeile nicht zurückgesetzt;
   alle folgenden ansonsten gültigen Zeilen würden verworfen.
6. Kanten entstehen ausschließlich durch exakte Koordinatengleichheit. Quellen,
   Einheiten, Toleranzen und beabsichtigte Topologie sind nicht gespeichert.
7. Breite und Typgeschwindigkeit sind im Code pauschal festgelegt; Druck,
   Flussbilanz, Pulsatilität und physiologische Zustände fehlen.

## Migrationsentscheidung

M2 übernimmt keine dieser Annahmen stillschweigend:

- Kanten und Wahrscheinlichkeiten werden explizit gespeichert.
- Schematische Koordinaten werden als solche gekennzeichnet und mit einer
  dokumentierten Einheit transformiert.
- Physikalische Parameter erhalten Quelle, Evidenzqualität und Unsicherheit.
- Fehlende Werte bleiben als offene Datenlücke sichtbar; insbesondere wird für
  Gefäß 9 keine Wahrscheinlichkeit geraten.
- Der neue Loader unterstützt nicht fortlaufende IDs und lehnt unvollständige
  Graphen vor dem Simulationsstart ab.

## Lizenz- und Provenienzgrenze

ADR-0007 erlaubt keine rückwirkende CC-BY-Umlizenzierung der bestehenden
Dateien. Deshalb bleibt der Legacy-Datensatz an seinem bisherigen Ort und wird
nicht in ein öffentliches Next-Datenpaket kopiert. M2.1 verwendet zunächst
einen unabhängig erzeugten synthetischen CC-BY-Referenzgraphen.

Die vollständige 95er-Migration wird erst eingecheckt, wenn Urheberschaft,
Datenquelle und Freigabe geklärt sind. Ein Konverter kann zuvor technisch
entwickelt werden; sein Ergebnis ist bis dahin kein freigegebener Next-Datensatz.
