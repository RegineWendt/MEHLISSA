<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0011: Expliziter Simulationskontext und Komponentenlebenszyklus

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M1/M2–M6; `SYS-002`, `SYS-003`, `SYS-005`, `SYS-007`

## Kontext

Die historischen Implementierungen koppeln globale Uhr, Zufall, Modellzustand,
Szenariologik und Ausgabe eng miteinander. Globale oder statische Zustände
erschweren parallele Experimente, reproduzierbare Tests und eine eindeutige
Objektlebensdauer. Für die späteren Körper-, Organ-, Zell- und
Kommunikationskomponenten wird ein gemeinsamer Laufkontext benötigt, ohne dass
der Kern medizinische Szenarioklassen kennen muss.

Initialisierungsfehler und vorzeitige Laufabbrüche dürfen Komponenten nicht
halb registriert oder mehrfach finalisiert zurücklassen. Zugleich soll eine
fehlgeschlagene Komponentenaktualisierung die gemeinsame Uhr nicht irrtümlich
fortschreiben.

## Entscheidung

1. Jeder Lauf besitzt genau einen nicht kopier- und nicht verschiebbaren
   `SimulationContext`.
2. Der Kontext besitzt die monotone Simulationsuhr, den Master-Seed und die
   persistenten, nach Namen erzeugten Zufallsströme. Diese Zustände sind nicht
   global.
3. Komponenten implementieren `SimulationComponent` mit stabilem Namen und den
   Phasen `initialize`, `advance` und `finalize`.
4. `ComponentHost` besitzt Komponenten exklusiv über `std::unique_ptr`.
   Nullzeiger, leere Namen, doppelte Namen und nachträgliche Registrierung
   werden abgelehnt.
5. Initialisierung erfolgt in Registrierungsreihenfolge. Bei einem Fehler
   werden nur zuvor erfolgreich initialisierte Komponenten in umgekehrter
   Reihenfolge finalisiert.
6. Reguläre Finalisierung erfolgt ebenfalls rückwärts und genau einmal.
   `finalize` ist `noexcept`, damit auch der Host-Destruktor den Vertrag sicher
   erfüllen kann.
7. Während `advance` sehen alle Komponenten denselben Intervallanfang und die
   explizite Schrittweite. Die gemeinsame Uhr wird erst übernommen, nachdem
   alle Komponenten erfolgreich waren.
8. Der Host ist ein Lebenszyklusmechanismus, noch kein medizinischer
   `ModelComponent` und kein Ereignisscheduler. Fachliche Layer-Verträge werden
   in M2 bis M6 auf dieser neutralen Grundlage definiert.

## Folgen

Positiv:

- Mehrere Experimente können unabhängige Uhren und Zufallszustände besitzen.
- Ownership und Finalisierungsreihenfolge sind eindeutig und durch Tests
  nachgewiesen.
- Fehlgeschlagene Initialisierung hinterlässt keine bereits initialisierten
  Komponenten ohne Abschluss.
- Eine fehlgeschlagene Aktualisierung schreibt die gemeinsame Zeit nicht fort.
- Der Kern bleibt frei von medizinischen Szenariotypen.

Negativ:

- Komponenten dürfen den übergebenen Kontext nur nichtbesitzend verwenden; der
  Typ kann das Speichern eines rohen Verweises nicht vollständig verhindern.
- Bereits erfolgte fachliche Zustandsänderungen früherer Komponenten werden bei
  einem Fehler in `advance` noch nicht zurückgerollt. Checkpoints und
  transaktionale Modellgrenzen folgen in M1.5 beziehungsweise späteren Layern.
- Finalisierung darf weder werfen noch Arbeit ausführen, die nur durch eine
  Ausnahme als fehlgeschlagen gemeldet werden könnte.
- Reihenfolgeabhängige Kopplungen benötigen später einen expliziten Scheduler
  oder Austauschvertrag statt stiller Registrierungsreihenfolge.

## Alternativen

- **Globale Singletons:** abgelehnt, weil Tests und parallele Läufe Zustände
  teilen würden.
- **Geteiltes Komponenten-Ownership:** abgelehnt, weil Zyklen und unklare
  Zerstörungszeitpunkte entstehen können.
- **Finalisierung nur im normalen Erfolgsfall:** abgelehnt, weil Fehlerpfade
  Ressourcen und unvollständigen Zustand zurücklassen.
- **Uhr vor Komponenten aktualisieren:** abgelehnt, weil ein Komponentenfehler
  dann einen nicht ausgeführten Simulationsfortschritt sichtbar macht.
- **Vollständiger Ereignisscheduler in M1.4:** zurückgestellt, bis M2 konkrete
  Transport- und Austauschereignisse liefert.
