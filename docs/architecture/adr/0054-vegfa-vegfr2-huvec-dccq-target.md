<!-- SPDX-FileCopyrightText: 2026 MEHLISSA contributors -->
<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# ADR-0054: VEGF-A165a/VEGFR2 HUVEC target for DCCQ-1

- **Status:** Accepted
- **Date:** 2026-09-06
- **Applies to:** DCCQ-1; `ARC-003`, `ARC-004`, `ARC-006`, `CAP-004`,
  `CAP-006`, `CELL-001`, `CELL-002`, and `CELL-003`

## Context

DCCQ-1.1 requires one named biochemical identity and cell context before
dynamic equations are chosen. The target needs time-resolved binding or
trafficking observations, an auditable source artifact, convertible units,
separately evaluated reuse rights, and at least one source-disjoint challenge.

The qualified BCQ CD95 mechanism is exact but fixes its ligand input, retains
unresolved source-native units, and represents an average HeLa cell. The
Kuehn-Checa angiogenesis model now has exact Apache-2.0 source tags, but it is a
large mouse embryoid-body agent platform with model-specific spatial scaling.
The 2025 Sarabipour model and experiments instead provide a human endothelial
VEGF receptor trafficking source with explicit molecule, time, volume, surface,
and rate definitions plus CC-BY Supporting Information.

## Decision

Select human VEGF-A165a binding to and trafficking with VEGFR2 in primary
HUVECs as the first DCCQ biological target. NRP1 remains an explicit structural
choice that DCCQ-1.3 must include or exclude prospectively. The subsequent
DCCQ-1.3 decision selected an explicit tracked-neutral reference plus excluded
and labelled facilitation sensitivity variants; see ADR-0055.

Use the 2025 and 2024 Sarabipour HUVEC publications and Supporting Information
for mechanism selection, parameterization, and within-family checks. Use Peach
2019 only as a source-disjoint kinetic challenge with a declared engineered
HEK293T context mismatch, and Zhao 2010 only as a source-disjoint directional
HUVEC check.

The public VEGFR-Trafficking-Projects repository is pinned and selected files
are content-hashed, but it contains no explicit repository licence. Its code
and standalone CSV files may be inspected for provenance but must not be copied,
modified, bundled, or redistributed under an assumed licence. DCCQ-1.3
subsequently specified a reduced MEHLISSA-native model independently from the
CC-BY publication record before authoritative output.

## Consequences

- DCCQ now has one stable ligand, primary receptor, coreceptor choice, human
  endothelial context, and dynamic response to constrain the next protocol.
- The molecule-per-cell source system was convertible to SI; DCCQ-1.3 later
  froze cell count, volumes, molecular mass, dimer convention, and rate
  conversions, allowing DCCQ-G1 to pass for the reduced candidate.
- The full 281-state reference is not the implementation plan. A reduced model
  must preserve ligand ownership and the selected observables.
- HUVEC evidence cannot establish pulmonary capillary physiology. No lung,
  in-vivo, participant, patient, diagnostic, therapeutic, or clinical claim is
  added.
- DCCQ-G7 remains blocked because the source-disjoint quantitative series uses
  engineered non-endothelial cells and the independent HUVEC evidence is only
  directional at one time point.

## Alternatives considered

- **Clegg and Mac Gabhann 2015:** retained as the strongest structural
  alternative; explicit CC-BY equations and parameters, but no exact licensed
  executable artifact was established and the phosphorylation model is larger
  than the first coupling needs.
- **Kuehn and Checa 2019 AngioABM:** retained for a later multicellular
  angiogenesis package; exact source tags and Apache-2.0 rights do not overcome
  the mouse sprouting context, model-specific units, or platform-scale import.
- **Kallenberger CD95 BCQ model:** retained unchanged as a computational cell
  regression; rejected as the coupling target because its biology and units do
  not match the endothelial SI boundary.
- **Implement the entire 281-state model:** rejected before protocol design.
  Complexity is not evidence, and the repository code lacks an explicit
  licence.
