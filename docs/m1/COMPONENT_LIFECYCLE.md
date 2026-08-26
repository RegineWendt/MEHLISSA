<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Simulationskontext und Komponentenlebenszyklus

`SimulationContext` bündelt den Zustand, der für genau einen MEHLISSA-Lauf gilt:

- monotone `SimulationClock`;
- unveränderlicher Master-Seed;
- persistente, benannte `RandomStream`-Instanzen.

Der Kontext ist weder global noch kopierbar. Derselbe Streamname liefert
innerhalb eines Laufs dieselbe fortgesetzte RNG-Instanz. Ein neuer Kontext mit
demselben Master-Seed reproduziert die Folge.

## Zustandsautomat des Hosts

```text
building --initialize()--> initialized --finalize()--> finalized
    |                           |
    | Initialisierungsfehler    | Destruktor
    +---------------------------+----------------------> finalized
```

- Komponenten dürfen nur in `building` registriert werden.
- Namen sind innerhalb des Hosts eindeutig.
- `initialize` läuft vorwärts; ein Rollback finalisiert bereits erfolgreiche
  Komponenten rückwärts.
- `advance(delta)` ist nur in `initialized` erlaubt. Alle Komponenten sehen die
  alte Uhrzeit; erst der Gesamterfolg übernimmt die vorab validierte neue Uhr.
- `finalize` läuft rückwärts, ist wiederholbar ohne Doppeleffekt und wird auch
  vom Destruktor aufgerufen.

## Ownership-Regeln

Der Host besitzt jede Komponente exklusiv. Komponenten erhalten den Kontext nur
für die Dauer eines Callbacks und dürfen daraus kein Ownership ableiten.
Abhängigkeiten zwischen Komponenten werden später über explizite
Austauschobjekte oder Dienste modelliert, nicht über gegenseitige
`shared_ptr`-Referenzen.

`finalize` ist garantiert ausnahmefrei. Persistente Ausgaben müssen daher
vorher abgeschlossen oder über einen getrennten, fehlermeldenden Flush-Schritt
behandelt werden. M1.5 ergänzt strukturierte Fehler und den Checkpointvertrag.

## Nachweis

`simulation_context_tests.cpp` prüft Seed- und Streamisolation.
`component_host_tests.cpp` prüft Zustandsübergänge, Reihenfolge,
Fehler-Rollback, ausbleibenden Uhrfortschritt und genau einmalige Finalisierung.
