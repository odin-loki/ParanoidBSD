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
        q = S.smoke_ql("sys_ffclock_setestimate", "sys/kern/kern_ffclock.c")
        self.assertIn('getName() = "sys_ffclock_setestimate"', q)
        self.assertIn("kern_ffclock.c", q)

    def test_build_script_runs_from_source_root(self):
        script = S.build_script("sys/kern/kern_ffclock.c", ["-I", "sys"], "clang")
        self.assertIn("cd ", script)
        self.assertIn("hbsd/src", script.replace("\\", "/"))
        self.assertIn("-c sys/kern/kern_ffclock.c", script)
        self.assertNotIn("codeql-smoke-src", script)

    def test_build_script_extra_cflag(self):
        script = S.build_script(
            "sys/kern/kern_ffclock.c", [], "clang", extra=["-DFFCLOCK"])
        self.assertIn("-DFFCLOCK", script)


if __name__ == "__main__":
    unittest.main(verbosity=2)
