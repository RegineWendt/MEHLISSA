<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0006: Lung as the First Reference Organ

- **Status:** Accepted
- **Date:** 26 August 2026
- **Decision makers:** MEHLISSA Next project leadership
- **Applies to:** M3–M5 and M7; `ORG-001` through `ORG-006`, `CAP-001` through `CAP-006`, `SCN-001`

## Context

The first body–organ coupling needs an organ that exercises the vertical
MEHLISSA path, is traceable through existing baselines, and can be refined
incrementally. The roadmap named the lung, kidney, and liver as candidates.

Project leadership prioritized the lung over the previously considered kidney.
This choice is particularly appropriate: all circulating entities pass through
the pulmonary circulation, the fingerprinting scenario includes FP9 with
published runtimes, and public SimVascular/VMR reference data are available.

## Decision

The **lung** becomes the first reference organ. The initial model scope is
explicitly the **pulmonary circulation**. Ventilation, respiratory mechanics,
gas exchange, and complete alveolar biology are added only in separate, later
model variants.

Development proceeds incrementally:

1. `LungCompartment`: an effective, mass-conserving transit/perfusion compartment;
2. `PulmonaryCirculation`: pulmonary artery, regional distribution, capillary surrogate, and pulmonary veins;
3. imported or derived 0D/1D vascular model from a qualified SimVascular/VMR reference case;
4. local alveolar capillary bed for fingerprinting, substance exchange, and cell coupling;
5. optional patient-specific geometry and physiological states.

The coarse variant remains permanently available as a fast reference and surrogate.

## Evaluation matrix

Scores range from 1 (weak) to 5 (strong); maximum weighted total is 5.

| Criterion | Weight | Lung | Kidney | Liver |
|---|---:|---:|---:|---:|
| existing fingerprinting baseline | 20% | 5 | 5 | 5 |
| importance for whole-body coupling | 25% | 5 | 3 | 3 |
| accessible vascular/hemodynamic reference | 20% | 5 | 3 | 3 |
| manageable initial abstraction | 15% | 3 | 4 | 2 |
| utility for capillary/cell coupling | 15% | 5 | 4 | 4 |
| reuse in further scenarios | 5% | 4 | 3 | 4 |
| **weighted score** | **100%** | **4.65** | **3.70** | **3.45** |

## Reference sources

- Dissertation: FP9 is assigned to the lung (historical organ index 61); first localization at 25 s, assembly in 15.99 s, and collection with 10,000 collectors in 91 s in the historical baseline.
- [SimVascular Healthy Pulmonary](https://simvascular.github.io/clinical/pulmonary.html): public pulmonary test case.
- [Vascular Model Repository](https://www.vascularmodel.com/): normal and pathological cardiovascular/pulmonary models with boundary conditions.
- [BodyParts3D](https://lifesciencedb.jp/bp3d/info_en/index.html): generic anatomy and a shared coordinate system.
- [Human Protein Atlas](https://www.proteinatlas.org/): versionable tissue-expression data for FP9.

## Consequences

Positive:

- Every complete circulation exercises the body–lung–body interface.
- Pulmonary transport has clear arterial and venous exchanges.
- Fingerprinting provides an early regression for arrival and end-to-end time.
- The organ later supports capillary recruitment, barrier, inflammation, and gas-exchange models.
- Relevant expertise exists in Lübeck and the ARCN/DZL environment and remains to be approached.

Negative:

- The capillary bed and alveolar barrier are more complex than a simple organ compartment.
- Pulmonary circulation, respiratory mechanics, and gas exchange must not accidentally merge into an untestable monolith.
- A VMR/SimVascular model cannot be adopted as generic normal anatomy without review.

## M3 acceptance criteria

- An agent and a substance flow reproducibly move from the body graph into the lung model and back.
- No agent or relevant amount of substance is duplicated or lost.
- Both model variants—the effective compartment and the more detailed pulmonary circulation—implement the same contract.
- Flow, pressure/transit time, and perfusion share have units, sources, uncertainty, and a reference comparison.
- The historical FP9 timer baseline runs without scenario-specific lung logic in the kernel.
