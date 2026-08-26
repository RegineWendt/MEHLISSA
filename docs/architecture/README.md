# Architekturentscheidungen

Architecture Decision Records (ADRs) dokumentieren Entscheidungen, die die weitere Entwicklung von MEHLISSA Next prägen. Sie erklären Kontext, Entscheidung, Folgen und verworfene Alternativen. Akzeptierte ADRs werden nicht nachträglich umgeschrieben; eine neue Entscheidung ersetzt sie durch ein weiteres ADR.

| ADR | Status | Entscheidung |
|---|---|---|
| [ADR-0001](adr/0001-new-kernel-and-legacy-policy.md) | Accepted | neuer Kern, Legacy als Referenz und Quelle selektiver Portierungen |
| [ADR-0002](adr/0002-four-layer-cosimulation.md) | Accepted | vier eigenständige Ebenen mit expliziten Kopplungsverträgen |
| [ADR-0003](adr/0003-cpp20-cmake-vcpkg.md) | Accepted | C++20-Kern mit CMake/vcpkg; Python später als Experiment-API |
| [ADR-0004](adr/0004-fingerprinting-first-vertical-slice.md) | Accepted | Fingerprinting als erster vertikaler Demonstrator |
| [ADR-0005](adr/0005-model-evidence-and-validity.md) | Accepted | Evidenz, Kalibrierung, Validierung und Hypothesen explizit trennen |
| [ADR-0006](adr/0006-lung-reference-organ.md) | Accepted | Lunge als erstes Referenzorgan und stufenweises pulmonales Modell |
| [ADR-0007](adr/0007-repository-license.md) | Proposed | GPL-2.0-only als repositoryweite Code-Lizenz; Rechteinhaberfreigabe ausstehend |

## Statusmodell

- `Proposed`: zur Diskussion gestellt;
- `Accepted`: verbindlich für neue Arbeit;
- `Superseded`: durch ein benanntes späteres ADR ersetzt;
- `Rejected`: geprüft, aber nicht gewählt;
- `Deprecated`: weiterhin dokumentiert, soll jedoch nicht mehr verwendet werden.

## Neue ADRs

Neue Dateien erhalten die nächste vierstellige Nummer. Sie sollen mindestens enthalten:

1. Status und Datum;
2. Kontext und zu lösende Kräfte;
3. konkrete Entscheidung;
4. positive und negative Folgen;
5. betrachtete Alternativen;
6. betroffene Anforderungen oder Roadmap-Gates.
