<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BCQ-1.1 Biological Cell-Model and Licence Selection

## Decision

The first biological cell-model qualification package will use the minimal
Kallenberger 2014 CD95L-CD95-caspase-8 model family. The two entering artifacts
are BioModels `BIOMD0000000523` for CD95-overexpressing HeLa cells and
`BIOMD0000000524` for wild-type HeLa cells. Both are the minimal cis/trans
variant, are encoded in SBML Level 2 Version 4, and are distributed under
CC0 1.0.

This completes candidate and licence selection only. No external model has been
added to the source tree, no simulation has been run through MEHLISSA, and no
biological qualification claim has passed. The machine-readable authority is
`data/qualification/biological-cell-model-candidate-register-v1.json`.

## Why this is the strongest first package

The current M5 models intentionally verify generic software behavior. They do
not name a biological ligand, receptor, cell line, or experimentally measured
pathway. The Kallenberger family closes that particular gap with a bounded
chain:

```text
CD95L (Fas ligand)
    -> CD95 receptor activation and DISC formation
    -> procaspase-8 cleavage and active intermediates
    -> reporter and BID cleavage
    -> tBID-threshold interpretation of apoptosis timing
```

The associated publication combines single-cell probe measurements with
population Western blots. It reports 80 single cells across two HeLa cell lines
and four ligand concentrations and, for the cell-death analysis, fits the
CD95-HeLa subset before predicting wild-type HeLa timing and variability. That
is a meaningful within-study separation between calibration-like and held-out
observations.

The published study concluded that the minimal cis/trans mechanism was
sufficient to explain the data. BCQ-1 therefore selects the corresponding
`BIOMD0000000523` and `BIOMD0000000524` pair rather than automatically retaining
the larger `BIOMD0000000525` candidate previously noted in the M5 evidence
document. The larger `BIOMD0000000525` and `BIOMD0000000526` pair remains a
useful structural-sensitivity comparison from the same paper; it is not an
independent evidence source.

## Audited entering artifacts

| Role | Accession and source identity | Frozen file identity | Audited structure |
|---|---|---|---|
| calibration-like average CD95-HeLa cell | `BIOMD0000000523`, `MODEL1403050000`, Git commit `8605e43f8e2fd364f122d579341891c0058ef778` | `BIOMD0000000523.xml`, SHA-256 `2afe6758ab396038e71fcb1716fefcfec67656b8bd0bfb3da8d4e1eda9524ff4` | one compartment, 18 species, 13 reactions, 12 global parameters |
| publication wild-type HeLa average cell | `BIOMD0000000524`, `MODEL1403050001`, Git commit `d091308a14fb4301a4a2b1b567ea874484bb97e6` | `BIOMD0000000524.xml`, SHA-256 `4bf4a5bcda5b43a551bcdda09fca91a5e777d2c5db1eafcb17dcb6f1574221bc` | one compartment, 18 species, 13 reactions, 12 global parameters |

The source repositories advertise the `20140916` release tag. Commit and
content hashes, rather than a moving branch or web response, are the BCQ-1.1
identity authority. The artifacts have been inspected in a disposable audit
workspace but have not been copied into MEHLISSA.

## Rights decision

BioModels applies the CC0 1.0 public-domain dedication to the encoded model
artifacts. That is compatible with later redistribution and modification in an
MPL-2.0 MEHLISSA codebase, provided provenance, original model identifiers,
source URLs, exact hashes, and the CC0 notice remain visible.

The model licence does **not** relicense the associated article, figures,
supplementary files, or experimental observations. The Kallenberger article is
under AAAS copyright. BCQ-1 may cite it and compare newly generated trajectories
with reported facts, but it must not copy figures or redistribute experimental
tables unless a separate licence or permission is established. No reusable
row-level single-cell data package or full ensemble parameter-distribution
package was established in this screen.

If the model files are later vendored, that must happen as an explicit
BCQ-1 increment with:

- the original CC0 licence text and an SPDX sidecar;
- accession, publication, source URL, source commit, and file hash;
- an unmodified-source area separated from MEHLISSA adapters and derived data;
- a recorded transformation log for any normalized representation; and
- a fresh hash and compatibility review if BioModels changes the artifact.

## What can be qualified

Subject to the later protocol and successful reproduction, BCQ-1 can support a
narrow statement of this form:

> MEHLISSA can execute or map a published CD95L-CD95-caspase-8 average-cell
> mechanism and reproduce predeclared outputs of the selected public SBML
> artifacts within a declared numerical tolerance. The mechanism has
> publication-level experimental support in CD95-overexpressing and wild-type
> HeLa cells.

It cannot establish:

- normal pulmonary endothelial-cell behavior;
- whole-organ, participant, patient, diagnostic, or clinical validity;
- a validated treatment response or safety claim;
- source-disjoint biological replication by the MEHLISSA team;
- population realism merely by running the one-average-cell SBML file; or
- a new biological result from matching a public model to itself.

The original paper's held-out cell-death prediction used ensemble distributions
and tBID thresholds in addition to the core mechanism. The standalone
`BIOMD0000000524` average-cell artifact must therefore not be relabelled as the
complete held-out validation execution. Once both artifacts and published
outcomes are visible to this project, their reproduction is one published
evidence-family check, not an untouched external validation set.

## Candidate comparison

Scores use six equally weighted zero-to-three dimensions: biological fit,
public artifact, reuse rights, experimental evidence, integration cost, and
scope clarity. They prioritize the first auditable increment; they are not a
measure of biological importance.

| Rank | Candidate | Score | Decision | Main reason |
|---:|---|---:|---|---|
| 1 | Kallenberger 2014 minimal CD95L-CD95-caspase-8 family | 18/18 | selected | named ligand/receptor/cell line, paired compact CC0 SBML, single-cell and population evidence, within-study prediction step |
| 2 | Rehm 2006 apoptosome-dependent effector-caspase model, `BIOMD0000000256` | 15/18 | fallback | strong CC0 model and single-cell perturbation evidence, but starts downstream of receptor binding and is substantially larger |
| 3 | Kuehn and Checa 2019 VEGF-A/VEGFR endothelial angiogenesis model | 9/18 | later coupling program | closer to capillary endothelium, but combines intracellular and two-dimensional agent dynamics and its named BioModels submission is not currently a released artifact |
| 4 | Eissing 2004 receptor-induced caspase bistability model | 8/18 | conceptual benchmark | compact and interpretable, but no stable licensed machine artifact was established in this screen |

### Rehm as fallback

BioModels `BIOMD0000000256` is a CC0 SBML model with two compartments,
27 species, 56 reactions, and 99 global parameters. The associated HeLa-cell
study used single-cell FRET measurements to test predictions about XIAP control
of effector-caspase substrate cleavage. It is valuable but begins downstream of
the ligand-receptor boundary that makes the selected Kallenberger family such a
direct extension of M5.

### VEGF/VEGFR as a later capillary candidate

The endothelial alternative is scientifically attractive for the later dynamic
capillary-tissue-cell package. It names VEGF-A, VEGFR1, VEGFR2, endothelial
cells, Dll4-Notch lateral inhibition, and sprouting outcomes. It also couples a
21-variable intracellular model to a stochastic two-dimensional agent-based
simulator and compares wild-type with VEGFR1-knockout sprouting. That makes it a
larger cross-layer research program rather than the safest first biological
cell-model reproduction. The paper names BioModels submission
`MODEL1804030001`, but the repository record found during this screen reports
the submission as not released. Exact source commits, supporting-file rights,
and dependency closure must be audited before later adoption.

## BCQ-1 sequence

BCQ means **Biological Cell-model Qualification**. It is a scientific
qualification track, not a new milestone gate and not a replacement for M5.

1. **BCQ-1.1 - candidate and licence selection, complete.** Freeze the selected
   source family, exact public identities, candidate ranking, rights boundary,
   and permissible claim before import.
2. **BCQ-1.2 - prospective reproduction protocol.** Before executing either
   model, freeze solver and version, time horizon and output grid, model
   variants, initial conditions, ligand conditions, observables, numerical
   tolerances, failure rules, negative controls, calibration-versus-evaluation
   labels, and the expected report structure. No fitting is permitted.
3. **BCQ-1.3 - independent external-solver reproduction.** Execute the frozen
   source files with a mature SBML solver such as COPASI command line or
   libRoadRunner, archive solver provenance, and compare the four reported
   observables `PrER_mGFP`, `PrNES_mCherry`, `p43`, and `p18` with the
   predeclared BioModels or publication references. Retain failed outputs.
4. **BCQ-1.4 - typed MEHLISSA adapter.** Map only declared stimulus, species,
   time, amount/concentration semantics, and outputs to the M5 contracts. Keep
   source equations separate from adapter code and do not refit parameters.
5. **BCQ-1.5 - cross-engine and structural checks.** Compare the MEHLISSA path
   with the independent solver, assess numerical convergence, vary declared
   solver tolerances, and use the larger 525/526 variant only as a same-family
   structural sensitivity analysis.
6. **BCQ-1.6 - population and uncertainty decision.** Import the ensemble only
   if its distributions and reuse basis can be reproduced. Otherwise retain the
   explicit average-cell limit. Quantify identifiable sensitivities and publish
   every passed, partial, blocked, and failed result.
7. **BCQ-1.7 - independent review and bounded claim.** Review source identity,
   licence, code-to-equation mapping, result archive, claim language, and User
   Guide/status updates before calling the variant biologically qualified.

The recommended first solver is COPASI's command-line engine because it can be
kept as an optional scientific reproduction dependency rather than a core
runtime dependency. libRoadRunner is a reasonable second engine for a later
cross-solver check. The exact choice becomes binding only in BCQ-1.2.

## Sources used for the screen

- [BioModels FAQ and curation description](https://www.ebi.ac.uk/biomodels/faq)
- [BioModels report for the previously noted 525 structural companion](https://www.ebi.ac.uk/biomodels/services/download/get-files/MODEL1403050002/3/BIOMD0000000525.pdf)
- [Kallenberger 2014 article](https://pmc.ncbi.nlm.nih.gov/articles/PMC4208692/)
- [Public BioModels Git mirror for 523](https://github.com/biomodels/BIOMD0000000523)
- [Public BioModels Git mirror for 524](https://github.com/biomodels/BIOMD0000000524)
- [Rehm 2006 article](https://pmc.ncbi.nlm.nih.gov/articles/PMC1570423/)
- [Public BioModels Git mirror for 256](https://github.com/biomodels/BIOMD0000000256)
- [Kuehn and Checa 2019 article and availability statement](https://pmc.ncbi.nlm.nih.gov/articles/PMC6445957/)
- [Eissing 2004 publication record](https://pubmed.ncbi.nlm.nih.gov/15208304/)

## Present state

BCQ-1.1 is a reproducible selection result and licence boundary. The next safe
step is BCQ-1.2. Human PCQ-1 access work remains available but is not required
for this cell-model reproduction path, and no external contact is implied by
this decision.
