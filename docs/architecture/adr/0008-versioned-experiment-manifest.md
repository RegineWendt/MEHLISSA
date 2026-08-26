<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0008: Versioniertes Experimentmanifest

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M1; `DATA-001`, `DATA-002`, `SYS-002`, `SYS-007`, `UX-001`

## Kontext

Reproduzierbare MEHLISSA-Läufe benötigen einen stabilen, maschinenlesbaren
Vertrag für Laufzeit, Zufallsseed, Modelle und Ausgaben. Freie oder implizite
Konfiguration würde ungültige Einheiten, unbekannte Felder und nicht
rekonstruierbare Standardwerte erst während der Simulation sichtbar machen.

## Entscheidung

1. Experimente werden als UTF-8-JSON-Dokumente beschrieben.
2. Jedes Dokument nennt eine semantisch versionierte `schema_version`.
3. Version `1.0.0` wird durch ein JSON Schema nach Draft 2020-12 definiert.
4. Das Schema ist strikt: unbekannte Felder sind unzulässig, Pflichtfelder
   fehlen nie stillschweigend und Einheiten werden ausdrücklich angegeben.
5. Zeitwerte bestehen aus ganzzahligem Wert und Einheit. Die Konvertierung in
   interne Nanosekunden prüft zusätzlich einen möglichen Überlauf.
6. Der Master-Seed ist Teil des Manifests; benannte Zufallsströme werden später
   deterministisch daraus abgeleitet.
7. Ein Manifest wird vollständig geladen und validiert, bevor Simulationszustand
   erzeugt oder ein Ausgabeverzeichnis verändert wird.
8. `jsoncons` übernimmt Parsing und Schema-Validierung. Die header-only
   BSL-1.0-Abhängigkeit wird reproduzierbar über vcpkg bezogen.

## Folgen

Positiv:

- Fehlerhafte Konfigurationen werden vor dem Lauf mit Kontext abgelehnt.
- Andere Werkzeuge und die spätere Python-API können dasselbe öffentliche
  Schema verwenden.
- Schemaversionen erlauben kontrollierte Migration statt stiller
  Bedeutungsänderungen.
- Der allgemeine Kern bleibt frei von JSON- und Dateisystemabhängigkeiten; die
  Funktion liegt in einer eigenen Experimentbibliothek.

Negativ:

- Jede kompatibilitätsbrechende Änderung erfordert eine neue Schemaversion und
  gegebenenfalls einen Migrator.
- Schema und C++-Dekodierung müssen durch Vertragstests synchron gehalten
  werden.
- Der vollständige Build benötigt die zusätzliche jsoncons-Abhängigkeit; der
  Offline-Smoke-Test prüft weiterhin nur den abhängigkeitenfreien Kern.

## Alternativen

- **Freies JSON ohne Schema:** abgelehnt, weil Fehler und unbekannte Felder zu
  spät erkannt würden.
- **YAML:** besser für handgeschriebene große Konfigurationen, aber komplexere
  Typ- und Parsersemantik; kann später als konvertierendes Frontend ergänzt
  werden.
- **Eigenes JSON- und Schema-Parsing:** abgelehnt, da sicherheits- und
  wartungskritische Infrastruktur ohne fachlichen Mehrwert dupliziert würde.
- **Protobuf als primäres Eingabeformat:** für interne Schnittstellen später
  denkbar, für versionierbare, menschenlesbare Experimente derzeit unnötig.
