<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Coarse–Detailed Body–Lung Scenario Comparison (M3.18)

## Result

M3.18 runs one schema-validated body–lung–body experiment with two pulmonary
model resolutions. The body graph, research question, injection, route, random
seed, synchronization step, population, substance amount, and volume flow are
declared once. Only the organ model definition changes.

Both candidates preserve the scenario's domain meaning exactly:

| Result | Coarse compartment | Five-lobe pulmonary 0D |
|---|---:|---:|
| returned entities | 25/25 | 25/25 |
| returned typed conserved transfers | 3/3 | 3/3 |
| exact entity identities | pass | pass |
| exact venous return route | pass | pass |
| ownership closed after return | pass | pass |
| exact population/substance/flow payload | pass | pass |
| internal anatomical beds | 0 | 5 |
| completion time | 2.0 s | 6.4 s |

The different completion times are intentional. The coarse card uses a
two-second software-test transit, whereas v7 uses its literature-derived
6.4-second aggregate pulmonary transit. Interchangeability means that the
experiment retains its question and contracts when resolution changes; it
does not mean that models with different evidence and internal structure must
produce identical scientific outputs.

## Shared scenario

The checked-in scenario is
`examples/scenarios/body-lung-resolution-comparison-v1.json`. It declares:

- the synthetic branching body graph;
- 25 nanodevice entities injected at `artery-10` at time zero;
- one pulmonary arterial departure and venous return route;
- master seed 2026 and a 100 ms synchronization grid;
- a population of 10,000 nanodevices;
- 2.5 mmol of oxygen as a typed substance amount;
- a volume flow of 0.0001 m³/s over a one-second interval;
- the coarse contract definition and the detailed v7 definition as the only
  replaceable candidates; and
- exact identity, route, ownership, and typed-payload acceptance rules.

The strict schema is
`data/schemas/body-organ-resolution-comparison/1.0.0.schema.json`. Candidate
objects cannot override the common route or other scenario inputs. Semantic
checks additionally require exactly one coarse and one detailed role, a shared
departure segment, and an integral synchronization grid.

## Executable comparison

The regression loads the body and each organ card through their normal
versioned loaders, creates the lung through `make_lung_model`, and wires it to
the body with `BodyOrganCoupler`. The body, coupler, and simulation kernel have
no branch for the selected resolution. Scenario wiring replaces only the
organ card's return target with the selected body model ID; physiological model
parameters are not changed.

The test sends all entity IDs and the three conserved transfers into each
organ. It advances the same host step until all values return or the shared
seven-second safety limit is reached. It then verifies exact IDs, target
segment, route headers, quantities, ownership counters, and candidate roles.
The detailed candidate must expose five parallel beds; the coarse candidate
must expose none.

Run the dedicated comparison after a Debug build:

```powershell
& build/windows-msvc/tests/Debug/mehlissa_organ_model_tests.exe `
  "[resolution-comparison]"
```

## Gate interpretation

This closes the literal Gate M3 requirement that a coarse compartment and a
more detailed organ model can be used with the same scenario. It also provides
an executable regression for `SYS-008` without introducing scenario-specific
logic into the kernel.

The result is deliberately narrower than general experiment orchestration.
The synthetic graph is a coupling fixture, not a physiological whole-body
reference. General command-line composition of arbitrary body and organ cards,
dynamic posture or exercise redistribution, and the historical FP9 scenario
remain separate work packages.
