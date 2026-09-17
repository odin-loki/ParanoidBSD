#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The ways a per-function coverage matrix lies.

The matrix exists to keep three states apart that every findings report
collapses into one: nobody ran, it ran and could not build, and it ran and
said nothing. Every test here is one of those three being allowed to look
like another.
"""
from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

import matrix as M  # noqa: E402


def _universe(rows):
    fh = tempfile.NamedTemporaryFile("w", suffix=".jsonl", delete=False)
    fh.write(json.dumps({"_meta": True}) + "\n")
    for tree, f, fn in rows:
        fh.write(json.dumps({"tree": tree, "file": f, "function": fn,
                             "src": "ledger"}) + "\n")
    fh.close()
    return Path(fh.name)


def _jsonl(records):
    fh = tempfile.NamedTemporaryFile("w", suffix=".jsonl", delete=False)
    for r in records:
        fh.write(json.dumps(r) + "\n")
    fh.close()
    return Path(fh.name)


def _m(rows):
    m = M.Matrix()
    m.load_universe(_universe(rows))
    return m


ROWS = [("hbsd", "a.c", "f"), ("hbsd", "a.c", "g"), ("hbsd", "b.c", "h")]


class Strength(unittest.TestCase):

    def test_ordering(self):
        self.assertEqual(M.strength({"cbmc": "PROVED"}), M.S_PROVED)
        self.assertEqual(M.strength({"cbmc": "FAILED"}), M.S_CHECKED)
        self.assertEqual(M.strength({"cbmc": "BOUNDED"}), M.S_CHECKED)
        self.assertEqual(M.strength({"clang-analyze": "TU-CLEAN"}),
                         M.S_SCANNED)
        self.assertEqual(M.strength({"clang-analyze": "REPORTED"}),
                         M.S_SCANNED)
        self.assertEqual(M.strength({}), M.S_UNTOUCHED)

    def test_a_proof_beats_a_scan_on_the_same_row(self):
        self.assertEqual(
            M.strength({"clang-analyze": "TU-CLEAN", "cbmc": "PROVED"}),
            M.S_PROVED)

    def test_no_answer_is_never_touched(self):
        """The whole premise. TIMEOUT, ERROR, NOFUNC and TU-ERROR say
        something about the instrument, not about the code."""
        for v in ("TIMEOUT", "ERROR", "NOFUNC", "TU-ERROR", "NOTRUN"):
            self.assertEqual(M.strength({"cbmc": v}), M.S_UNTOUCHED, v)

    def test_a_file_that_did_not_build_is_not_clean(self):
        cells = {"clang-analyze": "TU-ERROR"}
        self.assertEqual(M.strength(cells), M.S_UNTOUCHED)
        self.assertEqual(M.why_untouched(cells), "attempted-no-answer")

    def test_never_run_and_run_and_failed_are_different_work(self):
        self.assertEqual(M.why_untouched({}), "never-attempted")
        self.assertEqual(M.why_untouched({"cbmc": "NOTRUN"}),
                         "never-attempted")
        self.assertEqual(M.why_untouched({"cbmc": "TIMEOUT"}),
                         "attempted-no-answer")


class EsbmcVocabulary(unittest.TestCase):
    """esbmc_driver.py returns three verdicts CBMC has no equivalent of."""

    def test_k_induction_is_at_least_as_strong_as_a_bounded_proof(self):
        """PROVED-UNBOUNDED carries no unwind bound at all. Leaving it out
        of PROVEN would file the strongest verdict here as UNTOUCHED."""
        self.assertIn("PROVED-UNBOUNDED", M.PROVEN)
        self.assertEqual(M.strength({"esbmc": "PROVED-UNBOUNDED"}),
                         M.S_PROVED)

    def test_unknown_is_not_an_answer(self):
        """The solver neither proved nor refuted. Not clean."""
        self.assertEqual(M.strength({"esbmc": "UNKNOWN"}), M.S_UNTOUCHED)
        self.assertEqual(M.why_untouched({"esbmc": "UNKNOWN"}),
                         "attempted-no-answer")

    def test_notrun_means_the_engine_was_absent(self):
        """Different work from a timeout: this one needs an install."""
        self.assertEqual(M.why_untouched({"esbmc": "NOTRUN"}),
                         "never-attempted")

    def test_esbmc_verdicts_survive_ingest(self):
        m = _m(ROWS)
        m.ingest("esbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED-UNBOUNDED"},
            {"file": "a.c", "function": "g", "status": "UNKNOWN"},
            {"file": "b.c", "function": "h", "status": "NOTRUN"}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")]["esbmc"],
                         "PROVED-UNBOUNDED")
        self.assertEqual(m.rows[("hbsd", "a.c", "g")]["esbmc"], "UNKNOWN")
        self.assertEqual(m.rows[("hbsd", "b.c", "h")]["esbmc"], "NOTRUN")


class IngestFunctionEngine(unittest.TestCase):

    def test_status_lands_on_the_named_function_only(self):
        m = _m(ROWS)
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED"}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")], {"cbmc": "PROVED"})
        self.assertEqual(m.rows[("hbsd", "a.c", "g")], {})

    def test_a_verdict_for_an_unknown_function_is_counted_not_invented(self):
        """A run that names a function the universe does not hold means the
        UNIVERSE is wrong. Silently adding the row would hide that."""
        m = _m(ROWS)
        c = m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "nope", "status": "PROVED"}]))
        self.assertEqual(c["_unknown"], 1)
        self.assertNotIn(("hbsd", "a.c", "nope"), m.rows)

    def test_an_unrecognised_status_becomes_error_not_a_pass(self):
        m = _m(ROWS)
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "SPLENDID"}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")]["cbmc"], "ERROR")


class IngestTuEngine(unittest.TestCase):

    def test_a_clean_unit_marks_every_function_it_holds(self):
        m = _m(ROWS)
        m.ingest("clang-analyze", "hbsd", _jsonl([
            {"file": "a.c", "status": "OK", "findings": []}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")]["clang-analyze"],
                         "TU-CLEAN")
        self.assertEqual(m.rows[("hbsd", "a.c", "g")]["clang-analyze"],
                         "TU-CLEAN")
        self.assertEqual(m.rows[("hbsd", "b.c", "h")], {},
                         "a different file must not be touched")

    def test_a_finding_names_its_function(self):
        m = _m(ROWS)
        m.ingest("clang-analyze", "hbsd", _jsonl([
            {"file": "a.c", "status": "OK",
             "findings": [{"fn": "g", "checker": "core.NullDereference"}]}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")]["clang-analyze"],
                         "TU-CLEAN")
        self.assertEqual(m.rows[("hbsd", "a.c", "g")]["clang-analyze"],
                         "REPORTED")

    def test_a_unit_that_did_not_build_marks_every_function_tu_error(self):
        """THE test. Zero findings from a file that does not compile must
        not be readable as zero defects."""
        m = _m(ROWS)
        c = m.ingest("clang-analyze", "hbsd", _jsonl([
            {"file": "a.c", "status": "ERROR", "findings": [],
             "detail": "fatal error: no such file"}]))
        self.assertEqual(c["TU-ERROR"], 2)
        for fn in ("f", "g"):
            self.assertEqual(m.rows[("hbsd", "a.c", fn)]["clang-analyze"],
                             "TU-ERROR")
            self.assertEqual(M.strength(m.rows[("hbsd", "a.c", fn)]),
                             M.S_UNTOUCHED)


class Merging(unittest.TestCase):

    def test_ingest_is_idempotent(self):
        m = _m(ROWS)
        j = _jsonl([{"file": "a.c", "function": "f", "status": "PROVED"}])
        m.ingest("cbmc", "hbsd", j)
        before = json.dumps(m.rows[("hbsd", "a.c", "f")], sort_keys=True)
        m.ingest("cbmc", "hbsd", j)
        self.assertEqual(json.dumps(m.rows[("hbsd", "a.c", "f")],
                                    sort_keys=True), before)

    def test_absence_never_overwrites_a_verdict(self):
        """A later run that timed out does not erase an earlier proof from
        the same engine. The absence of data is not data."""
        m = _m(ROWS)
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED"}]))
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "TIMEOUT"}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")]["cbmc"], "PROVED")

    def test_a_newer_verdict_replaces_an_older_one(self):
        m = _m(ROWS)
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED"}]))
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "FAILED"}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")]["cbmc"], "FAILED")

    def test_engines_do_not_overwrite_each_other(self):
        m = _m(ROWS)
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED"}]))
        m.ingest("clang-analyze", "hbsd", _jsonl([
            {"file": "a.c", "status": "OK", "findings": []}]))
        self.assertEqual(m.rows[("hbsd", "a.c", "f")],
                         {"cbmc": "PROVED", "clang-analyze": "TU-CLEAN"})

    def test_a_tree_is_not_confused_with_another(self):
        m = _m(ROWS + [("kde", "a.c", "f")])
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED"}]))
        self.assertEqual(m.rows[("kde", "a.c", "f")], {})


class RoundTrip(unittest.TestCase):

    def test_save_then_load_preserves_every_cell(self):
        m = _m(ROWS)
        m.ingest("cbmc", "hbsd", _jsonl([
            {"file": "a.c", "function": "f", "status": "PROVED"}]))
        m.ingest("clang-analyze", "hbsd", _jsonl([
            {"file": "b.c", "status": "ERROR", "findings": []}]))
        out = Path(tempfile.mkstemp(suffix=".jsonl")[1])
        m.save(out)
        back = M.Matrix()
        back.load(out)
        self.assertEqual(back.rows, m.rows)
        self.assertEqual(set(back.provenance), set(m.provenance))


class Universe(unittest.TestCase):

    def test_an_empty_matrix_is_all_untouched_not_all_clean(self):
        m = _m(ROWS)
        for key in m.rows:
            self.assertEqual(M.strength(m.rows[key]), M.S_UNTOUCHED)
            self.assertEqual(M.why_untouched(m.rows[key]), "never-attempted")

    def test_engine_names_are_closed(self):
        m = _m(ROWS)
        with self.assertRaises(SystemExit):
            m.ingest("not-an-engine", "hbsd", _jsonl([]))

    def test_every_engine_is_one_kind_or_the_other(self):
        self.assertEqual(M.FUNC_ENGINES & M.TU_ENGINES, set())
        self.assertEqual(set(M.ENGINES), M.FUNC_ENGINES | M.TU_ENGINES)


if __name__ == "__main__":
    unittest.main(verbosity=2)
