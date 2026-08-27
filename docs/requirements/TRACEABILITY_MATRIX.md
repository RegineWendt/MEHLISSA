<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Traceability Matrix – MEHLISSA Next

**Stand:** 27. August 2026
**Bezugsdokument:** [Systemanforderungen](SYSTEM_REQUIREMENTS.md)

## 1. Zweck

Diese Matrix verbindet jede Systemanforderung mit ihrer fachlichen Quelle, dem heutigen Implementierungsstand, dem geplanten Roadmap-Gate und einem konkreten Nachweisartefakt. Sie ist die operative Prüfliste für Architektur-Reviews und Releases.

Statuscodes:

- `DONE`: im Next-Bootstrap vorhanden und automatisiert geprüft;
- `PART`: teilweise vorhanden, aber noch nicht vollständig gegen die Anforderung geprüft;
- `LEGACY`: nur in einer historischen Implementierung oder Publikation vorhanden;
- `SPEC`: spezifiziert, noch nicht implementiert;
- `RESEARCH`: benötigt zusätzlich fachliche Daten, Kalibrierung oder experimentelle Evidenz.

Ein `DONE` in dieser Matrix setzt einen erfolgreichen Nachweis voraus. Der bloße Name einer Klasse reicht nicht.

## 2. Fundament und Architektur

| ID | Quelle | Stand 27.08.2026 | Ziel | Vorgesehener Nachweis |
|---|---|---|---|---|
| SYS-001 | RM M1 | DONE | M1 | `simulation_clock_tests`, plattformübergreifender CTest |
| SYS-002 | RM 2.4 | PART | M1/M7 | bytegleicher M1-Kernreferenzlauf auf MSVC/GCC/Clang; Replikatplanung und fachliche Modellnachweise folgen |
| SYS-003 | RM M1 | DONE | M1 | `random_stream_tests` einschließlich Streamnamen |
| SYS-004 | RM 3.2 | DONE | M1 | dimensionssichere Typen für Zeit, Länge, Fläche, Volumen, Geschwindigkeit, Stoffmenge und Konzentration; Compile-/Unit-Tests |
| SYS-005 | RM 2.1–2.2 | PART | M0/M1 | Architekturprüfung der Zielstruktur und Abhängigkeitsregeln |
| SYS-006 | RM M0 | PART | M1 | CI-Regel und Review: `core/` importiert kein `scenarios/` |
| SYS-007 | RM M1 | DONE | M1 | stabile Fehlercodes/CLI-Statuswerte sowie Negativtests für Konfiguration, Überlauf, Lifecycle, Log- und Checkpointinvarianten |
| SYS-008 | DISS S. 95–97, 133 | SPEC | M3–M5 | Dasselbe Szenario mit Surrogat und Detailmodell |
| ARC-001 | DISS S. 94–96 | SPEC | M3–M5 | Vier getrennte `ModelComponent`-Implementierungen |
| ARC-002 | DISS S. 95 | SPEC | M3–M5 | Modellkarten mit Raum-/Zeitskala und Gültigkeitsbereich |
| ARC-003 | DISS S. 95–97 | SPEC | M3 | Schema- und Vertragstests für Austauschobjekte |
| ARC-004 | RM 3.3 | SPEC | M3 | Erhaltungs- und Identitätstests über Modellgrenzen |
| ARC-005 | DISS S. 99–100 | SPEC | M3–M5 | Rückgabe eines Detektions-/Zellereignisses nach oben |
| ARC-006 | RM 3.3 | SPEC | M3 | deterministischer Mehrzeitskalen-Kopplungstest |
| ARC-007 | DISS S. 96–97, 133 | SPEC | M4/M5 | Referenzadapter plus äquivalentes Surrogat |

## 3. Körper- und Organebene

| ID | Quelle | Stand 27.08.2026 | Ziel | Vorgesehener Nachweis |
|---|---|---|---|---|
| BODY-001 | DISS S. 100; BVS18 | DONE | M2 | vollständiger, stark zusammenhängender und schema-validierter 95-Segment-Graph; Konverter- und Graphinvariantentests |
| BODY-002 | DISS S. 100, 113–117 | DONE | M2 | SI-Schema und Validator für ID, Typ, Geometrie, Länge, Durchmesser, Querschnitt, Volumen, Fluss, Quellen und Unsicherheit; kanonischer M2.2-Datensatz |
| BODY-003 | DISS S. 101–104 | LEGACY | M2 | analytische 3D-Geometrietests und Modellbericht |
| BODY-004 | DISS S. 117, 140–143 | LEGACY | M2 | zweites Körpermodell ohne Rebuild laden |
| BODY-005 | DISS S. 99–104 | PART | M2 | terminierte Injektion, identitätserhaltender Kompartimenttransport, Einmalbewegungs- und Erhaltungstests; Entnahme folgt in M2.5 |
| BODY-006 | DISS S. 100–101, 118–122 | DONE | M2 | 23 Dissertationstransitionen, belegter Gefäß-9-Split, stationäre Flusserhaltung, reproduzierbare Verzweigung und schema-validierte Perfusionsregression; unabhängige physiologische Evidenz folgt mit dem separaten Profil in M2.6 |
| BODY-007 | DISS S. 115–117 | PART | M2/M4 | deterministisches Transitkompartiment implementiert; laminare/Stream-Variante und Referenzvergleich folgen |
| BODY-008 | DISS S. 120–122 | PART | M2/M3 | Ruhe-/Rückenlage-Baseline und Evidenzregeln dokumentiert; maschinenlesbare Belastungs-/Orthostaseprofile und Literaturvergleich folgen in M2.6 |
| BODY-009 | DISS S. 101 | RESEARCH | M4/M5 | dokumentierte Blutmodellvarianten und Sensitivität |
| BODY-010 | BVS18 S. 4–6 | DONE | M2 | deterministische 6.359-/63.590-Partikel-Regression, Gleichgewicht bei Minute 7, Injektionsortvergleich, exakte Populationserhaltung und schema-validierte Golden Reference |
| ORG-001 | DISS S. 118–126 | SPEC | M3 | Modellkarte und Tests des Referenzorgans |
| ORG-002 | DISS S. 118–122 | PART | M3 | M2.4 prüft 23 Soll-/Ist-Perfusionen des Ganzkörpergraphen; organspezifisches Lungenmodell und unabhängige physiologische Validierung folgen in M3/M2.6 |
| ORG-003 | DISS S. 95, 153–154 | SPEC | M3 | Roundtrip Körper → Organ → Körper |
| ORG-004 | DISS S. 122–123, Kap. 6 | LEGACY | M3/M7 | Lokalisierungsereignis mit Gewebe und Unsicherheit |
| ORG-005 | DISS S. 123–126 | SPEC | M3 | austauschbares Kompartiment-/Detailmodell |
| ORG-006 | DISS S. 124–126 | RESEARCH | M3/M8 | reproduzierbare Konvertierung mit Geometrieprüfung |

## 4. Kapillar- und Zellebene

| ID | Quelle | Stand 27.08.2026 | Ziel | Vorgesehener Nachweis |
|---|---|---|---|---|
| CAP-001 | DISS S. 126–129 | SPEC | M4 | parametrisierter Arteriole–Kapillare–Venole-Referenzfall |
| CAP-002 | DISS S. 126–129 | RESEARCH | M4 | organspezifische Parameterkarten und Literaturvergleich |
| CAP-003 | DISS S. 127–128 | SPEC | M4 | Sphinkter-/Aktivitäts-Szenariotest |
| CAP-004 | DISS S. 127 | SPEC | M4 | Stoffbilanz über Blut/Interstitium/Zelle |
| CAP-005 | DISS S. 128–129 | RESEARCH | M4/M5 | Transit-/Retention-Verteilungen mit Gültigkeitsbereich |
| CAP-006 | DISS S. 129, 154 | SPEC | M4 | analytischer Kanal und ein externer/Surrogat-Adapter |
| CAP-007 | DISS S. 129 | RESEARCH | M4/M6 | Erreichbarkeits- und Multi-Hop-Vergleich |
| CELL-001 | DISS S. 129–132 | SPEC | M5 | Biomarkerfeld mit analytischem Referenzfall |
| CELL-002 | DISS S. 130–132 | RESEARCH | M5/M7 | Bindungs-/Schwellenmodell mit FP/FN-Auswertung |
| CELL-003 | DISS S. 130–132, 153–154 | SPEC | M5 | Freisetzung–Diffusion–Bindung-End-to-End-Test |
| CELL-004 | DISS S. 131–133, 154 | RESEARCH | M5 | ODE/SSA- oder Verteilungsmodell gegen Referenzdaten |
| CELL-005 | DISS S. 153–154 | RESEARCH | M5 | Apoptoseereignis plus Rückkopplung an Szenario |

## 5. Nano-IoT und Forschungsdaten

| ID | Quelle | Stand 27.08.2026 | Ziel | Vorgesehener Nachweis |
|---|---|---|---|---|
| IOT-001 | DISS S. 113–115, 186–188 | LEGACY | M2/M7 | generischer Gerätetyp plus Locator/Collector-Komposition |
| IOT-002 | DISS S. 96–97 | SPEC | M6 | Nano-In-Body → Gateway → BAN → Station |
| IOT-003 | DISS S. 117–118, 187–190 | LEGACY | M2/M6 | Handgelenk-Gateway als expliziter Messort |
| IOT-004 | RM M6 | SPEC | M6 | Kommunikationsbericht mit Latenz/Verlust/Energie |
| IOT-005 | DISS S. 96–100; MEH25 | SPEC | M6 | austauschbarer Netzwerkadapter ohne Kernabhängigkeit |
| DATA-001 | RM 6.1 | PART | M1/M2 | versionierte Schemata und Validatoren für Experiment, Provenienz, Log und Checkpoint; fachliche M2-Datenschemata folgen |
| DATA-002 | RM 6.4 | DONE | M1 | JSON Schema `1.0.0`, Manifest- und CLI-Negativtests |
| DATA-003 | RM 2.4, 6.4 | PART | M1 | Schema `1.0.0`, automatisch erzeugtes `provenance.json`, SHA-256- und Vertragstests; Datenversionskatalog folgt mit realen Modellen |
| DATA-004 | VIS20; MEH25 | LEGACY | M2 | konfigurierbare Events/Aggregate/Trajektorien |
| DATA-005 | RM 2.5 | SPEC | M0/M1 | Evidenzklasse in jeder Modellkarte |
| DATA-006 | RM 6.2 | SPEC | M0 | dokumentierter Kalibrier-/Validierungssplit |
| DATA-007 | RM 6.3 | SPEC | M7 | Ergebnisbericht mit Intervallen und Sensitivität |
| DATA-008 | RM 6.2 | SPEC | M2 | statistische Regressionstests mit begründeter Toleranz |

## 6. Bedienung, Szenarien und Qualität

| ID | Quelle | Stand 27.08.2026 | Ziel | Vorgesehener Nachweis |
|---|---|---|---|---|
| UX-001 | RM Phase 0/10 | PART | M1 | headless CLI-Lauf in CI |
| UX-002 | VIS20 S. 1–2 | LEGACY | M7 | standardisiertes Ergebnisformat in neuer Visualisierung |
| UX-003 | VIS20 S. 1–2 | LEGACY | M7 | visueller Akzeptanztest und Laufvergleich |
| UX-004 | RM 6.5 | SPEC | M7 | identische Abbildung aus archiviertem Lauf |
| SCN-001 | DISS S. 185–190; FP23 | LEGACY | M7 | [Fingerprinting-Referenzszenario](FINGERPRINTING_SCENARIO.md) |
| SCN-002 | DISS S. 143–152 | LEGACY | Phase 8 | Monitoring-Referenzexperiment und Alarmmetriken |
| SCN-003 | DISS S. 155–160 | LEGACY | Phase 8 | Reproduktion der publizierten Detektionsraten |
| SCN-004 | DISS S. 153–154 | SPEC | Phase 8 | vollständiger Mehrschicht-Capstone |
| SCN-005 | MEH25 S. 1–2 | LEGACY | Phase 8 | CAR-T-Benchmark und Modellvergleich |
| SCN-006 | DISS S. 140–143 | SPEC | M8 | stufenweise personalisierter Forschungszwilling |
| QUA-001 | RM M1 | DONE | M1 | grüne MSVC-/GCC-/Clang-CI-Matrix |
| QUA-002 | RM M1 | DONE | M1 | clang-tidy, ASan/UBSan und Warnungen als Fehler in CI |
| QUA-003 | MEH25 S. 1–2 | SPEC | M2–M7 | versionierte Benchmarkberichte plus Ergebnisvergleich |
| QUA-004 | MEH25 S. 1–2 | SPEC | M4–M7 | Skalierungstest Agenten vs. Populationen/Surrogat |
| QUA-005 | RM 6.6 | PART | M1 fortlaufend | API-/Schema-/Modell-/Szenariodokumentation |
| QUA-006 | RM M8 | SPEC | M8 | Datenschutz- und Datenmanagement-Review |

## 7. M0-Abdeckungsprüfung

Phase 0 ist fachlich abgeschlossen, wenn zusätzlich zu diesen Dokumenten folgende Entscheidungen beziehungsweise Inventare vorliegen:

- [x] vier Ebenen und Verantwortlichkeiten verbindlich beschrieben (`ARC-001` bis `ARC-007`);
- [x] Legacy als Referenz, selektive Übernahme und neuer Kern entschieden (ADR-0001);
- [x] C++20/CMake/vcpkg als technisches Fundament entschieden (ADR-0003);
- [x] Fingerprinting als erster vertikaler Demonstrator entschieden (ADR-0004);
- [x] Evidenz- und Validitätsklassen festgelegt (ADR-0005 und `DATA-005/006`);
- [x] Dateninventar einschließlich externer Modelle vollständig;
- [x] Zielnutzer und priorisierte Arbeitsabläufe als M0-Baseline festgelegt;
- [x] Referenzorgan für M3 ausgewählt: Lunge (ADR-0006);
- [x] Partner-/Datenlücken für Proteomik, pulmonale Hämodynamik und Wetlab-Validierung benannt;
- [x] Mehrfachlizenzierung technisch umgesetzt: MPL-2.0 für unabhängigen
  Next-Code, GPL-2.0-only für Legacy und direkte Portierungen, CC-BY-4.0 für
  neue eigene Dokumentation und freigegebene eigene Daten (ADR-0007).

M0 ist damit abgeschlossen. Ungeklärte Rechte einzelner Bestandsdaten und
Publikationen werden als Release-Gate des jeweiligen Artefakts geführt; sie
blockieren weder M1 noch unabhängig entwickelte Next-Releases. Details stehen
im [M0 Gate Review](../m0/M0_GATE_REVIEW.md).
