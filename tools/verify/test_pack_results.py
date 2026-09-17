#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The ways a results package loses the thing it was supposed to carry.

Two failures matter and they pull in opposite directions. Dropping a
file that is NOT regenerable loses evidence for good, because the
machine that produced it is somebody's desktop and the run took a day.
Keeping everything makes a 92 MB artefact nobody transfers, so the
results stay on that desktop, which loses them just as completely.

And the digest has its own: a summary that lists findings and omits the
instruments that never ran is the same lie in a smaller file - and the
smaller file is the one people actually read.
"""
from __future__ import annotations

import json
import tarfile
import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pack_results as P  # noqa: E402


def _sweep(**files) -> Path:
    d = Path(tempfile.mkdtemp())
    for name, body in files.items():
        (d / name.replace("__", ".")).write_text(body)
    return d


CBMC = "\n".join(json.dumps(r) for r in [
    {"file": "a.c", "function": "f", "status": "PROVED"},
    {"file": "a.c", "function": "g", "status": "FAILED",
     "failures": [{"desc": "line 9 arithmetic overflow on signed + in a + b"}]},
])
ANALYZE = "\n".join(json.dumps(r) for r in [
    {"_meta": True, "analyzer": "clang"},
    {"file": "a.c", "status": "OK",
     "findings": [{"checker": "core.NullDereference", "where": "a.c:4"}]},
    {"file": "b.c", "status": "ERROR", "findings": []},
])
STATE = json.dumps({"stages": {
    "cbmc": {"status": "ok", "seconds": 12.0},
    "esbmc": {"status": "NOTRUN", "why": "esbmc not on PATH"},
    "cxx-analyze": {"status": "skipped", "kind_mismatch": True,
                    "why": "no cxx sources under these scopes"},
}})


class WhatIsDropped(unittest.TestCase):

    def test_the_regenerable_bulk_goes(self):
        d = _sweep(cbmc__jsonl=CBMC, universe__jsonl="x" * 5000,
                   matrix__jsonl="y" * 5000, state__json=STATE)
        out = Path(tempfile.mkstemp(suffix=".tar.gz")[1])
        _, _, dropped = P.pack(d, out)
        self.assertIn("universe.jsonl", dropped)
        self.assertIn("matrix.jsonl", dropped)

    def test_a_resume_file_goes(self):
        d = _sweep(cbmc__jsonl=CBMC, state__json=STATE)
        (d / "classes.json.partial.jsonl").write_text("z")
        out = Path(tempfile.mkstemp(suffix=".tar.gz")[1])
        _, _, dropped = P.pack(d, out)
        self.assertIn("classes.json.partial.jsonl", dropped)

    def test_the_evidence_stays(self):
        """The one that would lose a day of somebody's CPU."""
        d = _sweep(cbmc__jsonl=CBMC, analyze__jsonl=ANALYZE,
                   universe__jsonl="x" * 5000, state__json=STATE)
        out = Path(tempfile.mkstemp(suffix=".tar.gz")[1])
        P.pack(d, out)
        with tarfile.open(out) as tf:
            names = {Path(n).name for n in tf.getnames()}
        self.assertIn("cbmc.jsonl", names)
        self.assertIn("analyze.jsonl", names)
        self.assertIn("state.json", names)

    def test_an_unknown_file_is_kept_not_guessed_away(self):
        """Anything this tool does not recognise is evidence until
        proven otherwise. Dropping by pattern would lose a driver added
        after this file was written."""
        d = _sweep(cbmc__jsonl=CBMC, state__json=STATE,
                   something__new__jsonl="{}")
        out = Path(tempfile.mkstemp(suffix=".tar.gz")[1])
        P.pack(d, out)
        with tarfile.open(out) as tf:
            names = {Path(n).name for n in tf.getnames()}
        self.assertIn("something.new.jsonl", names)

    def test_the_rebuild_recipe_travels_with_it(self):
        d = _sweep(cbmc__jsonl=CBMC, universe__jsonl="x", state__json=STATE)
        out = Path(tempfile.mkstemp(suffix=".tar.gz")[1])
        P.pack(d, out)
        with tarfile.open(out) as tf:
            note = json.loads(
                tf.extractfile("sweep/HOW-TO-REBUILD.json").read())
        self.assertIn("universe.jsonl", note["dropped_because_regenerable"])
        self.assertIn("inventory.py",
                      note["dropped_because_regenerable"]["universe.jsonl"])

    def test_the_source_directory_is_not_disturbed(self):
        d = _sweep(cbmc__jsonl=CBMC, state__json=STATE)
        before = sorted(p.name for p in d.iterdir())
        P.pack(d, Path(tempfile.mkstemp(suffix=".tar.gz")[1]))
        self.assertEqual(sorted(p.name for p in d.iterdir()), before)


class TheDigest(unittest.TestCase):

    def test_the_absences_are_carried(self):
        """The whole reason the digest exists in this shape."""
        d = _sweep(cbmc__jsonl=CBMC, state__json=STATE)
        g = P.digest(d)
        names = {s["stage"] for s in g["did_not_run"]}
        self.assertEqual(names, {"esbmc", "cxx-analyze"})

    def test_a_kind_mismatch_is_marked_as_not_a_gap(self):
        d = _sweep(cbmc__jsonl=CBMC, state__json=STATE)
        g = P.digest(d)
        by = {s["stage"]: s for s in g["did_not_run"]}
        self.assertTrue(by["cxx-analyze"]["kind_mismatch"])
        self.assertFalse(by["esbmc"]["kind_mismatch"])

    def test_it_says_it_is_not_evidence(self):
        d = _sweep(cbmc__jsonl=CBMC, state__json=STATE)
        g = P.digest(d)
        self.assertIn("not evidence", g["_note"])
        self.assertIn("cannot be re-analysed", g["_note"])

    def test_verdicts_and_tu_status_are_kept_apart(self):
        """A per-function PROVED and a per-TU OK are different claims."""
        d = _sweep(cbmc__jsonl=CBMC, analyze__jsonl=ANALYZE,
                   state__json=STATE)
        g = P.digest(d)
        self.assertEqual(g["verdicts"]["PROVED"], 1)
        self.assertEqual(g["verdicts"]["FAILED"], 1)
        self.assertEqual(g["tu_status"]["ERROR"], 1)
        self.assertEqual(g["tu_status"]["OK"], 1)

    def test_the_meta_record_is_not_counted_as_a_result(self):
        d = _sweep(analyze__jsonl=ANALYZE, state__json=STATE)
        g = P.digest(d)
        self.assertEqual(g["totals"]["analyze.jsonl"], 2)

    def test_a_sweep_with_no_state_still_digests(self):
        """A killed run has results and no state.json; losing the
        findings because the bookkeeping is missing would be perverse."""
        d = _sweep(cbmc__jsonl=CBMC)
        g = P.digest(d)
        self.assertEqual(g["verdicts"]["PROVED"], 1)
        self.assertEqual(g["did_not_run"], [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
