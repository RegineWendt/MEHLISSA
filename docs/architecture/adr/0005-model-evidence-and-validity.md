<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0005: Modell-Evidenz und Gültigkeit explizit behandeln

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Betrifft:** alle Gates; `DATA-005` bis `DATA-008`

## Kontext

MEHLISSA kombiniert Literaturwerte, anatomische Datensätze, historische Simulationsergebnisse, externe Modelle und noch unbestätigte Annahmen. In früheren Prototypen wurden aus Praktikabilität Werte gesetzt oder Ergebnisse nachträglich angepasst. Ohne klare Kennzeichnung besteht die Gefahr, dass Kalibrierung als Validierung oder ein hypothetischer Mechanismus als medizinisch belegt interpretiert wird.

Die Plattform soll gerade dort nützlich sein, wo empirische Daten fehlen. Ungewissheit darf deshalb nicht verborgen, sondern muss ein modellierter Bestandteil sein.

## Entscheidung

Jede veröffentlichte Modellvariante und jeder wesentliche Parametersatz erhält eine Modellkarte mit mindestens:

- fachlicher Aussage und Gültigkeitsbereich;
- Evidenzklasse: `published-observation`, `independently-validated`, `calibrated`, `derived`, `expert-assumption` oder `research-hypothesis`;
- Quellen und Datenversionen;
- Einheiten und Populationsbezug;
- Kalibrierverfahren und getrennte Validierungsdaten;
- Unsicherheiten, Sensitivität und bekannte Gegenbeispiele;
- verantwortliche Person und Reviewdatum.

Ergebnisberichte vererben diese Informationen über Provenienz. Eine auf Zielwerte angepasste Modellvariante darf nicht mit denselben Daten als unabhängig validiert ausgewiesen werden. Historische MEHLISSA-Ergebnisse werden als Reproduktionsbaselines, nicht automatisch als physiologische Wahrheit klassifiziert.

## Folgen

Positiv:

- Wissenschaftliche Aussagen bleiben prüfbar und differenziert.
- Abweichungen zwischen Modellen können sachlich statt kosmetisch behandelt werden.
- Partner sehen gezielt, wo Wetlab-, klinische oder physiologische Daten fehlen.
- Szenarien können trotz Unsicherheit explorativ genutzt werden.

Negativ:

- Daten- und Modellpflege benötigt zusätzliche Zeit.
- Ergebnisse werden komplexer als einzelne Punktwerte.
- Manche heute verwendeten Parameter werden als unzureichend belegt sichtbar.

## Mindestregel für Veröffentlichungen

Jede aus MEHLISSA Next abgeleitete Publikation muss Softwarecommit, Datensatzversionen, Experimentmanifest, Seeds/Replikate, Kalibrier-/Validierungstrennung und die relevanten Modellkarten referenzieren.
