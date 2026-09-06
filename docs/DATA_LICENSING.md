<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Data Licensing

## Binding rule

New data sets created by the MEHLISSA project and approved for publication are
licensed under `CC-BY-4.0`. Each data set must have a manifest recording its
authors, sources, requested attribution, version, license, checksum, and
transformations.

This rule does not retroactively relicense existing or third-party data. Without
confirmed rights and an explicit SPDX declaration, the licensing status of an
existing data set remains unresolved and it must not be presented as CC BY data
in a public release.

## Current inventory

Release authorization for the three sources of the 1995 BVS/MEHLISSA data set
has been confirmed since 27 August 2026. They have `CC-BY-4.0` sidecars; the
manifest `data/legacy/bvs95/release-v1.json` records provenance, checksums,
attribution, transformations, and known limitations. The Next vascular graph
generated from these sources is also licensed under `CC-BY-4.0`.

The M2.6 profiles newly created under `data/body-states/` and their numerical
transformations are licensed under `CC-BY-4.0`. Scientific articles are cited
only; their publication rights are not transferred to the repository. Each
profile file states its sources, validity scope, and limitations and has a
license sidecar.

This authorization applies to the listed artifacts and does not automatically
cover other CSV files, extensions, or publication PDFs in the repository. Their
status remains documented in the [license and data inventory](m0/LICENSE_AND_DATA_INVENTORY.md).

The [BCQ-1 biological cell-model package](qualification/BCQ1_EXTERNAL_SOLVER_REPRODUCTION.md)
uses BioModels `BIOMD0000000523` and `BIOMD0000000524`. Their encoded SBML
model artifacts are CC0 1.0 and are not bundled in this repository. BCQ-1.3
publishes only newly generated COPASI trajectories, logs, metrics, and reports
under CC BY 4.0, with exact source and solver identities in the manifests.
BCQ-1.4–1.7 add an MPL-2.0 MEHLISSA implementation of the source equations and
CC-BY-4.0 MEHLISSA-generated trajectories, metrics, sensitivity diagnostics,
protocols, and reports. The official CC0 525/526 files are inspected for the
structural audit but are not copied into the repository. The model licence does not cover
the associated AAAS article, figures, supplementary files, or experimental
single-cell data. A later import must preserve the original CC0 notice,
accession, source commit, file hash, and transformations and must not imply
that third-party experimental data have been relicensed as MEHLISSA CC BY.

## Attribution

Unless a manifest specifies more precise wording, attribution should include at
least:

> MEHLISSA contributors, title and version of the data set, CC BY 4.0,
> repository URL, and associated scientific publication.

Third-party sources must additionally be attributed exactly as required by
their licenses.
