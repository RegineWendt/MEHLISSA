# ADR-0007: Repositoryweite Lizenz

- **Status:** Proposed
- **Datum:** 26. August 2026
- **Entscheidung erforderlich von:** Rechteinhaber/Projektleitung
- **Betrifft:** M0/M1-Release; alle Quell- und Datenartefakte

## Kontext

Die Quelldateien in `mehlissa/` und `mehlissa2.0/` enthalten GNU-GPL-Version-2-Hinweise. Als Rechteinhaber werden Universität zu Lübeck und teilweise Technische Universität Berlin genannt. Eine zentrale Lizenzdatei fehlt und GitHub erkennt keine Lizenz. Die neu angelegten Next-Dateien besitzen noch keine SPDX-Kennzeichnung.

Ohne eine eindeutige repositoryweite Erklärung ist für Nutzer und Beitragende unklar, unter welchen Bedingungen Next-Code, Dokumentation und Daten verwendet werden dürfen. Gleichzeitig dürfen Code- und Datenlizenzen nicht vermischt werden.

## Vorgeschlagene Entscheidung

1. MEHLISSA-Quellcode und eigene Softwaredokumentation werden unter `GPL-2.0-only` veröffentlicht.
2. Bestehende Copyright- und Autorenhinweise bleiben erhalten.
3. Neue C/C++-Dateien erhalten `SPDX-License-Identifier: GPL-2.0-only` und einen projektweiten Copyrightverweis.
4. Repositorydaten erhalten je Datensatz ein eigenes Manifest; selbst erstellte Daten werden nur nach Rechteprüfung unter einer ausdrücklich genannten Datenlizenz veröffentlicht.
5. Fremddaten, Publikationen und Drittsoftware bleiben unter ihren jeweiligen Bedingungen und werden nicht durch die Code-Lizenz umgewidmet.
6. `NOTICE.md` und `THIRD_PARTY_NOTICES.md` dokumentieren Rechteinhaber, Zitationen und Pflichten.

## Begründung

- Die Wahl entspricht den Lizenzhinweisen des vorhandenen Codes.
- Sie ist mit dem optionalen GPL-2.0-only-lizenzierten ns-3 kompatibel.
- Sie vermeidet eine unautorisierte Relizenzierung historischer Beiträge.
- Eine einheitliche SPDX-Kennung verbessert automatisierte Lizenzprüfungen.

## Folgen

Positiv:

- Nutzer und Beitragende erhalten eindeutige Bedingungen.
- Legacy- und Next-Code können rechtssicherer gemeinsam verteilt werden.
- Lizenzprüfung kann in CI automatisiert werden.

Negativ:

- Abgeleitete und verteilte Software muss die GPLv2-Bedingungen erfüllen.
- Eine spätere permissive oder proprietäre Lizenzierung benötigt gesonderte Zustimmung aller relevanten Rechteinhaber.
- Datensätze und PDFs benötigen weiterhin getrennte Rechteprüfung.

## Alternativen

- **Permissive Lizenz nur für Next:** möglich, aber gemeinsame Verteilung/Portierung von Legacy-Code wird komplex und benötigt klare Dateigrenzen sowie Rechteklärung.
- **Duale Lizenz:** flexibel, setzt jedoch umfassende Zustimmung aller Rechteinhaber und ein Contributor Agreement voraus.
- **Keine zentrale Lizenz:** abgelehnt; lässt zentrale Nutzungsfragen offen und verhindert einen sauberen öffentlichen Release.

## Annahmebedingung

Nach ausdrücklicher Bestätigung durch die Projektleitung wird dieses ADR auf `Accepted` gesetzt und in einem eigenen Commit werden `LICENSE`, `NOTICE.md`, `THIRD_PARTY_NOTICES.md`, Datenmanifeste und SPDX-Header ergänzt.
