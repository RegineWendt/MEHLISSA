# ADR-0002: Vier Ebenen als explizite Co-Simulation

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Betrifft:** M0, M3–M5; `ARC-001` bis `ARC-007`

## Kontext

Die Dissertation definiert Körper-, Organ-, Kapillar- und Zellebene als verbundene, aber unabhängige Module mit verschiedenen Raum- und Zeitskalen. Die historischen Implementierungen konzentrieren sich überwiegend auf den Ganzkörpertransport und bilden Ebenenwechsel teilweise durch Speziallogik in gemeinsamen Klassen ab.

Eine umfassende Simulation kann die biologischen Größenordnungen nicht auf allen Ebenen gleichzeitig mit derselben Auflösung berechnen. Die Architektur muss daher Detailmodelle lokal aktivieren, gröbere Modelle daneben weiterlaufen lassen und Zustände kontrolliert austauschen.

## Entscheidung

Jede Ebene wird als eigenständige `ModelComponent` mit eigenem Zustand, Zeitschritt-/Ereignismodell und Gültigkeitsbereich implementiert. Die Komponenten kommunizieren ausschließlich über versionierte Austauschobjekte und einen Orchestrator.

Mindestens folgende Austauscharten werden vorgesehen:

- einzelne Entitäten;
- aggregierte Populationen und Flüsse;
- physiologische Zustände;
- molekulare Signale;
- Detektions- und Zellereignisse;
- Aktuierungsbefehle und Messungen;
- Evidenz- und Unsicherheitsmetadaten.

Der Orchestrator koordiniert Synchronisationspunkte. Übergaben prüfen Zeitordnung, Identität und relevante Erhaltungssätze. Direkte Änderungen am internen Zustand einer anderen Ebene sind nicht zulässig.

## Folgen

Positiv:

- Modellvarianten sind unabhängig entwickel- und validierbar.
- Grobe und detaillierte Modelle können je nach Fragestellung kombiniert werden.
- Externe Simulatoren lassen sich über dieselben Verträge anbinden.
- Fehler an Schichtgrenzen werden beobachtbar und testbar.

Negativ:

- Kopplungsverträge und Synchronisation erzeugen zusätzlichen Entwicklungsaufwand.
- Zeit- und Rauminterpolation kann neue numerische Fehler einführen.
- Konservative Übergaben von Populationen und Stoffen benötigen klare Semantik.

## Verworfene Alternativen

- **Ein globaler Zeitschritt für alle Modelle:** einfach, aber für stark unterschiedliche Skalen ineffizient und teilweise numerisch ungeeignet.
- **Gemeinsamer monolithischer Objektgraph:** erschwert Austauschbarkeit, Validierung und skalierbare Abstraktionen.
- **Nur lose dateibasierte Pipeline:** nützlich für Offline-Kopplung, aber unzureichend für bidirektionale laufzeitnahe Szenarien.
