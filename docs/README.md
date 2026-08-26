<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA-Dokumentation

Die Lizenzzuordnung des Repositories ist in
[`LICENSE.md`](../LICENSE.md) beschrieben; Regeln für Daten stehen in
[`DATA_LICENSING.md`](DATA_LICENSING.md).

## Projektstand und Weiterentwicklung

- [Analyse des aktuellen Stands](IST_ANALYSE.md) – Abgleich von Literatur, MEHLISSA 1.x, MEHLISSA 2.0, Datensätzen und Anwendungsszenarien.
- [Roadmap für eine neue MEHLISSA-Generation](ROADMAP.md) – dissertationsnahe Zielarchitektur, Entwicklungsphasen, Qualitätsgates, Szenarien und langfristiger Digital-Twin-Pfad.
- [Systemanforderungen](requirements/SYSTEM_REQUIREMENTS.md) – 72 nummerierte fachliche, architektonische und qualitative Anforderungen mit Herkunft, Priorität und Nachweisart.
- [Traceability Matrix](requirements/TRACEABILITY_MATRIX.md) – Zuordnung aller Anforderungen zu Literatur, aktuellem Stand, Roadmap-Gate und vorgesehenem Nachweis.
- [Fingerprinting-Referenzszenario](requirements/FINGERPRINTING_SCENARIO.md) – fachliche Baseline, Referenzwerte, Akzeptanzstufen und Tests für den ersten vertikalen Demonstrator.
- [Architekturentscheidungen](architecture/README.md) – neuer Kern, Vier-Ebenen-Co-Simulation, Technologie, Szenariopriorität und Evidenzregeln.
- [M0 Gate Review](m0/M0_GATE_REVIEW.md) – bestandene Abnahme der fachlichen,
  technischen und lizenzbezogenen Fundamententscheidungen.
- [Lizenz- und Dateninventar](m0/LICENSE_AND_DATA_INVENTORY.md) – lokaler Bestand, Prüfsummen, Defekte, Fremdquellen und Freigaberegeln.
- [Zielnutzer und Arbeitsabläufe](m0/USERS_AND_WORKFLOWS.md) – Rollen, priorisierte Workflows und Release-Nutzbarkeit.
- [Datenlücken und Validierungspartner](m0/VALIDATION_AND_DATA_PARTNERS.md) – benötigte Evidenz, öffentliche Quellen und mögliche Kompetenzpartner.
- [Entwicklungsumgebung und Build](DEVELOPMENT.md) – reproduzierbarer CMake/vcpkg-Build, Testausführung und Qualitätswerkzeuge für MEHLISSA Next.
- [M1 – Trustworthy Kernel](m1/README.md) – aktueller Umsetzungsstand,
  Inkremente und Bedienung des versionierten Experimentmanifests.

Die historische Analyse bezieht sich auf den Legacy-Stand `4f4fc5a` (Tag `legacy-baseline-2026-08-26`). Die Entwicklungsdokumente beschreiben den Branch `mehlissa-next-generation`. Bei wesentlichen Änderungen an Architektur, Datensätzen oder Szenarien müssen Anforderungen, Traceability Matrix und Roadmap gemeinsam aktualisiert werden.
