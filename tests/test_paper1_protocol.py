# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import check_paper1_protocol as protocol  # noqa: E402


class Paper1ProtocolTests(unittest.TestCase):
    def test_locked_protocol_is_valid(self) -> None:
        protocol.validate()

    def test_modified_predecessor_is_rejected(self) -> None:
        document = copy.deepcopy(protocol.load(protocol.PROTOCOL))
        document["amendment"]["predecessor_modified"] = True
        self.assertTrue(protocol.errors(document))

    def test_missing_experiment_is_rejected(self) -> None:
        document = copy.deepcopy(protocol.load(protocol.PROTOCOL))
        document["experiments"].pop()
        self.assertTrue(protocol.errors(document))


if __name__ == "__main__":
    unittest.main()
