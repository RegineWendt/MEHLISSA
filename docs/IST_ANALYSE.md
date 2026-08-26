# MEHLISSA – Analyse des aktuellen Stands

**Analysestand:** 26. August 2026  
**Untersuchter Stand:** `main`, Commit `4f4fc5a`  
**Gegenstand:** Literatur in `literature/`, die ns-3-Implementierung in `mehlissa/`, die eigenständige Implementierung in `mehlissa2.0/` sowie die Körper- und Szenariodatensätze im Repository

## 1. Kurzurteil

MEHLISSA ist gegenwärtig ein wissenschaftlich gut begründetes Gesamtkonzept mit einem funktionsfähigen Forschungsprototyp der Körperkreislaufebene. Es ist noch keine vollständige „Medical Holistic Simulation Architecture“ im Sinne der Dissertation.

Der zentrale Abstand zwischen Vision und Implementierung lautet:

- Die Vision ist eine gekoppelte Mehrskalenarchitektur von der Ganzkörperzirkulation bis zu molekularer und intrazellulärer Kommunikation.
- Implementiert ist hauptsächlich der Transport individueller Partikel durch ein grobes Ganzkörper-Gefäßnetz.
- MEHLISSA 2.0 ersetzt ns-3 durch einen kleineren Simulationskern und verbessert die Laufzeit. Es implementiert jedoch nicht die vier in der Literatur beschriebenen Ebenen.
- Die medizinischen Szenarien sind wertvolle Proofs of Concept, aber noch keine validierten physiologischen oder klinischen Modelle.
- Der aktuelle `main`-Branch enthält technische Inkonsistenzen. Insbesondere ist die alte ns-3-Version nach der jüngsten Endokrin-/AVS-Erweiterung in ihrem derzeitigen Zustand nicht konsistent baubar.

### Reifegradübersicht

| Bereich | Einschätzung |
|---|---|
| Wissenschaftliche Vision | hoch |
| Ganzkörper-Transportmodell | Forschungsprototyp |
| Organmodelle | grobe symbolische Repräsentation |
| Kapillarmodelle | nicht implementiert |
| Zell- und Molekülmodelle | einzelne abstrahierte Partikelinteraktionen |
| Anwendungsszenarien | mehrere Demonstratoren, uneinheitlich integriert |
| Softwareplattform | frühe Alpha |
| Physiologische Prognosefähigkeit | noch nicht belastbar |
| Klinischer Digital Twin | Vision, noch keine Implementierung |

## 2. Quellenbasis

Die Analyse beruht insbesondere auf folgenden im Repository enthaltenen Arbeiten:

- [MEHLISSA: A Medical Holistic Simulation Architecture for Nanonetworks in Humans](<../literature/Mehlissa A Medical Holistic Simulation Architecture for Nanonetworks in Humans.pdf>)
- [Dissertation – Simulationskapitel](../literature/Diss_WENDT_Simulationchapters.pdf)
- [BloodVoyagerS – Simulation of the Work Environment of Medical Nanobots](<../literature/BloodVoyagerS - Simulation of the work environment of medical nanobots.pdf>)
- [BVS-Vis: A Web-based Visualizer for BloodVoyagerS](<../literature/BVS-VIS_A Web-based Visualizer for Blood Voyager S.pdf>)
- [Proteome Fingerprinting as a Localization Scheme for Nanobots](../literature/wendt_fingerprinting.pdf)
- [MEHLISSA 2.0: Accelerating Full-body Molecular Communication Simulations](../literature/3760544.3765642.pdf)

## 3. Zielbild aus der Literatur

MEHLISSA soll aus vier verbundenen, aber grundsätzlich eigenständigen Ebenen bestehen. Zwischen ihnen werden mindestens Mobilität, Zustand, Aktivität und Simulationsergebnisse ausgetauscht.

### 3.1 Körperebene

Die Körperebene soll Folgendes leisten:

- geschlossener Ganzkörper-Blutkreislauf;
- Transport und globale Verteilung von Nanogeräten, Zellen und Molekülen;
- realistische Verzweigungs- und Perfusionsverhältnisse;
- dreidimensionale Gefäße mit mehreren virtuellen Strömungen;
- dynamisch ladbare und langfristig patientenspezifische Körpermodelle;
- Injektion, Erfassung und globale Positionsausgabe;
- Body-Area-Network, Gateways sowie Anbindung externer Analyse- und Kontrollsysteme.

### 3.2 Organebene

Die Organebene soll organspezifische Eigenschaften abbilden:

- regionale Gefäßstruktur;
- Organvolumen und lokale Perfusion;
- belastungs-, aktivitäts- und situationsabhängige Durchblutung;
- Organlokalisierung von Nanogeräten;
- Übergang von der globalen Zirkulation in regionale Detailmodelle;
- gegebenenfalls organspezifische Gateways oder Router.

### 3.3 Kapillarebene

Die Kapillarebene soll Arteriolen, Venolen, Kapillarbetten und deren lokale Dynamik repräsentieren:

- Kapillardichte und Verzweigungsstruktur;
- präkapilläre Sphinkter und regulierte Perfusion;
- Stoffaustausch zwischen Blut und Gewebe;
- lokale Retention und Verteilung von Nanogeräten;
- Cluster- und Multi-Hop-Kommunikation;
- Einbindung existierender molekularer Kanalmodelle;
- Ableitung kompakter Ergebnisse für die Organ- und Körperebene.

### 3.4 Zellebene

Die Zellebene soll die eigentlichen biologischen Zielprozesse modellieren:

- Nanogerät-Zell- und Zell-Zell-Kommunikation;
- Erkennung von Biomarkern und Rezeptorbindung;
- Wirkstofffreisetzung und -aufnahme;
- intra- und extrazelluläre Konzentrationen;
- Signalwege und Reaktionskaskaden;
- Zellantworten bis hin zu Apoptose, Immunantwort oder Proliferation.

### 3.5 Nano-IoT und medizinischer Workflow

Die vier physiologischen Ebenen sollen um eine Kommunikations- und Anwendungsebene ergänzt werden:

- In-Body-Nanonetzwerk;
- Mikro-/Nano-Gateway;
- Body-Area-Network;
- externe Analyse- und Kontrollstation;
- Rückkanal für Aktivierung oder Therapie;
- medizinische Workflows für Monitoring, Diagnose, Behandlung und Digital Twins.

Die Literatur beschreibt damit ein Forschungsprogramm und eine Architekturvision. Sie ist keine vollständige technische Spezifikation: Viele Modelle sollen aus externen Simulatoren, Datenbanken oder zukünftigen Experimenten eingebunden werden.

## 4. Implementierter Stand nach Ebenen

### 4.1 Körperebene

Dies ist der mit Abstand reifste Teil von MEHLISSA.

Der aktuelle 95er-Datensatz enthält:

- 95 Gefäß- beziehungsweise Organabschnitte;
- 36 Arterien, 34 Venen und 25 als Organe modellierte Übergänge;
- ein geschlossenes Netz ohne Sackgassen;
- 24 topologische Verzweigungen;
- 23 explizite Datensätze mit Verzweigungswahrscheinlichkeiten;
- neun Organe mit Fingerprint-Bildungszeiten;
- 21 virtuelle Strömungskanäle pro Gefäß.

Partikel können injiziert, durch das Gefäßnetz bewegt, stochastisch an Verzweigungen verteilt und als CSV ausgegeben werden. Die literaturbasierten Übergangswahrscheinlichkeiten sind gegenüber dem ursprünglichen BVS-Modell eine relevante Verbesserung.

Die physiologische Aussagekraft wird jedoch durch starke Abstraktionen begrenzt:

- Alle Gefäße erhalten dieselbe Breite von `0,25`.
- Geschwindigkeiten hängen nur von den Kategorien Arterie, Vene und Organ ab.
- Blutdruck, Gefäßelastizität, Pulsatilität und Massenerhaltung über Querschnitte fehlen.
- Aktivität, Herzfrequenz, Körperhaltung und belastungsabhängige Perfusion fehlen.
- Die Koordinaten liegen fast ausschließlich auf `z = +2` und `z = -2`.
- Organe sind überwiegend vier Zentimeter lange Übergangssegmente und keine Organmodelle.

Das vorhandene Modell ist daher am besten als stochastische Transporttopologie und nicht als Hämodynamikmodell zu verstehen.

Eine weitere Datenauffälligkeit besteht bei Gefäß 9: Es besitzt zwei topologische Nachfolger, aber keinen Eintrag in `95_transitions.csv`. Es verwendet deshalb die Standardverteilung `1/0`, wodurch einer der beiden Abflüsse faktisch ungenutzt bleibt.

### 4.2 Organebene

Organe sind über IDs, statische Übergangswahrscheinlichkeiten und teilweise Fingerprint-Zeiten repräsentiert. Nicht vorhanden sind:

- detaillierte organspezifische Gefäßbäume;
- regulierte Organperfusion;
- Kopplung an Aktivität oder physiologischen Zustand;
- regionale Stoffkonzentrationen;
- Austausch zwischen Blut, Interstitium und Gewebe;
- Integration von BodyParts3D-, SimVascular-, Bildgebungs- oder CFD-Modellen.

Die Dissertation beschreibt vorbereitende Arbeiten für solche Modelle, diese sind aber nicht in die Software eingeflossen.

Die Dateien unter `bodymodels/` stellen noch keine belastbare Personalisierung dar. Insbesondere:

- handelt es sich primär um geometrische Skalierungen;
- enthält das weibliche Modell einen auf zwei Zeilen aufgeteilten Datensatz 51;
- liest MEHLISSA 2.0 Koordinaten mit `std::stoi` ein und verliert dadurch Dezimalstellen;
- bleibt nach einem fehlerhaften Datensatz das Parser-Fehlerflag gesetzt, sodass folgende Gefäße verworfen werden.

### 4.3 Kapillarebene

Die Kapillarebene ist nicht implementiert. Es gibt keine Softwarekomponenten für:

- Arteriolen, Venolen oder Kapillarbetten;
- Sphinkter und lokale Flussregulation;
- Kapillardurchmesser und blutzellbedingte Effekte;
- Stoffaustausch mit dem Gewebe;
- lokale Kommunikationscluster;
- Multi-Hop-Protokolle;
- molekulare Kanalmodelle;
- Übergabe abstrahierter Kapillarergebnisse an höhere Ebenen.

Es existiert derzeit auch keine klar definierte Schnittstelle, über die ein externer Kapillar- oder Kanalsimulator angebunden werden könnte.

### 4.4 Zellebene

`CancerCell`, `TCell` und `CarTCell` sind bewegliche Partikel im Blutstrom. Sie bilden kein Gewebe- oder Zellmodell im Sinne der Dissertation ab.

Nicht vorhanden sind unter anderem:

- Zellmembranen und Rezeptorbindung;
- intra- und extrazelluläre Konzentrationen;
- Signalwege und Reaktionsnetze;
- Wirkstoffaufnahme und Wirkstoffantwort;
- Tumorgewebe und dessen Mikroumgebung;
- Rückkopplung biologischer Prozesse auf Organ- oder Kreislaufzustände.

Die CAR-T-Klassen implementieren statische Wahrscheinlichkeiten für Tötung, Fratrizid und Proliferation. Dies ist ein Szenariodemonstrator, aber keine allgemeine Zellebene.

### 4.5 Kommunikation, Gateway und Nano-IoT

Trotz der ursprünglichen ns-3-Basis gibt es keine eigentliche Netzwerkimplementierung mit Paketen, Kanälen, Protokollen oder Routing. Das als Gateway bezeichnete Gefäß ist gegenwärtig ein passiver Mess- und CSV-Ausgabepunkt.

Nicht umgesetzt sind:

- Kommunikationsprotokolle zwischen Nanogeräten;
- molekulare Kanäle mit Rauschen und Interferenz;
- Gateway-Kommunikation;
- Body-Area-Network;
- externe Steuerung;
- sichere Rückkanäle und Aktuatorik.

## 5. Die beiden Implementierungen

### 5.1 `mehlissa/`: historische ns-3-Version

Die alte Version enthält die umfangreichste Sammlung von Szenarioideen:

- klassische Nanogeräte und globale Verteilung;
- Nanolokatoren und Nanokollektoren;
- LDL-/Monitoring-Modus;
- Liquid-Biopsy-Modus;
- CAR-T-Zellen;
- die neue Endokrin-/AVS-Erweiterung.

Sie ist jedoch eng mit ns-3 und mit globalen, hart codierten Dateien verbunden. Szenariologik befindet sich direkt in `Bloodcircuit` und `Bloodvessel`. Dadurch werden Simulationskern, Physiologie, Experimentkonfiguration und Ausgabe vermischt.

Der aktuelle Stand weist zusätzlich eindeutige Merge-/API-Probleme auf:

- In [`Bloodcircuit.cc`](../mehlissa/Bloodcircuit.cc#L102) wird `circuit` unmittelbar zweimal deklariert.
- Der erste Aufruf verwendet `numberOfLocators`, obwohl der Parameter `numOfLocators` heißt.
- Es wird eine nicht deklarierte Konstruktorvariante aufgerufen.
- Die Endokrin-Erweiterung verwendet `SetHormoneType`, `GetHormoneType`, `SetInjectionTime` und `GetInjectionTime`; diese Methoden existieren in `Nanoparticle` nicht.
- Der Build benötigt eine separate ns-3-Installation und ist aus diesem Repository allein nicht reproduzierbar.

Diese Version sollte deshalb als historische Referenz und Vergleichsimplementierung behandelt werden, nicht als Basis der weiteren Hauptentwicklung.

### 5.2 `mehlissa2.0/`: eigenständiger Simulationskern

MEHLISSA 2.0 entfernt die ns-3-Abhängigkeit und führt einen einfachen zeitschrittbasierten Simulator ein. Positiv sind:

- kleinerer technischer Kern;
- bessere Verzeichnis- und Namensraumstruktur;
- konfigurierbare Pfade für Gefäßnetz, Übergänge und Fingerprints;
- modernere Speicherverwaltung;
- deutlich reduzierte Laufzeit gegenüber der ns-3-Version;
- eine Kernbibliothek, die einen reinen C++23-Syntaxcheck besteht.

MEHLISSA 2.0 ist dennoch überwiegend eine Portierung der alten Körperkreislauflogik. Organ-, Kapillar-, Zell- und Kommunikationsschichten wurden nicht ergänzt.

Der Build- und Ausführungsweg ist unvollständig:

- [`CMakeLists.txt`](../mehlissa2.0/src/CMakeLists.txt#L32) erzeugt nur `MehlissaCancer`.
- Der allgemeine Einstieg `start-mehlissa.cc` wird nicht gebaut.
- [`start-mehlissa.cc`](../mehlissa2.0/src/experiments/start-mehlissa.cc#L34) enthält mehrfach deklarierte Variablen und würde nicht kompilieren.
- Der Standardpfad nennt `95_fingerprint.csv`, vorhanden ist `95_fingerprints.csv`.
- Es gibt keine automatisierten Tests, keine CI und keine reproduzierbaren Referenzläufe.

## 6. Zentrale technische Befunde

### 6.1 Zeitmodell

Der Simulator akzeptiert einen konfigurierbaren Zeitschritt. Jedes Gefäß verwendet jedoch fest `m_deltaT = 1` und bewegt Partikel damit immer um eine Sekunde weiter. Der konfigurierte Zeitschritt verändert nur die globale Uhr.

Die globale Uhr speichert Millisekunden als Ganzzahl, gibt Sekunden aber per Ganzzahldivision zurück. Subsekunden-Zeitschritte gehen dadurch verloren.

Folgen:

- Simulationen mit `simulationStep != 1` sind nicht numerisch konsistent.
- Bewegung, Alterung, Interaktion und Ereigniszeit können unterschiedliche Zeitbasen verwenden.
- Konvergenz- oder Zeitschrittstudien sind nicht möglich.

### 6.2 Geometrie

[`Position::CalcDistance`](../mehlissa2.0/src/utils/Position.cc#L44) verwendet für die z-Differenz `b.z - b.z` und ignoriert z vollständig. Weitere Längen- und Interaktionsberechnungen verwenden nur x und y. Organe werden über Sonderfälle in z behandelt.

Die behauptete 3D-Unterstützung ist deshalb gegenwärtig eine 2D-Geometrie mit einzelnen z-Sonderfällen.

### 6.3 Zufallszahlen und Reproduzierbarkeit

In `Randomizer` und `RandomStream` werden bei der Initialisierung lokale Zufallsgeneratoren deklariert, welche die eigentlichen statischen beziehungsweise Member-Generatoren verdecken. Die gewählte Seed-Initialisierung wirkt dadurch nicht wie vorgesehen.

Mögliche Folgen:

- unbeabsichtigt deterministische Abläufe;
- korrelierte Zufallsströme;
- unwirksamer `isDeterministic`-Parameter;
- schwer nachvollziehbare Resultate bei späterer Parallelisierung.

### 6.4 Partikel- und CAR-T-Logik

Bei einer CAR-T-Injektion zum Zeitpunkt null werden in [`BloodCircuit.cc`](../mehlissa2.0/src/bloodcircuit/BloodCircuit.cc#L348) versehentlich Krebszellen erzeugt.

Weitere Einschränkungen:

- CAR-T-Proliferation kann durch alle passenden Partikel im selben Gefäß stimuliert werden, unabhängig von der räumlichen Nähe.
- Fratrizid wird bei Distanz `<= 0` geprüft und kann die eigene Zelle einschließen.
- Zellinteraktionen vergleichen Partikel im Gefäß weitgehend paarweise und skalieren quadratisch.
- Replikations- und Lebensdauern liegen weit außerhalb der kurzen Benchmark-Zeiträume.
- Detektionsradien und räumliche Modellauflösung passen teilweise nicht zusammen.

Die CAR-T-Benchmarks bewerten daher primär Transport- und Laufzeitleistung und nicht die biologische Validität einer Therapievorhersage.

### 6.5 Ausgabe und Skalierung

Der Simulator schreibt standardmäßig jeden Partikelzustand in jedem Zeitschritt. Dies erzeugt große Datenmengen und kann die Laufzeit dominieren.

In [`Printer.cc`](../mehlissa2.0/src/utils/Printer.cc#L74) wird der konfigurierte Partikelausgabemodus bei jeder Ausgabe auf null gesetzt. Spezialisierte LDL- und Liquid-Biopsy-Ausgaben funktionieren dadurch nicht wie vorgesehen.

Parallelisierung ist nur vorbereitet. Bei `parallel > 1` läuft die Simulation weiterhin sequenziell.

### 6.6 Speicherverwaltung

Die Gefäße halten `shared_ptr` auf ihre Nachfolger. Da der Kreislauf geschlossen ist, entstehen Referenzzyklen. Zusätzlich ruft `BloodCircuit` den Destruktor des von einem `shared_ptr` verwalteten `Printer` manuell auf.

Vor größeren Experimenten müssen Lebensdauer, Besitzverhältnisse und Speicherlayout neu geordnet werden.

### 6.7 Datenmodell und Parametrisierung

Der aktuelle Zustand verwendet:

- implizite Einheiten;
- hart codierte Konstanten;
- CSV-Dateien ohne Schema- oder Versionsangabe;
- kaum Eingabevalidierung;
- keine dokumentierte Parameterprovenienz;
- keine Unsicherheiten oder Verteilungen für physiologische Parameter;
- keine zentrale Experimentkonfiguration.

Das erschwert Reproduzierbarkeit und wissenschaftliche Vergleichbarkeit.

## 7. Stand der Szenarien

### 7.1 Proteom-Fingerprinting

Fingerprinting ist der am weitesten ausgearbeitete verbindende Anwendungsfall. Implementiert ist eine abstrahierte Zustandsmaschine:

1. Ein Nanolokator erreicht seine Zielorgan-ID.
2. Ein organspezifischer Timer läuft.
3. Das Gefäß wird dauerhaft als „Fingerprint-Nachricht aktiv“ markiert.
4. Ein passender Nanokollektor erhält beim Durchqueren ein Detektionsflag.

Nicht simuliert werden Genexpression, Konzentration, Bindungswahrscheinlichkeit, Krankheitsmarker, Tile-Anzahl und chemische Assembly. Die aus NetTAS abgeleitete Assembly-Dauer wird als reine Verzögerung verwendet.

Trotzdem ist Fingerprinting der beste Kandidat für einen ersten vollständigen Mehrschicht-Demonstrator.

### 7.2 Kontinuierliches Monitoring und Liquid Biopsy

LDL und ctDNA sind als hart codierte Partikelmodi vorhanden. Partikel erhalten Größen-/Geschwindigkeitsfaktoren und werden über räumliche Nähe zu Nanogeräten gezählt.

Die veröffentlichten Wahrscheinlichkeiten beruhen teilweise auf vereinfachenden Unabhängigkeitsannahmen. Eine experimentelle Kalibrierung und Sensitivitätsanalyse fehlen. In MEHLISSA 2.0 sind diese Modi aktuell nicht regulär ausführbar, weil der allgemeine Programmeinstieg nicht gebaut wird und nicht kompiliert.

### 7.3 Digital Twin

Der Digital Twin ist gegenwärtig eine Vision mit geometrischer Skalierungsdemonstration. Es fehlen:

- Patientendatenmodell;
- Bildgebungsimport;
- Vital- und Laborparameter;
- persönliche Parameterkalibrierung;
- Therapieplanung und Rückkopplung;
- Unsicherheitsquantifizierung;
- Datenschutz- und Provenienzkonzept.

### 7.4 Metastasenprävention

Das Metastasenszenario verbindet die vier Ebenen am vollständigsten: Ablösung einer Krebszelle, Transport, Erkennung, molekulare Kommunikation, Wirkstofffreisetzung und Apoptose. Es ist bislang konzeptionell geblieben und eignet sich langfristig als Integrations- und Capstone-Szenario.

### 7.5 CAR-T

CAR-T ist der wichtigste Performance-Demonstrator von MEHLISSA 2.0. Die biologische Interaktion ist jedoch stark abstrahiert und weist die in Abschnitt 6.4 beschriebenen Probleme auf. Realistische Zellzahlen bleiben selbst nach der Laufzeitverbesserung um viele Größenordnungen außer Reichweite.

### 7.6 Endokrin-/AVS-Erweiterung

Die jüngste Erweiterung fügt getrennte Nebennieren-/Venenabschnitte sowie Aldosteron- und Cortisolideen hinzu. Sie liegt ausschließlich in der alten ns-3-Version und ist aktuell nicht lauffähig:

- fehlende Methoden in `Nanoparticle`;
- inkonsistente Konstruktoraufrufe;
- das 104er-Gefäßmodell wird nicht automatisch ausgewählt;
- Hormone erhalten `delay = 0`, was im vorhandenen Bewegungsmodell Geschwindigkeit null bedeutet;
- Sampling-IDs in README, Kommentaren und Implementierung stimmen nicht vollständig überein;
- simulierte Konzentrationen werden nachträglich an klinische Zielwerte angepasst, wodurch eine unabhängige Validierung zirkulär würde.

Die Erweiterung ist eine wertvolle Szenarioskizze, aber noch kein implementiertes Modell.

### 7.7 Visualisierung

BVS-Vis ist ein separates Three.js-basiertes Projekt und nicht Teil dieses Repositorys. MEHLISSA besitzt keine integrierte Ergebnisexploration, Experimentverwaltung oder Visualisierung.

## 8. Fehlende Engineering-Grundlagen

Für eine nachhaltige Forschungsplattform fehlen derzeit:

- ein reproduzierbarer Build für alle unterstützten Plattformen;
- Unit-, Integrations- und Regressionstests;
- Continuous Integration;
- versionierte Releases und Referenzdatensätze;
- klare Abgrenzung zwischen Kern, Modell, Szenario und Ausgabe;
- maschinenlesbare Konfigurationen mit Schema;
- explizite Einheiten;
- Seeds und Experimentmanifeste;
- strukturierte Logs und aggregierte Ergebnisformate;
- Benchmark- und Validierungssuite;
- Contributor-Dokumentation;
- repositoryweite Lizenzdatei trotz GPLv2-Hinweisen in Quelldateien.

## 9. Stärken des Bestands

Die kritischen Befunde bedeuten nicht, dass MEHLISSA neu erfunden oder verworfen werden muss. Der Bestand besitzt erhebliche Werte:

- eine klare und wissenschaftlich relevante Mehrskalenvision;
- einen geschlossenen und überwiegend konsistenten Ganzkörpergraphen;
- literaturbasierte statische Perfusionswahrscheinlichkeiten;
- mehrere publizierte Anwendungsszenarien;
- einen gegenüber ns-3 kleineren Simulationskern;
- einen mit etwa 9.000 C++-Zeilen noch überschaubaren Codebestand;
- vorhandene Partikel- und Gerätekonzepte als Ausgangspunkt für neue Schnittstellen;
- Fingerprinting als besonders geeigneten schichtübergreifenden Demonstrator.

## 10. Gesamteinschätzung

MEHLISSA ist eine starke Forschungsvision mit einem nützlichen, aber technisch und wissenschaftlich noch nicht ausreichend abgesicherten Ganzkörper-Transportprototyp.

Die alte ns-3-Version sollte als historische Referenz erhalten bleiben. Die weitere Entwicklung sollte auf einem stabilisierten, neu modularisierten Nachfolger des 2.0-Kerns aufbauen. Dabei sollten nicht weitere Szenarien direkt in `BloodCircuit` oder `BloodVessel` eingebettet werden.

Der nächste Entwicklungsschritt sollte daher nicht primär „mehr Features“ lauten, sondern:

> Eine reproduzierbare, validierbare und mehrskalige Plattform schaffen, in die Körper-, Organ-, Kapillar-, Zell- und Kommunikationsmodelle über wohldefinierte Schnittstellen eingebunden werden können.

Die daraus abgeleitete Entwicklungsplanung befindet sich in der [MEHLISSA-Roadmap](ROADMAP.md).
