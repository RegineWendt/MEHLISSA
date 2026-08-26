<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Datenlücken und Validierungspartner

**Stand:** 26. August 2026  
**Status:** Kandidaten- und Kompetenzinventar; keine Kooperationszusage wird behauptet

## 1. Ziel

Dieses Dokument benennt, welche Evidenz MEHLISSA Next nicht allein aus Softwareentwicklung gewinnen kann, welche öffentlichen Quellen als Startpunkt dienen und welche fachlichen Partnerrollen für eine belastbare Validierung benötigt werden.

## 2. Partner- und Datenmatrix

| Bereich | Benötigte Evidenz | Öffentliche Startquelle/Kandidat | Erwartetes Artefakt | Zeitpunkt |
|---|---|---|---|---|
| pulmonale Anatomie | Gefäßgeometrie, Ein-/Auslässe, Koordinaten, Segmentierung | [BodyParts3D](https://lifesciencedb.jp/bp3d/info_en/index.html), [Healthy Pulmonary](https://simvascular.github.io/clinical/pulmonary.html), [Vascular Model Repository](https://www.vascularmodel.com/) | versioniertes generisches Lungenmodell mit Lizenz und Geometriebericht | vor M3 |
| pulmonale Hämodynamik | Flüsse, Drücke, Widerstände, Transitzeiten, Randbedingungen in Ruhe/Belastung | SimVascular/VMR; pulmonalmedizinische und hämodynamische Fachbegutachtung | 0D-/1D-Referenzfall, Parameterbereiche und unabhängige Vergleichsdaten | M3/M4 |
| alveoläre Mikrozirkulation | Kapillardichte, Rekrutierung, Transit, Sphinkter-/Perfusionsersatzmodelle | Literatur plus experimentelle Lungenmikroskopie | Kapillarmodellkarte und Messdatensatz für Transit-/Verteilungswerte | M4 |
| Barriere und Stoffaustausch | Blut–Endothel–Interstitium–Alveole, Diffusion/Bindung | Institut für Anatomie Lübeck, AG Barriere-Organe als **Kooperationskandidat** | analytischer Referenzfall und experimentelle Parameterbereiche | M4/M5 |
| Lungenbildgebung | Segmentierung, Gewebe-/Gefäßvergleich, mögliche Individualisierung | Institut für Biomedizinische Optik und radiologische Kompetenz als **Kandidaten** | Bildgebungs-/Konvertierungsvalidierung, später patientenspezifischer Pilot | M3/M8 |
| Proteom-Fingerprints | interindividuelle Stabilität, Krankheitseinfluss, Konzentration, Nachweisgrenze | [Human Protein Atlas](https://www.proteinatlas.org/), Bioinformatik/Proteomik-Partner | versionierter FP9-Datensatz, Kohorten-/Robustheitsanalyse | vor Stufe B von M7 |
| DNA-Tile-Erkennung | Bindungsaffinität, Freisetzung, Assembly und Abbau | bestehende NaBoCom-/DNA-Nanonetzwerk-Expertise; Wetlab-Partner erforderlich | in-vitro Messreihen und validiertes Assembly-Surrogat | M5/M7 |
| Nanogeräteigenschaften | Größe, Strömungsverzögerung, Lebensdauer, Immuninteraktion | Material-/Nanomedizin- und Immunologiepartner | begründete Parameterbereiche und Sensitivitätsprioritäten | M4–M7 |
| Gateway am Handgelenk | Reichweite, Kontaktzeit, Lesefehler, Signalweg nach außen | Kommunikations-/Biosensorikpartner | Gateway-Kanalmodell und Messprotokoll | M6/M7 |
| Zell-/Reaktionsmodelle | Ligandenbindung, Signalwege, Apoptose | [BioModels](https://www.ebi.ac.uk/biomodels/), [Physiome Model Repository](https://models.physiomeproject.org/) und Zellbiologiepartner | qualifizierte Modellkarte plus unabhängiger Referenzdatensatz | M5 |
| translationale Szenarien | medizinische Fragestellung, Endpunkte, vertretbare Interpretation | [ARCN/DZL Nord](https://research.uni-luebeck.de/de/projects/dzl-deutsches-zentrum-f%C3%BCr-lungenforschung-arcn-airway-research-ce/) als **Netzwerkkandidat** | Szenario-Review und klinisch sinnvolle, nicht-diagnostische Endpunkte | M7/M8 |

Die Universität zu Lübeck weist für ihr Institut für Anatomie ausdrücklich Kooperationsangebote unter anderem für Licht-/Elektronenmikroskopie, intravitale Multiphotonenmikroskopie der Lunge und mRNA-Analysen aus. Das ARCN verbindet Universität zu Lübeck, UKSH, LungenClinic Großhansdorf, Forschungszentrum Borstel und weitere norddeutsche Lungenforschung. Diese Nähe ist ein starkes Potenzial, ersetzt aber keine formelle Projektvereinbarung.

## 3. Kritische Datenlücken für das Lungenreferenzmodell

### Vor Implementierung des ersten Organmodells

- eindeutige Definition, ob „Lunge“ zunächst beide Lungen als ein effektives Organ oder getrennte Seiten bezeichnet;
- pulmonalarterieller Einlass und pulmonalvenöser Auslass im neuen Körpergraphen;
- konsistente Einheiten und Koordinatentransformation zwischen MEHLISSA, BodyParts3D und VMR;
- Referenzwerte für Durchfluss, Druck, Volumen und Transitzeit;
- Lizenz und konkreter Versionsstand des gewählten VMR-Modells.

### Vor Kapillardetaillierung

- Verteilung pulmonaler Transitzeiten statt nur eines Mittelwerts;
- Kapillarnetzgröße und sinnvolle Abstraktionsstufe;
- Rekrutierung/Derekrutierung unter verschiedenen physiologischen Zuständen;
- Hämatokrit und zellfreie Randschicht auf relevanter Skala;
- Austauschparameter und Massenerhaltung über die alveoläre Barriere.

### Vor Fingerprinting-Stufe B

- exakte FP9-Genproduktkombination in einer versionierten HPA-Ausgabe;
- Variation in gesunden und erkrankten Lungengeweben;
- physisch detektierte Spezies: mRNA, Protein oder beide;
- lokale Konzentrations- und Bindungsparameter;
- falsch-positive und falsch-negative Nachweiswahrscheinlichkeit.

## 4. Vorgeschlagene Partneransprache

1. **Kurzer Modellsteckbrief:** Zweck, Forschungsstatus, keine klinische Behauptung.
2. **Konkrete Frage:** höchstens drei benötigte Parameter oder ein prüfbarer Referenzfall.
3. **Datenmanagement:** Eigentum, Einwilligung, Lizenz, Anonymisierung und Publikationsrecht vor Datenaustausch.
4. **Validierungsplan:** Kalibrier- und Testdaten vor Beginn trennen.
5. **Gegenseitiger Nutzen:** reproduzierbarer Modelladapter, gemeinsame Modellkarte und zitierbarer Datensatz/Benchmark.

## 5. Mindestbesetzung je Gate

| Gate | Erforderliche fachliche Freigabe zusätzlich zum Software-Review |
|---|---|
| M2 | Physiologie-/Kreislaufreview der Ganzkörperparameter |
| M3 | pulmonale Anatomie und Hämodynamik |
| M4 | Mikrozirkulation und Stofftransport |
| M5 | Zellbiologie/Pharmakologie |
| M6 | Kommunikationssysteme/Biosensorik |
| M7 | Proteomik, DNA-Nanonetzwerk und translationale Lungenmedizin |
| M8 | Datenschutz, Ethik, klinische Methodik und gegebenenfalls Regulierung |

## 6. Partnerstatus

M0 verlangt die **Identifikation** benötigter Kompetenzen und realistischer Kandidaten, nicht bereits geschlossene Kooperationen. Der aktuelle Status ist:

- öffentliche Daten- und Modellquellen identifiziert;
- lokale und überregionale Kompetenzfelder identifiziert;
- noch keine externe Stelle als verantwortlich oder zugesagt eingetragen;
- Kontaktaufnahme erfolgt erst nach Freigabe durch die Projektleitung.
