<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Systemanforderungen für MEHLISSA Next

**Status:** Baseline für Phase 0  
**Stand:** 26. August 2026  
**Ziel:** Die in der Dissertation beschriebene MEHLISSA-Vision in überprüfbare Anforderungen für eine neue, wissenschaftlich belastbare Simulationsplattform überführen.

## 1. Geltungsbereich

MEHLISSA Next ist eine Forschungsplattform zur Simulation medizinischer Nano- und Molekularkommunikationssysteme im menschlichen Körper. Sie verbindet Modelle auf Körper-, Organ-, Kapillar- und Zellebene mit Nanogeräten, Gateways, Body-Area-Networks und externen Analyse- oder Steuerkomponenten.

Das System ist zunächst **kein klinisches Medizinprodukt** und seine Ergebnisse sind nicht automatisch klinisch valide. Jede Modellvariante muss ihren Gültigkeitsbereich, ihre Datenbasis und ihre Unsicherheit offenlegen.

Die Anforderungen beschreiben den fachlichen Zielzustand. Ihre schrittweise Umsetzung ist in der [Roadmap](../ROADMAP.md) festgelegt. Die [Traceability Matrix](TRACEABILITY_MATRIX.md) ordnet sie Quellen, vorhandenem Code, Meilensteinen und Nachweisen zu.

## 2. Quellen und Leseschlüssel

### 2.1 Primärquellen

| Kürzel | Quelle |
|---|---|
| DISS | Dissertation, insbesondere Kapitel 4–6; Seitenangaben beziehen sich auf die gedruckte Seitenzahl |
| MEH20 | *MEHLISSA: A Medical Holistic Simulation Architecture for Nanonetworks in Humans* |
| BVS18 | *BloodVoyagerS – Simulation of the Work Environment of Medical Nanobots* |
| VIS20 | *BVS-Vis: A Web-based Visualizer for BloodVoyagerS* |
| FP23 | *Proteome Fingerprinting as a Localization Scheme for Nanobots* |
| MEH25 | *MEHLISSA 2.0: Accelerating Full-body Molecular Communication Simulations* |
| RM | [Roadmap für eine neue MEHLISSA-Generation](../ROADMAP.md) |

### 2.2 Herkunft

| Code | Bedeutung |
|---|---|
| `V` | unmittelbar aus der fachlichen Vision oder einer expliziten Forderung der Literatur |
| `B` | in einem vorhandenen MEHLISSA-/BVS-Stand als Verhalten oder Ergebnis beschrieben |
| `A` | für eine belastbare Umsetzung aus Vision und Roadmap abgeleitet |
| `N` | bewusst neue Ergänzung gegenüber der ursprünglichen Vision |

Mehrere Codes sind möglich. `B` bedeutet nicht automatisch, dass die bestehende Implementierung korrekt oder ausreichend validiert ist.

### 2.3 Priorität und Nachweis

| Code | Bedeutung |
|---|---|
| `P0` | Fundament; vor fachlicher Erweiterung erforderlich |
| `P1` | Kern der Dissertationsvision und des ersten vertikalen Demonstrators |
| `P2` | Ausbau zu einer vielseitigen Forschungsplattform |
| `P3` | langfristige Personalisierungs-, Skalierungs- oder kliniknahe Vision |
| `T` | automatisierter Software-, Komponenten- oder Regressionstest |
| `A` | analytischer/numerischer Vergleich oder Invariantenprüfung |
| `R` | Reproduktion eines publizierten Referenzlaufs |
| `E` | Vergleich mit unabhängigen experimentellen oder physiologischen Daten |
| `I` | Inspektion von Schema, Manifest, Dokumentation oder Benutzeroberfläche |

## 3. Übergreifende Systemanforderungen

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| SYS-001 | Das System muss Simulationszeit monoton, mit expliziter Einheit und ausreichender Subsekundenauflösung darstellen. | A | P0 | T/A | RM 3.1, M1 |
| SYS-002 | Ein Lauf muss bei identischer Software, Konfiguration, Eingabe und Seeds deterministisch reproduzierbar sein; statistische Modelle müssen zusätzlich Replikate unterstützen. | A/N | P0 | T/R | RM 2.4, M1 |
| SYS-003 | Zufallsvorgänge müssen benannte, voneinander entkoppelte Zufallsströme verwenden. | A/N | P0 | T | RM M1 |
| SYS-004 | Physikalische Größen müssen explizite, prüfbare Einheiten besitzen; unvereinbare Einheiten dürfen nicht stillschweigend kombiniert werden. | A/N | P0 | T/I | RM 3.2, 6.1 |
| SYS-005 | Kern, Modelle, Szenarien, Datenadapter und Auswertung müssen getrennte Verantwortlichkeiten besitzen. | A | P0 | I/T | RM 2.1–2.2 |
| SYS-006 | Szenariospezifische Logik darf den allgemeinen Simulationskern nicht verändern. | A | P0 | I | RM 2.2, M0 |
| SYS-007 | Das System muss kontrolliert auf ungültige Konfigurationen, numerische Fehler und verletzte Modellinvarianten reagieren. | A/N | P0 | T | RM 3.1, M1 |
| SYS-008 | Modelle müssen von groben Surrogaten bis zu detaillierten Varianten austauschbar sein, ohne dass ein Szenario seine fachliche Bedeutung verliert. | V/A | P1 | T/R | DISS S. 95–97, 133; RM 2.3 |

## 4. Mehrschicht- und Co-Simulationsarchitektur

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| ARC-001 | Das System muss die vier verbundenen, aber eigenständigen Ebenen Körper, Organ, Kapillare und Zelle repräsentieren. | V | P1 | I/T | DISS S. 94–96; MEH20 S. 1–2 |
| ARC-002 | Jede Ebene muss eine eigene räumliche und zeitliche Auflösung, ein eigenes Zustandsmodell und einen dokumentierten Gültigkeitsbereich besitzen. | V/A | P1 | I/T | DISS S. 95; RM 2.1 |
| ARC-003 | Ebenen müssen Entitäten, Populationen, Stoffflüsse, physiologische Zustände und Ereignisse über versionierte Verträge austauschen können. | V/A | P1 | T/A | DISS S. 95–97; RM 3.2 |
| ARC-004 | Übergaben zwischen Ebenen müssen Zeitordnung sowie relevante Erhaltungssätze und Identitäten bewahren. | A/N | P1 | T/A | RM 3.3, M3–M5 |
| ARC-005 | Die Kopplung muss bidirektional sein: untere Ebenen können Ereignisse und aggregierte Wirkungen an höhere Ebenen zurückgeben. | V/A | P1 | T | DISS S. 99–100; MEH20 S. 2–5 |
| ARC-006 | Der Orchestrator muss unterschiedliche Zeitschritte durch definierte Synchronisationspunkte oder Ereignisse koordinieren. | A/N | P1 | T/A | RM 3.3 |
| ARC-007 | Externe Simulatoren müssen über Adapter oder aus ihnen abgeleitete Surrogate eingebunden werden können, ohne Kern oder Ebenen an ein konkretes Fremdprodukt zu koppeln. | V/A | P2 | T/I | DISS S. 96–97, 129–133, 154; MEH20 S. 2–5 |

## 5. Körperebene

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| BODY-001 | Die Körperebene muss einen vollständigen geschlossenen Kreislauf mit großen Gefäßen, Organ-/Regionsübergängen und geeigneten Abstraktionen kleiner Gefäße modellieren. | V/B | P1 | T/R | DISS S. 100; BVS18 S. 3–6 |
| BODY-002 | Jedes Gefäß muss mindestens stabile ID, Typ, Start-/Endgeometrie, Länge, Durchmesser oder Querschnitt, Verbindungen und Datenherkunft besitzen. | V/A | P1 | T/I | DISS S. 100, 113–117 |
| BODY-003 | Das räumliche Modell muss alle relevanten Körperregionen einschließlich Extremitäten in einem dokumentierten 3D-Koordinatensystem abbilden. | V/B | P1 | T/R | DISS S. 101–104, 115–117; BVS18 S. 3–5 |
| BODY-004 | Neue oder personalisierte Körpermodelle müssen ohne Änderung des Simulationscodes geladen und schema-validiert werden können. | V/B/A | P1 | T/I | DISS S. 117, 140–143 |
| BODY-005 | Nanogeräte, seltene Zellen und andere mobile Entitäten müssen injiziert, transportiert, verfolgt und entnommen werden können. | V/B | P1 | T/R | DISS S. 99–104, 113–115 |
| BODY-006 | Bewegungen und Verzweigungen müssen durch konfigurierbare Blutfluss-/Perfusionswerte und Wahrscheinlichkeiten bestimmt werden; Verzweigungsanteile müssen sich zu eins summieren. | V/B/A | P1 | T/A/R | DISS S. 100–101, 118–122 |
| BODY-007 | Die Körperebene muss mindestens ein Kompartiment-/Graphmodell und optional virtuelle laminare Ströme oder importierte Flusslinien unterstützen. | V/B/A | P2 | T/R | DISS S. 115–117; BVS18 S. 4 |
| BODY-008 | Physiologische Zustände wie Ruhe, Belastung, Herzfrequenz und Körperhaltung müssen als austauschbare Parametersätze oder gekoppelte Modelle wirken können. | V/A | P2 | T/E | DISS S. 120–122; BVS18 S. 6 |
| BODY-009 | Bestandteile des Blutes und ihre chemischen, mechanischen oder kommunikationstechnischen Einflüsse müssen in abgestuften Modellvarianten darstellbar sein. | V | P2 | T/E | DISS S. 101; BVS18 S. 3, 6 |
| BODY-010 | Verteilungen müssen über Massenerhaltung, stationäre Verteilung und publizierte BVS-Referenzläufe geprüft werden können. | B/A | P1 | A/R | DISS S. 104–108, 134–137; BVS18 S. 4–6 |

## 6. Organebene

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| ORG-001 | Organe müssen eigenständige Modelle mit Geometrie, Gefäßstruktur, Gewebeklassen, Perfusion und Aktivitätszustand besitzen können. | V | P1 | T/E | DISS S. 118–126; MEH20 S. 3 |
| ORG-002 | Die Organperfusion muss aus literaturbasierten Sollwerten oder gekoppelten Modellen ableitbar und über Zustände veränderbar sein. | V/B | P1 | A/R/E | DISS S. 118–122 |
| ORG-003 | Entitäten und Stoffflüsse müssen über definierte Ein- und Austrittspunkte zwischen Körper- und Organebene übergeben werden. | V/A | P1 | T/A | DISS S. 95, 153–154; RM M3 |
| ORG-004 | Detektionen und Messungen müssen einem Organ oder Gewebe einschließlich Methode und Lokalisierungsunsicherheit zugeordnet werden können. | V | P1 | T/R | DISS S. 122–123, Kap. 6 |
| ORG-005 | Ein grobes Organ-Kompartiment und ein detaillierteres Organ-/Gefäßmodell müssen bei gleicher Schnittstelle austauschbar sein. | V/A | P2 | T/R | DISS S. 123–126 |
| ORG-006 | Import- und Konvertierungsschritte für BodyParts3D, SimVascular oder Patientendaten müssen Einheiten, Achsen, Provenienz und Lizenz nachvollziehbar erhalten. | V/A | P3 | T/I | DISS S. 124–126; RM M3/M8 |

## 7. Kapillarebene

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| CAP-001 | Die Kapillarebene muss Arteriolen, Kapillarbett und Venolen in mindestens einer parametrisierbaren Abstraktion darstellen. | V | P1 | T/A | DISS S. 126–129; MEH20 S. 3–4 |
| CAP-002 | Kapillardichte, Durchmesser, Transitzeit, Blutgeschwindigkeit und Verzweigung müssen organspezifisch konfigurierbar sein. | V/A | P1 | T/E | DISS S. 126–129 |
| CAP-003 | Aktivitätsabhängige Perfusion und präkapilläre Sphinkter müssen den durchströmten Anteil eines Kapillarbetts verändern können. | V | P2 | T/E | DISS S. 127–128 |
| CAP-004 | Austausch zwischen Blut, Endothel, Interstitium und Zelle muss mit expliziten Stoffmengen/Konzentrationen und Erhaltungskontrollen modellierbar sein. | V/A | P1 | T/A/E | DISS S. 127; MEH20 S. 3–4 |
| CAP-005 | Lokale Position, Aufenthaltsdauer, Retention und spätere Adhäsions-/Extravasationsmodelle müssen unterstützt oder als Unsicherheit ausgewiesen werden. | V/A | P2 | T/E | DISS S. 128–129 |
| CAP-006 | Molekulare, elektromagnetische oder andere Kanalmodelle müssen als austauschbare Detailmodelle oder validierte Laufzeit-/Erfolgsverteilungen angebunden werden können. | V/A | P1 | T/R | DISS S. 129, 154; MEH20 S. 3–5 |
| CAP-007 | Cluster-, Relay- und Multi-Hop-Kommunikation im Kapillarbett muss untersuchbar sein und eine abstrahierte Erreichbarkeit für höhere Ebenen liefern können. | V | P2 | T/R | DISS S. 129; MEH20 S. 3–4 |

## 8. Zellebene

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| CELL-001 | Die Zellebene muss Molekül-/Biomarkerfreisetzung, lokale Konzentrationen und Erkennung durch Nanogeräte modellieren können. | V | P1 | T/A/E | DISS S. 129–132 |
| CELL-002 | Rezeptor-/Ligandenbindung, Detektionsschwellen und stochastische Fehlentscheidungen müssen als parametrisierbare Modelle verfügbar sein. | V/A | P1 | T/A/E | DISS S. 130–132; FP23 S. 2–6 |
| CELL-003 | Nanogeräte müssen Signale oder Wirkstoffe freisetzen können; Diffusion, Bindung und Aufnahme müssen koppelbar sein. | V | P1 | T/A/R | DISS S. 130–132, 153–154; MEH20 S. 4–5 |
| CELL-004 | Intrazelluläre Reaktionen müssen über Reaktionszeitverteilungen, ODE/SSA-Modelle oder externe Simulatoren repräsentierbar sein. | V/A | P2 | T/A/E | DISS S. 131–133, 154 |
| CELL-005 | Mindestens Apoptose sowie ein generisches messbares Zellzustandsereignis müssen an höhere Ebenen zurückgegeben werden können. | V | P2 | T/R/E | DISS S. 153–154; MEH20 S. 4–5 |

## 9. Nanogeräte, Nano-IoT und Gateways

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| IOT-001 | Nanogeräte müssen Typ, Fähigkeiten, Nutzlast, internen Zustand, Ziel und Lebenszyklus besitzen; Szenarien müssen spezialisierte Typen ergänzen können. | V/B/A | P1 | T | DISS S. 113–115, 186–188 |
| IOT-002 | Das System muss Nano-IoT-Komponenten aus In-Body-Netz, Gateway, BAN-Gerät sowie Analyse-/Kontrollstation komponieren können. | V | P1 | T/I | DISS S. 96–97; MEH20 S. 2 |
| IOT-003 | Gateways müssen als räumliche Mess- und Kommunikationsorte mit Nano- und Makroseite modelliert werden können. | V | P1 | T/R | DISS S. 117–118, 187–190 |
| IOT-004 | Uplink-Messungen und Downlink-Befehle müssen Latenz, Verlust, Fehlerrate und gegebenenfalls Energie getrennt von biologischen Ergebnissen ausweisen. | V/A | P2 | T/R | DISS S. 96–97, 117–118; RM M6 |
| IOT-005 | Kommunikationsmodelle müssen optional über ns-3 oder andere Simulatoren ausführbar sein, ohne dass physiologische Modelle von ihnen abhängen. | V/A | P2 | T/I | DISS S. 96–100; MEH25 S. 1–2 |

## 10. Daten, Experimente, Evidenz und Ausgaben

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| DATA-001 | Alle Eingabedaten müssen ein versioniertes Schema, Einheiten, Koordinatensystem, Quelle, Lizenz und Prüfsumme besitzen. | A/N | P0 | T/I | RM 6.1 |
| DATA-002 | Ein Experimentmanifest muss Modellvarianten, Parameter, Seeds, Dauer, Injektionen, Beobachtungen und Abbruchbedingungen vollständig beschreiben. | A/N | P0 | T/I | RM 6.4 |
| DATA-003 | Jeder Lauf muss ein Provenienzmanifest mit Software-/Datenversionen, Build, Plattform und Laufzeit erzeugen. | A/N | P0 | T/I | RM 2.4, 6.4 |
| DATA-004 | Ausgaben müssen zwischen Ereignissen, Aggregaten, Stichproben und optionalen Trajektorien unterscheiden und ihr Volumen begrenzen können. | B/A | P1 | T/R | VIS20 S. 1–2; MEH25 S. 1–2; RM M2 |
| DATA-005 | Modellannahmen müssen als publiziert/beobachtet, kalibriert, validiert, abgeleitet oder hypothetisch klassifiziert werden. | A/N | P0 | I | RM 2.5, 6.2 |
| DATA-006 | Kalibrier- und Validierungsdaten müssen getrennt werden; eine Anpassung an Zielwerte darf nicht zugleich als unabhängige Validierung gelten. | A/N | P0 | I/E | RM 6.2 |
| DATA-007 | Medizinische Ergebnisse müssen Unsicherheit, Sensitivität und Gültigkeitsgrenzen zusammen mit dem Schätzwert ausgeben. | A/N | P1 | T/I/E | RM 6.3 |
| DATA-008 | Regressionsläufe müssen numerische/statistische Toleranzen statt ungeprüfter exakter Gleichheit verwenden, sofern stochastische Modelle verglichen werden. | A/N | P1 | T/R | RM 5/M2, 6.2 |

## 11. Visualisierung und Bedienung

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| UX-001 | Die Simulation muss ohne Visualisierung im lokalen Batch- und später im HPC-Betrieb ausführbar sein. | A/N | P1 | T | RM Phase 0/10 |
| UX-002 | Eine entkoppelte Visualisierung muss Körpermodell, Entitäten/Populationen, Zeitverlauf und Dichte-/Heatmap-Daten lesen können. | B/A | P2 | T/I | VIS20 S. 1–2; DISS S. 109–113 |
| UX-003 | Nutzer müssen Zeit navigieren, Ansichten drehen/verschieben/zoomen und Ebenen oder Läufe vergleichen können. | B/A | P2 | I | VIS20 S. 1–2; RM 6.5 |
| UX-004 | Visualisierungen und Berichte müssen reproduzierbar aus gespeicherten Ergebnissen entstehen und dürfen den Simulationszustand nicht verändern. | A/N | P1 | T/I | RM 6.5 |

## 12. Medizinische Szenarien

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| SCN-001 | Fingerprinting muss als erster vertikaler Demonstrator Injektion, Transport, Gewebeerkennung, Nachrichtenbildung, Sammlung und externes Auslesen abbilden. | V/B/A | P1 | R | DISS Kap. 6, bes. S. 185–190; FP23 S. 5–6 |
| SCN-002 | Kontinuierliches Monitoring muss zeitabhängige Biomarker, persönliche Baselines/Schwellen und Alarmwege modellieren können. | V | P2 | T/E | DISS S. 143–152 |
| SCN-003 | In-vivo Liquid Biopsy muss Freisetzung, Transport, Abbau, seltene ctDNA-Erkennung und Vergleich mit Blutproben modellieren können. | V/B | P2 | R/E | DISS S. 155–160 |
| SCN-004 | Metastasenprävention muss Zellablösung, Transport, Erkennung, Wirkstofffreisetzung, Bindung und Zellantwort über alle Ebenen verbinden können. | V | P2 | T/R/E | DISS S. 153–154; MEH20 S. 4–5 |
| SCN-005 | CAR-T muss Zellpopulationen und lokale Agentenmodelle kombinieren und publizierte Interaktionsmodelle als austauschbare Modellkomponente nutzen können. | B/A | P2 | R/E | MEH25 S. 1–2 |
| SCN-006 | Ein Forschungs-Digital-Twin muss schrittweise anatomische, physiologische und biochemische Personalisierung sowie fortlaufende Datenaktualisierung unterstützen. | V/A | P3 | I/E | DISS S. 140–143; RM M8 |

## 13. Nichtfunktionale Qualitätsziele

| ID | Anforderung | Herkunft | Prio | Nachweis | Quelle |
|---|---|---:|---:|---:|---|
| QUA-001 | Der C++-Kern muss auf Windows und Linux mit mindestens MSVC, GCC und Clang automatisiert gebaut und getestet werden. | N | P0 | T | RM M1 |
| QUA-002 | Kritischer Kerncode muss mit Warnungen als Fehler, statischer Analyse und geeigneten Sanitizern geprüft werden. | N | P0 | T | RM M1 |
| QUA-003 | Performance muss durch versionierte Benchmarks bewertet werden; Optimierungen dürfen Referenzergebnisse nur innerhalb festgelegter Toleranzen verändern. | A/N | P1 | T/R | MEH25 S. 1–2; RM Phase 10 |
| QUA-004 | Realistische Größenordnungen müssen durch Agenten-, Populations-, Kompartiment-, Feld- und Surrogatmodelle skalierbar werden. | A | P1 | T/R | MEH25 S. 1–2; RM 2.3 |
| QUA-005 | Öffentliche Schnittstellen, Modelle, Datenschemata und Szenarien müssen versioniert und für Forschende dokumentiert sein. | A/N | P1 | I | RM 6.6 |
| QUA-006 | Patientendaten dürfen erst nach einem dokumentierten Datenschutz-, Einwilligungs- und Pseudonymisierungskonzept verarbeitet werden. | N | P3 | I | RM M8 |

## 14. Abnahmeregeln

Eine Anforderung gilt erst als umgesetzt, wenn:

1. eine verantwortliche Implementierung oder ein versioniertes Datenartefakt existiert;
2. der in der Tabelle genannte Nachweis automatisiert oder nachvollziehbar dokumentiert ist;
3. Gültigkeitsbereich und bekannte Einschränkungen dokumentiert sind;
4. die Traceability Matrix auf Commit, Test, Datensatz oder Bericht verweist;
5. eine Änderung an einem publizierten Referenzverhalten erklärt und wissenschaftlich bewertet wurde.

Die Baseline darf über Architecture Decision Records geändert werden. Eine Abweichung von der Dissertation ist zulässig, muss aber als bewusste Entscheidung mit Nutzen, Kosten und wissenschaftlichen Folgen dokumentiert werden.
