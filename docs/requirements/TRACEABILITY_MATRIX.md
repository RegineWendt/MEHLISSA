<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Traceability Matrix – MEHLISSA Next

**As of:** 6 September 2026

**Reference document:** [system requirements](SYSTEM_REQUIREMENTS.md)

## 1. Purpose

This matrix connects every system requirement with its domain source, current
implementation maturity, evidence maturity, planned roadmap gate, and concrete
verification artifacts. It is the operational checklist for architecture
reviews and releases.

Implementation and evidence are deliberately independent. `DONE/PART`, for
example, means that the required software capability is present, while its
scientific, analytical, reference, or release-wide qualification remains
incomplete. A passed milestone therefore does not silently turn a synthetic
model into an independently validated physiological model.

Implementation status codes:

- `DONE`: the requirement's functional scope is present in MEHLISSA Next;
- `PART`: an executable subset is present, but functional scope remains open;
- `LEGACY`: capability is present only in a historical implementation or
  publication; and
- `SPEC`: specified or identified, but not implemented in MEHLISSA Next.

Evidence status codes:

- `VERIFIED`: the requirement's declared technical, analytical, reference, or
  inspection evidence is sufficient for its current bounded claim;
- `PART`: evidence exists for the implemented subset, but one or more declared
  qualification routes or scopes remain open;
- `UNVERIFIED`: no qualifying MEHLISSA Next verification currently exists; and
- `RESEARCH`: closing the evidence gap primarily requires new domain data,
  calibration, or experimental/physiological comparison.

`DONE` never implies `VERIFIED`: implementation and evidence must be reviewed
separately. Conversely, historical publication evidence does not verify a new
implementation. The final column records both the achieved evidence and the
remaining gap; merely naming a class or source is insufficient.

## 2. Foundation and architecture

| ID | Source | Implementation | Evidence | Target | Verification evidence and remaining gap |
|---|---|---|---|---|---|
| SYS-001 | RM M1 | DONE | VERIFIED | M1 | `simulation_clock_tests`, cross-platform CTest |
| SYS-002 | RM 2.4 | DONE | PART | M1/M7 | byte-identical M1 kernel reference run on MSVC/GCC/Clang; replicate planning and domain-model verification follow |
| SYS-003 | RM M1 | DONE | VERIFIED | M1 | `random_stream_tests`, including stream names |
| SYS-004 | RM 3.2 | DONE | VERIFIED | M1 | dimension-safe types for time, length, area, volume, speed, flow, amount, concentration, pressure, vascular resistance, and vascular compliance; compile/unit tests |
| SYS-005 | RM 2.1–2.2 | DONE | PART | M0/M1 | architecture review of target structure and dependency rules |
| SYS-006 | RM M0 | DONE | PART | M1 | CI rule and review: `core/` does not import `scenarios/` |
| SYS-007 | RM M1 | DONE | VERIFIED | M1 | stable error codes/CLI statuses and negative tests for configuration, overflow, lifecycle, log, and checkpoint invariants |
| SYS-008 | DISS pp. 95–97, 133 | DONE | VERIFIED | M3 | M3.18 runs one schema-validated body–lung–body scenario unchanged with the effective compartment and five-lobe v7 implementation; identity, route, ownership, population, substance, and flow meaning agree while model-specific timing remains observable |
| ARC-001 | DISS pp. 94–96 | DONE | VERIFIED | M3–M5 | generic `ModelComponent` lifecycle boundary plus independent body, organ, capillary, cell, and communication implementations and explicit couplers exist; a completely uniform interchangeable component/factory surface across every layer remains open |
| ARC-002 | DISS p. 95 | PART | PART | M3–M5 | schema-validated lung and capillary definitions bind scale and scientific evidence to execution; M4.3 derives capillary area, velocity, and transit from typed SI inputs, while physiological capillary uncertainty and qualification follow |
| ARC-003 | DISS pp. 95–97 | PART | PART | M3–M7 | versioned entity, population, substance-amount, volume-flow, body-state, detection, response, and communication-event contracts cross implemented boundaries; DCCQ-1.1 constrains the next transforming boundary to one biochemical identity, compatible units, exclusive amount ownership, and delayed feedback, and DCCQ-1.2 selects human VEGF-A165a/VEGFR2 HUVEC trafficking with NRP1 explicit; the reduced dynamic contract, SI mapping, and generic scenario composition remain open |
| ARC-004 | RM 3.3 | PART | PART | M3/M4 | entity round trips and lossless population, substance, and flow transit pass across both lung variants and the synthetic organ-capillary route; DCCQ-1.1 defines the seven-owner open-system ledger and forbids double-counting, while DCCQ-1.2 selects a source whose molecule-per-cell system is convertible but not yet mapped to SI; executable dynamic transforming exchange and body aggregate endpoints follow |
| ARC-005 | DISS pp. 99–100 | DONE | VERIFIED | M3–M5 | M5.7 returns a versioned neutral apoptosis-commitment event with stable source, target, measurement, and time identity; viable responses remain silent |
| ARC-006 | RM 3.3 | PART | PART | M3 | externally selected coarse/regional results agree at 0.5 s and 1 s, the 6.4 s pulmonary 0D route agrees at 0.1 s and 0.2 s, and M3.18 executes one 0.1 s scenario across coarse and five-lobe candidates; DCCQ-1.1 requires independent internal-step/coupling-interval analysis and rejects same-step feedback, and DCCQ-1.2 supplies the selected seconds-based source context; the executable asynchronous multirate synchronization contract and protocol remain open |
| ARC-007 | DISS pp. 96–97, 133 | PART | PART | M4/M5 | reference adapter plus equivalent surrogate |

## 3. Body and organ layers

| ID | Source | Implementation | Evidence | Target | Verification evidence and remaining gap |
|---|---|---|---|---|---|
| BODY-001 | DISS p. 100; BVS18 | DONE | VERIFIED | M2 | complete, strongly connected, schema-validated 95-segment graph; converter and graph-invariant tests |
| BODY-002 | DISS pp. 100, 113–117 | DONE | VERIFIED | M2 | SI schema and validator for ID, type, geometry, length, diameter, cross section, volume, flow, sources, and uncertainty; canonical M2.2 data set |
| BODY-003 | DISS pp. 101–104 | LEGACY | UNVERIFIED | M2 | analytical 3D geometry tests and model report |
| BODY-004 | DISS pp. 117, 140–143 | DONE | VERIFIED | M2 | the same validated loader and CLI load the synthetic four-segment and canonical 95-segment graphs; state profiles are also applied without rebuilding |
| BODY-005 | DISS pp. 99–104 | DONE | VERIFIED | M2 | scheduled injection and partial/complete extraction, identity-preserving transport, stable extraction selection, and conservation of active plus extracted equals injected |
| BODY-006 | DISS pp. 100–101, 118–122 | DONE | VERIFIED | M2 | 23 dissertation transitions, supported vessel-9 split, stationary flow conservation, reproducible branching, perfusion regression, and flow-conserving state recomputation; independent physiological validation remains separate |
| BODY-007 | DISS pp. 115–117 | DONE | VERIFIED | M2/M4 | deterministic transit compartment implemented; laminar/streamline variant and reference comparison follow |
| BODY-008 | DISS pp. 120–122 | DONE | PART | M2/M3 | machine-readable rest, 1.9× exercise, and 70° head-up-tilt profiles with sources, validity, and CLI exist; regional exercise redistribution, vertebral drainage, pressure, compliance, and dynamic state transitions follow in M3/M4 |
| BODY-009 | DISS p. 101 | SPEC | RESEARCH | M4/M5 | documented blood-model variants and sensitivity |
| BODY-010 | BVS18 pp. 4–6 | DONE | VERIFIED | M2 | deterministic 6,359/63,590-particle regression, equilibrium at minute 7, injection-site comparison, exact population conservation, and schema-validated golden reference |
| ORG-001 | DISS pp. 118–126 | PART | PART | M3 | coarse and serial-region surrogates plus resting, bounded flow-adaptive, age-/resistance-conditioned, and fixed/age-conditioned pressure-distensible literature-parameterized mean pulmonary 0D model cards and tests exist; anatomical, pulsatile, and regional activity-state variants follow |
| ORG-002 | DISS pp. 118–122 | DONE | PART | M3 | M2.4 checks whole-body perfusion; M3.7 adds source-scoped aggregate physiology; M3.8–M3.13 add independent evaluation and empirical flow/age refinements; M3.14–M3.15 test pressure distensibility without hiding inferior validation; M3.16 adds five anatomically named parallel lobe beds with a declared DE-CT proxy and exact aggregate equivalence; M3.17 independently qualifies all five fixed normal-supine shares against both published V/Q SPECT/CT reconstructions; PCQ-1.1/1.2 freeze participant-level endpoints and rank the Arizona iCPET and Bailey access paths; PCQ-1.3 freezes ten-participant hemodynamic/regional floors, observation models, and numeric gates while retaining the five-control Arizona result as pilot-only; measured participant trajectories and dynamic regional states remain open |
| ORG-003 | DISS pp. 95, 153–154 | DONE | VERIFIED | M3 | tested body → lung → body ownership round trip with named ports, stable identity, synchronization time, and explicit outside-body ledger |
| ORG-004 | DISS pp. 122–123, Ch. 6 | LEGACY | UNVERIFIED | M3/M7 | localization event with tissue and uncertainty |
| ORG-005 | DISS pp. 123–126 | DONE | VERIFIED | M3 | schema-selected coarse, three-region surrogate, and pulmonary 0D implementations share one component contract; M3.18 verifies unchanged end-to-end scenario meaning across coarse and five-lobe candidates; anatomical 1D/geometry refinement follows |
| ORG-006 | DISS pp. 124–126 | PART | PART | M3/M8 | external-data contract preserves checksum, format, axes, units, and transformations; official SimVascular arterial candidate reviewed with access/license/state/unit/coverage blockers; import adapter and geometry verification follow |

## 4. Capillary and cell layers

| ID | Source | Implementation | Evidence | Target | Verification evidence and remaining gap |
|---|---|---|---|---|---|
| CAP-001 | DISS pp. 126–129 | DONE | VERIFIED | M4 | strict v2/v3 schemas and executable arteriole–capillary–venule components derive volume, area, velocity, and transit from SI geometry and continuity flow, then preserve identity and conserved payloads through a complete organ-capillary-organ route; anatomical network refinement follows |
| CAP-002 | DISS pp. 126–129 | DONE | PART | M4 | M4.7 pulmonary card separates functional perfused volume from morphometric capacity, records parameter-level evidence and uncertainty, and enforces equivalent-geometry volume-flow-transit closure; PCQ-1.1/1.2 freeze the candidate and prioritize D'Souza 2025 functional-volume/flow plus a separate Lassen 2023 whole-pulmonary transit model; PCQ-1.3 locks a twelve-participant 0.80-1.25 functional-volume equivalence gate and keeps transit blocked until extra-capillary and mixing delays are independently fixed, so no participant-level capillary qualification is yet claimed |
| CAP-003 | DISS pp. 127–128 | DONE | PART | M4 | strict M4.4 overlay schedules aggregate sphincter groups at exact state boundaries; fixed-flow and equal-path fixed-pressure surrogates recompute perfusion and transit with step-size-independent in-flight progress, while physiological sphincter anatomy, feedback, and qualification follow |
| CAP-004 | DISS p. 127 | DONE | PART | M4 | optional M4.5 profile partitions typed substance amount across outgoing blood, endothelium, interstitium, and cell with an enforced balance record and complete organ-capillary-organ test; DCCQ-1.1 audits it as a staged synthetic regression and requires blood-free, endothelial-free, interstitial-free, receptor-bound, internalized, cleared/degraded, and outlet ownership. DCCQ-1.2 selects VEGF-A165a/VEGFR2 HUVEC trafficking to constrain the next mechanism, but dynamic physiological kinetics, reverse flux, metabolism, implementation, and qualification still follow. |
| CAP-005 | DISS pp. 128–129 | DONE | PART | M4/M5 | M4.6 exposes local position, exact regional residence, and normalized competing outcome likelihoods; M4.12 optionally samples those outcomes through a named deterministic stream and conserves every ID across organ return or an acknowledged, retryable hand-off to one explicit terminal owner and compartment; physiological rates, uncertainty distributions, reversible interactions, and executable M5 tissue dynamics follow |
| CAP-006 | DISS pp. 129, 154 | DONE | VERIFIED | M4 | M4.8 defines a typed implementation-neutral request/response and analytical free-diffusion adapter; M4.9–M4.11 add deterministic endpoint particles, bounded paths, and a conservative radial field; M4.13 adds a separate pulmonary-card-bound shared case in which analytical, 200,000-particle, and 256-to-512-cell field resolutions compare directed advection, diffusion, bulk reaction, and a surface-to-volume wall sink with explicit balance and refinement gates. DCCQ-1.1 retains these as transport-verification baselines, and DCCQ-1.2 selects a human endothelial VEGF-A165a/VEGFR2 mechanism without relabelling the synthetic wall sink as biological evidence; explicit radial wall encounters, selected receptor kinetics, external tools, noise, and physiological signal qualification follow without reopening the stable contracts. |
| CAP-007 | DISS p. 129 | SPEC | RESEARCH | M4/M6 | reachability and multi-hop comparison |
| CELL-001 | DISS pp. 129–132 | PART | PART | M5 | analytical and finite-volume biomarker-field references exist; DCCQ-1.1 freezes the required dynamic identity/unit discipline, consumptive tissue/receptor amount ledger, uncertainty classes, and evidence gates; DCCQ-1.2 selects human VEGF-A165a/VEGFR2 HUVEC trafficking with NRP1 explicit and audits convertible source units and evidence roles, but the reduced biological field equations, SI mapping, implementation, and qualification remain open |
| CELL-002 | DISS pp. 130–132 | DONE | PART | M5/M7 | M5.1–M5.7 provide the synthetic binding-to-response chain; BCQ-1.1–1.3 select, licence-screen, and independently reproduce the minimal Kallenberger `BIOMD0000000523`/`0524` family in COPASI; BCQ-1.4–1.7 add a typed no-refit M5 adapter and separate 13-reaction implementation, pass every one of 34,596 all-state/time COPASI comparisons, deterministic replay, RK4 refinement, invariants, structural checks, and 88 sensitivity-stability checks. DCCQ-1.1 correctly blocks direct dynamic SI reuse because the adapter fixes `CD95L=16.6`, retains unresolved source-native units, and represents average HeLa cells; DCCQ-1.2 ranks it fourth and selects VEGF-A165a/VEGFR2 HUVEC trafficking for the new prospective path. This proves a computationally qualified published average-cell mechanism; dynamic implementation, publication curves, a reusable population ensemble, external human attestation, individual-cell prediction, and biological or clinical qualification remain blocked. |
| CELL-003 | DISS pp. 130–132, 153–154 | PART | PART | M5 | M5.6 provides threshold-derived activation plus an analytical, amount-conserving device-to-extracellular-to-intracellular release/uptake chain; M5.7 consumes the final intracellular inventory in a separate synthetic response. DCCQ-1.1 requires dynamic free/bound/internalized/cleared ownership and delayed feedback, and DCCQ-1.2 supplies the named VEGF-A165a/VEGFR2 HUVEC evidence target; spatial diffusion-to-cell composition, consumptive receptor binding, prospective SI kinetics, calibration, and independent biological validation remain open. |
| CELL-004 | DISS pp. 131–133, 154 | DONE | PART | M5 | M5.5 retains its conserved synthetic ODE/SSA topology; BCQ-1.1–1.7 separately qualify the selected CC0 13-reaction CD95/caspase-8 mechanism computationally inside MEHLISSA against COPASI over all 18 states, with code-to-equation mapping, replay, convergence, invariants, nonnegativity, structural sensitivity, and bounded claim review. Publication-series and biological qualification remain open, so evidence stays PART rather than VERIFIED. |
| CELL-005 | DISS pp. 153–154 | DONE | PART | M5 | M5.7–M5.8 retain the synthetic apoptosis event and scalable cohort aggregate. BCQ-1.1–1.7 add a published average-cell caspase-8/tBID mechanism with exact source identity and cross-engine evidence, but explicitly decline to fabricate the unavailable correlated population ensemble. A Workbench scenario consumer, publication-curve comparison, calibrated heterogeneity, external human review, and biological qualification remain open. |

M5 is closed at the synthetic software-contract level by the formal gate
review. M5.1 through M5.8 advance `CELL-002`: they verify constant,
pulsed homogeneous binding, numerical convergence, detection and dissociation,
and connect a retained M4 tissue inventory through a neutral, time-scoped
observation and adapter. The first M5 gate statement is therefore satisfied at
the software-contract level, and receptor binding has analytical evidence toward
the third statement. M5.7 also satisfies the measurable higher-layer event
statement at the synthetic software-contract level. Dynamic capillary fields,
calibrated biological heterogeneity, and biological qualification remain open.
BCQ-1 adds one computationally qualified published average-cell mechanism
without changing that conclusion. M5.4 provides a
binding SSA/distribution; M5.5 provides a shared intracellular ODE/SSA network;
and M5.8 provides an exact compressed-population aggregate. `CELL-004` is
functionally implemented but remains evidence-partial: executable ODE/SSA and
distribution models exist, and one distinct published mechanism has completed
cross-engine numerical qualification, while publication-series, population,
external-review, and biological qualification gates remain blocked.
M5.6 advances `CELL-003` from specification to a partial executable contract:
a typed response event activates one addressed device/payload, and an analytical
chain conserves substance across device, extracellular, and intracellular
owners. M5.7 consumes that final intracellular inventory through a separate
synthetic Hill response, changes the declared cell state to
`apoptosis_committed`, and emits a neutral versioned event without coupling the
cell model to a scenario. It does not yet connect spatial diffusion,
drug-receptor binding, concentration/exposure kinetics, mechanistic apoptosis,
or biological calibration. See
[M5 implementation evidence](../m5/README.md),
[the M5.2 hand-off](../m5/CAPILLARY_CELL_SIGNAL_HANDOFF.md),
[the M5.3 time-varying baseline](../m5/TIME_VARYING_RECEPTOR_BINDING.md),
[the M5.4 stochastic baseline](../m5/STOCHASTIC_RECEPTOR_BINDING.md),
[the M5.5 intracellular network](../m5/INTRACELLULAR_RESPONSE_NETWORK.md),
[the M5.6 conservative drug delivery](../m5/CONSERVATIVE_DRUG_DELIVERY.md),
[the M5.7 apoptosis response](../m5/APOPTOSIS_AND_HIGHER_LAYER_FEEDBACK.md),
[the M5.8 population validity guide](../m5/POPULATION_SCALE_AND_VALIDITY.md),
[the M5 evidence qualification](../m5/M5_EVIDENCE_QUALIFICATION.md),
[the M5 gate review](../m5/M5_GATE_REVIEW.md),
[the BCQ-1.4–1.7 bounded qualification result](../qualification/BCQ1_MEHLISSA_QUALIFICATION_RESULT.md),
[ADR-0034](../architecture/adr/0034-analytical-receptor-ligand-baseline.md),
[ADR-0035](../architecture/adr/0035-non-consuming-capillary-cell-signal-handoff.md),
[ADR-0036](../architecture/adr/0036-time-varying-receptor-ligand-ode.md),
[ADR-0037](../architecture/adr/0037-stochastic-receptor-binding-and-population-classification.md),
[ADR-0038](../architecture/adr/0038-shared-intracellular-ode-ssa-network.md),
[ADR-0039](../architecture/adr/0039-conservative-nanodevice-release-and-uptake.md),
[ADR-0040](../architecture/adr/0040-synthetic-apoptosis-and-higher-layer-feedback.md), and
[ADR-0041](../architecture/adr/0041-cohort-compressed-apoptosis-population.md).

Gate M4 is closed by the formal
[M4 gate review](../m4/M4_GATE_REVIEW.md). `CAP-001` through `CAP-005` remain
implemented to the scope recorded above. `CAP-002` through `CAP-005` remain
evidence-partial because their scientific qualification extends beyond the
explicit software gate; `CAP-007` remains specified with a research evidence
gap. The review records why these visible limitations do not invalidate the
four accepted milestone statements.

## 5. Nano-IoT and research data

| ID | Source | Implementation | Evidence | Target | Verification evidence and remaining gap |
|---|---|---|---|---|---|
| IOT-001 | DISS pp. 113–115, 186–188 | DONE | VERIFIED | M6 | M6.1 provides a schema-validated free device type, composable capabilities, typed/discrete payload inventory, target, internal energy/message state, bounded resources, checked lifecycle, and synthetic locator/collector profiles |
| IOT-002 | DISS pp. 96–97 | DONE | VERIFIED | M6 | M6.5 composes the in-body network, active gateway, replaceable BAN adapter, and external analysis/control station and returns one explicitly governed command to a routed local actuator |
| IOT-003 | DISS pp. 117–118, 187–190 | DONE | PART | M2/M6 | passive segment measurement is joined by a resource-bounded active gateway and M6.5 BAN/station loop; physical placement, range, and hardware qualification remain open |
| IOT-004 | RM M6 | DONE | PART | M6 | M6.7 consolidates exact local/BAN attempts, deliveries, bytes, latency, prescribed loss/corruption/expiry, transmitter/receiver/link energy, and bounded station/simulator/device capacities in a strict resilience profile; calibrated noise, interference, throughput, queueing, and channel statistics remain open |
| IOT-005 | DISS pp. 96–100; MEH25 | DONE | VERIFIED | M6 | M6.6 provides a versioned metadata-only external network-simulator request/response boundary, typed and JSON clients, strict identity/validity checks, and a `BanTransportAdapter` implementation without kernel or physiological dependencies; a concrete ns-3 model remains an optional integration |
| DATA-001 | RM 6.1 | PART | PART | M1–M7 | versioned schemas and validators cover experiment, provenance, log, checkpoint, body, organ, capillary, cell, communication, scenario, result, report, and campaign artifacts; release-wide enforcement of source, licence, checksum, units, and coordinates remains incomplete |
| DATA-002 | RM 6.5 | DONE | VERIFIED | M1 | JSON Schema `1.0.0`, manifest and CLI negative tests |
| DATA-003 | RM 2.4, 6.5 | PART | PART | M1–M7 | schema-validated provenance, SHA-256 identities, model/example catalog, unique CLI run directories, M7 results, reports, campaigns, and Workbench audits exist; uniform provenance for every component and standalone benchmark path remains open |
| DATA-004 | VIS20; MEH25 | DONE | VERIFIED | M2 | separate extraction events, measurement-site counters, bounded individual observations, time aggregates, and optional complete/first-N trajectories with explicit truncation indicators and JSON Schema |
| DATA-005 | RM 2.5 | DONE | PART | M0/M1 | the Paper 1 evidence matrix enforces evidence class, intended use, population/state, sources, licences, uncertainty/ranges, validity, limitations, blockers, and linked artifacts across all six current executable families; equivalent enforcement for every future model and every individual parameter follows |
| DATA-006 | RM 6.2 | DONE | PART | M0 | the Paper 1 matrix and source-role audit separate calibration/validation/verification across six families; M3.8–M3.17 retain disjoint and negative pulmonary findings; PCQ-1.1–1.3 add a no-refit design, source register, guarded source roles, and amendment-after-access rules; PCQ-1.4 now enforces confirmed cohort disjointness, frozen source/schema identity, and rights before data access; participant data and completed independent qualification still follow |
| DATA-007 | RM 6.3 | DONE | VERIFIED | M7 | result 2.0.0 combines all artifact/schema hashes, qualified stages, Levels B-D outcomes, Level-E cases, sensitivity/specificity and false-positive/false-negative Wilson intervals, plus explicit limitations |
| DATA-008 | RM 6.2 | DONE | PART | M5/M7 | stochastic moments, Wilson intervals, particle comparisons, campaign summaries, and Paper 1 conditions/seeds/tolerances use declared statistics; PCQ-1.1–1.4 freeze endpoint rules and carry method uncertainty into strict adapters; PCQ-1.5 now covers all six uncertainty classes and nine endpoints with covariance envelopes, a seven-structure ensemble, converged local sensitivities, and nine identifiability rank analyses; global variance attribution remains evidence-blocked, while locked participant-data execution and release-wide propagation remain open |

## 6. Operation, scenarios, and quality

| ID | Source | Implementation | Evidence | Target | Verification evidence and remaining gap |
|---|---|---|---|---|---|
| UX-001 | RM Phase 0/10 | PART | PART | M1/Phase 10 | headless local CLI scenario and campaign execution passes in CI and does not require visualization; scheduler integration and validated HPC operation remain open |
| UX-002 | VIS20 pp. 1–2 | PART | PART | UX-6/Phase 10 | Workbench 1.0 reads decoupled versioned scenario/campaign results and exposes stages, cases, comparisons, provenance, and descriptive plots; body geometry, entity/population time series, density, and heatmaps remain open |
| UX-003 | VIS20 pp. 1–2 | PART | PART | UX-6/Phase 10 | Workbench 1.0 provides scenario/campaign comparison and accessible interactive result navigation; temporal navigation, 3D rotate/pan/zoom, and cross-layer spatial views remain open |
| UX-004 | RM 6.6 | DONE | VERIFIED | UX-3/UX-6 | non-overwriting UX-3 text/CSV/HTML reports and Workbench dashboards/JSON/CSV/SVG exports are generated from retained versioned results through accepted readers without modifying simulation state; parity and source-hash tests pass |
| UX-005 | RM UX-6.1; ADR-0050 | DONE | VERIFIED | UX-6 | the complete UX-6.1–UX-6.8 browser workflow delegates discovery, editing, validation, execution, result reading, comparison, audit, and analysis to accepted host/process interfaces; browser code owns presentation only |
| UX-006 | RM UX-6.1; ADR-0050 | DONE | VERIFIED | UX-6 | loopback allow-list, ephemeral header capability, host validation, explicit static-file and artifact allow-lists, safe text insertion, security headers, no remote assets/telemetry, bounded execution/files, negative tests, and UX-6.8 release review |
| UX-007 | RM UX-6.1; WCAG 2.2 | DONE | VERIFIED | UX-6 | English semantic structure, skip link, programmatic labels, native controls/dialogs, logical keyboard order and focus placement, live status/alert regions, accessible SVG titles/descriptions and table fallback, visible non-color focus/status, narrow reflow, overflow containment, static assertions, and desktop/mobile accessibility-tree review; diverse-user evaluation remains continuous product research |
| UX-008 | RM UX-6.2; ADR-0050 | DONE | VERIFIED | UX-6.2 | schema-derived controls; complete source view; explicit dirty-state, reset, source-change, and navigation handling; server-side editable-path allow-list; authoritative CLI validation; bounded repository-local workspace; exclusive save-as; automated valid round trip, retention, unknown-field, overwrite, and path-escape tests |
| UX-009 | RM UX-6.3; ADR-0050 | DONE | VERIFIED | UX-6.3 | debounced complete-candidate validation through `mehlissa scenario validate`; schema-derived field hints; native/stable issue codes; field/document locations; repair text; warning/error distinction; SHA-256-bound copyable summary; 422 save rejection and closed run gate; positive, structural, semantic, cross-file, and warning tests |
| UX-010 | RM UX-6.4; ADR-0050 | DONE | VERIFIED | UX-6.4 | exact-candidate revalidation; explicit plan confirmation; accepted scenario/campaign process APIs; repository-bounded unique outputs; stage/progress monitor; actual child-process cancellation; versioned atomic run record; exact input, seeds, results, provenance, logs, summaries, tables, failure/cancellation state; allowlisted artifact access; positive reference and six-run campaign plus negative security tests |
| UX-011 | RM UX-6.5; ADR-0050 | DONE | VERIFIED | UX-6.5 | `load_result`-backed scenario outcomes/stages/cases; `load_campaign_result` groups and paired differences; two-completed-scenario comparison; authoritative JSON/CSV and sandboxed UX-3 report drill-through; explicit zero-observation response and comparison rejection for incomplete, failed, or cancelled jobs; reader-parity and browser acceptance tests |
| UX-012 | RM UX-6.6; ADR-0050 | DONE | VERIFIED | UX-6.6 | capability-protected `GET /api/run/audit`; exact retained provenance round trip; scenario/campaign seed, software, input, manifest, model, schema, and result identities; server-side SHA-256 verification; explicit altered/missing states; source/licence and model-maturity panels; persistent non-clinical/non-patient-specific boundary; downloadable complete audit JSON; positive, tamper, incomplete-evidence, non-result, and browser acceptance tests |
| UX-013 | RM UX-6.7; ADR-0050 | DONE | VERIFIED | UX-6.7 | capability-protected `GET /api/run/analysis`; `load_campaign_result`-backed observations; declared sensitivity hooks; replicate mean, sample standard deviation, and observed range; sweep and same-seed paired-difference views; metric/unit/sample-count labels; exact-value table; source-result SHA-256 in JSON/CSV/accessible SVG exports; explicit descriptive-versus-deterministic-versus-inferential boundary; reader-parity, non-result, export, and desktop/mobile browser acceptance tests |
| UX-014 | RM UX-6.8; ADR-0050 | DONE | VERIFIED | UX-6.8 | Workbench 1.0.0 PEP 517 wheel and `mehlissa-workbench` entry point; packaged static resources and MPL-2.0 licence; clean isolated installation plus `--version` and repository/executable `--check`; documented package boundary and example workspace; semantic, keyboard, screen-reader-tree, responsive, error-recovery, desktop/mobile, full-regression, and supported-CI release gates |
| SCN-001 | DISS pp. 185–190; FP23 | DONE | PART | M3/M7 | M3.19 executes the published FP9 Level-A timer cohorts; M7.1-M7.7 add strict M2-M6 selection, typed physiological initialization, causal identity trace, concentration binding, explicit tiles, executed locator-to-station communication, misclassification analysis, and holistic result. Exact FP9 biology and physical channels remain post-gate research work |
| SCN-002 | DISS pp. 143–152 | LEGACY | UNVERIFIED | Phase 8 | monitoring reference experiment and alert metrics |
| SCN-003 | DISS pp. 155–160 | LEGACY | UNVERIFIED | Phase 8 | reproduction of published detection rates |
| SCN-004 | DISS pp. 153–154 | SPEC | UNVERIFIED | Phase 8 | complete multilayer capstone |
| SCN-005 | MEH25 pp. 1–2 | LEGACY | UNVERIFIED | Phase 8 | CAR-T benchmark and model comparison |
| SCN-006 | DISS pp. 140–143 | SPEC | UNVERIFIED | M8 | incrementally personalized research twin |
| QUA-001 | RM M1 | DONE | VERIFIED | M1 | green MSVC/GCC/Clang CI matrix |
| QUA-002 | RM M1 | DONE | VERIFIED | M1 | clang-tidy, ASan/UBSan, and warnings as errors in CI |
| QUA-003 | MEH25 pp. 1–2 | DONE | PART | continuous | the frozen full RQ4 campaign now retains 112/112 completed attempts, exact cross-policy state/RNG/population invariants, bounded outputs, resource summaries, raw artifacts, and a rejected setup deviation; cross-platform replication and optimization-regression history remain open |
| QUA-004 | MEH25 pp. 1–2 | PART | PART | M5/Phase 10 | explicit entities, stochastic populations, cohort-compressed one-trillion-cell aggregation, compartment transport, finite-volume fields, analytical/surrogate comparisons, and a small repeated 1,000/10,000-collector M7 resource baseline exist; representative end-to-end scale envelopes, profiling, parallel execution, and HPC evidence remain open |
| QUA-005 | RM 6.7 | DONE | VERIFIED | continuous | versioned API/schema/model/scenario documents plus a two-level English User Guide cover non-expert purpose, non-claims, mental model, guided experiment families, decision aid, glossary, and accepted M0–M7 and Workbench 1.0 workflows; formal gate and user-visible delivery reviews require synchronized Roadmap, status brief/PDF, architecture, traceability, and User Guide impact checks |
| QUA-006 | RM M8 | PART | PART | M8 | PCQ-1.4 documents and tests a bounded participant-data concept: manifest-first authorization, pseudonyms only, direct-identifier denial, no raw terminal output, outside-Git quarantine, retention/redistribution metadata, and no read before release; institutional data-protection, consent, security, deletion, and cross-project governance review remain required before real participant processing |
| QUA-007 | RM 6.8; ADR-0051 | DONE | VERIFIED | platform portability | native `macos-apple-clang` configure/build/test presets, Workbench executable discovery, and the complete suite pass on the pinned macOS 15 ARM64 Apple Clang runner in [CI run 33956456353](https://github.com/RegineWendt/MEHLISSA/actions/runs/33956456353); no binary distribution is claimed |

## 7. M0 coverage review

Phase 0 is complete at domain level when the following decisions and inventories
exist in addition to these documents:

- [x] four layers and responsibilities defined as binding (`ARC-001` through `ARC-007`);
- [x] legacy as reference, selective adoption, and a new kernel decided (ADR-0001);
- [x] C++20/CMake/vcpkg selected as the technical foundation (ADR-0003);
- [x] fingerprinting selected as the first vertical demonstrator (ADR-0004);
- [x] evidence and validity classes defined (ADR-0005 and `DATA-005/006`);
- [x] data inventory, including external models, complete;
- [x] target users and prioritized workflows defined as the M0 baseline;
- [x] reference organ for M3 selected: lung (ADR-0006);
- [x] partner/data gaps for proteomics, pulmonary hemodynamics, and wet-lab validation identified;
- [x] multiple licensing implemented technically: MPL-2.0 for independent Next code, GPL-2.0-only for legacy and direct ports, and CC-BY-4.0 for new original documentation and approved original data (ADR-0007).

M0 is therefore complete. Unresolved rights for individual existing data and
publications are tracked as release gates for the respective artifacts; they
block neither M1 nor independently developed Next releases. Details are in the
[M0 gate review](../m0/M0_GATE_REVIEW.md).
