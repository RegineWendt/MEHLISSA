# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the prospective BCQ-1.2 no-refit reproduction protocol."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "data/qualification/biological-cell-model-reproduction-protocol-v1.json"
SCHEMA = ROOT / "data/schemas/biological-cell-model-reproduction-protocol/1.0.0.schema.json"

EXPECTED_BASELINE = "3f7c555bdc604b1acb9da20cd632b1b7a2770d62"
EXPECTED_PARENT_HASH = "3cd72bc3e3f71f26089b448d3ae6e80000a2d1156abc99cf95669a2f18a723be"
EXPECTED_ARTIFACTS = {
    "BIOMD0000000523": {
        "role": "calibration-like-CD95-HeLa-average-cell",
        "source_commit": "8605e43f8e2fd364f122d579341891c0058ef778",
        "sha256": "2afe6758ab396038e71fcb1716fefcfec67656b8bd0bfb3da8d4e1eda9524ff4",
        "initial_values": {
            "CD95": 116.0, "FADD": 93.0, "DISC": 0.0,
            "p55free": 155.0, "DISCp55": 0.0, "p30": 0.0,
            "p43": 0.0, "p18": 0.0, "p18inactive": 0.0,
            "Bid": 236.0, "tBid": 0.0, "PrNES_mCherry": 973.0,
            "PrNES": 0.0, "mCherry": 0.0, "PrER_mGFP": 5178.0,
            "PrER": 0.0, "mGFP": 0.0, "CD95L": 16.6,
        },
    },
    "BIOMD0000000524": {
        "role": "publication-wild-type-HeLa-average-cell",
        "source_commit": "d091308a14fb4301a4a2b1b567ea874484bb97e6",
        "sha256": "4bf4a5bcda5b43a551bcdda09fca91a5e777d2c5db1eafcb17dcb6f1574221bc",
        "initial_values": {
            "CD95": 12.0, "FADD": 90.0, "DISC": 0.0,
            "p55free": 127.0, "DISCp55": 0.0, "p30": 0.0,
            "p43": 0.0, "p18": 0.0, "p18inactive": 0.0,
            "Bid": 224.0, "tBid": 0.0, "PrNES_mCherry": 1909.0,
            "PrNES": 0.0, "mCherry": 0.0, "PrER_mGFP": 3316.0,
            "PrER": 0.0, "mGFP": 0.0, "CD95L": 16.6,
        },
    },
}
EXPECTED_PARAMETERS = {
    "kon_FADD": 0.000811711012144556,
    "koff_FADD": 0.00566528253772301,
    "kDISC": 0.000491828591049766,
    "kD216": 0.0114186392006403,
    "kD374trans_p55": 0.000446994772958953,
    "kD374trans_p43": 0.00343995957326369,
    "kdiss_p18": 0.0949914492651531,
    "kBid": 0.00052867403363568,
    "kD374probe": 0.00152252549827479,
    "KDR": 8.98496674617627,
    "KDL": 15.421878766215,
}
EXPECTED_ASSIGNMENT = (
    "CD95act = (CD95^3 * KDL^2 * CD95L) / ((CD95L + KDL) * "
    "(CD95^2 * KDL^2 + KDR * CD95L^2 + 2 * KDR * KDL * CD95L + KDR * KDL^2))"
)
EXPECTED_RUNS = [
    ("BCQ-RUN-523-P", "BIOMD0000000523", "primary", "primary reproduction"),
    ("BCQ-RUN-523-R", "BIOMD0000000523", "primary", "deterministic replay"),
    ("BCQ-RUN-523-T", "BIOMD0000000523", "tightened", "numerical convergence"),
    ("BCQ-RUN-524-P", "BIOMD0000000524", "primary", "primary reproduction"),
    ("BCQ-RUN-524-R", "BIOMD0000000524", "primary", "deterministic replay"),
    ("BCQ-RUN-524-T", "BIOMD0000000524", "tightened", "numerical convergence"),
]
EXPECTED_OBSERVABLES = ["PrER_mGFP", "PrNES_mCherry", "p43", "p18"]
EXPECTED_INVARIANTS = {
    "INV-FADD-COMPLEX": ["FADD", "DISC", "DISCp55", "p30", "p43"],
    "INV-P55-FAMILY": ["p55free", "DISCp55", "p30", "p43", "p18", "p18inactive"],
    "INV-BID-FAMILY": ["Bid", "tBid"],
    "INV-NES-SUBSTRATE": ["PrNES_mCherry", "PrNES"],
    "INV-NES-FLUOROPHORE": ["PrNES_mCherry", "mCherry"],
    "INV-ER-SUBSTRATE": ["PrER_mGFP", "PrER"],
    "INV-ER-FLUOROPHORE": ["PrER_mGFP", "mGFP"],
    "INV-CD95-CONSTANT": ["CD95"],
    "INV-CD95L-CONSTANT": ["CD95L"],
}


class ReproductionProtocolError(ValueError):
    """Raised when the frozen BCQ-1.2 protocol is inconsistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    result: list[str] = []
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"{location}: {error.message}")
    if result:
        return result

    protocol = document["protocol"]
    if protocol["baseline_commit"] != EXPECTED_BASELINE:
        result.append("frozen pre-trajectory baseline commit changed")
    parent = protocol["parent_register"]
    if parent["path"] != "data/qualification/biological-cell-model-candidate-register-v1.json":
        result.append("BCQ-1.2 must bind the BCQ-1.1 candidate register")
    parent_path = (root / parent["path"]).resolve()
    try:
        parent_path.relative_to(root.resolve())
    except ValueError:
        result.append("parent register escapes repository root")
    else:
        if not parent_path.is_file():
            result.append("parent register does not exist")
        elif sha256(parent_path) != parent["sha256"] or parent["sha256"] != EXPECTED_PARENT_HASH:
            result.append("parent register hash changed")

    artifacts = document["source_artifacts"]
    if [item["accession"] for item in artifacts] != list(EXPECTED_ARTIFACTS):
        result.append("selected artifact identities or order changed")
    for artifact in artifacts:
        expected = EXPECTED_ARTIFACTS.get(artifact["accession"])
        if expected is None:
            continue
        for field in ("role", "source_commit", "sha256"):
            if artifact[field] != expected[field]:
                result.append(f"{artifact['accession']}: frozen {field} changed")
        if artifact["initial_values"] != expected["initial_values"]:
            result.append(f"{artifact['accession']}: frozen initial values changed")
        if artifact["file"] != f"{artifact['accession']}.xml":
            result.append(f"{artifact['accession']}: source filename changed")
        if artifact["license"] != "CC0-1.0":
            result.append(f"{artifact['accession']}: source model licence changed")

    semantics = document["shared_model_semantics"]
    if semantics["kinetic_parameters"] != EXPECTED_PARAMETERS:
        result.append("frozen kinetic parameter set changed")
    if semantics["assignment_rule"] != EXPECTED_ASSIGNMENT:
        result.append("frozen CD95act assignment rule changed")
    if semantics["stimulus"]["initial_value"] != 16.6:
        result.append("frozen CD95L stimulus changed")

    unit_policy = document["unit_policy"]
    if any(
        unit_policy[key]
        for key in (
            "sbml_declares_model_time_unit",
            "sbml_declares_model_substance_unit",
            "sbml_declares_compartment_unit",
            "conversion_allowed",
        )
    ):
        result.append("undeclared SBML units cannot be promoted or converted")
    if unit_policy["time_unit"] != "unresolved-model-native" or unit_policy["state_unit"] != "unresolved-model-native":
        result.append("time and state units must remain unresolved-model-native")

    solver = document["solver"]
    expected_solver = {
        "name": "COPASI command-line engine (CopasiSE)",
        "version": "4.46",
        "build": 300,
        "release_tag": "Build-300",
        "source_commit": "e9c47d912b55eccd56f70b72e52f19d61f5ab2e2",
        "license": "Artistic-2.0",
        "task": "deterministic time course",
        "method": "LSODA",
        "reduced_model": False,
    }
    for field, value in expected_solver.items():
        if solver[field] != value:
            result.append(f"frozen solver {field} changed")
    if solver["primary_settings"] != {
        "relative_tolerance": 1e-9,
        "absolute_tolerance": 1e-12,
        "maximum_internal_steps": 100000,
    }:
        result.append("primary solver settings changed")
    if solver["tightened_settings"] != {
        "relative_tolerance": 1e-11,
        "absolute_tolerance": 1e-13,
        "maximum_internal_steps": 200000,
    }:
        result.append("tightened solver settings changed")
    if solver["optimization_or_parameter_estimation_enabled"]:
        result.append("optimization or parameter estimation cannot be enabled")

    grid = document["execution"]["time_grid"]
    if grid != {
        "start": 0.0,
        "end": 240.0,
        "interval": 0.25,
        "points_including_endpoints": 961,
        "unit": "unresolved-model-native",
        "output_at_exact_grid": True,
    }:
        result.append("frozen time grid changed")
    runs = [tuple(item[field] for field in ("id", "accession", "settings", "purpose")) for item in document["execution"]["matrix"]]
    if runs != EXPECTED_RUNS:
        result.append("six-run execution matrix changed")
    if not document["execution"]["retain_failed_runs"]:
        result.append("failed runs must be retained")

    observable_ids = [item["id"] for item in document["observables"]]
    if observable_ids != EXPECTED_OBSERVABLES:
        result.append("four primary observables or their order changed")

    initial_by_accession = {
        accession: expected["initial_values"] for accession, expected in EXPECTED_ARTIFACTS.items()
    }
    invariant_ids = [item["id"] for item in document["invariants"]]
    if invariant_ids != list(EXPECTED_INVARIANTS):
        result.append("source-derived invariant identities or order changed")
    for invariant in document["invariants"]:
        expected_species = EXPECTED_INVARIANTS.get(invariant["id"])
        if expected_species is not None and invariant["species"] != expected_species:
            result.append(f"{invariant['id']}: invariant species changed")
        for accession, expected_total in invariant["expected_initial_total_by_accession"].items():
            actual_total = sum(initial_by_accession[accession][species] for species in invariant["species"])
            if actual_total != expected_total:
                result.append(f"{invariant['id']}: initial total disagrees with {accession}")

    rules = document["acceptance_rules"]
    if set(rules) != {
        "source_identity", "import_structure", "complete_output", "initial_state",
        "deterministic_replay", "numerical_convergence", "conservation",
        "nonnegativity", "reporter_direction", "publication_alignment",
    }:
        result.append("acceptance-rule set changed")
    if "BLOCKED" not in rules["publication_alignment"]:
        result.append("quantitative publication alignment must remain blocked without numeric reference data")

    control_ids = [item["id"] for item in document["negative_controls"]]
    if control_ids != [f"BCQ-NC-{index:02d}" for index in range(1, 11)]:
        result.append("negative controls must remain the ordered BCQ-NC-01 through BCQ-NC-10 set")

    archive = document["result_archive"]
    if not archive["root"].startswith("results/bcq1/"):
        result.append("result archive must remain under the BCQ-1 results namespace")
    if archive["raw_article_or_participant_data_allowed"]:
        result.append("article or participant data cannot enter the result archive")
    required_entries = set(archive["required_entries"])
    for required in (
        "protocol.json", "source-manifest.json", "solver-provenance.json",
        "reproduction-metrics.json", "validation-report.json",
        "validation-report.md", "sha256sums.json",
    ):
        if required not in required_entries:
            result.append(f"result archive entry is missing: {required}")

    full_text = json.dumps(document).lower()
    for phrase in (
        "no-refit", "no-trajectory", "unresolved-model-native", "failed",
        "not a mehlissa runtime dependency", "complete held-out population prediction",
        "clinical validity", "pre-outcome amendment",
    ):
        if phrase not in full_text:
            result.append(f"required BCQ-1.2 safeguard is absent: {phrase}")
    return result


def validate(path: Path = PROTOCOL, root: Path = ROOT) -> dict[str, Any]:
    document = load_json(path)
    found = errors(document, root)
    if found:
        raise ReproductionProtocolError("\n".join(found))
    return document


def main() -> int:
    try:
        document = validate()
        print(
            "BCQ-1.2 biological cell-model reproduction protocol: ok "
            f"({len(document['source_artifacts'])} artifacts, "
            f"{len(document['execution']['matrix'])} frozen runs, "
            f"{len(document['negative_controls'])} negative controls, "
            "no trajectory executed, no biological qualification claimed)"
        )
    except (OSError, json.JSONDecodeError, ReproductionProtocolError) as error:
        print(f"BCQ-1.2 biological cell-model reproduction protocol: FAILED\n{error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
