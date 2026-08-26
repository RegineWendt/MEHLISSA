# Gate Review M0 – Projektauftrag und Architekturentscheidungen

**Reviewdatum:** 26. August 2026  
**Ergebnis:** fachlich und technisch abgeschlossen; formale Lizenzfreigabe ausstehend

## 1. Abnahmekriterien

| Kriterium | Status | Artefakt/Nachweis |
|---|---|---|
| Dissertation in nachverfolgbare Anforderungen überführt | erfüllt | [Systemanforderungen](../requirements/SYSTEM_REQUIREMENTS.md), [Traceability Matrix](../requirements/TRACEABILITY_MATRIX.md) |
| Vision, bestehende Baseline, Ableitung und neue Ergänzung getrennt | erfüllt | Herkunftscodes und ADR-0005 |
| vier Ebenen und Verantwortlichkeiten verbindlich | erfüllt | ADR-0002, `ARC-001` bis `ARC-007` |
| Legacy-Übernahmeprinzip entschieden | erfüllt | ADR-0001 |
| Technologieentscheidung dokumentiert | erfüllt | ADR-0003; C++20/CMake/vcpkg-Bootstrap |
| erster vertikaler Demonstrator priorisiert | erfüllt | ADR-0004 und Fingerprinting-Spezifikation |
| Zielnutzer und Arbeitsabläufe festgelegt | erfüllt | [Zielnutzer und Workflows](USERS_AND_WORKFLOWS.md) |
| Datenbestände und externe Quellen inventarisiert | erfüllt | [Lizenz- und Dateninventar](LICENSE_AND_DATA_INVENTORY.md) |
| erstes Referenzorgan ausgewählt | erfüllt | ADR-0006: Lunge |
| Forschungsfragen, Datenlücken und Partnerrollen benannt | erfüllt | [Datenlücken und Validierungspartner](VALIDATION_AND_DATA_PARTNERS.md) |
| Legacy-Stände archiviert | erfüllt | Tag `legacy-baseline-2026-08-26` |
| Modellvalidität und Releasequalität definiert | erfüllt | ADR-0005, Systemanforderungen und Abnahmeregeln |
| repositoryweite Lizenz formal freigegeben | **offen** | ADR-0007 `Proposed`; Bestätigung der Rechteinhaber erforderlich |

## 2. Verbindliche M0-Entscheidungen

- MEHLISSA Next verwendet einen neuen Kern und qualifiziert Legacy-Code vor jeder Portierung.
- Körper-, Organ-, Kapillar- und Zellebene sind eigenständige Co-Simulationskomponenten.
- C++20/CMake/vcpkg bilden das technische Fundament; Python folgt als Experiment-API.
- Fingerprinting ist der erste vertikale Demonstrator.
- Die Lunge ist das erste Organmodell; begonnen wird mit dem pulmonalen Kreislauf, nicht mit einer vollständigen Atemmechanik.
- Reproduzierbarkeit, Einheiten, Provenienz, Evidenzklasse und Unsicherheit sind Produktfunktionen.
- Öffentliche Daten und Fremdmodelle werden nur mit versionierter Lizenz- und Transformationsprovenienz aufgenommen.

## 3. Eintritt in M1

Technische Arbeit an M1 darf beginnen. Vor einem öffentlichen M1-Release oder der Annahme externer Beiträge muss ADR-0007 angenommen und die zentrale Lizenz-/Notice-Struktur umgesetzt sein.

Die ersten M1-Arbeitspakete sind:

1. versioniertes Experimentmanifest und Schema;
2. Provenienzmanifest für jeden Lauf;
3. vollständiges Einheitensystem;
4. `SimulationContext` und Komponentenlebenszyklus;
5. strukturierte Fehler und Konfigurationsvalidierung;
6. anschließend Migration des 95er-Gefäßmodells als M2-Vorbereitung.

## 4. Restentscheidung der Projektleitung

Empfehlung zur Freigabe:

> Das gesamte MEHLISSA-Repository wird unter `GPL-2.0-only` geführt. Bestehende Rechteinhaberhinweise bleiben erhalten; Fremddaten und Drittsoftware werden mit ihren eigenen Lizenzen und Notices dokumentiert.

Nach Bestätigung werden `LICENSE`, `NOTICE.md`, `THIRD_PARTY_NOTICES.md` und SPDX-Header in einem eigenen, überprüfbaren Commit ergänzt. Danach kann M0 ohne Vorbehalt als `passed` markiert werden.
