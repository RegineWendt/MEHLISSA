<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2 – Validated Body Layer

**Status:** in Arbeit
**Beginn:** 27. August 2026

M2 ersetzt die implizite Legacy-Topologie durch einen versionierten,
dimensionssicheren und semantisch validierten Gefäßgraphen. Erst auf diesem
Vertrag folgen Partikeltransport und wissenschaftliche BVS-Regressionen.

## Geplante Inkremente

| Inkrement | Status | Gate-Nachweis |
|---|---|---|
| M2.1 Gefäßgraphvertrag | abgeschlossen | JSON Schema, SI-Datentypen, Graph-/Geometrie-/Flussinvarianten, synthetischer Referenzgraph und Plattform-CI |
| M2.2 Legacy-95-Migration | vorbereitet | verlustfreier Konverter, Provenienz- und Lizenzfreigabe, Klärung Gefäß 9 |
| M2.3 deterministischer Kompartimenttransport | offen | Injektions-, Übergangs-, Einmalbewegungs- und Massenerhaltungstests |
| M2.4 BVS-Referenzlauf | offen | publizierte Verteilungen innerhalb vorab definierter Toleranzen |
| M2.5 Ausgabe und Messorte | offen | begrenzbare Trajektorien, Aggregate, Entnahme und Gateway-Messpunkte |
| M2.6 alternative Körpermodelle und Zustände | offen | Laden ohne Rebuild; Ruhe-/Belastungsparametersätze |

## M2.1-Grundsätze

- IDs sind stabile Zeichenketten und müssen nicht fortlaufend sein.
- Das Dateiformat verwendet ausschließlich explizit benannte SI-Felder.
- Topologie ist explizit; Koordinatengleichheit allein erzeugt keine Kante.
- Ein geschlossener Referenzkreislauf muss stark zusammenhängend sein.
- Länge, Querschnitt, Volumen, Geschwindigkeit und Fluss werden gegeneinander
  geprüft.
- Übergangswahrscheinlichkeiten müssen vollständig, eindeutig und auf eins
  normiert sein.
- Quellen, Lizenz, Evidenzqualität, Unsicherheit und Gültigkeitsbereich gehören
  zum Modellvertrag.
- Medizinische IDs und Organwissen bleiben außerhalb von `core/`.

Der Legacy-Befund und die Migrationsgrenze stehen in
[`LEGACY_95_DATA_AUDIT.md`](LEGACY_95_DATA_AUDIT.md).

## M2.1 benutzen

Vom Repository-Root validiert die CLI zunächst das JSON Schema und danach alle
semantischen Graph- und Flussinvarianten:

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe validate-body `
  --model examples/body-models/synthetic-branching-circuit.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

Ohne `--schema` wird relativ zum aktuellen Arbeitsverzeichnis
`data/schemas/vascular-graph/1.0.0.schema.json` verwendet.
