<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Lizenz- und Dateninventar

**Stand:** 26. August 2026  
**Status:** M0-Inventar und Lizenzmodell beschlossen; Rechteprüfung einzelner Bestandsdaten bleibt artefaktbezogen
**Hinweis:** Dieses Dokument ist eine technische Bestandsaufnahme, keine Rechtsberatung.

## 1. Ergebnis in Kürze

Der historische MEHLISSA-Code enthält durchgängig Hinweise auf **GNU GPL Version 2** und nennt als Rechteinhaber Universität zu Lübeck und – für spätere Anteile – Technische Universität Berlin. Das Repository verwendet deshalb bewusst ein Lizenzmodell pro Datei und Artefakt.

Die beschlossene und umgesetzte Struktur ist:

1. unabhängig entwickelten MEHLISSA-Next-Code als `MPL-2.0` veröffentlichen;
2. Legacy-Code und direkte Portierungen unter `GPL-2.0-only` erhalten;
3. neue, projekteigene Dokumentation und freigegebene eigene Daten unter
   `CC-BY-4.0` stellen;
4. vollständige Lizenztexte unter `LICENSES/` und die Zuordnung in
   `LICENSE.md` führen;
5. Urheber und Drittmaterial in `NOTICE.md` beziehungsweise
   `THIRD_PARTY_NOTICES.md` dokumentieren;
6. ungeklärte Bestandsdaten und Publikations-PDFs nicht in öffentliche
   Next-Daten- oder Softwarepakete aufnehmen.

Die Entscheidung folgt für den Bestand den vorhandenen Lizenzhinweisen, bleibt
mit einer optionalen ns-3-Anbindung kompatibel und öffnet den unabhängig
entwickelten Next-Kern für breitere Integration. Sie ist in
[ADR-0007](../architecture/adr/0007-repository-license.md) verbindlich.

## 2. Quellcode und Werkzeuge

| Bestandteil | Nutzung | Festgestellte Lizenz | Befund/Aktion |
|---|---|---|---|
| `mehlissa/` | Legacy MEHLISSA 1.x/ns-3-Modul | GPL-2.0-only gemäß vorhandenen Headern | Rechteinhaber und Autoren pro Datei erhalten; vollständiger Text unter `LICENSES/` |
| `mehlissa2.0/` | Legacy MEHLISSA 2.0 | GPL-2.0-only gemäß vorhandenen Headern | Universität zu Lübeck und TU Berlin genannt; vollständiger Text unter `LICENSES/` |
| `core/`, `apps/`, `tests/`, `cmake/` | unabhängig entwickeltes MEHLISSA Next | MPL-2.0 | SPDX-Header vorhanden; GPL-Portierungen müssen als solche getrennt bleiben |
| Catch2 | Unit-Tests, über vcpkg | BSL-1.0 | kompatible Testabhängigkeit; Copyrightdatei im Buildartefakt erhalten |
| vcpkg | Paketmanager | MIT | Portbibliotheken behalten jeweils ihre eigene Lizenz |
| CMake/Compiler | Buildwerkzeuge | nicht in MEHLISSA vendort | keine Codeübernahme; Version in Provenienz erfassen |
| ns-3 | optionaler Legacy-/Kommunikationsadapter | GPL-2.0-only | keine Pflichtabhängigkeit des Next-Kerns; Lizenzkompatibilität erhalten |
| SimVascular | künftiger externer Adapter | BSD-3-Clause | nicht vendorn, sofern nicht nötig; Version und Copyright dokumentieren |

Offizielle Referenzen: [Catch2](https://github.com/catchorg/Catch2), [vcpkg](https://github.com/microsoft/vcpkg), [ns-3](https://www.nsnam.org/), [SimVascular](https://simvascular.github.io/).

## 3. Repositorydaten

Die CSV-Dateien besitzen keine eingebetteten Metadaten oder individuellen Lizenzhinweise. Die Git-Historie dokumentiert ihre Herkunft, ersetzt aber keine explizite Datenlizenz.

| Datei | Zeilen | SHA-256 | Herkunft/Befund |
|---|---:|---|---|
| `fingerprint.csv` | 9 | `a6e42f40d6ff5f159224ab85e9d0a6c73e75f23217a922e3b49e854f259e467c` | Regine Wendt, Commit `4757903`; identisch mit 2.0-Kopie; uneinheitliches Abschlusskomma |
| `transitions95.csv` | 23 | `186650341da23f233ec483fcebfd212833b48071922345f297bc36a771709a8a` | Regine Wendt, Commit `4757903`; identisch mit 2.0-Kopie; uneinheitliches Abschlusskomma |
| `vasculature_transitions95.csv` | 95 | `8e04a2e004e42fd37a4cb1d225f4c0dd8b6bf27d05afec4e40a51fbb18c31588` | Regine Wendt, Commit `4757903`; kombiniertes Legacyformat ohne Header |
| `vasculature_transitions_endocrine_avs.csv` | 104 | `90e3429ab30e07606b507e3eb3c76cf0c4e9688658e81f6a888483ead99ba241` | Saswati Pal, Commit `c14ad46`; AVS-Erweiterung ohne Datenschema/Einheitenmetadaten |
| `bodymodels/vasculature_transitions95female.csv` | 96 nichtleere Zeilen | `b3a3d30cdbe0d95ba2ffb15e605992095999ad965c67970325ea36e8cd950f23` | Regine Wendt, Commit `a3f3090`; **bekannter Defekt:** Datensatzzeile 51 ist über Dateizeilen 51 und 53 getrennt |
| `bodymodels/vasculature_transitions95male.csv` | 95 | `3fbc694159554624169cab0f4420d054037f5d1262b3f6df4e88feb2d4ab072e` | Regine Wendt, Commit `a3f3090`; ohne Header/Einheitenmetadaten |
| `mehlissa2.0/data/95_fingerprints.csv` | 9 | `a6e42f40d6ff5f159224ab85e9d0a6c73e75f23217a922e3b49e854f259e467c` | Lisa Y. Debus, Commit `59cccdf`; Byte-identisch mit Root-Datei |
| `mehlissa2.0/data/95_transitions.csv` | 23 | `186650341da23f233ec483fcebfd212833b48071922345f297bc36a771709a8a` | Lisa Y. Debus, Commit `59cccdf`; Byte-identisch mit Root-Datei |
| `mehlissa2.0/data/95_vasculature.csv` | 95 | `de4b2b730bf3f88d3cb59213d67fcaf3d4764a3aca69be892e151109eb8c9db8` | Lisa Y. Debus, Commit `59cccdf`; konsistente zehn Felder, aber ohne Header/Schema |

### 3.1 Verbindliche Migrationsregel

Legacydateien werden nicht stillschweigend korrigiert oder überschrieben. Die Migration in `data/` muss:

- Quelldatei und SHA-256 speichern;
- Feldbedeutung, Einheit und Koordinatensystem explizit machen;
- Transformationen als reproduzierbares Werkzeug ausführen;
- bekannte Defekte als Validierungsfehler erkennen;
- neue kanonische IDs von historischen Gefäßnummern trennen;
- Lizenz, Urheber, Publikationsquelle und Gültigkeitsbereich im Datenmanifest führen.

## 4. Literaturverzeichnis im Repository

Unter `literature/` liegen sechs Publikations-PDFs. Sie sind fachliche Primärquellen, enthalten aber Verlags- beziehungsweise Publikationsrechte und keine repositoryweite Weiterverbreitungserlaubnis.

Regel bis zur Rechteprüfung:

- PDFs dürfen lokal zur wissenschaftlichen Rückverfolgbarkeit verwendet werden;
- automatisierte Releasepakete und Container schließen `literature/` aus;
- öffentliche Spiegelung oder Weitergabe wird nicht aus der MEHLISSA-Code-Lizenz abgeleitet;
- DOI und bibliografische Referenz sind die dauerhafte öffentliche Referenz;
- Autorenversion, Verlagserlaubnis oder Open-Access-Status wird pro PDF nachgetragen.

## 5. Vorgesehene externe Datenquellen

| Quelle | Zweck | Lizenz/Nutzungsbedingung | Aufnahmebedingung |
|---|---|---|---|
| [Human Protein Atlas](https://www.proteinatlas.org/about/licence) | Fingerprint-Genprodukte und Gewebeexpression | CC BY 4.0 für urheberrechtlich schützbare Daten; Drittquellen gesondert beachten | exakte HPA-Version, Gene/URLs und Zitation speichern |
| [BodyParts3D](https://lifesciencedb.jp/bp3d/info_en/license/index.html) | generische Organ-/Körpergeometrie | CC BY-SA 2.1 Japan | Share-Alike und vorgeschriebenen Credit erhalten; Versions-/Koordinatenwarnungen dokumentieren |
| [Vascular Model Repository](https://www.vascularmodel.com/FAQs.html) | pulmonale und andere Gefäß-/Randbedingungsmodelle | Nutzung für Forschung und Entwicklung mit Copyright-/README- und Anerkennungspflichten | konkretes Modell und dessen `README-COPYRIGHT` archivieren; nicht pauschal als Open Data bezeichnen |
| [SimVascular](https://simvascular.github.io/) | Segmentierung, 0D/1D/3D-Hämodynamik, Konvertierung | BSD-3-Clause | Werkzeugversion und Konvertierungsworkflow protokollieren |
| [BioModels](https://www.ebi.ac.uk/biomodels/faq) | kuratierte Reaktions-/Zellmodelle | Modelle laut FAQ unter CC0 | Modell-ID, Revision, Originalpublikation und Curation-Status speichern |
| [Physiome Model Repository](https://models.physiomeproject.org/) | CellML-/Kreislauf-/Mehrskalenmodelle | öffentlich zugängliche Inhalte grundsätzlich CC BY 3.0; einzelne Einträge können abweichend/unklar sein | Lizenz jedes konkreten Modells prüfen, persistenten Stand und Zitation speichern |

## 6. Freigabegates

Vor der Aufnahme externer Daten oder Modelle müssen folgende Felder vollständig sein:

```text
identifier
title
source_url
source_version_or_date
retrieved_at
sha256
license_spdx_or_text
required_attribution
original_citation
transformations
units
coordinate_system
population_and_validity
known_limitations
responsible_reviewer
```

## 7. Verbleibende artefaktbezogene Rechteprüfung

Die repositoryweite Lizenzentscheidung ist getroffen. Offen bleibt nicht das
Lizenzmodell, sondern die Rechtekette einzelner vorhandener CSV-Dateien und
Publikations-PDFs. Die Dateien werden erst dann als CC-BY-4.0-Daten
gekennzeichnet beziehungsweise in öffentliche Next-Pakete aufgenommen, wenn
die jeweils genannten Beitragenden oder zuständigen Rechteinhaber die Freigabe
bestätigt haben. Neue eigene Daten verwenden von Beginn an ein Manifest und
`CC-BY-4.0`.
