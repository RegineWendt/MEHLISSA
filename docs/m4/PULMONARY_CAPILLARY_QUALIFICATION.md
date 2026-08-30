<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Pulmonary Capillary Reference Candidate

## 1. Decision and scope

M4.7 adds the first organ-specific capillary card:

```text
examples/capillary-models/pulmonary-healthy-adult-rest-supine-v1.json
```

It represents a healthy-adult, resting, recumbent pulmonary capillary bed at
the nominal 6 L/min flow of the M3 pulmonary reference. The card is a
**literature-parameterized reference candidate**, not a validated patient
model. Its capillary region is an equivalent parallel-tube closure whose blood
volume and transit are evidence-anchored. Its arteriole and venule regions are
explicit numerical transition surrogates.

Schema `capillary-bed-definition/3.0.0` makes the evidence role, uncertainty,
derivation, source IDs, geometry semantics, and closure tolerance executable.
It also preserves schema 2.0.0 loading for the synthetic verification card.

## 2. Evidence review

### 2.1 MEHLISSA concept

Dissertation section 4.4, pages 126-129, motivates a serial
arteriole-capillary-venule layer, highly parallel capillary flow, continuity,
perfusion control, and explicit residence questions. It reports general
capillary ranges from anatomy and physiology textbooks, including a 5-10 µm
diameter and 0.02 cm/s mean velocity. Those values are not lung-specific
primary measurements, so they are context rather than executable pulmonary
parameters.

### 2.2 Functional pulmonary capillary blood volume

Lewis et al. measured carbon-monoxide diffusing capacity at multiple oxygen
tensions in 19 normal subjects and used the Roughton-Forster separation to
estimate pulmonary capillary blood volume (`Vc`). The study reports:

| State | Sample | Published Vc result | Use here |
|---|---:|---:|---|
| seated rest | 19 normal subjects | mean 65.0 mL | contextual comparison |
| moderate exercise | 4 subjects | 64.1 to 75.8 mL, group means | recruitment context only |
| seated to recumbent | 4 subjects | 59.2 to 85.9 mL, group means | executable functional-volume anchor |

The four recumbent individual values are 87.1, 101.2, 66.4, and 88.8 mL.
MEHLISSA recomputes a sample SD of 14.424 mL while retaining the published
rounded mean of 85.9 mL. This derived SD is marked as such in the card.

The authors explicitly state that no direct method existed for measuring
`Vc`, that the estimates give an order of magnitude rather than absolute
values, and that an assumed pulmonary-capillary hematocrit affects `Vc`.
Consequently the card does not silently add a hematocrit parameter.

Primary source: Lewis BM, Lin TH, Noe FE, Komisaruk R. *J Clin Invest*.
1958;37:1061-1070. <https://doi.org/10.1172/JCI103687>.

### 2.3 Morphometric capacity and equivalent diameter

Muehlfeld et al. reanalysed semi-thin sections from five normal human lungs
previously investigated by Gehr et al. The cases were aged 20-40 years (mean
31), with four male and one female lung. The reported whole-lung means were:

| Quantity | Mean ± SD | Executable role |
|---|---:|---|
| capillary endothelial surface | 130 ± 48 m² | retained evidence, not yet consumed by transit runtime |
| capillary lumen volume | 196 ± 62 mL | total equivalent capacity |
| model-based equivalent radius | 3.15 ± 0.61 µm | doubled to 6.30 ± 1.22 µm equivalent diameter |

The same study obtained incompatible total-length estimates: 6,950 ± 3,108 km
with a model-based estimator and 2,746 ± 722 km with a design-based estimator.
It concludes that total alveolar-capillary length should not be estimated with
either approach because the sheet-like, branched network violates their
assumptions; volume and surface estimates should be preferred. M4.7 therefore
does not claim an anatomical total length or vessel count.

Primary source: Muehlfeld C, Weibel ER, Hahn U, Kummer W, Nyengaard JR, Ochs M.
*Anat Rec (Hoboken)*. 2010;293:1270-1275.
<https://doi.org/10.1002/ar.21158>.

The original eight-lung Gehr et al. morphometry reported 126 ± 12 m² capillary
surface and 213 ± 31 mL capillary volume. It remains corroborating historical
context rather than a second executable anchor, avoiding an undocumented blend
of cohorts. Primary source: Gehr P, Bachofen M, Weibel ER. *Respir Physiol*.
1978;32:121-140. <https://doi.org/10.1016/0034-5687(78)90104-4>.

## 3. Executable closure

The M3 reference supplies the nominal flow

```text
Q = 6 L/min = 1.0e-4 m3/s.
```

The current runtime requires cylindrical diameter, representative path length,
and integer path count. M4.7 uses these equations while preserving their
evidence roles:

```text
d_eq = 2 r_eq = 6.30 µm
A_one = pi d_eq^2 / 4

N_perfused = 4,000,000,000                    [round numerical choice]
L_eq = V_functional / (N_perfused A_one)     = 688.910 µm

N_total = round(V_morphometric / (A_one L_eq))
        = 9,126,891,735

t_capillary = V_functional / Q               = 0.859 s
```

`N_perfused`, `N_total`, and `L_eq` are equivalent discretization variables.
They close the measured volumes against the continuity runtime but do not mean
that the human lung contains that number of discrete cylindrical capillaries.
The resting functional-to-morphometric volume ratio is about 0.438; it is not
labelled a directly measured recruitment fraction because the two values come
from different cohorts and methods.

Each boundary transition is assigned 5 mL, ten million 30 µm equivalent
vessels, and 0.05 s transit. These choices keep the existing three-region
contract executable without inventing physiologically qualified pulmonary
arteriole or venule volumes. The complete card therefore transits in 0.959 s,
of which 0.859 s is the evidence-anchored capillary region.

## 4. What schema 3.0.0 guarantees

The loader rejects a qualified card when:

- an evidence quantity references an unknown source ID;
- required SI units are inconsistent;
- functional volume does not equal perfused area times representative length;
- morphometric capacity does not equal total equivalent area times length;
- diameter or representative length differs from executable geometry;
- capillary transit does not equal functional volume divided by flow; or
- either boundary transition fails its declared numerical-volume closure.

`CapillaryRegionMetrics` now exposes the derived regional blood volume alongside
area, velocity, and transit. Automated tests verify 85.9 mL and 0.859 s for the
capillary region, 5 mL and 0.05 s for each transition, source metadata, and
schema-to-runtime loading.

## 5. Interpretation and remaining evidence gaps

Appropriate uses include a reproducible resting pulmonary reference case,
software comparisons among later surrogate and detailed capillary variants,
and volume-residence sensitivity studies. It is not appropriate for clinical
prediction, exercise simulation, disease, lobar heterogeneity, or anatomical
path counting.

The next pulmonary qualification work should seek jointly measured flow and
functional capillary volume in a larger contemporary cohort, make body
position explicit, and investigate hematocrit with human evidence. Recruitment
states should be calibrated independently before the morphometric-to-functional
ratio is interpreted physiologically. Surface area becomes operational when a
barrier-flux or molecular-channel model consumes it.

M4 can now proceed to a stable analytical molecular-channel contract using
this card as one evidence-bounded reference case.
