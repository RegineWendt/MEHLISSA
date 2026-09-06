# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_biological_cell_model_reproduction_protocol as protocol  # noqa: E402


class BiologicalCellModelReproductionProtocolTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = protocol.load_json(protocol.PROTOCOL)

    def test_checked_in_protocol_is_valid_and_pre_trajectory(self) -> None:
        document = protocol.validate()
        self.assertFalse(document["protocol"]["first_trajectory_generated"])
        self.assertEqual(document["decision"]["m5_evidence_status"], "software_test_surrogate")

    def test_parent_register_hash_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["protocol"]["parent_register"]["sha256"] = "0" * 64
        self.assertTrue(any("parent register hash" in item for item in protocol.errors(document)))

    def test_source_hash_and_initial_values_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["source_artifacts"][0]["sha256"] = "0" * 64
        document["source_artifacts"][1]["initial_values"]["CD95"] = 116.0
        found = protocol.errors(document)
        self.assertTrue(any("sha256" in item for item in found))
        self.assertTrue(any("initial values" in item for item in found))

    def test_undeclared_units_cannot_be_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["unit_policy"]["time_unit"] = "minute"
        document["unit_policy"]["conversion_allowed"] = True
        self.assertTrue(any("unit" in item for item in protocol.errors(document)))

    def test_solver_and_tolerances_are_frozen(self) -> None:
        document = copy.deepcopy(self.document)
        document["solver"]["method"] = "Runge-Kutta"
        found = protocol.errors(document)
        self.assertTrue(any("method" in item for item in found))

        document = copy.deepcopy(self.document)
        document["solver"]["primary_settings"]["relative_tolerance"] = 1e-6
        found = protocol.errors(document)
        self.assertTrue(any("primary solver settings" in item for item in found))

    def test_parameter_fitting_cannot_be_enabled(self) -> None:
        document = copy.deepcopy(self.document)
        document["solver"]["optimization_or_parameter_estimation_enabled"] = True
        document["execution"]["parameter_refitting"] = "allowed"
        self.assertTrue(protocol.errors(document))

    def test_assignment_rule_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["shared_model_semantics"]["assignment_rule"] = "CD95act = 0"
        self.assertTrue(any("assignment rule" in item for item in protocol.errors(document)))

    def test_run_matrix_and_observables_cannot_change(self) -> None:
        document = copy.deepcopy(self.document)
        document["execution"]["matrix"][2]["settings"] = "primary"
        document["observables"].reverse()
        found = protocol.errors(document)
        self.assertTrue(any("execution matrix" in item for item in found))
        self.assertTrue(any("observables" in item for item in found))

    def test_invariant_totals_are_derived_from_frozen_initial_values(self) -> None:
        document = copy.deepcopy(self.document)
        document["invariants"][0]["expected_initial_total_by_accession"]["BIOMD0000000523"] += 1.0
        self.assertTrue(any("initial total" in item for item in protocol.errors(document)))

    def test_publication_alignment_cannot_silently_pass(self) -> None:
        document = copy.deepcopy(self.document)
        document["acceptance_rules"]["publication_alignment"] = "PASS"
        self.assertTrue(any("publication alignment" in item for item in protocol.errors(document)))

    def test_command_validation_rejects_changed_protocol(self) -> None:
        document = copy.deepcopy(self.document)
        document["protocol"]["first_trajectory_generated"] = True
        with tempfile.TemporaryDirectory(prefix="mehlissa-bcq-negative-") as directory:
            invalid = Path(directory) / "invalid.json"
            invalid.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(protocol.ReproductionProtocolError):
                protocol.validate(invalid)


if __name__ == "__main__":
    unittest.main()
