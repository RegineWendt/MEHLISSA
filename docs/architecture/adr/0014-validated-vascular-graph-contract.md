<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0014: Validierter Gefäßgraphvertrag

- **Status:** Accepted
- **Datum:** 27. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M2; `BODY-001`, `BODY-002`, `BODY-005`, `BODY-006`, `DATA-001`

## Kontext

Der Legacy-Kreislauf leitet Kanten durch exakte Koordinatengleichheit ab und
ergänzt Durchmesser, Geschwindigkeit und fehlende Übergänge implizit im Code.
Damit sind Daten, Annahmen und Algorithmen nicht trennbar. Ungültige oder
unvollständige Netze werden zudem erst während der Simulation sichtbar.

Die neue Körperebene benötigt einen eigenständigen Datenvertrag, der
alternative Modelle ohne Rebuild lädt und physikalische Inkonsistenzen vor dem
Simulationsstart erkennt. Er gehört fachlich in `models/body`, nicht in den
szenarioneutralen Kern.

## Entscheidung

1. Gefäßmodelle verwenden JSON Schema `vascular-graph/1.0.0` und explizite,
   nicht zwingend fortlaufende Zeichenketten-IDs.
2. Version 1.0.0 beschreibt ausschließlich geschlossene Kreisläufe. Jeder
   Graph muss eine einzige starke Zusammenhangskomponente bilden und jedes
   Segment mindestens einen expliziten Nachfolger besitzen.
3. Koordinaten, Länge, Durchmesser, Querschnitt, Volumen, Fluss und mittlere
   Geschwindigkeit sind Pflichtfelder mit im Namen fixierten SI-Einheiten.
4. Der semantische Validator prüft zusätzlich zum Schema:
   - eindeutige Segment-, Nachfolger- und Quellen-IDs;
   - geometrische Kontinuität jeder Kante;
   - euklidische Länge, Kreisquerschnitt und Zylindervolumen;
   - `Fluss = Querschnitt × mittlere Geschwindigkeit`;
   - Flusserhaltung an jeder geometrischen Verzweigung und Zusammenführung;
   - auf eins normierte Übergänge und Übereinstimmung ihrer Anteile mit den
     Flüssen der Nachfolger.
5. Modellgültigkeit, Quellenzitat und -lizenz, Evidenzqualität sowie relative
   Unsicherheit sind Datenfelder und keine freie Begleitnotiz.
6. Kontrollierte fachliche Datenfehler verwenden `MEHLISSA-E2005`.
7. Der erste Referenzgraph ist bewusst synthetisch und CC BY 4.0. Er enthält
   vier Segmente, eine Verzweigung, eine Zusammenführung und nicht fortlaufende
   IDs, trägt aber keine physiologische Aussage.
8. Der 95er-Legacy-Datensatz wird wegen der ungeklärten Rechtekette nicht in
   den Next-Datenbereich kopiert. Seine Migration ist außerdem fachlich
   blockiert, bis insbesondere der fehlende Übergang von Gefäß 9 und belastbare
   Flussannahmen dokumentiert sind.

## Folgen

Positiv:

- Ein Modell kann nicht mehr allein durch zufällig passende Koordinaten eine
  Kante erhalten.
- Einheiten- und Flussfehler werden vor Beginn eines Experiments abgelehnt.
- Quellen und Annahmen reisen mit dem Modell und können später in Provenienz
  und Modellkarten übernommen werden.
- Synthetische Tests bleiben rechtlich und wissenschaftlich klar von
  historischen Daten getrennt.

Negativ und Grenzen:

- Der strikte Vertrag verlangt Parameter, die der Legacy-Datensatz nicht
  enthält; eine scheinbar schnelle 1:1-Konvertierung ist deshalb nicht möglich.
- Kreisquerschnitt und gerades Zylindersegment sind M2-Abstraktionen. Spätere
  1D-/CFD-Modelle benötigen eigene Geometrie- und Strömungsverträge.
- Exakte Massenerhaltung ist für stationäre Referenzparametersätze sinnvoll;
  zeitabhängige Compliance und Speicherung erfordern eine neue Modellversion.
- Relative Unsicherheit wird zunächst dokumentiert, aber noch nicht propagiert.

## Alternativen

- **CSV plus implizite Regeln:** abgelehnt, weil Schema, Quellen und Topologie
  nicht gemeinsam validierbar wären.
- **Kanten weiter aus Koordinaten ableiten:** abgelehnt, weil Rundung und
  unbeabsichtigte Punktgleichheit die Topologie verändern können.
- **Fehlende Flüsse optional lassen:** abgelehnt für den stationären
  M2-Referenzvertrag, weil dann Übergänge und Massenerhaltung nicht prüfbar
  wären.
- **Legacywerte mit Defaults auffüllen:** abgelehnt, weil erfundene
  Physiologie als Mess- oder Literaturdaten erscheinen könnte.
