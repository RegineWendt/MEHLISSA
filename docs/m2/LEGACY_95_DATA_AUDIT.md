<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Audit of the Legacy 1995 Vascular Data Set

**As of:** 27 August 2026

**Repository sources:** `mehlissa2.0/data/95_vasculature.csv`,
`95_transitions.csv`, `95_fingerprints.csv`, and the associated `BloodCircuit`
parser

## Reproducible structural review

| Property | Finding |
|---|---:|
| records | 95 |
| unique IDs / range | 95 / 1–95 |
| arteries / veins / organ transitions | 36 / 34 / 25 |
| directed edges | 119 |
| branches with two successors | 24 |
| dead ends / nodes with more than two successors | 0 / 0 |
| reachable from vessel 1 | 95 |
| vessels that can reach vessel 1 again | 95 |
| explicit probability records | 23 |
| maximum deviation of probability sum from 1 | 0 |
| fingerprint timing records | 9 |

The coordinate-based legacy algorithm therefore creates a strongly connected
directed graph. The 24 branching IDs are:

`1, 2, 3, 4, 5, 8, 9, 10, 12, 13, 14, 16, 19, 22, 26, 31, 34, 38, 41, 42, 44, 46, 48, 50`.

## Confirmed anomalies

1. The source lacks transition probabilities for vessel 9 even though coordinate logic creates two successors (`81` left and `83` right). Legacy code therefore implicitly uses `1/0`. For the rest/supine profile, M2.2 corrects this to the supported `0.2875/0.7125`; source, validity scope, and variability are documented in the profile.
2. All start and end points lie on only two planes, `z = -2` and `z = +2`. This is schematic topology, not anatomical 3D geometry.
3. Every vessel line ends with a delimiter. A strict split therefore yields ten fields; the ninth column, still read by the parser, is always `0`, and the tenth field is empty. Neither has documented domain meaning.
4. The parser uses `std::stoi` for all coordinates and would lose or reject decimal places in future files.
5. Its `errorflag` is not reset after an incomplete line; all subsequent otherwise valid lines would be rejected.
6. Edges arise solely from exact coordinate equality. Sources, units, tolerances, and intended topology are not stored.
7. Width and type-specific velocity are hard-coded globally; pressure, flow balance, pulsatility, and physiological states are absent.

## Migration decision

M2 does not adopt any of these assumptions silently:

- Edges and probabilities are stored explicitly.
- Schematic coordinates are labeled as such and transformed with a documented unit.
- Physical parameters receive sources, evidence quality, and uncertainty.
- Missing values remain visible as data gaps. Vessel 9 is the only domain-corrected transition and references the underlying MRI cohort; the historical `1/0` run remains in the raw source.
- The new loader supports non-contiguous IDs and rejects incomplete graphs before simulation begins.

## Licensing and provenance boundary

On 27 August 2026, project leadership confirmed that the three 1995 files may
be used and relicensed. They and the canonical migration are therefore licensed
under `CC-BY-4.0`. Sidecars and `data/legacy/bvs95/release-v1.json` document
attribution, SHA-256 values, authorization basis, transformations, and
limitations.

The reproducible converter rejects structural deviations from the 95 segments
and 23 transition records. Its output is the complete schema- and
semantics-validated graph
`data/body-models/bvs95-dissertation-rest-v1.json`.
