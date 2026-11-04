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
        # A real C scope: the kind check runs BEFORE the dependency
        # check, deliberately, because "nothing here is of that kind"
        # is a stronger and more useful answer than "its input failed".
        ctx = {"dir": Path("/tmp"), "scopes": ["sys/geom/uzip"], "jobs": 1,
               "timeout": 1, "resume": False}
        r = S.run_stage(st, ctx, self._state(classify="failed"), dry=True)
        self.assertEqual(r["status"], "blocked")
        self.assertIn("classify", r["why"])

    def test_in_a_dry_run_a_dependency_that_would_run_counts(self):
        """Otherwise every dependent stage reads `blocked' in a plan for a
        machine where everything IS installed, and the estimate is wrong
        for the stage that dominates it."""
        st = S.STAGE_BY_NAME["cbmc"]
        # A real C scope: the kind check runs BEFORE the dependency
        # check, deliberately, because "nothing here is of that kind"
        # is a stronger and more useful answer than "its input failed".
        ctx = {"dir": Path("/tmp"), "scopes": ["sys/geom/uzip"], "jobs": 1,
               "timeout": 1, "resume": False}
        r = S.run_stage(st, ctx, self._state(classify="would-run"), dry=True)
        self.assertEqual(r["status"], "would-run")

    def test_outside_a_dry_run_would_run_does_not_satisfy_anything(self):
        st = S.STAGE_BY_NAME["cbmc"]
        # A real C scope: the kind check runs BEFORE the dependency
        # check, deliberately, because "nothing here is of that kind"
        # is a stronger and more useful answer than "its input failed".
        ctx = {"dir": Path("/tmp"), "scopes": ["sys/geom/uzip"], "jobs": 1,
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
            self.assertEqual(S.count_units(["sys"]), (-1, "none"))
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


class TreeFixture:
    """A tree and a ledger the test owns.

    docs/port_plan.json is generated and .gitignore'd, so in a fresh
    checkout - which is what CI is - it does not exist, and neither
    does whatever the developer's machine happens to have under
    hbsd/src. A test that asserts against those is asserting about the
    machine it runs on. This builds both, so an assertion about
    count_units is about count_units.
    """

    def __init__(self, files=(), ledger_paths=None):
        self.root = Path(tempfile.mkdtemp())
        for rel in files:
            f = self.root / rel
            f.parent.mkdir(parents=True, exist_ok=True)
            f.write_text("int main(void) { return 0; }\n")
        self._root = S.ROOT
        self._ledger = S.LEDGER
        S.ROOT = self.root
        if ledger_paths is None:
            S.LEDGER = self.root / "there-is-no-ledger.json"
        else:
            S.LEDGER = self.root / "port_plan.json"
            S.LEDGER.write_text(json.dumps(
                {"records": [{"path": x} for x in ledger_paths]}))

    def undo(self):
        S.ROOT = self._root
        S.LEDGER = self._ledger


class KindRouting(unittest.TestCase):
    """A C scope handed to a C++ stage is sweep_all's bug, not the
    stage's. The C++ drivers correctly refuse a scope matching no files;
    recording that as `failed' produces a line that means nothing, and
    a failure list with noise in it is one a reader learns to skim.

    The stages here are synthetic on purpose. Asking the real cppcheck
    stage would test whether cppcheck is installed - it is not, in CI -
    and availability is decided before kind, so the answer would be
    NOTRUN and the routing would never be reached.
    """

    def _ctx(self, scopes):
        return {"dir": Path("/tmp"), "scopes": scopes, "jobs": 1,
                "timeout": 1, "resume": False}

    def _stage(self, kind, feeds=None):
        return S.Stage("x", None, "sh", None, lambda c: ["true"],
                       feeds=feeds, kind=kind)

    def test_a_c_scope_skips_the_cxx_stages(self):
        self.addCleanup(TreeFixture(["sys/geom/uzip/g_uzip.c"]).undo)
        r = S.run_stage(self._stage("cxx"), self._ctx(["sys/geom/uzip"]),
                        {"stages": {}}, dry=True)
        self.assertEqual(r["status"], "skipped")
        self.assertTrue(r["kind_mismatch"])
        self.assertIn("no cxx sources", r["why"])

    def test_a_cxx_scope_skips_the_c_stages(self):
        self.addCleanup(TreeFixture(["kde/f/kcoreaddons/k.cpp"]).undo)
        r = S.run_stage(self._stage("c"), self._ctx(["kde/f/kcoreaddons"]),
                        {"stages": {}}, dry=True)
        self.assertEqual(r["status"], "skipped")
        self.assertTrue(r["kind_mismatch"])
        self.assertIn("no c sources", r["why"])

    def test_a_scope_of_the_right_kind_is_not_skipped(self):
        """The counterpart: the skip has to be the kind's doing, not
        something that fires whatever the scope holds."""
        self.addCleanup(TreeFixture(["sys/geom/uzip/g_uzip.c"]).undo)
        r = S.run_stage(self._stage("c"), self._ctx(["sys/geom/uzip"]),
                        {"stages": {}}, dry=True)
        self.assertEqual(r["status"], "would-run")

    def test_every_real_stage_declares_the_kind_it_reads(self):
        """The routing above is only reached if the real stages carry a
        kind. An unlabelled stage would default to c and be handed every
        C++ scope in the tree."""
        for st in S.STAGES:
            self.assertIn(st.kind, ("c", "cxx", "meta"), st.name)
        self.assertEqual(S.STAGE_BY_NAME["cppcheck"].kind, "c")
        self.assertEqual(S.STAGE_BY_NAME["cxx-analyze"].kind, "cxx")

    def test_a_kind_mismatch_is_not_counted_as_a_coverage_gap(self):
        """There were no functions of that kind to leave untouched."""
        import io
        import contextlib
        state = {"stages": {
            "cbmc": {"status": "ok", "seconds": 1.0},
            "cxx-analyze": {"status": "skipped", "kind_mismatch": True,
                            "why": "no cxx sources"},
        }}
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            S.do_report(Path("/tmp"), state, ["x"], 10, {})
        out = buf.getvalue()
        self.assertIn("0 of 2 instruments produced NO data", out)
        self.assertIn("not a coverage gap", out)

    def test_the_kind_check_runs_before_the_dependency_check(self):
        """Order matters and is asserted. `there is nothing here of that
        kind' is a stronger answer than `its input failed', and getting
        it backwards would report a C++ stage as blocked-on-classify in
        every C-only scope in the tree."""
        self.addCleanup(TreeFixture(["sys/geom/uzip/g_uzip.c"]).undo)
        st = self._stage("cxx", feeds=["classify"])
        r = S.run_stage(st, self._ctx(["sys/geom/uzip"]),
                        {"stages": {"classify": {"status": "failed"}}},
                        dry=True)
        self.assertEqual(r["status"], "skipped")

    def test_the_meta_stage_is_never_kind_skipped(self):
        """universe covers every tree; it has no kind."""
        self.assertEqual(S.STAGE_BY_NAME["universe"].kind, "meta")


class UnitCount(unittest.TestCase):
    """The ledger names NOTHING under kde/ - 0 of 35,050 records. A KDE
    scope counted from the ledger is zero units, every per-unit estimate
    reads 0.00 h, and a three-hour job prints as free."""

    def test_a_scope_the_ledger_names_is_counted_from_it(self):
        self.addCleanup(TreeFixture(
            files=["sys/geom/uzip/g_uzip.c"],
            ledger_paths=["sys/geom/uzip/g_uzip.c",
                          "sys/geom/uzip/g_uzip_lzma.c",
                          "sys/geom/uzip/g_uzip.h",
                          "sys/geom/other/x.c"]).undo)
        self.assertEqual(S.count_units(["sys/geom/uzip"]), (2, "ledger"))

    def test_a_scope_the_ledger_does_not_name_falls_back_to_disk(self):
        self.addCleanup(TreeFixture(
            files=["kde/f/kcoreaddons/a.cpp", "kde/f/kcoreaddons/b.cpp"],
            ledger_paths=["sys/geom/uzip/g_uzip.c"]).undo)
        n, src = S.count_units(["kde/f/kcoreaddons"])
        self.assertEqual((n, src), (2, "disk"))

    def test_the_provenance_is_returned_not_just_the_number(self):
        """A disk count and a ledger count do not mean the same thing,
        so the caller must be able to say which it printed."""
        self.addCleanup(TreeFixture(
            files=["sys/geom/uzip/g_uzip.c", "kde/f/kcoreaddons/a.cpp"],
            ledger_paths=["sys/geom/uzip/g_uzip.c"]).undo)
        for scope in (["sys/geom/uzip"], ["kde/f/kcoreaddons"]):
            self.assertIn(S.count_units(scope)[1], ("ledger", "disk"))

    def test_a_scope_with_nothing_in_it_is_zero_not_a_refusal_to_count(self):
        """Zero and `cannot count' are different answers: one means the
        scope is empty, the other means the ledger is missing. main()
        refuses on both, but for different reasons, and the report has
        to name the right one."""
        self.addCleanup(TreeFixture(
            files=["sys/geom/uzip/g_uzip.c"],
            ledger_paths=["sys/geom/uzip/g_uzip.c"]).undo)
        self.assertEqual(S.count_units(["does/not/exist"]), (0, "nothing"))

    def test_a_scope_with_nothing_in_it_refuses(self):
        self.addCleanup(TreeFixture(
            files=["sys/geom/uzip/g_uzip.c"],
            ledger_paths=["sys/geom/uzip/g_uzip.c"]).undo)
        rc = S.main(["--scope", "does/not/exist", "--out",
                     tempfile.mkdtemp(), "--dry-run"])
        self.assertEqual(rc, 2)

    def test_a_missing_ledger_still_refuses_rather_than_counting_disk(self):
        self.addCleanup(TreeFixture(files=["sys/geom/uzip/g_uzip.c"]).undo)
        self.assertEqual(S.count_units(["sys"]), (-1, "none"))


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

    def test_an_estimate_always_says_whose_machine_it_came_from(self):
        """This used to assert `no rates file means no estimate'. A
        reference profile ships now, so a first run gets a number - and
        the contract that replaces it is stricter, not looser: an
        estimate must never be UNLABELLED. A figure from the wrong
        hardware with no provenance is worse than none, because a reader
        does not think to ask."""
        d = Path(tempfile.mkdtemp())
        r = S.load_rates(d)
        self.assertEqual(r.get("_source"), "reference")
        self.assertIn("_host", r)
        self.assertIn("not your machine", r["_note"].lower())

    def test_a_local_profile_takes_precedence_and_says_so(self):
        d = Path(tempfile.mkdtemp())
        (d / "rates.json").write_text(json.dumps(
            {"units": 9, "jobs": 2, "per_unit": {"cbmc": 1.0}}))
        r = S.load_rates(d)
        self.assertEqual(r["_source"], "this machine")
        self.assertEqual(r["units"], 9)

    def test_with_no_reference_either_there_is_no_estimate(self):
        real = S.REFERENCE_RATES
        try:
            S.REFERENCE_RATES = Path("/nonexistent/rates.json")
            self.assertEqual(S.load_rates(Path(tempfile.mkdtemp())), {})
        finally:
            S.REFERENCE_RATES = real


if __name__ == "__main__":
    unittest.main(verbosity=2)
