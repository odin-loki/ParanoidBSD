#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""codeql_smoke.py is a one-TU extractor check, not a tree taint run."""
from __future__ import annotations

import unittest
from pathlib import Path

import codeql_smoke as S

ROOT = Path(__file__).resolve().parents[2]


class SmokeContract(unittest.TestCase):
    def test_default_source_exists(self):
        self.assertTrue((ROOT / "hbsd" / "src" / S.SMOKE_REL).is_file())

    def test_query_names_abs(self):
        self.assertIn('getName() = "abs"', S.SMOKE_QL)


if __name__ == "__main__":
    unittest.main(verbosity=2)
