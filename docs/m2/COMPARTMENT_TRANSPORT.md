<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Deterministischer Kompartimenttransport

## Zweck und Modellgrenze

`CompartmentTransport` ist die erste ausführbare Transportinterpretation des
validierten Gefäßgraphen. Sie soll früh die entscheidenden Softwareinvarianten
beweisen, noch ohne eine räumlich aufgelöste laminare Strömung oder eine
physiologisch validierte 95-Segment-Parametrisierung vorzutäuschen.

Jedes Segment ist ein gerichtetes Transitkompartiment. Mobile Entitäten werden
weiter einzeln geführt, damit ihre Identität später über Körper-, Organ-,
Kapillar- und Zellebene erhalten bleiben kann. Die aktuelle Komponente kennt
noch keinen biologischen Entitätstyp; sie transportiert ausschließlich stabile
numerische IDs.

## Zeit- und Übergangsmodell

Für Segment `i` gilt die nominale Transitzeit

`t_i = Länge_i / mittlere Geschwindigkeit_i`.

Sie wird beim Aufbau in ganzzahlige Nanosekunden umgerechnet und konservativ
aufgerundet. Ein `advance(delta)` darf höchstens so lang sein wie die kürzeste
Segmenttransitzeit des geladenen Graphen. Dadurch kann keine Entität innerhalb
eines Aufrufs zwei Kanten passieren. Alle fälligen Übergänge werden zunächst
in einem getrennten Puffer gesammelt und erst danach übernommen. Die
Reihenfolge der Segmente im JSON kann somit keine Kaskadenbewegung im selben
Simulationszeitpunkt verursachen.

An einer Verzweigung zieht jede fällige Entität genau eine 64-Bit-Zahl aus dem
benannten Strom `body.compartment-transport.transitions`. Die oberen 53 Bit
werden in das exakt darstellbare Raster `[0, 1)` abgebildet und gegen die
kumulierten Wahrscheinlichkeiten geprüft. Ein Segment mit nur einem Nachfolger
verbraucht keine Zufallszahl. Dieses Verfahren vermeidet die zwischen
Standardbibliotheken nicht zugesicherte Reproduzierbarkeit von
`std::discrete_distribution`.

## Injektionen

Injektionen sind vorab terminierte Ereignisse mit

- Simulationszeit in Nanosekunden,
- Startsegment-ID und
- positiver Anzahl neuer Entitäten.

Ereignisse bei Zeit null werden während `initialize()` ausgeführt. Spätere
Ereignisse werden in dem Zeitschritt aktiviert, dessen geschlossenes Ende ihre
Zeit erreicht. Bei gleicher Zeit bleibt die Eingabereihenfolge stabil. IDs
werden ab eins streng monoton vergeben.

## Automatisch geprüfte Invarianten

- Injektionszeiten sind nicht negativ, Anzahlen positiv und Startsegmente
  vorhanden.
- Jeder Zeitschritt ist positiv und überschreitet die sichere Obergrenze nicht.
- Eine Entität wechselt pro `advance()` höchstens einmal das Segment.
- Ein Übergang verändert weder Identität noch Gesamtzahl der Entitäten.
- Summe aller Segmentpopulationen entspricht stets der Zahl injizierter
  Entitäten.
- Gleicher Seed und gleiche Eingabe ergeben identische Orte, Restzeiten,
  Populationen, Übergangszähler und Zufallsstrom-Zähler.
- Andere Seeds können an echten Verzweigungen andere Wege erzeugen.

Die Nachweise liegen in `tests/compartment_transport_tests.cpp`.

## Bewusste Grenzen von M2.3

- Die Transitzeit ist für alle Entitäten eines Segments gleich; es gibt noch
  kein radiales Geschwindigkeitsprofil.
- Es gibt noch keine Entnahme, Messorte, begrenzte Trajektorienausgabe oder
  Checkpoint-Serialisierung des Transportzustands. Diese Punkte gehören zu
  M2.5 beziehungsweise zur späteren Experimentintegration.
- Zeitabhängige Flüsse, Gefäßcompliance, Pulsatilität und physiologische
  Zustände sind noch nicht modelliert.
- Das synthetische Vier-Segment-Modell ist ein Softwaretest und keine
  physiologische Evidenz.
- Fachliche Verteilungs- und Gleichgewichtsregressionen beginnen mit M2.4.
