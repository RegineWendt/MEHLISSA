<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PCQ-1.4 Rights-Aware Data Ingress

## 1. Decision and status

PCQ-1.4 implements the controlled boundary between a prospective data grant
and the locked PCQ-1.3 analysis. It is complete with outcome-blind synthetic
fixtures. No candidate participant-level outcome record was requested,
downloaded, received, opened, or inspected during this increment.

The machine authority is
`data/qualification/pulmonary-capillary-data-ingress-policy-v1.json`. It binds
the committed PCQ-1.3 amendment by SHA-256 and maps each selected source role
to one strict normalized schema, activation status, and sample floor. The
checker and adapter are implemented in
`scripts/check_pulmonary_capillary_data_ingress.py`.

Passing PCQ-1.4 proves that the repository can reject structurally,
scientifically, legally, or spatially ineligible input before evaluation. It
does not prove that any source will grant access, that a granted data set will
pass, or that MEHLISSA agrees with human physiology.

## 2. Mandatory trust sequence

The adapter uses a fail-closed order:

```text
rights/provenance manifest
        |
        v
schema + frozen source + schema-hash checks
        |
        v
authorization + privacy + cohort-independence checks
        |
        v
resolved path must be outside Git and inside explicit quarantine root
        |
        v
only now may the participant data file be opened
        |
        v
content hash + family schema + physiological eligibility checks
        |
        v
normalized in-memory records; metadata-only command output
```

For measured data, the resolved file path must be outside the repository and
inside an existing, absolute quarantine directory supplied by the operator.
There is no default quarantine path. A missing, relative, repository-internal,
or mismatched root rejects the request before the data file is read.

Synthetic fixtures are the only participant-shaped records permitted in Git.
They require an explicit `--allow-synthetic` switch, are marked
`synthetic_test_only`, and can never become measured evidence.

## 3. Rights and provenance manifest

Every data file requires a separate manifest conforming to
`data/schemas/pulmonary-capillary-source-manifest/1.0.0.schema.json`. The
manifest is inspected before the data and declares:

- stable data-set, measurement-family, and selected-candidate identities;
- file and schema SHA-256 values and the expected participant count;
- data controller, authorization state, approval reference, permitted
  processing/analysis/publication, redistribution, and retention;
- pseudonymization, removal of direct identifiers, and repository-storage
  policy;
- source/calibration cohort identities and a reviewed disjointness state; and
- quarantine requirement plus explicit release to the adapter.

Measured input is released only when processing and analysis are approved,
privacy conditions are satisfied, cohort independence is
`confirmed_disjoint`, no cohort identifier overlaps calibration, authorization
has not expired, quarantine is required, and release is explicit. Pending or
denied rights do not permit a trial read.

The command never prints raw observations. Its successful JSON response is
limited to data-set/family/candidate identity, participant and normalized-record
counts, evidence status, sample status, and activation state.

## 4. Normalized measurement families

| Family | Frozen source role | Strict primary context | Key rejection boundaries | Activation after valid ingestion |
|---|---|---|---|---|
| `pcq_hemodynamics` | Arizona iCPET, `PCQ-SRC-H-001` | healthy age 20-40, supine/recumbent, rest plus at least two ordered exercise stages | fixed units; joint mPAP/PAWP/output; uncertainty floors; contiguous stages; nondecreasing workload; at least 2 L/min output span; optional sPAP/dPAP/HR all or none | locked, awaiting authorized measured data; sample size still determines pilot/inconclusive/decision-floor status |
| `pcq_lobar_perfusion` | Bailey five-lobe data, `PCQ-SRC-R-002` | healthy age 20-40, supine rest, five anatomical lobes | exactly five named fractions summing to one; per-lobe uncertainty at least 0.02; no four-vein or spatial-zone substitution | locked, awaiting feasibility and authorized measured data |
| `pcq_capillary_volume` | D'Souza functional `Vc`, `PCQ-SRC-CJ-001` | healthy age 20-40, recumbent/semirecumbent baseline rest | positive functional `Vc` and same-state flow; measured hemoglobin; one documented correction; regression `r²` at least 0.95; method CV at least 0.08 | locked, awaiting authorized measured data; PCQ-J remains partial |
| `pcq_whole_pulmonary_transit` | Lassen transit, `PCQ-SRC-C-003` | healthy age 20-40, supine rest, pulmonary trunk to left atrium | fixed anatomy and method; 0.172 repeatability floor; 0.25 tracer-bias sensitivity; no derived PBV as independent evidence | **blocked-observation-model**, regardless of record count |

Schema-valid input is not automatically decision-ready. The adapter reports
one of `inconclusive-insufficient-sample`, `bounded-pilot-sample`,
`decision-sample-floor-met`, or `blocked-observation-model`. Even
`decision-sample-floor-met` only permits the later frozen evaluation; it is not
a qualification result.

## 5. Privacy and restricted-field boundary

All normalized records use a study pseudonym. Strict schemas reject unknown
fields, and the adapter independently scans the full input tree for direct or
operational identifiers including names, dates of birth, contact/address
fields, medical-record identifiers, free-text notes, source-system identifiers,
and examination/admission dates. This second deny-list is intentional defense
in depth.

An actual grant may require stronger local controls, encryption, access
logging, deletion deadlines, or institutional processing infrastructure. The
manifest records those obligations but does not replace institutional data
protection review. No identifiable record belongs in Git, a test fixture, a
shared report, or terminal output.

## 6. Operator workflow for a future authorized source

Do not copy measured data into this repository. An authorized data steward
should instead:

1. create an institution-controlled quarantine directory outside the checkout;
2. place the pseudonymized source extract there without opening it in a project
   editor;
3. prepare and independently review a source manifest containing the exact file
   and schema hashes, rights, privacy, retention, and cohort-independence facts;
4. run the adapter with explicit paths; and
5. retain the metadata-only summary and permitted aggregate results according
   to the grant.

Example with placeholders:

```powershell
python scripts/check_pulmonary_capillary_data_ingress.py `
  --manifest C:\approved-manifests\source.manifest.json `
  --data D:\institutional-quarantine\source.json `
  --quarantine-root D:\institutional-quarantine
```

The command does not search for data, infer permission, move files, request
access, or write normalized observations back to disk. A successful ingress
summary is necessary but not sufficient for PCQ-1.6 evaluation.

## 7. Outcome-blind verification fixtures

Four CC0 fixtures under `tests/data/pulmonary-capillary-ingress/` exercise the
schemas and adapters. Their values are arbitrary, deliberately not derived from
published aggregates, and visibly labelled as software tests. The test suite
verifies:

- all four source/schema/family mappings and content hashes;
- explicit synthetic authorization and permanent non-evidence status;
- rejection of measured data inside the repository before file opening;
- pending rights, expired or missing authorization, and cohort overlap;
- incorrect checksum or schema identity;
- age, unit, stage-tuple, order, flow-span, and five-lobe-sum failures;
- recursive direct-identifier rejection; and
- persistence of the whole-pulmonary observation-model block.

Run the complete fixture check with:

```powershell
python -m pip install -e ".[publication]"
python scripts/check_pulmonary_capillary_data_ingress.py
python -m unittest tests.test_pulmonary_capillary_data_ingress -v
```

For a single synthetic fixture, `--allow-synthetic` is required. Never use that
switch to relabel or process measured data.

## 8. Remaining limitations and next increment

PCQ-1.4 does not contain source-native CSV, spreadsheet, DICOM, or database
converters because no exact granted source layout exists. Adding a converter
before seeing a documented source format would invent a contract. Once a grant
provides a data dictionary, a source-specific converter may be added before
this normalized boundary; it must receive its own fixtures and must not weaken
the manifest-first rule.

PCQ-1.5 has now supplied the separate, outcome-blind
[uncertainty and identifiability plan](PCQ1_UNCERTAINTY_IDENTIFIABILITY.md).
PCQ-1.6 is next: pass only rights-authorized normalized records from this
boundary into the frozen no-refit evaluator and retain every eligible,
partial, blocked, and failed result. Until such access exists, execution remains
`blocked-access-or-rights` rather than falling back to repository fixtures.

PCQ-1.4 is research infrastructure only. It is not a clinical data platform,
medical device, ethics approval, data-use agreement, or physiological
qualification result.
