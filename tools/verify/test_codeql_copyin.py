#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
from pathlib import Path
import unittest

HERE = Path(__file__).resolve().parent


class CopyinQuery(unittest.TestCase):
    def test_query_names_copyin(self):
        q = (HERE / "codeql" / "copyin_call.ql").read_text(encoding="utf-8")
        self.assertIn('getName() = "copyin"', q)
        self.assertTrue((HERE / "codeql" / "qlpack.yml").is_file())

    def test_dest_used_query_is_local_not_taint(self):
        q = (HERE / "codeql" / "copyin_dest_used.ql").read_text(encoding="utf-8")
        self.assertIn('getName() = "copyin"', q)
        self.assertIn("getArgument(1)", q)
        self.assertNotIn("TaintTracking", q)
        self.assertIn("Not a taint path", q)


if __name__ == "__main__":
    unittest.main(verbosity=2)
