<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA – Current-State Analysis

**Analysis date:** 26 August 2026

**Revision examined:** `main`, commit `4f4fc5a`

**Scope:** Literature in `literature/`, the ns-3 implementation in `mehlissa/`,
the standalone implementation in `mehlissa2.0/`, and the body and scenario data
sets in the repository

> **Historical baseline, not current project status.** This document is a
> frozen analysis of legacy commit `4f4fc5a` from 26 August 2026. It explains
> the starting point for MEHLISSA Next. For the implemented M0-M7 platform,
> Workbench 1.0, present scientific limitations, and current development focus,
> use the [Project Status and Collaboration Brief](PROJECT_STATUS_AND_COLLABORATION_BRIEF.md)
> and the [Roadmap](ROADMAP.md).

## 1. Executive assessment

MEHLISSA is currently a scientifically well-founded overall concept with a
functional research prototype of the whole-body circulation layer. It is not
yet a complete “Medical Holistic Simulation Architecture” in the sense of the
dissertation.

The central gap between vision and implementation is:

- The vision is a coupled multiscale architecture from whole-body circulation to molecular and intracellular communication.
- The implementation mainly transports individual particles through a coarse whole-body vascular network.
- MEHLISSA 2.0 replaces ns-3 with a smaller simulation kernel and improves runtime, but does not implement the four layers described in the literature.
- The medical scenarios are valuable proofs of concept, but are not yet validated physiological or clinical models.
- The current `main` branch contains technical inconsistencies. In particular, after the latest endocrine/AVS extension, the old ns-3 version cannot be built consistently in its current state.

### Maturity overview

| Area | Assessment |
|---|---|
| Scientific vision | high |
| Whole-body transport model | research prototype |
| Organ models | coarse symbolic representation |
| Capillary models | not implemented |
| Cell and molecular models | isolated abstract particle interactions |
| Application scenarios | several demonstrators, inconsistently integrated |
| Software platform | early alpha |
| Physiological predictive capability | not yet robust |
| Clinical digital twin | vision, not yet implemented |

## 2. Source basis

The analysis is based especially on the following works contained in the repository:

- [MEHLISSA: A Medical Holistic Simulation Architecture for Nanonetworks in Humans](<../literature/Mehlissa A Medical Holistic Simulation Architecture for Nanonetworks in Humans.pdf>)
- [Dissertation – simulation chapters](../literature/Diss_WENDT_Simulationchapters.pdf)
- [BloodVoyagerS – Simulation of the Work Environment of Medical Nanobots](<../literature/BloodVoyagerS - Simulation of the work environment of medical nanobots.pdf>)
- [BVS-Vis: A Web-based Visualizer for BloodVoyagerS](<../literature/BVS-VIS_A Web-based Visualizer for Blood Voyager S.pdf>)
- [Proteome Fingerprinting as a Localization Scheme for Nanobots](../literature/wendt_fingerprinting.pdf)
- [MEHLISSA 2.0: Accelerating Full-body Molecular Communication Simulations](../literature/3760544.3765642.pdf)

## 3. Target vision from the literature

MEHLISSA is intended to consist of four connected but fundamentally independent
layers. At least mobility, state, activity, and simulation results are exchanged
between them.

### 3.1 Body layer

The body layer is intended to provide:

- a closed whole-body blood circulation;
- transport and global distribution of nanodevices, cells, and molecules;
- realistic branching and perfusion ratios;
- three-dimensional vessels with multiple virtual flow paths;
- dynamically loadable and eventually patient-specific body models;
- injection, collection, and global position output;
- a body-area network, gateways, and connection to external analysis and control systems.

### 3.2 Organ layer

The organ layer is intended to represent organ-specific properties:

- regional vascular structure;
- organ volume and local perfusion;
- perfusion dependent on exercise, activity, and situation;
- organ localization of nanodevices;
- transition from global circulation into regional detailed models;
- organ-specific gateways or routers where appropriate.

### 3.3 Capillary layer

The capillary layer is intended to represent arterioles, venules, capillary beds,
and their local dynamics:

- capillary density and branching structure;
- precapillary sphincters and regulated perfusion;
- substance exchange between blood and tissue;
- local retention and distribution of nanodevices;
- cluster and multi-hop communication;
- integration of existing molecular channel models;
- derivation of compact results for organ and body layers.

### 3.4 Cell layer

The cell layer is intended to model the actual biological target processes:

- nanodevice–cell and cell–cell communication;
- biomarker detection and receptor binding;
- drug release and uptake;
- intra- and extracellular concentrations;
- signaling pathways and reaction cascades;
- cell responses up to apoptosis, immune response, or proliferation.

### 3.5 Nano-IoT and medical workflow

The four physiological layers are intended to be extended by communication and
application functions:

- an in-body nanonetwork;
- a micro/nano gateway;
- a body-area network;
- an external analysis and control station;
- a return channel for activation or treatment;
- medical workflows for monitoring, diagnosis, treatment, and digital twins.

The literature therefore describes a research program and architectural vision,
not a complete technical specification. Many models are intended to be
integrated from external simulators, databases, or future experiments.

## 4. Implemented status by layer

### 4.1 Body layer

This is by far the most mature part of MEHLISSA.

The current 1995 data set contains:

- 95 vessel or organ sections;
- 36 arteries, 34 veins, and 25 transitions modeled as organs;
- a closed network without dead ends;
- 24 topological branches;
- 23 explicit records of branching probabilities;
- nine organs with fingerprint-assembly times;
- 21 virtual flow channels per vessel.

Particles can be injected, moved through the vascular network, distributed
stochastically at branches, and exported as CSV. The literature-based transition
probabilities are a relevant improvement over the original BVS model.

Physiological meaning is nevertheless limited by strong abstractions:

- All vessels receive the same width of `0.25`.
- Velocities depend only on the categories artery, vein, and organ.
- Blood pressure, vascular elasticity, pulsatility, and mass conservation through cross sections are absent.
- Activity, heart rate, posture, and exercise-dependent perfusion are absent.
- Coordinates lie almost exclusively at `z = +2` and `z = -2`.
- Organs are mostly four-centimetre transition segments rather than organ models.

The existing model is therefore best understood as a stochastic transport
topology rather than a hemodynamic model.

Vessel 9 is another data anomaly: it has two topological successors but no entry
in `95_transitions.csv`. It therefore uses the default distribution `1/0`,
effectively leaving one drainage path unused.

### 4.2 Organ layer

Organs are represented through IDs, static transition probabilities, and in
some cases fingerprint times. The following are absent:

- detailed organ-specific vascular trees;
- regulated organ perfusion;
- coupling to activity or physiological state;
- regional substance concentrations;
- exchange between blood, interstitium, and tissue;
- integration of BodyParts3D, SimVascular, imaging, or CFD models.

The dissertation describes preparatory work for such models, but it has not
entered the software.

Files under `bodymodels/` do not yet provide robust personalization. In particular:

- they are primarily geometric scalings;
- the female model contains record 51 split across two lines;
- MEHLISSA 2.0 reads coordinates with `std::stoi`, losing decimal places;
- after a defective record, the parser error flag remains set and subsequent vessels are discarded.

### 4.3 Capillary layer

The capillary layer is not implemented. There are no software components for:

- arterioles, venules, or capillary beds;
- sphincters and local flow regulation;
- capillary diameters and blood-cell effects;
- substance exchange with tissue;
- local communication clusters;
- multi-hop protocols;
- molecular channel models;
- exchange of abstracted capillary results with higher layers.

There is also no clearly defined interface for attaching an external capillary
or channel simulator.

### 4.4 Cell layer

`CancerCell`, `TCell`, and `CarTCell` are mobile particles in the bloodstream.
They do not constitute tissue or cell models in the sense of the dissertation.

Missing features include:

- cell membranes and receptor binding;
- intra- and extracellular concentrations;
- signaling pathways and reaction networks;
- drug uptake and response;
- tumor tissue and its microenvironment;
- feedback from biological processes to organ or circulatory state.

The CAR-T classes implement static probabilities for killing, fratricide, and
proliferation. This is a scenario demonstrator, not a general cell layer.

### 4.5 Communication, gateway, and nano-IoT

Despite its original ns-3 base, there is no actual network implementation with
packets, channels, protocols, or routing. The vessel called a gateway is
currently a passive measurement and CSV output point.

Not implemented are:

- communication protocols between nanodevices;
- molecular channels with noise and interference;
- gateway communication;
- a body-area network;
- external control;
- secure return channels and actuation.

## 5. The two implementations

### 5.1 `mehlissa/`: historical ns-3 version

The old version contains the broadest collection of scenario ideas:

- conventional nanodevices and global distribution;
- nanolocators and nanocollectors;
- LDL/monitoring mode;
- liquid-biopsy mode;
- CAR-T cells;
- the new endocrine/AVS extension.

It is tightly coupled to ns-3 and global hard-coded files. Scenario logic
resides directly in `Bloodcircuit` and `Bloodvessel`, mixing the simulation
kernel, physiology, experiment configuration, and output.

The current revision also has clear merge/API problems:

- In [`Bloodcircuit.cc`](../mehlissa/Bloodcircuit.cc#L102), `circuit` is declared twice in immediate succession.
- The first call uses `numberOfLocators`, although the parameter is named `numOfLocators`.
- An undeclared constructor overload is called.
- The endocrine extension uses `SetHormoneType`, `GetHormoneType`, `SetInjectionTime`, and `GetInjectionTime`; these methods do not exist in `Nanoparticle`.
- The build requires a separate ns-3 installation and is not reproducible from this repository alone.

This version should therefore be retained as a historical reference and
comparison implementation, not as the basis for further primary development.

### 5.2 `mehlissa2.0/`: standalone simulation kernel

MEHLISSA 2.0 removes the ns-3 dependency and introduces a simple time-stepped
simulator. Positive aspects are:

- a smaller technical kernel;
- improved directory and namespace structure;
- configurable paths for the vascular network, transitions, and fingerprints;
- more modern memory management;
- substantially reduced runtime compared with the ns-3 version;
- a kernel library that passes a pure C++23 syntax check.

MEHLISSA 2.0 nevertheless remains mainly a port of the old whole-body
circulation logic. Organ, capillary, cell, and communication layers were not
added.

Its build and execution path is incomplete:

- [`CMakeLists.txt`](../mehlissa2.0/src/CMakeLists.txt#L32) creates only `MehlissaCancer`.
- The general entry point `start-mehlissa.cc` is not built.
- [`start-mehlissa.cc`](../mehlissa2.0/src/experiments/start-mehlissa.cc#L34) contains multiply declared variables and would not compile.
- The default path names `95_fingerprint.csv`, while the existing file is `95_fingerprints.csv`.
- There are no automated tests, CI, or reproducible reference runs.

## 6. Central technical findings

### 6.1 Time model

The simulator accepts a configurable time step. Every vessel, however, uses
fixed `m_deltaT = 1` and therefore always moves particles forward by one second.
The configured time step changes only the global clock.

The global clock stores milliseconds as an integer but returns seconds using
integer division. Subsecond steps are therefore lost.

Consequences:

- Simulations with `simulationStep != 1` are numerically inconsistent.
- Movement, aging, interaction, and event time can use different time bases.
- Convergence or time-step studies are impossible.

### 6.2 Geometry

[`Position::CalcDistance`](../mehlissa2.0/src/utils/Position.cc#L44) uses
`b.z - b.z` for the z difference and ignores z entirely. Further length and
interaction calculations use only x and y. Organs are handled through special
cases in z.

The claimed 3D support is therefore currently 2D geometry with isolated z
special cases.

### 6.3 Random numbers and reproducibility

In `Randomizer` and `RandomStream`, initialization declares local random
generators that shadow the actual static or member generators. The selected seed
initialization therefore does not act as intended.

Possible consequences:

- unintentionally deterministic behavior;
- correlated random streams;
- an ineffective `isDeterministic` parameter;
- results that become difficult to trace under later parallelization.

### 6.4 Particle and CAR-T logic

For a CAR-T injection at time zero, [`BloodCircuit.cc`](../mehlissa2.0/src/bloodcircuit/BloodCircuit.cc#L348)
mistakenly creates cancer cells.

Further limitations:

- CAR-T proliferation can be stimulated by all matching particles in the same vessel, regardless of spatial proximity.
- Fratricide is checked at distance `<= 0` and may include the cell itself.
- Cell interactions compare particles in a vessel largely pairwise and scale quadratically.
- Replication and lifetimes are far outside the short benchmark periods.
- Detection radii and spatial model resolution are partly inconsistent.

The CAR-T benchmarks therefore primarily assess transport and runtime
performance, not biological validity of a treatment prediction.

### 6.5 Output and scaling

By default, the simulator writes every particle state at every time step. This
creates large data volumes and can dominate runtime.

In [`Printer.cc`](../mehlissa2.0/src/utils/Printer.cc#L74), the configured
particle-output mode is reset to zero during every output operation. Specialized
LDL and liquid-biopsy output therefore does not work as intended.

Parallelization is only prepared. With `parallel > 1`, simulation still runs
sequentially.

### 6.6 Memory management

Vessels hold `shared_ptr` references to successors. Because circulation is
closed, this creates reference cycles. `BloodCircuit` also manually invokes the
destructor of a `Printer` managed by `shared_ptr`.

Object lifetime, ownership, and memory layout must be reorganized before larger
experiments.

### 6.7 Data model and parameterization

The current state uses:

- implicit units;
- hard-coded constants;
- CSV files without schema or version information;
- little input validation;
- no documented parameter provenance;
- no uncertainties or distributions for physiological parameters;
- no central experiment configuration.

This impedes reproducibility and scientific comparability.

## 7. Scenario status

### 7.1 Proteome fingerprinting

Fingerprinting is the most developed connecting application. An abstract state
machine is implemented:

1. A nanolocator reaches its target-organ ID.
2. An organ-specific timer runs.
3. The vessel is permanently marked “fingerprint message active.”
4. A matching nanocollector receives a detection flag when passing through.

Gene expression, concentration, binding probability, disease marker, tile
count, and chemical assembly are not simulated. Assembly duration derived from
NetTAS is used only as a delay.

Fingerprinting is nevertheless the best candidate for a first complete
multilayer demonstrator.

### 7.2 Continuous monitoring and liquid biopsy

LDL and ctDNA exist as hard-coded particle modes. Particles receive size/speed
factors and are counted through spatial proximity to nanodevices.

Published probabilities partly rely on simplifying independence assumptions.
Experimental calibration and sensitivity analysis are absent. In MEHLISSA 2.0,
these modes are not currently executable through the regular path because the
general program entry point is neither built nor compilable.

### 7.3 Digital twin

The digital twin is currently a vision with a geometric-scaling demonstration.
Missing are:

- a patient data model;
- imaging import;
- vital and laboratory parameters;
- personal parameter calibration;
- treatment planning and feedback;
- uncertainty quantification;
- a data-protection and provenance concept.

### 7.4 Metastasis prevention

The metastasis scenario connects the four layers most completely: cancer-cell
detachment, transport, detection, molecular communication, drug release, and
apoptosis. It has remained conceptual and is suitable as a long-term integration
and capstone scenario.

### 7.5 CAR-T

CAR-T is the main performance demonstrator for MEHLISSA 2.0. Biological
interaction is strongly abstracted and has the problems described in Section
6.4. Even after the runtime improvement, realistic cell counts remain many
orders of magnitude out of reach.

### 7.6 Endocrine/AVS extension

The latest extension adds separate adrenal-gland/vein sections and ideas for
aldosterone and cortisol. It exists only in the old ns-3 version and is not
currently functional:

- missing methods in `Nanoparticle`;
- inconsistent constructor calls;
- the 104-vessel model is not selected automatically;
- hormones receive `delay = 0`, which means zero velocity in the existing movement model;
- sampling IDs in README, comments, and implementation are not fully consistent;
- simulated concentrations are adjusted afterward to clinical targets, making independent validation circular.

The extension is a valuable scenario sketch, but not yet an implemented model.

### 7.7 Visualization

BVS-Vis is a separate Three.js project and is not part of this repository.
MEHLISSA has no integrated result exploration, experiment management, or
visualization.

## 8. Missing engineering foundations

A sustainable research platform currently lacks:

- a reproducible build for all supported platforms;
- unit, integration, and regression tests;
- continuous integration;
- versioned releases and reference data sets;
- clear separation among kernel, model, scenario, and output;
- machine-readable configurations with schemas;
- explicit units;
- seeds and experiment manifests;
- structured logs and aggregate result formats;
- a benchmark and validation suite;
- contributor documentation;
- at the time of analysis, a missing repository-wide license file despite GPLv2 notices in source files; this has since been resolved by ADR-0007 and the multiple-license structure.

## 9. Strengths of the existing work

The critical findings do not mean that MEHLISSA must be reinvented or rejected.
The existing work has considerable value:

- a clear and scientifically relevant multiscale vision;
- a closed and largely consistent whole-body graph;
- literature-based static perfusion probabilities;
- several published application scenarios;
- a smaller simulation kernel than ns-3;
- a still manageable codebase of about 9,000 C++ lines;
- existing particle and device concepts as a starting point for new interfaces;
- fingerprinting as a particularly suitable cross-layer demonstrator.

## 10. Overall assessment

MEHLISSA is a strong research vision with a useful whole-body transport
prototype that is not yet sufficiently secured technically or scientifically.

The old ns-3 version should be retained as a historical reference. Further
development should build on a stabilized, newly modularized successor to the
2.0 kernel. Additional scenarios should not be embedded directly in
`BloodCircuit` or `BloodVessel`.

The next development step should therefore not primarily be “more features,”
but:

> Create a reproducible, validatable, multiscale platform into which body,
> organ, capillary, cell, and communication models can be integrated through
> well-defined interfaces.

The resulting development plan is documented in the
[MEHLISSA roadmap](ROADMAP.md).
