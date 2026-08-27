<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0017: Getrennte BVS-Dynamik- und Perfusionsregression

- **Status:** Accepted
- **Datum:** 27. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M2.4; `BODY-006`, `BODY-010`, `ORG-002`

## Kontext

BloodVoyagerS berichtet Verteilungsexperimente mit 94 Gefäßen. Die Dissertation
ergänzt ein 95. Gefäß und kalibriert Organ- und Regionalflüsse wesentlich
feiner. Ein einzelner vermeintlich exakter Reproduktionswert würde diese beiden
Modellgenerationen vermischen. Hinzu kommt, dass die im Paper als
„standard deviation“ bezeichnete Rechnung methodisch als mittlere absolute
Gefäßabweichung beschrieben wird.

## Entscheidung

1. BVS-Gleichgewicht, Injektionsort und Populationsskalierung werden als
   vergleichende dynamische Claims geprüft, nicht als identische Reproduktion
   des 94-Gefäß-Codes.
2. Die 23 Perfusionssollwerte der Dissertation werden in einem getrennten
   stationären Gate geprüft und ausdrücklich als Kalibrierungsregression, nicht
   als unabhängige physiologische Validierung bezeichnet.
3. Die dynamische Metrik heißt normierte mittlere absolute Abweichung. Die
   Populationsskalierung verwendet Total Variation.
4. Alle Schwellen werden vor dem ersten vollständigen Lauf festgelegt und im
   maschinenlesbaren Bericht gespeichert.
5. Der Lauf ist ereignisgetrieben, integriert Aufenthaltszeiten exakt, nutzt
   einen benannten deterministischen Zufallsstrom und prüft exakte
   Populationserhaltung.
6. Ein versioniertes JSON Schema und eine bytegenaue Golden Reference machen
   Methode, Eingaben, Ergebnisse und Gates automatisiert prüfbar.

## Folgen

Positiv:

- Die publizierten Aussagen werden reproduzierbar operationalisiert.
- Unterschiede zwischen BVS 2018 und dem Dissertationsprofil bleiben sichtbar.
- Nachträgliches Verschieben von Toleranzen wird durch Bericht und Golden
  Reference erkennbar.
- Lange Läufe bleiben ohne künstlichen globalen Integrationszeitschritt
  ausführbar.

Grenzen:

- Der Referenzläufer prüft das gleiche Übergangs- und Transitzeitmodell, ist
  aber eine spezialisierte ereignisgetriebene Messimplementierung und nicht der
  allgemeine M2.3-Zeitschritt-Host.
- Eine bestandene Kalibrierungsregression belegt keine klinische oder
  patientenspezifische Gültigkeit.
- Das künftige normative Profil benötigt unabhängige Daten und eigene Gates.

## Alternativen

- **Historischen Code unverändert portieren:** verworfen als Hauptgate, weil
  dies den überholten 94-Gefäß-Vertrag und dessen implizite Annahmen
  konservieren würde. Er bleibt eine Vergleichsquelle.
- **Paperzahlen exakt als Sollwerte erzwingen:** verworfen, weil Topologie,
  Übergänge und Geschwindigkeitsmodell verschieden sind.
- **Nur den stationären Flussgraphen prüfen:** verworfen, weil damit weder
  transienter Transport noch Injektionsort- und Populationseffekte erfasst
  würden.

## Nachweis

- `docs/m2/BVS_REFERENCE_REGRESSION.md`
- `data/reference-results/bvs95-dissertation-rest-m2.4.json`
- `bvs_reference_tests`
