#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The ways an orchestrator lies about what it ran.

A harness that runs twelve instruments and prints the findings of the
five that worked is worse than one that runs five, because the reader
cannot tell the difference. Every test here is a way a stage could
disappear from the record.
"""
from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

import sweep_all as S  # noqa: E402


class Availability(unittest.TestCase):

    def test_a_driver_not_in_the_tree_is_named_not_skipped(self):
        st = S.Stage("x", "cbmc", None, "tools/verify/does_not_exist.py",
                     lambda c: ["true"])
        status, why = st.availability()
        self.assertEqual(status, "missing")
        self.assertIn("does_not_exist.py", why)

    def test_a_tool_not_on_path_is_notrun_with_the_install_line(self):
        st = S.Stage("x", "cbmc", "definitely-not-a-binary-xyzzy", None,
                     lambda c: ["true"],
                     install={"pkg": "foo", "apt": "libfoo-dev"})
        status, why = st.availability()
        self.assertEqual(status, "NOTRUN")
        self.assertIn("pkg install foo", why)
        self.assertIn("apt install libfoo-dev", why)

    def test_a_tool_with_no_package_says_where_to_get_it(self):
        st = S.Stage("x", "esbmc", "definitely-not-a-binary-xyzzy", None,
                     lambda c: ["true"])
        status, why = st.availability()
        self.assertEqual(status, "NOTRUN")
        self.assertIn("taxonomy.py --missing", why)

    def test_a_present_tool_is_ok(self):
        st = S.Stage("x", None, "sh", None, lambda c: ["true"])
        self.assertEqual(st.availability()[0], "ok")

    def test_every_stage_that_names_an_engine_names_a_real_one(self):
        """A typo in an engine name would make matrix.py reject the ingest
        at the very end of a twelve-hour run."""
        import matrix
        for st in S.STAGES:
            if st.engine is not None:
                self.assertIn(st.engine, matrix.ENGINES, st.name)

    def test_every_stage_that_names_a_driver_points_somewhere_plausible(self):
        for st in S.STAGES:
            if st.driver:
                self.assertTrue(st.driver.startswith("tools/"), st.name)


class Dependencies(unittest.TestCase):

    def _state(self, **kw):
        return {"stages": {k: {"status": v} for k, v in kw.items()}}

    def test_a_stage_whose_input_failed_is_blocked_and_says_which(self):
        st = S.STAGE_BY_NAME["cbmc"]
        ctx = {"dir": Path("/tmp"), "scopes": ["x"], "jobs": 1,
               "timeout": 1, "resume": False}
        r = S.run_stage(st, ctx, self._state(classify="failed"), dry=True)
        self.assertEqual(r["status"], "blocked")
        self.assertIn("classify", r["why"])

    def test_in_a_dry_run_a_dependency_that_would_run_counts(self):
        """Otherwise every dependent stage reads `blocked' in a plan for a
        machine where everything IS installed, and the estimate is wrong
        for the stage that dominates it."""
        st = S.STAGE_BY_NAME["cbmc"]
        ctx = {"dir": Path("/tmp"), "scopes": ["x"], "jobs": 1,
               "timeout": 1, "resume": False}
        r = S.run_stage(st, ctx, self._state(classify="would-run"), dry=True)
        self.assertEqual(r["status"], "would-run")

    def test_outside_a_dry_run_would_run_does_not_satisfy_anything(self):
        st = S.STAGE_BY_NAME["cbmc"]
        ctx = {"dir": Path("/tmp"), "scopes": ["x"], "jobs": 1,
               "timeout": 1, "resume": False}
        r = S.run_stage(st, ctx, self._state(classify="would-run"), dry=False)
        self.assertEqual(r["status"], "blocked")

    def test_cbmc_reads_what_classify_writes(self):
        self.assertIn("classify", S.STAGE_BY_NAME["cbmc"].feeds)
        self.assertIn("classify", S.STAGE_BY_NAME["esbmc"].feeds)


class Cost(unittest.TestCase):

    def test_the_universe_is_a_fixed_cost(self):
        """It is built over the WHOLE tree whatever --scope says. Pricing
        it per unit of the scope inflates a tree-wide estimate by the
        square of the tree - it read 1.34 h for an 11-second job."""
        self.assertTrue(S.STAGE_BY_NAME["universe"].fixed_cost)

    def test_nothing_else_is_a_fixed_cost(self):
        for st in S.STAGES:
            if st.name != "universe":
                self.assertFalse(st.fixed_cost, st.name)


class MissingLedger(unittest.TestCase):
    """The generated ledger is not in the repository, and CI is a fresh
    checkout. Zero units would make every per-unit estimate read 0.00 h
    and the whole plan look free."""

    def test_no_ledger_counts_minus_one_not_zero(self):
        real = S.LEDGER
        try:
            S.LEDGER = Path("/nonexistent/port_plan.json")
            self.assertEqual(S.count_units(["sys"]), -1)
        finally:
            S.LEDGER = real

    def test_no_ledger_refuses_rather_than_planning_a_free_run(self):
        import tempfile
        real = S.LEDGER
        try:
            S.LEDGER = Path("/nonexistent/port_plan.json")
            rc = S.main(["--scope", "sys", "--out",
                         tempfile.mkdtemp(), "--dry-run"])
            self.assertEqual(rc, 2)
        finally:
            S.LEDGER = real

    def test_the_hint_names_the_command_that_fixes_it(self):
        self.assertIn("tools/port_plan.py", S.LEDGER_HINT)


class Contract(unittest.TestCase):

    def test_there_is_no_whole_tree_default(self):
        """A twelve-hour run is asked for explicitly."""
        with self.assertRaises(SystemExit):
            S.main([])

    def test_list_works_without_a_scope(self):
        self.assertEqual(S.main(["--list"]), 0)

    def test_the_report_names_what_did_not_run(self):
        import io
        import contextlib
        state = {"stages": {
            "cbmc": {"status": "ok", "seconds": 1.0},
            "esbmc": {"status": "NOTRUN", "why": "esbmc not on PATH"},
            "cppcheck": {"status": "missing", "why": "no driver"},
        }}
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            S.do_report(Path("/tmp"), state, ["x"], 10, {})
        out = buf.getvalue()
        self.assertIn("NOTRUN", out)
        self.assertIn("missing", out)
        self.assertIn("esbmc not on PATH", out)
        self.assertIn("UNTOUCHED, not", out)
        self.assertIn("2 of 3 instruments produced NO data", out)

    def test_a_clean_run_still_prints_the_absences(self):
        """The last thing on screen after a long run is what did NOT
        happen, even when everything that could run did."""
        import io
        import contextlib
        state = {"stages": {"cbmc": {"status": "ok", "seconds": 1.0}}}
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            S.do_report(Path("/tmp"), state, ["x"], 10, {})
        self.assertIn("0 of 1 instruments produced NO data",
                      buf.getvalue())


class Resume(unittest.TestCase):

    def test_state_round_trips(self):
        d = Path(tempfile.mkdtemp())
        st = {"stages": {"cbmc": {"status": "ok", "seconds": 3.0}},
              "started": 1.0}
        S.save_state(d, st)
        self.assertEqual(S.load_state(d), st)

    def test_no_state_file_is_an_empty_run_not_a_crash(self):
        d = Path(tempfile.mkdtemp())
        self.assertEqual(S.load_state(d)["stages"], {})

    def test_no_rates_file_means_no_estimate_not_a_guess(self):
        d = Path(tempfile.mkdtemp())
        self.assertEqual(S.load_rates(d), {})


if __name__ == "__main__":
    unittest.main(verbosity=2)
