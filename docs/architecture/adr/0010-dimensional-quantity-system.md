<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0010: Dimensionssicheres SI-Größensystem

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M1/M2; `SYS-001`, `SYS-004`, `BODY-002`, `DATA-001`

## Kontext

Historische MEHLISSA-Modelle kodieren Einheiten teilweise nur in Variablennamen
oder Kommentaren. Dadurch sind unbemerkte Verwechslungen von Metern und
Millimetern sowie dimensionswidrige Rechnungen möglich. Künftige Körper-,
Organ-, Zell- und Kommunikationsmodelle tauschen Größen über mehrere Skalen aus;
eine alleinige Dokumentationskonvention reicht dafür nicht.

Die Simulationsuhr benötigt zusätzlich ganzzahlige Nanosekunden für exakte,
monotone Ereignisreihen. Physikalische Modellgleichungen benötigen dagegen
dimensionssichere Multiplikation und Division mit abgeleiteten Größen.

## Entscheidung

1. Der Kern definiert `Quantity<Dimension<L, T, N>>`. Die Exponenten stehen für
   Länge, Zeit und Stoffmenge in SI-Basisdimensionen.
2. Öffentliche Alias-Typen decken in M1 `Length`, `Time`, `Area`, `Volume`,
   `Speed`, `Amount` und `Concentration` ab.
3. Größen unterschiedlicher Dimension dürfen nicht addiert oder subtrahiert
   werden. Multiplikation und Division leiten die Ergebnisdimension zur
   Compile-Zeit her.
4. Es gibt keine implizite Konvertierung zwischen `double` und einer
   physikalischen Größe. Benannte Fabrik- und Ausgabefunktionen machen die
   verwendete Einheit am Aufruf sichtbar.
5. Intern werden SI-Werte verwendet: Meter, Sekunde, Kubikmeter, Meter pro
   Sekunde, Mol und Mol pro Kubikmeter. Häufige medizinische Präfixe werden an
   der Grenze exakt skaliert.
6. `SimulationClock` behält ganzzahlige `std::chrono::nanoseconds`.
   Modellgleichungen verwenden den dimensionssicheren Zeittyp; Adapter zwischen
   Ereigniszeit und Modellgröße werden dort eingeführt, wo sie benötigt werden.
7. `Position3D` speichert dimensionssichere Längen und gibt eine `Length`
   zurück. Nackte Koordinatenwerte sind in der neuen API nicht mehr zulässig.
8. Positivität, Endlichkeit und fachliche Wertebereiche sind Invarianten des
   jeweiligen Modells oder Eingabeschemas, nicht der Dimension selbst.

## Folgen

Positiv:

- Dimensionsfehler werden beim Kompilieren statt erst im Experiment sichtbar.
- Eingabe-, Modell- und Ausgabecode benennt Konversionen ausdrücklich.
- Abgeleitete Größen wie Geschwindigkeit und Konzentration entstehen aus
  gewöhnlicher, aber typsicherer Arithmetik.
- Das System ist klein, header-only und bleibt Bestandteil des
  abhängigkeitenfreien Offline-Kerns.

Negativ:

- Bestehender Code mit nackten `double`-Werten muss an den Grenzen bewusst
  migriert werden.
- Die interne Darstellung als `double` löst noch keine Unsicherheits- oder
  numerische Konditionierungsprobleme.
- Weitere Basisdimensionen wie Masse oder Temperatur erfordern eine
  kontrollierte Erweiterung des Dimensionsvektors.
- Uhrzeit und physikalische Zeit bleiben absichtlich getrennte Typen und
  benötigen explizite Adapter.

## Alternativen

- **Einheit nur im Variablennamen:** abgelehnt, weil der Compiler keine
  Invarianten prüfen kann.
- **Einheit als Laufzeit-String:** für Metadaten weiterhin nötig, aber für
  Kernarithmetik zu spät und fehleranfällig.
- **Alle Werte als SI-`double`:** vermeidet Konvertierungen im Inneren, aber
  nicht die Verwechslung verschiedener Dimensionen.
- **Externe Units-Bibliothek:** fachlich möglich, für den derzeit kleinen Satz
  aber zusätzliche API-, Build- und Migrationskomplexität. Die Entscheidung
  wird neu bewertet, wenn erweiterte Dimensionen, Formatierung oder
  Standardinteroperabilität den eigenen kleinen Vertrag übersteigen.
