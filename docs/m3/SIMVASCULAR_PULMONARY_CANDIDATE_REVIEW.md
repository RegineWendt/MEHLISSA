<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# SimVascular Healthy Pulmonary Candidate Review

**Review date:** 27 August 2026

**Decision:** conditionally suitable as an imported pulmonary-artery technical
case; not suitable as the sole M3 physiological lung reference

## 1. Reviewed primary material

The review used the official SimVascular Healthy Pulmonary documentation at
immutable website commit
[`e251ecd`](https://github.com/SimVascular/simvascular.github.io/tree/e251ecdafb86b2965ce6fb456f2b5042e69b065e/clinical/pulmonary):

- [clinical data](https://github.com/SimVascular/simvascular.github.io/blob/e251ecdafb86b2965ce6fb456f2b5042e69b065e/clinical/pulmonary/data/readme.md);
- [model description](https://github.com/SimVascular/simvascular.github.io/blob/e251ecdafb86b2965ce6fb456f2b5042e69b065e/clinical/pulmonary/description/readme.md);
- [boundary conditions](https://github.com/SimVascular/simvascular.github.io/blob/e251ecdafb86b2965ce6fb456f2b5042e69b065e/clinical/pulmonary/boundary/readme.md); and
- [simulation/results description](https://github.com/SimVascular/simvascular.github.io/blob/e251ecdafb86b2965ce6fb456f2b5042e69b065e/clinical/pulmonary/results/readme.md).

The official page points to
[SimTK file 4302](https://simtk.org/frs/download_confirm.php/file/4302/HealthyPulmonary.zip?group_id=930),
`HealthyPulmonary.zip`, in project `sv_tests`. On the review date the
confirmation URL redirected anonymous access to a SimTK login. The archive
itself was therefore not downloaded or inspected.

## 2. Case inventory

| Property | Official description | MEHLISSA interpretation |
|---|---|---|
| subject | 67-year-old female | patient-specific case, not a generic adult reference |
| imaging | CT, RAS coordinate system | axes are explicit and compatible with the definition schema |
| voxel spacing | 0.5859 × 0.5859 × 1.25 mm | anisotropic source resolution must be preserved |
| voxel dimensions | 512 × 512 × 198 | source image extent is reproducible |
| anatomical scope | main pulmonary artery through left/right pulmonary arterial branches | arterial tree only; no capillary bed or pulmonary veins |
| topology summary | 1 inlet, 100 outlets, 101 paths, 591 segmentations | useful stress case for branching import and outlet mapping |
| geometric summary | 89.08 cm³ volume, 345.13 cm² surface area | reference checks are available but require unit conversion and tolerance |
| inlet | 1.00 s period, 4.9799 L/min, plug profile | nominally described as average healthy resting flow |
| outlets | 100 resistance outlets with 7.0 mmHg pressure offset | values are available, but the page calls them “exercise conditions” |
| solver | 3D finite element Navier–Stokes, rigid no-slip walls | high-fidelity technical baseline, not automatically a validated clinical truth |
| temporal setup | 1,000 fixed steps/cycle, several cycles, final cycle reported | convergence/periodicity evidence must be reconstructed from the archive |

## 3. Blocking findings

### Anatomical coverage

The case ends at 100 pulmonary-artery outlets. MEHLISSA's organ contract returns
to the body through a pulmonary venous port. This data set cannot provide that
complete path by itself. A capillary/venous surrogate or a separately qualified
downstream network remains necessary.

### Physiological-state ambiguity

The inlet is described as an average resting waveform, while the outlet
resistances are explicitly described as assigned for exercise conditions. The
case cannot be labeled `rest` or `exercise` until this apparent mismatch is
resolved from the archive metadata, a publication, or the maintainers.

### Units and metadata quality

The description lists viscosity as `0.04 g/cm•s²`, which is dimensionally
suspect for dynamic viscosity; the expected cgs form would normally be
`g/(cm·s)`. The outlet table also labels resistance only as cgs rather than
stating the complete dimension in the table. MEHLISSA must not infer either
unit silently.

### License and redistribution

The [BSD-3-Clause terms on the SimVascular software website](https://simvascular.github.io/)
apply to software components and do not by themselves establish the license of
`HealthyPulmonary.zip`. A
[Vascular Model Repository specification example](https://www.vascularmodel.com/vmr-pdfs/0028_H_ABAO_H.pdf)
uses a separate research-and-development data permission that requires
preservation of copyright/README material, but that license cannot be assumed
for a distinct SimTK archive. The actual archive license and every included
third-party file must be inspected before redistribution or derivation in this
repository.

### Validation scope

The official page provides geometry, boundary settings, and qualitative result
figures but no uncertainty, cohort distribution, or independent validation
targets. A single 67-year-old female case cannot establish a normative human
pulmonary model. It can support software verification and a patient-specific
case study after qualification.

## 4. Recommended use in MEHLISSA

Use the case for an `externally_derived` pulmonary-artery variant only after the
archive is acquired and qualified. Do not replace the effective compartment or
claim that the imported tree represents a full lung circulation.

The first derived implementation should:

1. preserve the RAS axes and source millimetres in the immutable import record;
2. convert geometry, flow, pressure, resistance, density, and viscosity to SI
   with explicit formulas and dimensional tests;
3. derive a stable centerline graph and verify 1-inlet/100-outlet connectivity;
4. preserve every outlet ID and boundary condition;
5. compare computed volume and surface area with the source summary;
6. wrap the arterial network behind the existing pulmonary entry contract;
7. attach an explicitly labeled capillary/venous return surrogate; and
8. compare 0D/1D results with the original 3D case before using it in a body
   scenario.

## 5. Required next actions

- obtain authorized SimTK access and download the exact archive without adding
  it to Git;
- record retrieval time, byte size, SHA-256, archive inventory, and all license
  files;
- determine whether redistribution and publication of derived networks are
  permitted;
- resolve the rest/exercise boundary-condition mismatch and viscosity unit;
- identify independent healthy pulmonary pressure, flow-split, resistance, and
  transit targets with population and uncertainty; and
- decide whether the first scientific reference combines this arterial case
  with a sourced 0D capillary/venous model or uses a different complete case.

Until these actions are complete, the checked-in model definitions correctly
remain `software_test_surrogate` rather than `externally_derived` or
`literature_parameterized` physiological references.
