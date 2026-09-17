#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The ways a coverage-of-defect-classes number lies.

Every test here is a way the headline figure could be inflated without
anybody noticing. Two of them are mistakes a first draft of a taxonomy
makes by default:

  - dropping the out-of-scope classes out of the denominator, which
    raises the score by naming fewer problems;
  - rounding a SOME up to a COVERED, which is how "we look for it" turns
    into "we would find it".

The third is the one that matters most in practice: an instrument named
in a class row but misspelt relative to INSTRUMENTS silently contributes
nothing, and the row still LOOKS covered to a reader. That is the paper
equivalent of a translation unit that does not compile.
"""
from __future__ import annotations

import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

import taxonomy as T  # noqa: E402


class Vocabulary(unittest.TestCase):

    def test_every_strength_is_a_known_word(self):
        """A typo'd strength ranks 0 and quietly weakens a row."""
        allowed = {T.PROVES, T.FINDS, T.SOME}
        for c in T.CLASSES:
            for tool, s in c["seen"].items():
                self.assertIn(s, allowed, f"{c['id']}/{tool}: {s!r}")

    def test_every_instrument_named_in_a_row_exists(self):
        """A misspelt instrument contributes nothing and looks like it does."""
        for c in T.CLASSES:
            for tool in c["seen"]:
                self.assertIn(tool, T.INSTRUMENTS,
                              f"{c['id']} names unknown instrument {tool!r}")

    def test_class_ids_are_unique(self):
        ids = [c["id"] for c in T.CLASSES]
        self.assertEqual(len(ids), len(set(ids)),
                         "a duplicate id makes one row invisible")

    def test_every_class_says_what_it_is(self):
        for c in T.CLASSES:
            self.assertTrue(c["what"].strip(), c["id"])
            self.assertTrue(c["name"].strip(), c["id"])
            self.assertIn(c["scope"], ("in", "out"), c["id"])


class Verdicts(unittest.TestCase):

    def test_nothing_sees_it_is_a_gap_not_a_pass(self):
        cls = T.C("X", "x", [], "x", {})
        self.assertEqual(T.verdict(cls), T.GAP)

    def test_some_only_is_partial_never_covered(self):
        """The rounding-up bug. SOME means the silence is worth nothing."""
        cls = T.C("X", "x", [], "x", {"cbmc": T.SOME, "cppcheck": T.SOME})
        self.assertEqual(T.verdict(cls), T.PARTIAL)

    def test_one_finds_beats_any_number_of_somes(self):
        cls = T.C("X", "x", [], "x",
                  {"cbmc": T.SOME, "cppcheck": T.SOME,
                   "clang-analyze": T.FINDS})
        self.assertEqual(T.verdict(cls), T.COVERED)

    def test_a_tool_not_installed_does_not_cover_anything(self):
        """The whole point of --available."""
        cls = T.C("X", "x", [], "x", {"codeql": T.PROVES})
        self.assertEqual(T.verdict(cls), T.COVERED)          # on paper
        self.assertEqual(T.verdict(cls, tools=set()), T.GAP)  # in the room
        self.assertEqual(T.verdict(cls, tools={"cbmc"}), T.GAP)


class Denominator(unittest.TestCase):

    def test_out_of_scope_classes_are_not_in_the_headline_denominator(self):
        cov = T.coverage()
        in_scope = sum(1 for c in T.CLASSES if c["scope"] == "in")
        self.assertEqual(cov["classes_in_scope"], in_scope)

    def test_out_of_scope_classes_are_still_counted_and_reported(self):
        """Named, not dropped. Deleting them would RAISE the score."""
        cov = T.coverage()
        out = sum(1 for c in T.CLASSES if c["scope"] == "out")
        self.assertEqual(cov["classes_out_of_scope"], out)
        self.assertGreater(out, 0,
                           "a taxonomy with nothing out of scope is lying")

    def test_the_three_verdicts_partition_the_in_scope_classes(self):
        cov = T.coverage()
        self.assertEqual(cov["covered"] + cov["partial"] + cov["gap"],
                         cov["classes_in_scope"])

    def test_concurrency_and_correctness_are_explicitly_out_of_scope(self):
        """If these ever drift into scope, the headline number is wrong."""
        by_id = {c["id"]: c for c in T.CLASSES}
        for cid in ("CONC-DATA-RACE", "CONC-TOCTOU",
                    "LOGIC-WRONG-RESULT", "SPEC-MISSING"):
            self.assertIn(cid, by_id, f"{cid} must stay named")
            self.assertEqual(by_id[cid]["scope"], "out", cid)


class Monotonicity(unittest.TestCase):

    def test_installing_a_tool_can_never_lower_coverage(self):
        have = T.available_tools()
        base = T.coverage(have)["covered"]
        for name in T.INSTRUMENTS:
            more = T.coverage(have | {name})["covered"]
            self.assertGreaterEqual(more, base, name)

    def test_no_tools_at_all_covers_nothing(self):
        cov = T.coverage(set())
        self.assertEqual(cov["covered"], 0)
        self.assertEqual(cov["partial"], 0)
        self.assertEqual(cov["gap"], cov["classes_in_scope"])
        self.assertEqual(cov["fraction_covered"], 0.0)

    def test_available_is_a_subset_of_everything(self):
        self.assertLessEqual(T.available_tools(), set(T.INSTRUMENTS))

    def test_available_coverage_never_exceeds_paper_coverage(self):
        self.assertLessEqual(T.coverage(T.available_tools())["covered"],
                             T.coverage()["covered"])


class Probes(unittest.TestCase):

    def test_presence_is_probed_and_not_asserted(self):
        """A tool is present because it was found, not because it is listed."""
        saved = dict(T.INSTRUMENTS)
        try:
            T.INSTRUMENTS["ghost"] = dict(
                bin="definitely-not-a-real-binary-xyzzy", pkg=None, apt=None,
                what="a tool that does not exist")
            self.assertNotIn("ghost", T.available_tools())
        finally:
            T.INSTRUMENTS.clear()
            T.INSTRUMENTS.update(saved)

    def test_a_tree_shipped_driver_counts_as_present(self):
        """fusebmc and the lints have no binary; they are files in this tree."""
        self.assertIn("pbsd-lints", T.available_tools())
        self.assertIn("fusebmc", T.available_tools())


class Honesty(unittest.TestCase):

    def test_the_most_important_class_is_not_claimed_as_solved(self):
        """Taint from a trust boundary. If this ever reads COVERED with the
        instruments actually installed, somebody has overstated a row."""
        by_id = {c["id"]: c for c in T.CLASSES}
        cls = by_id["TRUST-UNVALIDATED-INPUT"]
        here = T.verdict(cls, T.available_tools())
        self.assertNotEqual(
            here, T.COVERED,
            "nothing installed here does taint tracking; if this passes to "
            "COVERED, either codeql was installed (good - update this test) "
            "or a row was inflated (bad)")

    def test_no_bounded_model_checker_claims_to_prove_cxx_classes(self):
        """CBMC and ESBMC do not handle the C++ KDE is written in. A PROVES
        on a CXX- row would be the single most misleading cell in the file."""
        for c in T.CLASSES:
            if not c["id"].startswith("CXX-"):
                continue
            for engine in ("cbmc", "esbmc", "fusebmc"):
                self.assertNotEqual(
                    c["seen"].get(engine), T.PROVES,
                    f"{c['id']} claims {engine} PROVES a C++ class")

    def test_runtime_only_instruments_are_marked_as_such(self):
        """Sanitizers reach exactly as far as the test corpus runs, which for
        kernel code is nearly nowhere. The note has to say so."""
        spec = T.INSTRUMENTS["sanitizers"]
        text = spec["what"] + " " + spec.get("note", "")
        self.assertIn("RUNTIME", text)
        self.assertIn("EXECUTE", text)

    def test_class_coverage_is_documented_as_needing_code_coverage(self):
        """The one sentence that keeps this file from being quotable as a
        finished score."""
        self.assertIn("CODE COVERAGE", T.__doc__)
        self.assertIn("MULTIPLY", T.__doc__)


if __name__ == "__main__":
    unittest.main(verbosity=2)
