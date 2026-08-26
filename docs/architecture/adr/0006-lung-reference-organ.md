# ADR-0006: Lunge als erstes Referenzorgan

- **Status:** Accepted
- **Datum:** 26. August 2026
- **Entscheider:** Projektleitung MEHLISSA Next
- **Betrifft:** M3–M5 und M7; `ORG-001` bis `ORG-006`, `CAP-001` bis `CAP-006`, `SCN-001`

## Kontext

Für die erste Körper–Organ-Kopplung wird ein Organ benötigt, das den vertikalen MEHLISSA-Pfad fachlich fordert, durch bestehende Baselines rückverfolgbar ist und schrittweise detailliert werden kann. Die Roadmap nannte Lunge, Niere und Leber als Kandidaten.

Die Projektleitung hat die Lunge gegenüber der zuvor erwogenen Niere priorisiert. Diese Wahl ist fachlich besonders passend: Sämtliche zirkulierenden Entitäten passieren den Lungenkreislauf, das Fingerprinting-Szenario enthält FP9 mit publizierten Laufzeiten und öffentliche SimVascular-/VMR-Referenzdaten sind verfügbar.

## Entscheidung

Die **Lunge** wird das erste Referenzorgan. Der erste Modellumfang ist ausdrücklich der **pulmonale Blutkreislauf**. Ventilation, Atemmechanik, Gasaustausch und vollständige alveoläre Biologie werden erst in getrennten, späteren Modellvarianten ergänzt.

Die Entwicklung erfolgt stufenweise:

1. `LungCompartment`: ein effektives, massenerhaltendes Transit-/Perfusionskompartiment;
2. `PulmonaryCirculation`: Pulmonalarterie, regionale Verteilung, Kapillarersatz und Pulmonalvenen;
3. importiertes oder abgeleitetes 0D-/1D-Gefäßmodell aus einem qualifizierten SimVascular-/VMR-Referenzfall;
4. lokales alveoläres Kapillarbett für Fingerprinting, Stoffaustausch und Zellkopplung;
5. optionale patientenspezifische Geometrie und physiologische Zustände.

Die grobe Variante bleibt dauerhaft als schnelle Referenz und Surrogat erhalten.

## Bewertungsmatrix

Bewertung 1 (schwach) bis 5 (stark); gewichteter Gesamtwert maximal 5.

| Kriterium | Gewicht | Lunge | Niere | Leber |
|---|---:|---:|---:|---:|
| vorhandene Fingerprinting-Baseline | 20 % | 5 | 5 | 5 |
| Bedeutung für Ganzkörperkopplung | 25 % | 5 | 3 | 3 |
| zugängliche Gefäß-/Hämodynamikreferenz | 20 % | 5 | 3 | 3 |
| beherrschbare erste Abstraktion | 15 % | 3 | 4 | 2 |
| Nutzen für Kapillar-/Zellkopplung | 15 % | 5 | 4 | 4 |
| Wiederverwendung in weiteren Szenarien | 5 % | 4 | 3 | 4 |
| **gewichteter Wert** | **100 %** | **4,65** | **3,70** | **3,45** |

## Referenzquellen

- Dissertation: FP9 wird der Lunge (historischer Organindex 61) zugeordnet; erste Lokalisation 25 s, Assembly 15,99 s und Erfassung mit 10.000 Kollektoren 91 s in der historischen Baseline.
- [SimVascular Healthy Pulmonary](https://simvascular.github.io/clinical/pulmonary.html): öffentlicher pulmonaler Testfall.
- [Vascular Model Repository](https://www.vascularmodel.com/): normale und pathologische kardiovaskuläre/pulmonale Modelle mit Randbedingungen.
- [BodyParts3D](https://lifesciencedb.jp/bp3d/info_en/index.html): generische Anatomie und gemeinsames Koordinatensystem.
- [Human Protein Atlas](https://www.proteinatlas.org/): versionierbare Gewebeexpressionsdaten für FP9.

## Folgen

Positiv:

- Jeder vollständige Kreislauf übt die Körper–Lunge–Körper-Schnittstelle aus.
- Pulmonaler Transport besitzt klare arterielle und venöse Übergaben.
- Fingerprinting bietet schon früh eine Regression für Ankunft und End-to-End-Zeit.
- Das Organ eignet sich später für Kapillarrekrutierung, Barriere-, Entzündungs- und Gasaustauschmodelle.
- In Lübeck und im ARCN/DZL-Umfeld bestehen passende, noch anzufragende Kompetenzfelder.

Negativ:

- Das Kapillarbett und die alveoläre Barriere sind komplexer als ein einfaches Organ-Kompartiment.
- Pulmonaler Kreislauf, Atemmechanik und Gasaustausch dürfen nicht versehentlich zu einem unprüfbaren Monolithen verschmelzen.
- Ein VMR-/SimVascular-Modell kann nicht ungeprüft als generische Normalanatomie übernommen werden.

## Abnahmekriterien für M3

- Ein Agent und ein Stofffluss wechseln reproduzierbar vom Körpergraphen in das Lungenmodell und zurück.
- Kein Agent und keine relevante Stoffmenge wird dupliziert oder verloren.
- Beide Modellvarianten – effektives Kompartiment und detaillierterer pulmonaler Kreislauf – implementieren denselben Vertrag.
- Fluss, Druck/Transitzeit und Perfusionsanteil besitzen Einheiten, Quelle, Unsicherheit und Referenzvergleich.
- Die historische FP9-Timerbaseline läuft ohne szenariospezifische Lungenlogik im Kern.
