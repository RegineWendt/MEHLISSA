<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Physiologische Basis der M2-Körpermodelle

**Stand:** 27. August 2026  
**Zweck:** Trennung von historischer Reproduktion, physiologischer Referenz und
späterer Validierung

## Zwei Profile mit unterschiedlichen Aussagen

| Profil | Status | Aussage |
|---|---|---|
| `bvs95-dissertation-rest-v1` | ausführbar und validiert | historische 95er-Topologie und Dissertationstransitionen als reproduzierbare Transportbaseline |
| `reference-adult-female-rest-supine-v1` | fachlich definiert, noch nicht auf alle 95 Segmente gemappt | normative Referenz für eine gesunde erwachsene Frau in Ruhe und Rückenlage |

Das erste Profil ist ein Software- und Publikationsvergleich. Das zweite soll
später die physiologische Bewertung tragen. Ergebnisse des ersten dürfen nicht
als Validierung des zweiten bezeichnet werden.

## Ausführbares Dissertationprofil

Der Konverter erzeugt
`data/body-models/bvs95-dissertation-rest-v1.json` reproduzierbar aus den
freigegebenen CSV-Quellen. Seine Regeln sind:

- alle 95 IDs, Typen und Koordinaten bleiben bijektiv erhalten;
- Legacy-Zentimeter werden mit Faktor `0,01` in Meter konvertiert;
- Kanten entstehen einmalig aus der Legacy-Koordinatenregel und werden danach
  explizit gespeichert;
- alle 23 vorhandenen Verzweigungsquoten bleiben unverändert;
- Gefäß 9 wird belegt auf links `0,2875`, rechts `0,7125` ergänzt;
- Gefäß 2 wird auf `0,0001 m³/s`, also 6,0 L/min, normalisiert;
- segmentale Flüsse werden entlang aller Verzweigungen fortgeschrieben und an
  Zusammenführungen addiert;
- historische mittlere Geschwindigkeiten werden in SI verwendet;
- äquivalente Fläche, Durchmesser und Volumen folgen aus
  `A = Q/v`, `d = sqrt(4A/pi)` und `V = A*L`.

Die 6,0 L/min sind eine transparente Normalisierung nahe typischer Ruhewerte,
keine Messung der historischen Referenzperson. Abgeleitete Durchmesser sind
Transportparameter und keine anatomischen Gefäßradien. Nicht quantifizierte
Unsicherheit bleibt im Datensatz leer, statt durch erfundene Prozentwerte
scheinpräzise zu werden.

## Gefäß 9 und Gültigkeitsbereich

Legacy-Gefäß 9 ist das Kopf-Organbett. Seine Nachfolger sind:

| Nachfolger | Anatomie | Wahrscheinlichkeit |
|---:|---|---:|
| 81 | linke V. jugularis interna | 0,2875 |
| 83 | rechte V. jugularis interna | 0,7125 |

Die Werte stammen aus den Mittelwerten 161 und 399 ml/min einer
Phasenkontrast-MRT-Kohorte gesunder junger Erwachsener. Sie gelten im Modell
nur für Ruhe und Rückenlage. Interindividuelle Seiten- und Haltungsunterschiede
werden später als Parametersätze beziehungsweise durch zusätzliche vertebrale
und extrajuguläre Abflusswege modelliert.

Vorgesehene Sensitivitätsfälle sind `0,5/0,5`, ausgeprägte Rechtsdominanz und
Linksdominanz. Sie ersetzen den Referenzwert nicht.

## Normatives weibliches Referenzprofil

Die historische BVS-Geometrie wurde von einer 1,72 m großen und 69 kg schweren
Frau abgeleitet. Deshalb ist eine weibliche Referenz der kleinste Bruch zum
Bestand. Für `reference-adult-female-rest-supine-v1` gelten zunächst:

| Größe | Wert | Evidenz |
|---|---:|---|
| Herzzeitvolumen | 5,9 L/min | ICRP 89, gesunde erwachsene Frau, Ruhe/Rückenlage |
| Gesamtblutvolumen | 3,9 L | ICRP 89, weiblicher Referenzwert |
| direkte Organanteile | ICRP-89-Tabelle | Literaturwerte; vor Nutzung auf disjunkte terminale Betten abzubilden |
| Kopfseitenabfluss | 0,2875/0,7125 | Stoquart-ElSankari et al. 2009; zustands- und populationsgebunden |

Regionale BVS-Kompartimente wie Schulter, Arm, Becken und Bein entsprechen
nicht direkt den gewebebasierten ICRP-Kategorien. Bis eine nachvollziehbare
Aggregationsregel vorliegt, werden dafür keine vermeintlich physiologischen
Zahlen in einen ausführbaren Graphen geschrieben. Die Dissertationanteile
bleiben im Reproduktionsprofil; eine spätere Zuordnung nutzt Gefäßterritorien
aus einem Ganzkörperatlas oder einem geeigneten MRA-Datensatz.

## Ableitungs- und Validierungsregel

Für ein vollständiges stationäres Profil gilt:

```text
Q_Organ = Q_Herz * f_Organ
Q_Segment = Summe aller nachgelagerten terminalen Q_Organ
p(i -> j) = Q_j / Summe(Q_aller_Nachfolger)
```

Portaler Fluss und Organwerte, die bereits Zuflüsse anderer Organe enthalten,
müssen explizit als solche modelliert werden. Sie dürfen nicht doppelt als
terminale Anteile summiert werden.

Ein Profil wird erst als physiologisch validiert bezeichnet, wenn:

1. Fluss und Masse an jeder Verzweigung und Zusammenführung erhalten bleiben;
2. alle Organanteile disjunkt oder ihre Überlappung explizit modelliert ist;
3. Kalibrier- und Validierungsdaten getrennt sind;
4. Herzzeitvolumen, Organflüsse, Transitzeiten und ausgewählte Gefäßwerte
   innerhalb vorab definierter Toleranzen liegen;
5. Sensitivitäten für Herzzeitvolumen, Organanteile, Geschwindigkeit und
   juguläre Seitenquote berichtet werden.

## Quellen und nächste Datenstufe

- Regine Wendt, *Einsatz von Nanotechnologien in der Präzisionsmedizin*, 2024.
- Stoquart-ElSankari et al., DOI `10.1038/jcbfm.2009.29`.
- ICRP Publication 89, 2002.
- Für spätere anatomische Radien und zusätzliche Wege: ADAVN sowie ein
  geeignetes geschlossenes 1D/0D-Ganzkörpermodell. Solche Fremddaten werden
  erst nach eigener Lizenz-, Versions- und Provenienzprüfung übernommen.

Das Modell ist eine Forschungsreferenz, kein patientenspezifisches Modell und
kein Medizinprodukt.
