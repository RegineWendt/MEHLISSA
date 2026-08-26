# Zielnutzer und priorisierte Arbeitsabläufe

**Stand:** 26. August 2026  
**Status:** M0-Baseline

## 1. Produktauftrag

MEHLISSA Next ist primär eine wissenschaftliche Modellierungs- und Experimentplattform. Sie soll verschiedene Fachdisziplinen an einer reproduzierbaren Mehrskalensimulation zusammenarbeiten lassen, ohne von jeder Nutzergruppe C++-Kernentwicklung zu verlangen.

Klinische Entscheidungsunterstützung, Echtzeitbetrieb an Patientinnen und Patienten und regulatorische Produktfunktionen gehören nicht zum ersten Produktumfang.

## 2. Nutzerrollen

### P1 – Simulations- und Plattformentwickler

**Ziel:** Kern, Kopplung, Datenformate und Performance verlässlich weiterentwickeln.  
**Benötigt:** C++-API, CMake/CI, Debugging, Invarianten, Benchmarks, ADRs.  
**Erfolg:** Änderungen sind getestet, reproduzierbar und beeinflussen Referenzläufe nur erklärt.

### P1 – Nanonetzwerk-/Kommunikationsforscher

**Ziel:** Geräte-, Kanal-, Gateway- und Protokollvarianten in realistischer Mobilität vergleichen.  
**Benötigt:** Szenariokonfiguration, austauschbare Kommunikationsmodelle, Netzwerkmetriken, Ensembles.  
**Erfolg:** Ein Kommunikationsmodell kann ohne Änderung physiologischer Modelle ersetzt werden.

### P1 – Biomedizinischer Modellentwickler

**Ziel:** Organ-, Kapillar-, Zell- oder Reaktionsmodelle beitragen und validieren.  
**Benötigt:** dokumentierte Modellverträge, Einheiten, Python-/Dateiadapter, Modellkarten, Referenzfälle.  
**Erfolg:** Ein Modell lässt sich unabhängig testen und mit grober/detaillierter Variante vergleichen.

### P1 – Experimenteller oder klinischer Forschungspartner

**Ziel:** Annahmen, Parameterbereiche und Simulationsergebnisse an realen Messungen prüfen.  
**Benötigt:** verständliche Modellkarten, Datenwörterbuch, Sensitivität, Unsicherheit, exportierbare Berichte.  
**Erfolg:** Mess- und Simulationsgrößen sind eindeutig zugeordnet; Daten für Kalibrierung und Validierung bleiben getrennt.

### P2 – Szenarioautor und wissenschaftlicher Anwender

**Ziel:** Experimente konfigurieren, Replikate ausführen, vergleichen und publizieren.  
**Benötigt:** validiertes Manifest, CLI/Python-API, Vorlagen, Ergebniszusammenfassung, Provenienz.  
**Erfolg:** Ein neues Experiment benötigt keine Änderung des Simulationskerns.

### P2 – Studierende und Lehrende

**Ziel:** Modelle verstehen, kleine Varianten untersuchen und reproduzierbare Übungen durchführen.  
**Benötigt:** installierbare Releases, Tutorials, kleine Datensätze, Visualisierung, kurze Laufzeiten.  
**Erfolg:** Ein Referenzexperiment läuft lokal mit nachvollziehbarem Ergebnis.

### P2 – Projektleitung, Reviewer und Publikationsleser

**Ziel:** wissenschaftliche Aussage, Evidenz und Reproduzierbarkeit beurteilen.  
**Benötigt:** Traceability, Modellkarten, Commit/Datenversion, Vergleichs- und Validierungsbericht.  
**Erfolg:** Jede veröffentlichte Zahl lässt sich auf Experiment, Modell und Quelle zurückführen.

## 3. Priorisierte Arbeitsabläufe

| Rang | Workflow | Primäre Rolle | M-Abnahme |
|---:|---|---|---|
| 1 | vorhandenen Referenzlauf aus Manifest reproduzieren | alle technischen Nutzer | M1/M2 |
| 2 | Parameter/Seed variieren und Replikate vergleichen | Szenarioautor | M1 |
| 3 | 95er-Körpermodell laden, validieren und BVS-Verteilung reproduzieren | Plattformentwickler | M2 |
| 4 | neue Modellvariante über stabilen Vertrag ergänzen | biomedizinischer Modellentwickler | M3–M5 |
| 5 | Fingerprinting von Injektion bis Handgelenk ausführen | interdisziplinäres Team | M7 |
| 6 | externen Datensatz importieren und Provenienz prüfen | Daten-/Modellentwickler | M2/M3 |
| 7 | Kalibrierung, unabhängige Validierung und Unsicherheit berichten | experimenteller Partner | M3–M7 |
| 8 | Ensemble-/Sensitivitätslauf lokal oder im Batch/HPC starten | wissenschaftlicher Anwender | M7/M10 |
| 9 | gespeicherte Läufe visualisieren und vergleichen | Studierende/Forschende | M7 |
| 10 | reproduzierbares Publikationspaket exportieren | Projektleitung/Autor | M7 |

## 4. Bedienoberflächen nach Reifegrad

1. **M1–M2:** C++-CLI, validierte Manifestdatei, strukturierte Resultate.
2. **M2–M4:** Python-API für Experimente, Ensembles und Analyse.
3. **M3–M7:** standardisierte Adapter für externe Modelle und Daten.
4. **M7:** entkoppelte interaktive Visualisierung und Laufvergleich.
5. **M8+:** kontrollierte Oberflächen für patientenspezifische Forschungsdaten.

Eine grafische Oberfläche ist kein Ersatz für ein archiviertes Manifest. Jeder interaktiv erzeugte Lauf muss in dieselbe reproduzierbare Repräsentation exportiert werden.

## 5. Definition eines nutzbaren Releases

Ein Release ist für eine Rolle nutzbar, wenn:

- Installation und Minimalexperiment dokumentiert sind;
- mindestens ein typischer Workflow ohne Quellcodeänderung durchläuft;
- Eingabefehler vor Simulationsstart verständlich gemeldet werden;
- Ergebnisse Einheiten, Provenienz und Gültigkeitsgrenzen enthalten;
- die Laufzeit auf der Zielhardware dokumentiert ist;
- eine zitierbare Version und ein archiviertes Referenzexperiment existieren.

## 6. Nicht im kurzfristigen Umfang

- Diagnose oder Therapieentscheidung für individuelle Patientinnen und Patienten;
- automatische Verarbeitung identifizierbarer Gesundheitsdaten;
- Echtzeitsteuerung realer Nanogeräte;
- Zertifizierung als Medizinprodukt;
- universelle biologische Genauigkeit ohne szenariospezifische Validierung.
