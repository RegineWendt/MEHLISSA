<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M2.4 – BVS-Referenzregression

**Status:** bestanden  
**Referenzprofil:** `bvs95-dissertation-rest-v1`  
**Seed:** `2018`  
**Maschinenlesbarer Nachweis:**
`data/reference-results/bvs95-dissertation-rest-m2.4.json`

## Ziel und wissenschaftliche Grenze

M2.4 prüft zwei verschiedene Aussagen, die nicht vermischt werden dürfen:

1. **Dynamische BVS-Aussagen:** Nähert sich die Partikelverteilung nach etwa
   sieben Minuten einem Langzeitwert, wird sie weitgehend unabhängig vom
   Injektionsort und bleibt sie bei zehnfacher Population ähnlich?
2. **Dissertationsperfusion:** Bildet der 95-Segment-Graph die 23 in Tabelle
   4.1 vorgegebenen Organ- und Regionalanteile reproduzierbar ab?

Der dynamische Vergleich ist keine byte- oder zahlengenaue Reproduktion des
BloodVoyagerS-Laufs von 2018. Dieser verwendete 94 Gefäße, überwiegend
1:1-Verzweigungen, einzelne 1:3-Verzweigungen und zufällig variierte
Geschwindigkeiten. MEHLISSA Next verwendet dagegen den aus der Dissertation
rekonstruierten, flusserhaltenden 95-Segment-Graphen. M2.4 prüft deshalb die
publizierten qualitativen und größenordnungsmäßigen Aussagen unter dem neuen
Modellvertrag.

## Rekonstruierte Referenzbedingungen

Aus dem BloodVoyagerS-Paper wurden übernommen:

- 6.359 Partikel als Referenzpopulation und 63.590 als zehnfacher Lauf;
- Injektion in Gefäß 1, Aorta ascendens;
- Vergleich von über die jeweils vorangehende Minute gemittelten
  Gefäßpopulationen;
- Minute 7 als Gleichgewichtsprüfung und Minute 120 als Langzeitreferenz;
- alternative Injektion in die linke Poplitealregion;
- publizierte Vergleichswerte 3,11 % mittlere normierte Abweichung und
  3,95 % Injektionsortabweichung.

Die Dissertation liefert den ergänzten 95. Koronarzweig sowie Soll- und
Simulationsanteile für 23 Regionen. Diese Werte werden separat gegen die
stationären Flüsse des kanonischen Graphen geprüft.

## Metriken

Für zwei gemittelte Verteilungen `x` und `y` mit `S = 95` Segmenten gilt:

```text
MAD = (1/S) * Summe_i |x_i - y_i|
normierte MAD [%] = MAD / (N/S) * 100
```

Das Paper bezeichnet seine gemittelte absolute Gefäßabweichung an einer
Stelle als „standard deviation“, beschreibt aber eine Mittelung der absoluten
Differenzen. M2.4 benennt die implementierte Größe daher eindeutig als
**mean absolute difference (MAD)** und behauptet keine Identität mit einer
statistischen Standardabweichung.

Der Populationsvergleich verwendet die Total-Variation-Distanz der auf die
jeweilige Partikelzahl normierten Minute-7-Verteilungen. Die Perfusionsfehler
werden in Prozentpunkten berechnet.

## Vor dem Lauf festgelegte Gates

| Gate | Grenze | Begründung |
|---|---:|---|
| Minute 7 gegen Minute 120 | höchstens 5 % normierte MAD | umfasst die publizierten 3,11 % und Modellunterschiede |
| Aorta gegen Poplitea bei Minute 7 | höchstens 5 % normierte MAD | umfasst die publizierten 3,95 % |
| 6.359 gegen 63.590 Partikel | höchstens 2 % Total Variation | prüft Skalierungsstabilität statt identischer Zählwerte |
| mittlerer Fehler gegen Perfusionssoll | höchstens 0,01 Prozentpunkte | deterministischer stationärer Graph |
| maximaler Fehler gegen Perfusionssoll | höchstens 0,01 Prozentpunkte | verhindert lokal verdeckte Ausreißer |
| mittlere Differenz zur Dissertationssimulation | höchstens 0,5 Prozentpunkte | historischer stochastischer Lauf, kein Sollwert |
| Populationserhaltung | exakt | harte Transportinvariante |

Die Grenzen wurden vor dem ersten vollständigen M2.4-Lauf festgelegt und
danach nicht verändert.

## Ergebnis

| Nachweis | Ergebnis | Grenze | Status |
|---|---:|---:|---|
| Minute 7 gegen Minute 120 | 1,843506 % | 5 % | bestanden |
| Aorta gegen Poplitea, Minute 7 | 1,741359 % | 5 % | bestanden |
| Populationsskalierung | 0,760720 % | 2 % | bestanden |
| mittlerer Perfusions-Sollfehler | 0,002140 Prozentpunkte | 0,01 | bestanden |
| maximaler Perfusions-Sollfehler | 0,007685 Prozentpunkte | 0,01 | bestanden |
| Mittel gegen Dissertation-Istwerte | 0,314012 Prozentpunkte | 0,5 | bestanden |
| Populationserhaltung | exakt | exakt | bestanden |

Minute 1 liegt mit 10,886912 % erwartungsgemäß noch deutlich weiter von der
Langzeitverteilung entfernt. Dass Minute 15 mit 1,847935 % nicht monoton unter
Minute 7 liegt, ist bei endlicher stochastischer Population plausibel; das
Gate fordert Konvergenz in einen stabilen Streubereich, keine monotone Folge.

## Implementierung und Reproduktion

Der Referenzläufer verarbeitet Segmentwechsel ereignisgetrieben. Er integriert
die exakte Partikel-Aufenthaltszeit je Segment und bildet daraus die
60-Sekunden-Mittelwerte. Damit wird der lange Lauf nicht durch einen frei
gewählten globalen Zeitschritt verzerrt. Übergänge werden in stabiler
Reihenfolge aus einem benannten, vom Master-Seed abgeleiteten Zufallsstrom
gezogen. Die Gesamtpopulation wird nach jedem vollständigen Lauf exakt
erhalten.

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe reference-bvs `
  --model data/body-models/bvs95-dissertation-rest-v1.json `
  --output data/reference-results/bvs95-dissertation-rest-m2.4.json `
  --schema data/schemas/vascular-graph/1.0.0.schema.json `
  --report-schema data/schemas/bvs-reference-report/1.0.0.schema.json
```

Der Bericht wird vor dem Schreiben gegen das Schema
`data/schemas/bvs-reference-report/1.0.0.schema.json` validiert. Ein Test
erzeugt ihn erneut und vergleicht ihn bytegenau mit der eingecheckten Golden
Reference.

## Aussage und verbleibende Risiken

M2.4 zeigt, dass der Next-Transport für dieses Profil massenerhaltend,
reproduzierbar und gegenüber den drei BVS-Szenarien robust ist. Außerdem zeigt
es, dass die aus der Dissertation migrierten stationären Flüsse deren Sollwerte
treffen.

M2.4 ist **keine unabhängige physiologische Validierung**: Die Sollanteile
haben die 95er-Übergänge kalibriert und dürfen daher nicht zugleich als externe
Validierungsdaten gelten. Anatomische Gefäßradien, pulsatile Strömung,
Geschwindigkeitsverteilungen innerhalb eines Segments und ein unabhängiges
weibliches Ruheprofil bleiben Aufgaben von M2.6 und späteren
Validierungsinkrementen.

## Quellen

- Wendt et al., *BloodVoyagerS – Simulation of the work environment of medical
  nanobots*, 2018, insbesondere S. 4–5.
- Regine Wendt, *Einsatz von Nanotechnologien in der Präzisionsmedizin*, 2024,
  Abschnitt 4.3.1, Tabelle 4.1 und Tabelle A.1.
- Öffentliche Referenzimplementierung:
  <https://github.com/RegineWendt/blood-voyager-s>.
