<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Datenlizenzierung

## Verbindliche Regel

Neue, vom MEHLISSA-Projekt selbst erstellte und zur Veröffentlichung
freigegebene Datensätze stehen unter `CC-BY-4.0`. Jeder Datensatz erhält ein
Manifest mit Urheber, Quellen, gewünschter Namensnennung, Version, Lizenz,
Prüfsumme und Transformationen.

Diese Regel lizenziert bestehende oder fremde Daten nicht rückwirkend um. Ohne
bestätigte Rechte und ausdrückliche SPDX-Angabe bleibt der Lizenzstatus eines
Bestandsdatensatzes ungeklärt; er darf dann nicht als CC-BY-Datensatz in einem
öffentlichen Release ausgewiesen werden.

## Aktueller Bestand

Für die drei Quellen des 95er-BVS/MEHLISSA-Datensatzes ist die Freigabe seit
dem 27. August 2026 bestätigt. Sie besitzen `CC-BY-4.0`-Sidecars; das Manifest
`data/legacy/bvs95/release-v1.json` erfasst Herkunft, Prüfsummen,
Namensnennung, Transformationen und bekannte Grenzen. Der daraus erzeugte
Next-Gefäßgraph steht ebenfalls unter `CC-BY-4.0`.

Diese Freigabe gilt artefaktbezogen und erfasst nicht automatisch die weiteren
CSV-Dateien, Erweiterungen oder Publikations-PDFs im Repository. Deren Status
bleibt im [Lizenz- und Dateninventar](m0/LICENSE_AND_DATA_INVENTORY.md)
dokumentiert.

## Namensnennung

Sofern das jeweilige Manifest nichts Präziseres vorgibt, soll die Namensnennung
mindestens enthalten:

> MEHLISSA contributors, Titel und Version des Datensatzes, CC BY 4.0,
> Repository-URL und zugehörige wissenschaftliche Publikation.

Drittquellen werden zusätzlich exakt so genannt, wie es ihre Lizenz verlangt.
