#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tests for the ways a C++ coverage number LIES.

Not "does cxx_analyze.py exit 0". A tool that cannot build anything
exits 0 and reports nothing, and that is the failure being tested for,
not the success. Every test here is an assertion about one of the three
states staying distinct:

    NOT RUN      no record exists
    ERROR        a record exists, status ERROR, counted
    CLEAN        a record exists, status OK, findings == []

and about the instrument demonstrably WORKING - a planted use-after-free
and a planted new[]/delete mismatch have to come back, or the whole
corpus of zeroes this tool produces means nothing. analyze.py's
docstring records a version of itself that ran NOTHING and reported
zero across sixty translation units; this file is what stops that
happening again in C++.

Pure stdlib unittest. No pytest.
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import cxx_analyze  # noqa: E402

TOOL = HERE / "cxx_analyze.py"

# A translation unit with a defect of each shape this tier claims to
# find. If the tool comes back quiet on this file, every zero it has
# ever printed is worthless and the suite says so.
DEFECTS = """\
#include <string>
#include <utility>
#include <new>

int use_after_free() {
    int *p = new int(7);
    delete p;
    return *p;
}

int mismatched_delete() {
    int *a = new int[4];
    delete a;
    return 0;
}

void leaked() {
    int *q = new int(3);
    (void)q;
}

int null_deref(int *p) {
    if (!p) { }
    return *p;
}

int div_zero(int n) {
    int d = 0;
    return n / d;
}

std::string moved() {
    std::string s = "hello";
    std::string t = std::move(s);
    return s.substr(0, 1);
}

std::string string_from_null() {
    const char *p = nullptr;
    return std::string(p);
}
"""

# Nothing wrong with it. Must come back OK with an EMPTY findings list -
# which is a different thing from not being looked at.
CLEAN = """\
#include <memory>
int fine() {
    auto p = std::make_unique<int>(4);
    return *p + 1;
}
"""

# Will not build: the header does not exist anywhere. This is the
# 1,386-of-1,392 case in miniature.
UNBUILDABLE = """\
#include <QObject>
int also_broken() { int *p = new int; delete p; return *p; }
"""


def run(args, cwd=None):
    p = subprocess.run([sys.executable, str(TOOL), *args],
                       capture_output=True, text=True, timeout=900, cwd=cwd)
    return p


def records(path):
    out = []
    with open(path, encoding="utf-8") as fh:
        for ln in fh:
            if ln.strip():
                out.append(json.loads(ln))
    return out


def meta(recs):
    for r in recs:
        if r.get("_meta"):
            return r
    return {}


def findings(recs):
    return [f for r in recs if not r.get("_meta") for f in r.get("findings", [])]


class Tree(unittest.TestCase):
    """A throwaway tree with one of each kind of translation unit."""

    @classmethod
    def setUpClass(cls):
        cls.root = Path(tempfile.mkdtemp(prefix="cxxtest_"))
        (cls.root / "good").mkdir()
        (cls.root / "bad").mkdir()
        (cls.root / "clean").mkdir()
        (cls.root / "empty").mkdir()
        (cls.root / "good" / "defects.cpp").write_text(DEFECTS)
        (cls.root / "bad" / "unbuildable.cpp").write_text(UNBUILDABLE)
        (cls.root / "clean" / "fine.cpp").write_text(CLEAN)
        cls.out = cls.root / "out"
        cls.out.mkdir()

        cls.r_good = run(["--root", str(cls.root), "--scope", "good",
                          "--out", str(cls.out / "good.jsonl"),
                          "--jobs", "2", "--no-shim"])
        cls.r_bad = run(["--root", str(cls.root), "--scope", "bad",
                         "--out", str(cls.out / "bad.jsonl"),
                         "--jobs", "1", "--no-shim"])
        cls.r_clean = run(["--root", str(cls.root), "--scope", "clean",
                           "--out", str(cls.out / "clean.jsonl"),
                           "--jobs", "1", "--no-shim"])
        cls.r_empty = run(["--root", str(cls.root), "--scope", "empty",
                           "--out", str(cls.out / "empty.jsonl"),
                           "--jobs", "1", "--no-shim"])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.root, ignore_errors=True)


# ------------------------------------------------- 1. the instrument works

class TestInstrumentActuallyFindsThings(Tree):
    """A tool that finds nothing is indistinguishable from a broken one.

    analyze.py's own docstring: the first version passed
    -analyzer-disable-all-checks and then listed the wanted checkers,
    which runs NOTHING and reports zero. It would have printed "0
    findings across 60 translation units" and that number would have
    meant nothing.
    """

    def test_planted_use_after_free_is_found(self):
        f = findings(records(self.out / "good.jsonl"))
        self.assertTrue(
            any(x["checker"] == "cplusplus.NewDelete" for x in f),
            "the planted use-after-free was NOT found; every zero this "
            f"tool prints is now suspect. got: {sorted({x['checker'] for x in f})}")

    def test_planted_new_delete_mismatch_is_found(self):
        f = findings(records(self.out / "good.jsonl"))
        self.assertTrue(
            any(x["checker"] == "unix.MismatchedDeallocator" for x in f),
            "the planted new[]/delete mismatch was NOT found")

    def test_the_cplusplus_package_is_live(self):
        """Not just core.*: the C++-specific checkers must be firing.

        The C checker list alone would pass the two tests above (unix.
        MismatchedDeallocator is a C checker) and leave the entire
        reason this file exists unexercised.
        """
        got = {x["checker"] for x in findings(records(self.out / "good.jsonl"))}
        for want in ("cplusplus.NewDelete", "cplusplus.NewDeleteLeaks",
                     "cplusplus.Move", "cplusplus.StringChecker"):
            self.assertIn(want, got,
                          f"{want} did not fire on a file with its defect "
                          f"planted. got: {sorted(got)}")

    def test_every_checker_is_accounted_for_as_evidence(self):
        """No checker may be in the list without a claim about it.

        'It is in CHECKERS' and 'it fires' are different claims and this
        project has been burned by conflating them.
        """
        ev = cxx_analyze.CHECKER_EVIDENCE
        accounted = set(ev["fired_on_probe"]) | set(ev["listed_not_probed"])
        self.assertEqual(set(cxx_analyze.CHECKERS) - accounted, set())

    def test_unavailable_checkers_are_recorded_not_forgotten(self):
        """A checker that was wanted and is silent must stay named.

        cplusplus.SelfAssignment is registered in clang 18 and emits
        nothing. Dropping it from the list silently would turn a known
        hole into an invisible one.
        """
        self.assertIn("cplusplus.SelfAssignment",
                      cxx_analyze.CHECKER_EVIDENCE["unavailable"])
        self.assertNotIn("cplusplus.SelfAssignment", cxx_analyze.CHECKERS)


# --------------------------------------- 2. ERROR does not read as clean

class TestErrorIsNotClean(Tree):

    def test_unbuildable_tu_is_ERROR_not_OK(self):
        recs = [r for r in records(self.out / "bad.jsonl")
                if not r.get("_meta")]
        self.assertEqual(len(recs), 1)
        self.assertEqual(recs[0]["status"], "ERROR")
        self.assertNotEqual(recs[0]["status"], "OK")

    def test_unbuildable_tu_is_counted_and_has_a_reason(self):
        recs = [r for r in records(self.out / "bad.jsonl")
                if not r.get("_meta")]
        self.assertEqual(recs[0]["findings"], [])
        self.assertIn("QObject", recs[0].get("detail", ""),
                      "an ERROR without a reason cannot be bucketed, which "
                      "is how 5,687 of them sat unexamined in the C tier")

    def test_zero_built_exits_nonzero(self):
        """The load-bearing one.

        A run that built nothing prints '0 findings'. That sentence is
        true and it means the opposite of what it reads like. The shell
        must be able to tell without parsing prose.
        """
        self.assertEqual(self.r_bad.returncode, 2, self.r_bad.stdout[-800:])
        self.assertIn("0 translation units BUILT", self.r_bad.stdout)

    def test_a_clean_scope_exits_zero(self):
        """...and the same run over code that BUILDS and is clean does not.

        Without this assertion the previous test is satisfied by a tool
        that always exits 2.
        """
        self.assertEqual(self.r_clean.returncode, 0, self.r_clean.stdout[-800:])

    def test_error_summary_buckets_the_reason(self):
        self.assertIn("missing header: QObject", self.r_bad.stdout)

    def test_findings_are_discarded_from_an_ERROR_tu(self):
        """unbuildable.cpp contains a real use-after-free on line 2.

        It must NOT be reported, because the TU did not build and a
        partial result presented as a whole one is the lie.
        """
        self.assertEqual(findings(records(self.out / "bad.jsonl")), [])


# ------------------------- 3. not-run is distinguishable from found-nothing

class TestNotRunIsNotClean(Tree):

    def test_clean_scope_has_records_with_empty_findings(self):
        recs = [r for r in records(self.out / "clean.jsonl")
                if not r.get("_meta")]
        self.assertEqual(len(recs), 1)
        self.assertEqual(recs[0]["status"], "OK")
        self.assertEqual(recs[0]["findings"], [])

    def test_never_run_scope_has_no_records_at_all(self):
        """The distinguishing fact: a scope nobody ran produces NO
        RECORD, where a clean scope produces an OK record with an empty
        list. Both print 'no findings'. Only one of them means it."""
        clean = [r for r in records(self.out / "clean.jsonl")
                 if not r.get("_meta")]
        never = [r for r in records(self.out / "empty.jsonl")
                 if not r.get("_meta")]
        self.assertEqual(len(never), 0)
        self.assertEqual(len(clean), 1)
        self.assertNotEqual(len(never), len(clean),
                            "not-run and found-nothing have collapsed into "
                            "the same observation")

    def test_a_scope_matching_no_files_exits_nonzero_and_says_so(self):
        self.assertEqual(self.r_empty.returncode, 2, self.r_empty.stdout[-800:])
        self.assertIn("MATCHED NO FILES", self.r_empty.stdout)
        self.assertIn("NOT RUN", self.r_empty.stdout)

    def test_meta_names_every_scope_and_its_unit_count(self):
        """So a reader of the JSONL alone can tell the two apart.

        Without this, a consumer handed only empty.jsonl sees a file
        with no findings and no way to know whether that is because
        nothing was wrong or because nothing was looked at.
        """
        m = meta(records(self.out / "empty.jsonl"))
        self.assertEqual(m["scope_units"], {"empty": 0})
        self.assertEqual(m["scopes_matching_nothing"], ["empty"])
        m2 = meta(records(self.out / "clean.jsonl"))
        self.assertEqual(m2["scope_units"], {"clean": 1})
        self.assertEqual(m2["scopes_matching_nothing"], [])


# --------------------------------------------- 4. guessed flags are marked

class TestGuessedFlagsAreMarked(Tree):

    def test_records_carry_flagsrc(self):
        recs = [r for r in records(self.out / "good.jsonl")
                if not r.get("_meta")]
        self.assertTrue(all(r["flagsrc"] == "guess" for r in recs))

    def test_findings_carry_guessed_so_they_survive_being_copied(self):
        f = findings(records(self.out / "good.jsonl"))
        self.assertTrue(f)
        self.assertTrue(all(x.get("guessed") for x in f),
                        "a finding copied out of its record into a triage "
                        "list must keep knowing its flags were invented")

    def test_guessed_findings_can_be_filtered_out(self):
        f = findings(records(self.out / "good.jsonl"))
        hard = [x for x in f if not x.get("guessed")]
        self.assertEqual(hard, [],
                         "with no compile_commands.json, NOTHING may "
                         "present itself as hard evidence")

    def test_compile_commands_are_used_and_recorded_as_such(self):
        """The other branch: when the project DID configure, the record
        must say so, or a strong finding is discounted as a weak one."""
        db = self.root / "cc.json"
        src = self.root / "good" / "defects.cpp"
        db.write_text(json.dumps([{
            "directory": str(self.root),
            "file": str(src),
            "arguments": ["clang++", "-std=c++17", "-c", str(src),
                          "-o", "defects.o"],
        }]))
        out = self.out / "withdb.jsonl"
        p = run(["--root", str(self.root), "--scope", "good",
                 "--out", str(out), "--jobs", "1", "--no-shim",
                 "--compile-commands", str(db)])
        self.assertEqual(p.returncode, 0, p.stdout[-800:])
        recs = [r for r in records(out) if not r.get("_meta")]
        self.assertEqual([r["flagsrc"] for r in recs], ["compile_commands"])
        f = findings(records(out))
        self.assertTrue(f, "the database run found nothing at all")
        self.assertTrue(all("guessed" not in x for x in f))

    def test_meta_counts_the_guessed_units(self):
        m = meta(records(self.out / "good.jsonl"))
        self.assertEqual(m["guessed_units"], m["units"])


# ------------------------------------- 5. the schema the other tools read

class TestSchemaMatchesTheCTier(Tree):
    """confidence.py and report.py must consume this unchanged.

    If a field name differs from tools/verify/analyze.py's, this tool is
    wrong and analyze.py is right.
    """

    def test_record_has_analyze_py_s_keys(self):
        recs = [r for r in records(self.out / "good.jsonl")
                if not r.get("_meta")]
        for r in recs:
            for k in ("file", "status", "findings", "flags"):
                self.assertIn(k, r)

    def test_where_is_path_colon_line_with_no_column(self):
        """report.py's agree() uses re.fullmatch(r"([^\\s:]+):(\\d+)").

        A column inside `where` makes every C++ finding invisible to the
        one section whose job is spotting two instruments on one line.
        """
        import re
        for x in findings(records(self.out / "good.jsonl")):
            self.assertRegex(x["where"], r"^[^\s:]+:\d+$")

    def test_meta_record_has_no_findings_key(self):
        """Every reader that iterates findings skips _meta by that test."""
        m = meta(records(self.out / "good.jsonl"))
        self.assertTrue(m)
        self.assertNotIn("findings", m)

    def test_status_vocabulary_is_analyze_py_s(self):
        for p in ("good.jsonl", "bad.jsonl", "clean.jsonl"):
            for r in records(self.out / p):
                if r.get("_meta"):
                    continue
                self.assertIn(r["status"], {"OK", "ERROR", "TIMEOUT"})

    def test_confidence_py_counts_an_ERROR_against_visibility(self):
        """The arithmetic confidence.py does, reproduced on these records.

        Not a mock: the same expression, so that if confidence.py's
        notion of 'analysed_err' ever stops matching what this writes,
        a test fails rather than a number quietly improving.
        """
        recs = [r for r in records(self.out / "bad.jsonl")
                if not r.get("_meta")]
        a_ok = sum(1 for r in recs if r.get("status") == "OK")
        a_err = sum(1 for r in recs if r.get("status") not in ("OK", None))
        self.assertEqual((a_ok, a_err), (0, 1))

    def test_dedup_key_is_stable_and_carries_file_line_col_check(self):
        for x in findings(records(self.out / "good.jsonl")):
            self.assertEqual(
                x["key"],
                f"{x['where']}:{x['col']}:{x['checker']}")
            self.assertTrue(x["dedup"].endswith(x["checker"]))

    def test_dedup_strips_the_clang_tidy_prefix(self):
        self.assertEqual(
            cxx_analyze.dedup_checker("clang-analyzer-cplusplus.NewDelete"),
            "cplusplus.NewDelete")
        self.assertEqual(
            cxx_analyze.dedup_checker("cplusplus.NewDelete"),
            "cplusplus.NewDelete")


# ------------------------------------------------ 6. the error inventory

class TestErrorInventory(Tree):

    def test_check_errors_fails_on_an_uninventoried_error(self):
        inv = self.root / "inv_empty.json"
        inv.write_text(json.dumps({"files": {}, "prefixes": {}}))
        p = run(["--root", str(self.root), "--scope", "bad",
                 "--out", str(self.out / "inv1.jsonl"), "--jobs", "1",
                 "--no-shim", "--check-errors", "--expected", str(inv)])
        self.assertIn("is not in the inventory", p.stdout)
        # exit 2 wins over exit 1 here: nothing built at all, which is
        # the louder fact.
        self.assertNotEqual(p.returncode, 0)

    def test_check_errors_accepts_an_inventoried_error(self):
        inv = self.root / "inv_full.json"
        inv.write_text(json.dumps({
            "files": {"bad/unbuildable.cpp": "needs Qt6; no Qt in tree"},
            "prefixes": {}}))
        p = run(["--root", str(self.root), "--scope", "bad",
                 "--out", str(self.out / "inv2.jsonl"), "--jobs", "1",
                 "--no-shim", "--check-errors", "--expected", str(inv)])
        self.assertIn("on the record", p.stdout)
        self.assertNotIn("is not in the inventory", p.stdout)

    def test_check_errors_flags_a_stale_exemption(self):
        """A file listed as unbuildable that builds now is a lie in the
        other direction, and it hides the fact that coverage improved."""
        inv = self.root / "inv_stale.json"
        inv.write_text(json.dumps({
            "files": {"clean/fine.cpp": "was broken once"}, "prefixes": {}}))
        p = run(["--root", str(self.root), "--scope", "clean",
                 "--out", str(self.out / "inv3.jsonl"), "--jobs", "1",
                 "--no-shim", "--check-errors", "--expected", str(inv)])
        self.assertEqual(p.returncode, 1, p.stdout[-800:])
        self.assertIn("is stale", p.stdout)

    def test_prefix_entries_absorb_a_subtree(self):
        inv = self.root / "inv_pre.json"
        inv.write_text(json.dumps({
            "files": {},
            "prefixes": {"bad/": "the whole KDE-shaped case"}}))
        p = run(["--root", str(self.root), "--scope", "bad",
                 "--out", str(self.out / "inv4.jsonl"), "--jobs", "1",
                 "--no-shim", "--check-errors", "--expected", str(inv)])
        self.assertNotIn("is not in the inventory", p.stdout)


# ------------------------------------------------------- 7. the real tree

class TestAgainstTheRealTreeIfPresent(unittest.TestCase):
    """Skipped when the ParanoidBSD tree is not mounted."""

    ROOT = Path(os.environ.get("PBSD_ROOT", str(Path(__file__).resolve().parents[2])))

    def setUp(self):
        if not (self.ROOT / "kde").is_dir():
            self.skipTest(f"{self.ROOT}/kde not present")

    def test_kde_reports_ERROR_and_never_clean(self):
        """The headline claim of the survey, asserted rather than told.

        Without Qt6 the KDE tier is 100% ERROR, and a run over it must
        exit non-zero so that no pipeline reads 'no findings' off it.
        """
        with tempfile.TemporaryDirectory() as d:
            out = Path(d) / "kde.jsonl"
            p = run(["--root", str(self.ROOT),
                     "--scope", "kde/frameworks/kwindowsystem",
                     "--out", str(out), "--jobs", "4", "--limit", "8"])
            recs = [r for r in records(out) if not r.get("_meta")]
            self.assertTrue(recs, "no records: the scope matched nothing")
            oks = [r for r in recs if r["status"] == "OK"]
            if not oks:
                self.assertEqual(p.returncode, 2, p.stdout[-600:])
                self.assertIn("0 translation units BUILT", p.stdout)
            for r in recs:
                if r["status"] == "ERROR":
                    self.assertEqual(r["findings"], [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
