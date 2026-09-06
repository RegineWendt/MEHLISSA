# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import run_biological_cell_model_reproduction as runner  # noqa: E402


def row(value: float = 0.0) -> dict[str, float]:
    return {"model_time": 0.0, **{species: value for species in runner.SPECIES_ORDER}}


class BiologicalCellModelReproductionRunnerTests(unittest.TestCase):
    def test_execution_protocol_binds_base_and_amendment(self) -> None:
        _, amendment, base_hash, amendment_hash, identity = runner.load_execution_protocol()
        self.assertEqual(base_hash, amendment["amendment"]["base_protocol"]["sha256"])
        self.assertEqual(len(amendment_hash), 64)
        self.assertEqual(identity, "5dd09984d838c0a4237d9d100256a91a4e4db5f4473ac0601749db105912d5dd")

    def test_replay_accepts_only_the_prospective_equivalence_margin(self) -> None:
        primary = [row()]
        replay = [row()]
        primary[0]["p18"] = 1.5e-12
        observed = runner.compare_replay(primary, replay, 1e-12, 1e-12)
        self.assertLessEqual(observed["maximum_fraction_of_allowed_difference"], 1.0)
        self.assertFalse(observed["bit_identical"])

        outside = copy.deepcopy(primary)
        outside[0]["p18"] = 2.1e-12
        with self.assertRaises(runner.ReproductionError):
            runner.compare_replay(outside, replay, 1e-12, 1e-12)

    def test_forbidden_unit_label_and_claim_fail_closed(self) -> None:
        with self.assertRaises(runner.ReproductionError):
            runner.reject_unit_label("minute")
        with self.assertRaises(runner.ReproductionError):
            runner.reject_claim("biologically validated held-out population")

    def test_source_hash_mismatch_fails_before_xml_import(self) -> None:
        protocol, _, _, _, _ = runner.load_execution_protocol()
        artifact = runner.artifact_by_accession(protocol, "BIOMD0000000523")
        with self.assertRaises(runner.ReproductionError):
            runner.verify_source(ROOT / "README.md", artifact)


if __name__ == "__main__":
    unittest.main()
