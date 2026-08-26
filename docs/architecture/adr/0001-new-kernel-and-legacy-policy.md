<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0001: Neuer Kern und Legacy-Strategie

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M0, M1; `SYS-005`, `SYS-006`, `QUA-003`

## Kontext

MEHLISSA 1.x erweitert ns-3 und enthält wertvolle Körper-, Szenario- und Fingerprinting-Logik, ist aber eng gekoppelt und trägt technische sowie fachliche Altlasten. MEHLISSA 2.0 entfernt ns-3 und verbessert die Laufzeit deutlich, bleibt jedoch ein kleiner, szenarionaher Simulationskern. Keine historische Version bildet die vier Ebenen der Dissertation vollständig ab.

Eine direkte Fortsetzung würde vorhandene Annahmen, Datenformate und Klassenkopplungen zum Fundament der neuen Architektur machen. Ein vollständiges Ignorieren des Altbestands würde dagegen publizierte Referenzläufe, Datensätze und erprobte Regeln verlieren.

## Entscheidung

MEHLISSA Next erhält einen neuen, kleinen, szenariounabhängigen Simulationskern. Die Verzeichnisse `mehlissa/` und `mehlissa2.0/` bleiben auf dem Legacy-Tag als wissenschaftliche und technische Referenz erhalten.

Code wird nur selektiv portiert, wenn:

1. sein fachliches Verhalten verstanden und einer Anforderung zugeordnet ist;
2. Eigentum und Lizenz eine Übernahme erlauben;
3. Einheiten, Datenabhängigkeiten und Annahmen dokumentiert sind;
4. automatisierte Tests das gewünschte Verhalten sichern;
5. die Portierung die neue Schicht- und Szenariotrennung respektiert.

Historische Ergebniswerte werden als Regression oder Vergleich verwendet, nicht ungeprüft als Wahrheit.

## Folgen

Positiv:

- Kern und Architektur können von Beginn an reproduzierbar und testbar aufgebaut werden.
- ns-3 bleibt optionaler Kommunikationsadapter statt Pflichtabhängigkeit.
- Publizierte Ergebnisse und Daten bleiben als Vergleich nutzbar.
- Szenarien müssen nicht länger Architekturentscheidungen im Kern erzwingen.

Negativ:

- Kurzfristig entsteht Doppelarbeit.
- Historische Funktionen stehen erst nach qualifizierter Portierung wieder zur Verfügung.
- Abweichungen von Legacy-Resultaten müssen systematisch untersucht werden.

## Verworfene Alternativen

- **MEHLISSA 1.x direkt modernisieren:** zu starke Kopplung an ns-3 und szenariospezifische Klassen.
- **MEHLISSA 2.0 unverändert erweitern:** bessere Ausgangslage als 1.x, aber noch keine belastbaren Ebenen-, Daten- und Evidenzverträge.
- **Vollständiger Greenfield-Start ohne Legacy-Vergleich:** würde wissenschaftliche Rückverfolgbarkeit und wertvolle Referenzfälle verlieren.
