<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0007: Lizenzmodell des Repositories

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M0/M1-Release; alle Quell- und Datenartefakte

## Kontext

Die Quelldateien in `mehlissa/` und `mehlissa2.0/` enthalten GNU-GPL-Version-2-Hinweise. Als Rechteinhaber werden Universität zu Lübeck und teilweise Technische Universität Berlin genannt. Eine zentrale Lizenzdatei fehlt und GitHub erkennt keine Lizenz. Die neu angelegten Next-Dateien besitzen noch keine SPDX-Kennzeichnung.

Ohne eine eindeutige repositoryweite Erklärung ist für Nutzer und Beitragende unklar, unter welchen Bedingungen Next-Code, Dokumentation und Daten verwendet werden dürfen. Gleichzeitig dürfen Code- und Datenlizenzen nicht vermischt werden.

## Entscheidung

1. Vollständig neu und unabhängig entwickelter MEHLISSA-Next-Quellcode,
   Buildskripte und Tests werden unter `MPL-2.0` veröffentlicht.
2. Der historische Code in `mehlissa/` und `mehlissa2.0/` bleibt
   `GPL-2.0-only`. Direkte Portierungen, Änderungen und aus diesem Code
   abgeleitete Integrationskomponenten behalten `GPL-2.0-only`.
3. Neue, projekteigene Dokumentation und zur Veröffentlichung freigegebene
   eigene Daten werden unter `CC-BY-4.0` veröffentlicht.
4. Die Lizenz gilt pro Datei beziehungsweise Artefakt. Neue Dateien erhalten
   SPDX-Kennzeichnungen; nicht kommentierbare Formate erhalten eine
   `.license`-Sidecar-Datei oder ein eindeutiges Datenmanifest.
5. Bestehende Copyright- und Autorenhinweise bleiben erhalten.
6. Bestehende Datensätze werden durch diese Entscheidung nicht rückwirkend
   umlizenziert. Ihre Freigabe unter CC BY 4.0 setzt eine bestätigte
   Rechtekette voraus; bis dahin bleiben sie außerhalb öffentlicher
   Next-Datenpakete.
7. Fremddaten, Publikationen und Drittsoftware bleiben unter ihren jeweiligen
   Bedingungen. `LICENSE.md`, `NOTICE.md`, `THIRD_PARTY_NOTICES.md` und
   Datenmanifeste dokumentieren die Grenzen.

## Begründung

- GPL-2.0-only entspricht den Lizenzhinweisen des vorhandenen Codes und bleibt
  mit ns-3 kompatibel.
- MPL-2.0 hält Änderungen am neuen Kern auf Dateiebene offen, erleichtert aber
  dessen Einbindung in größere Forschungs- und Industrieanwendungen.
- Die getrennte Lizenzierung vermeidet eine unautorisierte Relizenzierung
  historischer Beiträge.
- CC BY 4.0 ist für Dokumentation und zitierbare Forschungsdaten geeigneter als
  eine Softwarelizenz.
- SPDX-Kennzeichnungen machen die jeweilige Lizenz maschinenlesbar.

## Folgen

Positiv:

- Nutzer und Beitragende erhalten eindeutige Bedingungen pro Artefakt.
- Der neue Kern bleibt offen und ist zugleich leichter wiederverwendbar als
  unter starkem Copyleft für das Gesamtwerk.
- Lizenzprüfung kann in CI automatisiert werden.

Negativ:

- Die Grenze zwischen unabhängiger Neuentwicklung und GPL-Portierung muss in
  Reviews konsequent geprüft werden.
- Ein aus Legacy-Code abgeleitetes Werk kann nicht allein durch Ablage in einem
  Next-Verzeichnis zu MPL-Code werden.
- Bestandsdaten und Publikations-PDFs benötigen weiterhin eine getrennte
  Rechteprüfung.
- Release- und Paketierungsprozesse müssen die Mehrfachlizenzierung abbilden.

## Alternativen

- **GPL-2.0-only für das gesamte Repository:** rechtlich und operativ einfacher,
  aber mit höheren Integrationshürden für unabhängige Next-Komponenten.
- **BSD-3-Clause/MIT für Next:** maximale Wiederverwendbarkeit, verlangt aber
  nicht, Verbesserungen an den betroffenen Dateien offenzulegen.
- **Apache-2.0 für Next:** wegen der Inkompatibilität mit GPL-2.0-only für die
  geplanten Integrationspfade ungünstig.
- **Duale Lizenz:** flexibel, setzt jedoch umfassende Zustimmung aller
  Rechteinhaber und ein Contributor Agreement voraus.
- **Keine zentrale Lizenz:** abgelehnt; lässt zentrale Nutzungsfragen offen und verhindert einen sauberen öffentlichen Release.

## Umsetzungs- und Prüfregel

Die Entscheidung ist mit `LICENSE.md`, vollständigen Texten unter `LICENSES/`,
Notices und SPDX-Kennzeichnungen umgesetzt. Jeder Pull Request mit portiertem
Legacy-Code oder neuen Daten muss die zutreffende Lizenz und Provenienz explizit
prüfen. Vor einer institutionellen öffentlichen Freigabe soll die Rechts- oder
Transferstelle die dokumentierten Grenzen bestätigen; diese Prüfung ändert
nicht rückwirkend die Lizenzen fremder Artefakte.
