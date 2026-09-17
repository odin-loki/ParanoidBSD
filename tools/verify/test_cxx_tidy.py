#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tests for the ways clang-tidy's coverage number LIES.

Everything test_cxx_analyze.py asserts about the three states applies
here too and is asserted again rather than assumed, because the two
tools share a schema and not an implementation: cxx_tidy.py detects a
failed build by SEEING A DIAGNOSTIC, not by a non-zero exit, which is a
different mechanism and can break on its own.

Two failure modes are specific to this tool and get their own sections:

  PARTIAL RESULTS   clang-tidy prints whatever it managed before the
                    compile failed and then says "Error while
                    processing". Those findings must be discarded and
                    the TU marked ERROR. A half-checked file reported as
                    checked is the same lie as an unchecked one reported
                    as clean, with a number attached to make it
                    convincing.
  DOUBLE COUNTING   clang-tidy runs the static analyser too. The same
                    use-after-free must not be counted once by each tool
                    and read as two instruments agreeing.

Pure stdlib unittest. No pytest.
"""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import cxx_tidy  # noqa: E402

TIDY = HERE / "cxx_tidy.py"
ANALYZE = HERE / "cxx_analyze.py"

# Defects of the two kinds: one the path-sensitive analyser finds, and
# one only an AST matcher finds. Both must come back, or the tool is not
# adding anything over cxx_analyze.py and should not exist.
DEFECTS = """\
#include <cstring>
#include <string>
#include <utility>

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

struct S {
    char *buf;
    S &operator=(const S &o) {
        delete[] buf;
        buf = new char[8];
        std::memcpy(buf, o.buf, 8);
        return *this;
    }
};

std::string moved() {
    std::string s = "hello";
    std::string t = std::move(s);
    return s.substr(0, 1);
}
"""

CLEAN = """\
#include <memory>
int fine() {
    auto p = std::make_unique<int>(4);
    return *p + 1;
}
"""

# Fails to compile AFTER a diagnosable defect, so clang-tidy has
# something to say before it gives up. That is the partial-result trap.
PARTIAL = """\
int leaky() {
    int *a = new int[4];
    delete a;
    return 0;
}
#include <QObject>
"""

UNBUILDABLE = """\
#include <QObject>
int broken() { int *p = new int; delete p; return *p; }
"""


def run(tool, args):
    return subprocess.run([sys.executable, str(tool), *args],
                          capture_output=True, text=True, timeout=1800)


def records(path):
    with open(path, encoding="utf-8") as fh:
        return [json.loads(ln) for ln in fh if ln.strip()]


def meta(recs):
    for r in recs:
        if r.get("_meta"):
            return r
    return {}


def findings(recs):
    return [f for r in recs if not r.get("_meta") for f in r.get("findings", [])]


class Tree(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.root = Path(tempfile.mkdtemp(prefix="tidytest_"))
        for d in ("good", "bad", "clean", "partial", "empty"):
            (cls.root / d).mkdir()
        (cls.root / "good" / "defects.cpp").write_text(DEFECTS)
        (cls.root / "bad" / "unbuildable.cpp").write_text(UNBUILDABLE)
        (cls.root / "clean" / "fine.cpp").write_text(CLEAN)
        (cls.root / "partial" / "partial.cpp").write_text(PARTIAL)
        cls.out = cls.root / "out"
        cls.out.mkdir()

        def t(scope, name, extra=()):
            return run(TIDY, ["--root", str(cls.root), "--scope", scope,
                              "--out", str(cls.out / name), "--jobs", "2",
                              "--no-shim", *extra])

        cls.r_good = t("good", "good.jsonl")
        cls.r_bad = t("bad", "bad.jsonl")
        cls.r_clean = t("clean", "clean.jsonl")
        cls.r_partial = t("partial", "partial.jsonl")
        cls.r_empty = t("empty", "empty.jsonl")
        # ...and the same tree through the OTHER tool, for the merge test.
        cls.r_an = run(ANALYZE, ["--root", str(cls.root), "--scope", "good",
                                 "--out", str(cls.out / "an_good.jsonl"),
                                 "--jobs", "2", "--no-shim"])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.root, ignore_errors=True)


# ------------------------------------------------- 1. the instrument works

class TestInstrumentActuallyFindsThings(Tree):

    def test_planted_use_after_free_is_found(self):
        got = {f["checker"] for f in findings(records(self.out / "good.jsonl"))}
        self.assertIn("clang-analyzer-cplusplus.NewDelete", got,
                      f"planted use-after-free NOT found. got {sorted(got)}")

    def test_planted_new_delete_mismatch_is_found(self):
        got = {f["checker"] for f in findings(records(self.out / "good.jsonl"))}
        self.assertIn("clang-analyzer-unix.MismatchedDeallocator", got,
                      f"planted new[]/delete mismatch NOT found. got {sorted(got)}")

    def test_an_ast_only_defect_is_found(self):
        """The reason this tool exists alongside cxx_analyze.py.

        MEASURED: `clang --analyze` reports nothing on this
        self-assignment bug, with default checkers AND with
        -analyzer-checker=cplusplus.SelfAssignment explicitly enabled.
        If bugprone-unhandled-self-assignment stops firing, cxx_tidy.py
        is no longer buying anything and the suite must say so.
        """
        got = {f["checker"] for f in findings(records(self.out / "good.jsonl"))}
        self.assertIn("bugprone-unhandled-self-assignment", got,
                      f"got {sorted(got)}")

    def test_the_curated_list_is_not_secretly_empty(self):
        """A --checks string that disables everything reports zero and
        looks exactly like a clean tree. Assert it resolves to real
        checks by asking clang-tidy itself."""
        p = subprocess.run(
            ["clang-tidy", f"--checks={cxx_tidy.CHECKS_ARG}", "--list-checks"],
            capture_output=True, text=True, timeout=120)
        enabled = [ln.strip() for ln in p.stdout.splitlines()
                   if ln.startswith("    ")]
        self.assertGreater(len(enabled), 50,
                           f"only {len(enabled)} checks resolve from the "
                           f"curated list")

    def test_the_excluded_families_really_are_excluded(self):
        """A policy call that does not take effect is not a policy call."""
        p = subprocess.run(
            ["clang-tidy", f"--checks={cxx_tidy.CHECKS_ARG}", "--list-checks"],
            capture_output=True, text=True, timeout=120)
        enabled = {ln.strip() for ln in p.stdout.splitlines()
                   if ln.startswith("    ")}
        for banned in ("readability-", "modernize-", "llvm-", "google-",
                       "cppcoreguidelines-", "hicpp-"):
            leaked = sorted(c for c in enabled if c.startswith(banned))
            self.assertEqual(leaked, [], f"{banned}* leaked in: {leaked[:5]}")
        for named in cxx_tidy.EXCLUDE:
            self.assertNotIn(named, enabled)
        self.assertNotIn("concurrency-mt-unsafe", enabled)

    def test_analyzer_check_list_is_imported_not_restated(self):
        """The two tools must not drift apart about what is checked."""
        import cxx_analyze
        self.assertEqual(
            cxx_tidy.ANALYZER,
            ["clang-analyzer-" + c for c in cxx_analyze.CHECKERS])


# --------------------------------------- 2. ERROR does not read as clean

class TestErrorIsNotClean(Tree):

    def test_unbuildable_tu_is_ERROR(self):
        recs = [r for r in records(self.out / "bad.jsonl")
                if not r.get("_meta")]
        self.assertEqual([r["status"] for r in recs], ["ERROR"])
        self.assertEqual(recs[0]["findings"], [])

    def test_zero_built_exits_nonzero(self):
        self.assertEqual(self.r_bad.returncode, 2, self.r_bad.stdout[-800:])
        self.assertIn("0 translation units BUILT", self.r_bad.stdout)

    def test_a_clean_scope_exits_zero(self):
        self.assertEqual(self.r_clean.returncode, 0,
                         self.r_clean.stdout[-800:])

    def test_compile_diagnostics_are_never_counted_as_findings(self):
        """clang-diagnostic-error is a build failure wearing a
        diagnostic's clothes. Counting it inflates the finding total on
        exactly the files that were not checked."""
        for f in findings(records(self.out / "bad.jsonl")):
            self.assertNotIn("clang-diagnostic", f["checker"])

    def test_partial_results_are_discarded_and_the_TU_is_ERROR(self):
        """partial.cpp has a real new[]/delete mismatch on line 3 and
        then fails to include <QObject>. clang-tidy will report the
        mismatch and then say it could not process the file.

        The mismatch must NOT be reported: half a check presented as a
        whole one is the lie with a number attached."""
        recs = [r for r in records(self.out / "partial.jsonl")
                if not r.get("_meta")]
        self.assertEqual(len(recs), 1)
        self.assertEqual(recs[0]["status"], "ERROR",
                         f"a TU that could not be processed came back "
                         f"{recs[0]['status']} with "
                         f"{len(recs[0]['findings'])} finding(s)")
        self.assertEqual(recs[0]["findings"], [])


# ------------------------- 3. not-run is distinguishable from found-nothing

class TestNotRunIsNotClean(Tree):

    def test_clean_scope_has_an_OK_record_with_no_findings(self):
        recs = [r for r in records(self.out / "clean.jsonl")
                if not r.get("_meta")]
        self.assertEqual(len(recs), 1)
        self.assertEqual(recs[0]["status"], "OK")
        self.assertEqual(recs[0]["findings"], [])

    def test_never_run_scope_has_no_records(self):
        never = [r for r in records(self.out / "empty.jsonl")
                 if not r.get("_meta")]
        clean = [r for r in records(self.out / "clean.jsonl")
                 if not r.get("_meta")]
        self.assertEqual(len(never), 0)
        self.assertEqual(len(clean), 1)

    def test_empty_scope_exits_nonzero_and_says_NOT_RUN(self):
        self.assertEqual(self.r_empty.returncode, 2,
                         self.r_empty.stdout[-800:])
        self.assertIn("MATCHED NO FILES", self.r_empty.stdout)
        self.assertIn("NOT RUN", self.r_empty.stdout)

    def test_meta_distinguishes_them_in_the_jsonl_alone(self):
        self.assertEqual(meta(records(self.out / "empty.jsonl"))["scope_units"],
                         {"empty": 0})
        self.assertEqual(meta(records(self.out / "clean.jsonl"))["scope_units"],
                         {"clean": 1})


# --------------------------------------------- 4. guessed flags are marked

class TestGuessedFlagsAreMarked(Tree):

    def test_records_and_findings_both_carry_the_mark(self):
        recs = [r for r in records(self.out / "good.jsonl")
                if not r.get("_meta")]
        self.assertTrue(all(r["flagsrc"] == "guess" for r in recs))
        f = findings(records(self.out / "good.jsonl"))
        self.assertTrue(f)
        self.assertTrue(all(x.get("guessed") for x in f))

    def test_guessed_findings_can_be_filtered_out(self):
        f = findings(records(self.out / "good.jsonl"))
        self.assertEqual([x for x in f if not x.get("guessed")], [])

    def test_compile_commands_branch_is_marked_differently(self):
        db = self.root / "cc.json"
        src = self.root / "good" / "defects.cpp"
        db.write_text(json.dumps([{
            "directory": str(self.root), "file": str(src),
            "arguments": ["clang++", "-std=c++17", "-c", str(src),
                          "-o", "defects.o"]}]))
        out = self.out / "tidy_db.jsonl"
        p = run(TIDY, ["--root", str(self.root), "--scope", "good",
                       "--out", str(out), "--jobs", "1", "--no-shim",
                       "--compile-commands", str(db)])
        self.assertEqual(p.returncode, 0, p.stdout[-800:])
        recs = [r for r in records(out) if not r.get("_meta")]
        self.assertEqual([r["flagsrc"] for r in recs], ["compile_commands"])
        f = findings(records(out))
        self.assertTrue(f)
        self.assertTrue(all("guessed" not in x for x in f))


# ------------------------------------------------ 5. the merge with the
#                                                    other instrument

class TestDeduplicationAcrossInstruments(Tree):

    def test_the_same_defect_from_both_tools_has_one_dedup_key(self):
        """Otherwise one instrument counted twice reads as two agreeing."""
        an = findings(records(self.out / "an_good.jsonl"))
        ti = findings(records(self.out / "good.jsonl"))
        self.assertTrue(an and ti)
        shared = {f["dedup"] for f in an} & {f["dedup"] for f in ti}
        self.assertIn("good/defects.cpp:8:12:cplusplus.NewDelete", shared,
                      f"the use-after-free did not merge. analyze: "
                      f"{sorted({f['dedup'] for f in an})} tidy: "
                      f"{sorted({f['dedup'] for f in ti})}")

    def test_the_raw_keys_still_differ_so_provenance_survives(self):
        """Merging must not destroy which tool said it."""
        an = {f["key"] for f in findings(records(self.out / "an_good.jsonl"))}
        ti = {f["key"] for f in findings(records(self.out / "good.jsonl"))}
        self.assertEqual(an & ti, set(),
                         "key must be tool-distinguishable; dedup is the "
                         "one that merges")

    def test_a_tidy_only_finding_survives_the_merge(self):
        an = {f["dedup"] for f in findings(records(self.out / "an_good.jsonl"))}
        ti = {f["dedup"] for f in findings(records(self.out / "good.jsonl"))}
        only = {d for d in ti - an if "self-assignment" in d}
        self.assertTrue(only, "the AST-only finding vanished in the merge, "
                              "which is the second instrument being thrown "
                              "away")

    def test_dedup_is_reconstructible_from_the_other_fields(self):
        """No hidden state in the key: a later merge pass must be able to
        rebuild it from file, line, column and check-name alone."""
        import cxx_analyze
        for f in findings(records(self.out / "good.jsonl")):
            path, _, line = f["where"].rpartition(":")
            self.assertEqual(
                f["key"], f"{path}:{line}:{f['col']}:{f['checker']}")
            self.assertEqual(
                f["dedup"],
                f"{path}:{line}:{f['col']}:"
                f"{cxx_analyze.dedup_checker(f['checker'])}")


# ------------------------------------- 6. the schema the other tools read

class TestSchemaMatchesTheCTier(Tree):

    def test_record_has_analyze_py_s_keys(self):
        for r in records(self.out / "good.jsonl"):
            if r.get("_meta"):
                continue
            for k in ("file", "status", "findings", "flags"):
                self.assertIn(k, r)

    def test_where_is_path_colon_line_with_no_column(self):
        for f in findings(records(self.out / "good.jsonl")):
            self.assertRegex(f["where"], r"^[^\s:]+:\d+$")

    def test_status_vocabulary(self):
        for p in ("good.jsonl", "bad.jsonl", "clean.jsonl", "partial.jsonl"):
            for r in records(self.out / p):
                if r.get("_meta"):
                    continue
                self.assertIn(r["status"], {"OK", "ERROR", "TIMEOUT"})

    def test_meta_has_no_findings_key(self):
        m = meta(records(self.out / "good.jsonl"))
        self.assertTrue(m)
        self.assertNotIn("findings", m)
        self.assertEqual(m["tool"], "cxx_tidy.py")

    def test_meta_records_the_excluded_families_on_the_run(self):
        """So a reader of a year-old .jsonl can tell what was NOT asked,
        without going to find the version of the script that made it."""
        m = meta(records(self.out / "good.jsonl"))
        self.assertIn("readability-*", m["excluded_families"])
        self.assertIn("concurrency-mt-unsafe", m["excluded_families"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
