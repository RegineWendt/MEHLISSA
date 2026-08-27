<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0015: Deterministischer identitätserhaltender Kompartimenttransport

- **Status:** Accepted
- **Datum:** 27. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M2.3; `BODY-005`, `BODY-006`, `BODY-007`, `SYS-003`

## Kontext

Nach dem Gefäßgraphvertrag benötigt MEHLISSA eine erste ausführbare
Transportsemantik. Die Legacy-Implementierung kann eine Entität abhängig von
der Iterationsreihenfolge innerhalb desselben Simulationszeitpunkts mehrfach
weiterreichen. Außerdem vermischt sie Bewegung, harte Gefäßtypen,
Zufallsentscheidungen und medizinische Geräteklassen.

Für die spätere Mehrschichtkopplung müssen sowohl Populationserhaltung als
auch die Identität einzelner Nanogeräte oder seltener Zellen nachweisbar sein.
Gleichzeitig darf der erste Baustein keine noch nicht validierte
Strömungsphysik behaupten.

## Entscheidung

1. Das erste Strömungsmodell ist ein identitätserhaltendes
   Transitkompartimentmodell in `models/body`.
2. Aufenthaltszeiten stammen ausschließlich aus den validierten
   Segmentfeldern `Länge / mittlere Geschwindigkeit` und werden intern als
   ganzzahlige Nanosekunden geführt.
3. Injektionen sind terminierte, validierte Ereignisse. Jede Entität erhält
   eine stabile, streng monotone ID.
4. Übergänge werden zweiphasig ausgeführt: erst bestimmen, dann gemeinsam
   übernehmen. Eine Entität kann daher pro `advance()` höchstens eine Kante
   passieren.
5. Der zulässige Zeitschritt ist auf die kürzeste Segmenttransitzeit begrenzt.
   Das verhindert, dass die Einmalbewegungsregel physikalische Transitzeit
   verwirft; überschüssige Aufenthaltszeit wird ins Nachfolgersegment
   übernommen.
6. Verzweigungen nutzen ausschließlich den benannten Zufallsstrom
   `body.compartment-transport.transitions`. Die Auswahl verwendet ein
   festgelegtes 53-Bit-Raster statt `std::discrete_distribution`.
7. Einzelne Nachfolger sind deterministisch und verbrauchen keine
   Zufallszahl. Damit hängt der Stromverbrauch nur von echten
   Verzweigungsentscheidungen ab.
8. Nach jedem Schritt wird die exakte Populationserhaltung geprüft.

## Folgen

Positiv:

- Iterationsreihenfolge kann keine Mehrfachbewegung mehr auslösen.
- Identität und Population bleiben über Verzweigungen und Zusammenführungen
  erhalten.
- Zufallsentscheidungen sind durch Seed, Streamname und Ziehungszahl
  reproduzierbar und prüfbar.
- Dasselbe Transportmodell funktioniert ohne Rebuild mit jedem Graphen des
  M2.1-Vertrags.
- Spätere Strömungsmodelle können hinter einer eigenen Komponente ergänzt und
  gegen dieselben Invarianten getestet werden.

Negativ und Grenzen:

- Einzelentitäten benötigen mehr Speicher als reine Populationsvektoren. Ein
  populationsbasierter Modus bleibt für Skalierungstests erforderlich.
- Der feste Transit pro Segment bildet weder laminare Profile noch
  Transitzeitstreuung ab.
- Sehr große Simulationsschritte werden bewusst abgelehnt. Ein späterer
  ereignisgetriebener Scheduler kann mehrere physikalische Übergänge innerhalb
  eines äußeren Kopplungsschritts ausführen, muss dabei aber weiterhin
  eindeutige Zeitpunkte garantieren.
- Entnahme, Messorte, Trajektorienbegrenzung und Checkpoints sind nicht Teil
  dieser Entscheidung.

## Alternativen

- **Direktes Portieren der Legacy-Bewegung:** abgelehnt, weil es die
  Iterationsabhängigkeit und medizinische Speziallogik übernehmen würde.
- **Nur Segmentpopulationen ohne IDs:** vorerst abgelehnt, weil
  Mehrschicht-Szenarien die Identität einzelner Geräte und seltener Zellen
  benötigen. Eine aggregierte Implementierung kann später parallel folgen.
- **`std::discrete_distribution`:** abgelehnt, weil deren konkrete Abbildung
  von Engine-Ausgaben auf Ergebnisse nicht plattformübergreifend festgelegt
  ist.
- **Beliebig große Schritte mit mehreren Übergängen:** zurückgestellt, bis ein
  expliziter Ereignisscheduler Reihenfolge und Kopplungszeitpunkte sauber
  definiert.
