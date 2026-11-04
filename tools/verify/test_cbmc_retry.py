#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""cbmc_driver.py --retry-status: drop those rows on resume, keep the rest.

Mirrors test_esbmc_driver.py's retry-status case. CBMC has no mock binary
in this tree, so the resume rewrite is tested directly: the records with
the named statuses leave the jsonl and `done`, and PROVED/FAILED (and a
BOUNDED not in the list) stay. RESULT_VERSION must not move for this.
"""
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import cbmc_driver as D  # noqa: E402

DRIVER = HERE / "cbmc_driver.py"


def rec(fn: str, status: str, v=None, **kw) -> dict:
    r = {
        "v": D.RESULT_VERSION if v is None else v,
        "file": "lib/x/a.c",
        "function": fn,
        "status": status,
        "tier": "ub",
        "unwind": 16,
    }
    r.update(kw)
    return r


def line(fn: str, status: str, **kw) -> str:
    return json.dumps(rec(fn, status, **kw))


class ParseRetryStatus(unittest.TestCase):
    def test_comma_separated(self):
        self.assertEqual(
            D.parse_retry_status(["TIMEOUT,BOUNDED,ERROR"]),
            {"TIMEOUT", "BOUNDED", "ERROR"})

    def test_repeatable(self):
        self.assertEqual(
            D.parse_retry_status(["TIMEOUT", "BOUNDED", "ERROR"]),
            {"TIMEOUT", "BOUNDED", "ERROR"})

    def test_repeatable_and_comma_mix(self):
        self.assertEqual(
            D.parse_retry_status(["TIMEOUT,BOUNDED", "ERROR"]),
            {"TIMEOUT", "BOUNDED", "ERROR"})


class ResumeFilter(unittest.TestCase):
    def test_retry_drops_timeout_bounded_error_and_keeps_proved_failed(self):
        lines = [
            line("proved", "PROVED"),
            line("failed", "FAILED"),
            line("bounded", "BOUNDED"),
            line("timeout", "TIMEOUT"),
            line("error", "ERROR"),
            line("nofunc", "NOFUNC"),
        ]
        keep, done, stale, dropped = D.resume_filter(
            lines, {"TIMEOUT", "BOUNDED", "ERROR"})
        self.assertEqual(stale, 0)
        self.assertEqual(dropped, 3)
        kept_fn = {json.loads(l)["function"] for l in keep}
        self.assertEqual(kept_fn, {"proved", "failed", "nofunc"})
        self.assertEqual(done, {
            ("lib/x/a.c", "proved"),
            ("lib/x/a.c", "failed"),
            ("lib/x/a.c", "nofunc"),
        })

    def test_bounded_stays_when_not_in_retry_list(self):
        keep, done, stale, dropped = D.resume_filter(
            [line("bounded", "BOUNDED"), line("timeout", "TIMEOUT")],
            {"TIMEOUT", "ERROR"})
        self.assertEqual(dropped, 1)
        self.assertEqual(stale, 0)
        self.assertEqual([json.loads(l)["function"] for l in keep], ["bounded"])
        self.assertIn(("lib/x/a.c", "bounded"), done)
        self.assertNotIn(("lib/x/a.c", "timeout"), done)

    def test_older_version_is_stale_not_done(self):
        keep, done, stale, dropped = D.resume_filter(
            [line("old", "PROVED", v=1), line("new", "PROVED")],
            set())
        self.assertEqual(stale, 1)
        self.assertEqual(dropped, 0)
        self.assertEqual([json.loads(l)["function"] for l in keep], ["new"])
        self.assertEqual(done, {("lib/x/a.c", "new")})

    def test_retry_status_does_not_bump_result_version(self):
        # A schema-unchanged resume flag must not invalidate every
        # existing PROVED/FAILED row.
        self.assertEqual(D.RESULT_VERSION, 2)


class ApplyResumeRewrite(unittest.TestCase):
    def test_retry_rewrites_jsonl_so_old_rows_are_gone(self):
        with tempfile.TemporaryDirectory() as t:
            out = Path(t) / "cbmc.jsonl"
            out.write_text("\n".join([
                line("proved", "PROVED"),
                line("bounded", "BOUNDED"),
                line("timeout", "TIMEOUT"),
                line("error", "ERROR"),
                line("failed", "FAILED"),
            ]) + "\n")
            done = D.apply_resume(out, {"TIMEOUT", "BOUNDED", "ERROR"})
            recs = [json.loads(l) for l in out.read_text().splitlines()
                    if l.strip()]
            by_fn = {r["function"]: r["status"] for r in recs}
            self.assertEqual(by_fn, {"proved": "PROVED", "failed": "FAILED"})
            self.assertEqual(done, {
                ("lib/x/a.c", "proved"),
                ("lib/x/a.c", "failed"),
            })
            self.assertEqual(len(recs), 2)

    def test_no_rewrite_when_nothing_is_stale_or_retried(self):
        with tempfile.TemporaryDirectory() as t:
            out = Path(t) / "cbmc.jsonl"
            original = line("proved", "PROVED") + "\n"
            out.write_text(original)
            done = D.apply_resume(out, {"TIMEOUT", "BOUNDED", "ERROR"})
            self.assertEqual(out.read_text(), original)
            self.assertEqual(done, {("lib/x/a.c", "proved")})


class PairList(unittest.TestCase):
    def test_tab_lines_blank_and_hash_ignored(self):
        want = D.parse_pair_list(
            "# comment\n"
            "\n"
            "sys/kern/subr_prf.c\tkprintf\n"
            " sys/kern/kern_synch.c\tsleepq_type \n")
        self.assertEqual(want, {
            ("sys/kern/subr_prf.c", "kprintf"),
            ("sys/kern/kern_synch.c", "sleepq_type"),
        })

    def test_rejects_space_separated(self):
        with self.assertRaises(ValueError):
            D.parse_pair_list("sys/kern/subr_prf.c kprintf\n")

    def test_help_exposes_pair_list(self):
        p = subprocess.run([sys.executable, str(DRIVER), "--help"],
                           capture_output=True, text=True, timeout=30)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertIn("--pair-list", p.stdout)


class DriverFlag(unittest.TestCase):
    def test_help_exposes_retry_status_unwind_timeout(self):
        p = subprocess.run([sys.executable, str(DRIVER), "--help"],
                           capture_output=True, text=True, timeout=30)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertIn("--retry-status", p.stdout)
        self.assertIn("--unwind", p.stdout)
        self.assertIn("--timeout", p.stdout)
        self.assertIn("--resume", p.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
