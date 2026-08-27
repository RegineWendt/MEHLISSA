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
| M2.2 Legacy-95-Migration | abgeschlossen | deterministischer Konverter, CC-BY-Provenienz, 95-Segment-SI-Graph, belegte Aufteilung von Gefäß 9 und Profiltrennung |
| M2.3 deterministischer Kompartimenttransport | abgeschlossen | zeitgesteuerte Injektion, datengetriebene Transitzeiten, benannter Übergangs-Zufallsstrom, Einmalbewegungs- und Populationserhaltungstests |
| M2.4 BVS-Referenzlauf | abgeschlossen | 6.359-/63.590-Läufe, Gleichgewicht, Injektionsort, Perfusion, exakte Erhaltung und schema-validierte Golden Reference innerhalb vorab definierter Toleranzen |
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

## M2.2 – 95er-Migration

Die freigegebenen Quellen werden durch einen strikten Konverter in das
kanonische Profil
`data/body-models/bvs95-dissertation-rest-v1.json` überführt. Die Quelldateien
bleiben unverändert. IDs, Typen und Koordinaten werden bijektiv abgebildet,
Einheiten explizit nach SI konvertiert, Flüsse erhalten und alle Kanten
gespeichert. Gefäß 9 verwendet für Ruhe/Rückenlage den belegten Split
`0,2875/0,7125` auf linke/rechte V. jugularis interna.

```powershell
build/windows-msvc/Debug/apps/mehlissa.exe migrate-legacy-95 `
  --vasculature mehlissa2.0/data/95_vasculature.csv `
  --transitions mehlissa2.0/data/95_transitions.csv `
  --output data/body-models/bvs95-dissertation-rest-v1.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json
```

Lizenz, Hashes und Transformationen stehen in
`data/legacy/bvs95/release-v1.json`. Die methodische Trennung zwischen dem
ausführbaren Reproduktionsprofil und der künftigen weiblichen
Ruhe-/Rückenlage-Referenz beschreibt
[`PHYSIOLOGICAL_BASELINE.md`](PHYSIOLOGICAL_BASELINE.md). Sie ist in
[ADR-0016](../architecture/adr/0016-legacy-95-release-and-reference-profiles.md)
verbindlich entschieden.

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

## M2.3 – deterministischer Kompartimenttransport

Der erste Transportbaustein behandelt jedes Gefäßsegment als durchströmtes
Kompartiment, behält aber die stabile Identität jeder mobilen Entität. Eine
Entität sammelt im aktuellen Segment Aufenthaltszeit. Nach der aus
`Länge / mittlere Geschwindigkeit` berechneten Transitzeit wird sie anhand der
im Graphen hinterlegten Übergangswahrscheinlichkeiten an genau einen
Nachfolger übergeben.

„Deterministisch“ bedeutet hier: Verzweigungen bleiben stochastisch, aber ein
Experiment mit demselben Master-Seed, denselben Ereignissen und demselben
Zeitschritt erzeugt exakt dieselbe Folge. Dafür verwendet die Komponente den
eigenen benannten Zufallsstrom
`body.compartment-transport.transitions` und keinen
implementierungsabhängigen Standard-Verteilungsalgorithmus.

Die Invarianten, Grenzen und API stehen in
[`COMPARTMENT_TRANSPORT.md`](COMPARTMENT_TRANSPORT.md). Die Architekturentscheidung
ist in [ADR-0015](../architecture/adr/0015-deterministic-compartment-transport.md)
dokumentiert.

## M2.4 – BVS-Referenzregression

Der ereignisgetriebene Referenzläufer operationalisiert die publizierten
BVS-Aussagen zu Gleichgewicht nach etwa sieben Minuten, Injektionsort und
zehnfacher Population. Unabhängig davon prüft er die 23 Perfusionssollwerte der
Dissertation gegen den 95-Segment-Graphen. Alle Gates wurden vor dem ersten
vollständigen Lauf festgelegt; der erzeugte JSON-Bericht wird schema-validiert
und bytegenau als Golden Reference geprüft.

Methodik, Ergebnisse und Grenzen – insbesondere die Trennung zwischen
94-Gefäß-BVS, 95-Gefäß-Dissertationsprofil und physiologischer Validierung –
stehen in [`BVS_REFERENCE_REGRESSION.md`](BVS_REFERENCE_REGRESSION.md) und
[ADR-0017](../architecture/adr/0017-bvs-reference-regression.md).
