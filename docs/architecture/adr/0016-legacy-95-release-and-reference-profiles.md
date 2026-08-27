<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0016: Freigabe der 95er-Daten und Trennung der Referenzprofile

- **Status:** Accepted
- **Datum:** 27. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M2.2; `BODY-001`, `BODY-002`, `BODY-005`, `BODY-006`, `DATA-001`

## Kontext

ADR-0007 und ADR-0014 ließen die Migration des 95er-Datensatzes offen, weil
eine ausdrückliche Datenfreigabe, der fehlende Übergang von Gefäß 9 und eine
absolute hämodynamische Parametrisierung fehlten. Die Projektleitung hat nun
bestätigt, dass `95_vasculature.csv`, `95_transitions.csv` und
`95_fingerprints.csv` verwendet und neu lizenziert werden dürfen.

Eine einzige Datei kann gleichzeitig jedoch nicht historische Ergebnisse
reproduzieren und eine anatomisch-physiologische Referenz vortäuschen. Die
Legacy-Geometrie ist schematisch; die Übergänge bilden die in der Dissertation
kalibrierte Ruheperfusion ab, während viele absolute Gefäßparameter fehlen.

## Entscheidung

1. Die drei freigegebenen Quelldateien und daraus erzeugte Next-Daten stehen
   unter `CC-BY-4.0`. Manifest, Prüfsummen und Sidecars dokumentieren die
   Freigabe und Transformation.
2. Die Quelldateien bleiben bytegenau erhalten. Der Konverter ändert sie nicht
   und bildet jede Legacy-ID bijektiv auf `bvs95-NNN` ab.
3. Das ausführbare Profil `bvs95-dissertation-rest-v1` dient Reproduktion und
   Regression. Es übernimmt Topologie und Übergänge, konvertiert Koordinaten
   von Zentimetern in Meter und normalisiert den Kreislauf explizit auf
   6,0 L/min.
4. Die historischen Geschwindigkeitsklassen von 10 cm/s für Arterien,
   3,7 cm/s für Venen und 1 cm/s für Organbetten – mit 5 cm/s für die beiden
   Herzsegmente – bleiben für Transitzeitvergleiche erhalten. Querschnitt und
   Durchmesser werden aus `A = Q/v` als äquivalente Transportparameter
   abgeleitet und nicht als anatomische Messwerte bezeichnet.
5. Gefäß 9 besitzt in der kanonischen Migration die Nachfolger 81 links und
   83 rechts. Der Ruhe-/Rückenlagewert wird aus den veröffentlichten
   Kohortenmitteln 161 ml/min links und 399 ml/min rechts abgeleitet:
   `0,2875/0,7125`.
6. Der Legacy-Lauf mit implizitem `1/0` bleibt ausschließlich über die
   unveränderte Quelle als `legacy-as-run` nachvollziehbar. Er wird wegen der
   Nullwahrscheinlichkeit nicht als gültiger Next-Gefäßgraph ausgegeben.
7. Ein getrenntes künftiges Profil
   `reference-adult-female-rest-supine-v1` verwendet normative physiologische
   Werte. Ausgangspunkte sind 5,9 L/min Herzzeitvolumen und 3,9 L Blutvolumen
   aus ICRP 89. Es wird erst ausführbar, wenn Organ- und Regionalflüsse ohne
   Doppelzählung auf den Graphen abgebildet und unabhängig geprüft sind.
8. Jeder Parameter trägt Evidenzklasse, Quelle, Gültigkeitsbereich und – sofern
   quantifizierbar – Unsicherheit. Ein kalibrierter Wert ist kein unabhängiger
   Validierungsnachweis.

## Folgen

Positiv:

- M2.2 besitzt einen reproduzierbaren, schema- und semantikvalidierten
  95-Segment-Datensatz.
- Die Korrektur von Gefäß 9 ist belegt und als zustandsabhängige Annahme
  sichtbar.
- Historische Regression bleibt möglich, ohne schematische Parameter als
  klinische Physiologie auszugeben.
- Der Konverter prüft die dokumentierte Legacy-Struktur streng und erzeugt
  Fluss-, Geometrie- und Übergangsinvarianten deterministisch.

Negativ und Grenzen:

- Der M2.2-Datensatz ist trotz konsistenter SI-Größen kein anatomisch
  validierter Ganzkörperkreislauf.
- Der juguläre Mittelwert beschreibt weder individuelle Seitenverhältnisse
  noch den veränderten extrajugulären Abfluss im Stehen.
- Ein vollständiges physiologisches Profil benötigt weitere Zuordnung und
  fachliche Validierung; diese Arbeit gehört zu M2.6 und späteren
  Validierungsinkrementen.

## Quellen

- Regine Wendt, *Einsatz von Nanotechnologien in der Präzisionsmedizin*, 2024,
  Abschnitt 4.3.1 und Tabelle A.1.
- Stoquart-ElSankari et al., *A phase-contrast MRI study of physiologic
  cerebral venous flow*, J Cereb Blood Flow Metab 29 (2009),
  DOI `10.1038/jcbfm.2009.29`.
- ICRP Publication 89, *Basic Anatomical and Physiological Data for Use in
  Radiological Protection: Reference Values*, 2002.

## Alternativen

- **Gefäß 9 auf 50/50 setzen:** einfach, aber ohne Bezug zum gefundenen
  Kohortenmittel; nur als Sensitivitätsvariante geeignet.
- **Arteriellen Kopfzufluss als venöse Seitenquote verwenden:** verworfen,
  weil ein gemischtes Organbett und kollateraler venöser Abfluss daraus keine
  valide juguläre Quote machen.
- **Den Legacy-Graph direkt physiologisch nennen:** verworfen, weil Geometrie,
  Geschwindigkeiten und Regionalzuordnung dafür nicht hinreichend validiert
  sind.
