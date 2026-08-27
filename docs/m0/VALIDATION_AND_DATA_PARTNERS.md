<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Data Gaps and Validation Partners

**As of:** 26 August 2026

**Status:** Candidate and expertise inventory; no collaboration commitment is claimed

## 1. Purpose

This document identifies evidence that MEHLISSA Next cannot obtain through
software development alone, public sources that can serve as starting points,
and expert partner roles required for robust validation.

## 2. Partner and data matrix

| Area | Required evidence | Public starting source/candidate | Expected artifact | Time |
|---|---|---|---|---|
| pulmonary anatomy | vascular geometry, inlets/outlets, coordinates, segmentation | [BodyParts3D](https://lifesciencedb.jp/bp3d/info_en/index.html), [Healthy Pulmonary](https://simvascular.github.io/clinical/pulmonary.html), [Vascular Model Repository](https://www.vascularmodel.com/) | versioned generic lung model with license and geometry report | before M3 |
| pulmonary hemodynamics | flows, pressures, resistances, transit times, boundary conditions at rest/exercise | SimVascular/VMR; pulmonary-medicine and hemodynamics review | 0D/1D reference case, parameter ranges, and independent comparison data | M3/M4 |
| alveolar microcirculation | capillary density, recruitment, transit, sphincter/perfusion surrogate models | literature plus experimental lung microscopy | capillary model card and measurement data set for transit/distribution values | M4 |
| barrier and substance exchange | blood–endothelium–interstitium–alveolus, diffusion/binding | Lübeck Institute of Anatomy, Barrier Organs group as a **collaboration candidate** | analytical reference case and experimental parameter ranges | M4/M5 |
| lung imaging | segmentation, tissue/vessel comparison, potential individualization | Institute of Biomedical Optics and radiology expertise as **candidates** | imaging/conversion validation, later a patient-specific pilot | M3/M8 |
| proteome fingerprints | interindividual stability, disease effects, concentration, detection limit | [Human Protein Atlas](https://www.proteinatlas.org/), bioinformatics/proteomics partner | versioned FP9 data set, cohort/robustness analysis | before level B of M7 |
| DNA-tile detection | binding affinity, release, assembly, and degradation | existing NaBoCom/DNA nanonetwork expertise; wet-lab partner required | in-vitro measurement series and validated assembly surrogate | M5/M7 |
| nanodevice properties | size, flow retardation, lifetime, immune interaction | materials/nanomedicine and immunology partner | justified parameter ranges and sensitivity priorities | M4–M7 |
| wrist gateway | range, contact time, read errors, external signal path | communication/biosensor partner | gateway channel model and measurement protocol | M6/M7 |
| cell/reaction models | ligand binding, signaling pathways, apoptosis | [BioModels](https://www.ebi.ac.uk/biomodels/), [Physiome Model Repository](https://models.physiomeproject.org/), and cell-biology partner | qualified model card plus independent reference data set | M5 |
| translational scenarios | medical research question, endpoints, defensible interpretation | [ARCN/DZL North](https://research.uni-luebeck.de/de/projects/dzl-deutsches-zentrum-f%C3%BCr-lungenforschung-arcn-airway-research-ce/) as a **network candidate** | scenario review and clinically meaningful, non-diagnostic endpoints | M7/M8 |

Universität zu Lübeck explicitly advertises collaboration opportunities at its
Institute of Anatomy, including light/electron microscopy, intravital
multiphoton microscopy of the lung, and mRNA analyses. ARCN connects Universität
zu Lübeck, UKSH, LungenClinic Großhansdorf, Forschungszentrum Borstel, and other
northern German lung-research institutions. This proximity offers strong
potential but does not replace a formal project agreement.

## 3. Critical data gaps for the lung reference model

### Before implementation of the first organ model

- unambiguous definition of whether “lung” initially represents both lungs as one effective organ or separate sides;
- pulmonary-arterial inlet and pulmonary-venous outlet in the new body graph;
- consistent units and coordinate transformation between MEHLISSA, BodyParts3D, and VMR;
- reference values for flow, pressure, volume, and transit time;
- license and exact revision of the selected VMR model.

### Before capillary refinement

- distribution of pulmonary transit times rather than a mean only;
- capillary-network size and an appropriate abstraction level;
- recruitment/derecruitment under different physiological states;
- hematocrit and cell-free marginal layer at the relevant scale;
- exchange parameters and mass conservation across the alveolar barrier.

### Before fingerprinting level B

- exact FP9 gene-product combination in a versioned HPA release;
- variation in healthy and diseased lung tissues;
- physically detected species: mRNA, protein, or both;
- local concentration and binding parameters;
- false-positive and false-negative detection probability.

## 4. Proposed partner outreach

1. **Short model profile:** purpose, research status, no clinical claim.
2. **Concrete question:** at most three required parameters or one testable reference case.
3. **Data management:** ownership, consent, license, anonymization, and publication rights before data exchange.
4. **Validation plan:** separate calibration and test data before starting.
5. **Mutual benefit:** reproducible model adapter, shared model card, and citable data set/benchmark.

## 5. Minimum expert approval per gate

| Gate | Required domain approval in addition to software review |
|---|---|
| M2 | physiology/circulation review of whole-body parameters |
| M3 | pulmonary anatomy and hemodynamics |
| M4 | microcirculation and substance transport |
| M5 | cell biology/pharmacology |
| M6 | communication systems/biosensing |
| M7 | proteomics, DNA nanonetworking, and translational pulmonary medicine |
| M8 | data protection, ethics, clinical methodology, and regulation where applicable |

## 6. Partner status

M0 requires **identification** of required expertise and realistic candidates,
not already concluded collaborations. Current status:

- public data and model sources identified;
- local and supraregional areas of expertise identified;
- no external institution recorded as responsible or committed yet;
- contact begins only after approval by project leadership.
