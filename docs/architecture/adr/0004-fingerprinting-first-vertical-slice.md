# ADR-0004: Fingerprinting als erster vertikaler Demonstrator

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Betrifft:** M0/M7; `SCN-001`

## Kontext

Die Roadmap benötigt ein Szenario, das die Architektur früh gegen eine medizinisch motivierte End-to-End-Frage prüft. Ein reiner Körpertransporttest fordert Organ-, Kapillar-, Zell- und Nano-IoT-Schnittstellen nicht. Das Metastasenszenario deckt zwar alle Ebenen ab, benötigt aber besonders viele noch ungesicherte biologische Modelle.

Das Proteom-Fingerprinting ist in Paper und Dissertation fachlich beschrieben und in einer abstrahierten historischen MEHLISSA-Version simuliert. Für neun Gewebe liegen Konfiguration, Geräteklassen, Assembly-Zeiten und End-to-End-Referenzwerte vor.

## Entscheidung

Fingerprinting wird der erste vertikale Demonstrator. Die Entwicklung erfolgt in fünf Akzeptanzstufen:

1. historische Timerbaseline;
2. organspezifische konzentrations-/bindungsbasierte Erkennung;
3. Assembly-Surrogat oder NetTAS-Detailmodell;
4. explizites Handgelenk-Gateway und Nano-IoT-Pfad;
5. Sensitivitäts-, Unsicherheits- und Fehlklassifikationsanalyse.

Die verbindliche Baseline steht in der [Szenariospezifikation](../../requirements/FINGERPRINTING_SCENARIO.md).

## Folgen

Positiv:

- Bereits publizierte Werte ermöglichen Regression und Vergleich.
- Das Szenario zwingt zu sauberen Übergaben von Körper bis Gateway.
- Detaillierung kann stufenweise erfolgen, ohne das fachliche Ziel zu wechseln.
- Proteomik-, Geräte- und Gateway-Datenlücken werden früh sichtbar.

Negativ:

- Die heutige Baseline nutzt starke biologische Vereinfachungen.
- Ein vollständiger Demonstrator ist erst nach wesentlichen Teilen von M3–M6 möglich.
- Die 2-Gen-Fingerprint-Hypothese benötigt unabhängige biologische und experimentelle Prüfung.

## Verworfene Alternativen

- **CAR-T zuerst:** gute Performance-Baseline, aber keine vollständige Nano-IoT-/Schichtkopplung und biologisch extreme Zellzahlen.
- **Liquid Biopsy zuerst:** vorhandene Zahlen, aber schwächerer Organ-/Nachrichtenbezug.
- **Metastasenprävention zuerst:** bester langfristiger Capstone, für den Start jedoch zu viele gleichzeitige Forschungsunsicherheiten.
