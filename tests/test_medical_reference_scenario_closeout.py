# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

import unittest
from scripts import check_medical_reference_scenario_closeout as closeout


class MrsqCloseoutTests(unittest.TestCase):
    def test_frozen_archive_and_boundaries_pass(self): self.assertEqual(closeout.errors(), [])
    def test_all_increments_are_explicit(self):
        document = closeout.load(closeout.CLOSEOUT)
        self.assertEqual({item["id"] for item in document["increments"]}, {f"MRSQ-1.{i}" for i in range(1, 8)})


if __name__ == "__main__": unittest.main()
