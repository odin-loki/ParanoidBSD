#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""run_timeout_retry.py must not treat the live jsonl as writable."""
from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

import run_timeout_retry as R  # noqa: E402


def rec(**kw) -> str:
    d = {
        "file": "sys/kern/subr_prf.c",
        "function": "kprintf",
        "status": "TIMEOUT",
        "class": "SCALAR",
    }
    d.update(kw)
    return json.dumps(d)


class ExtractPairs(unittest.TestCase):
    def test_filters_scope_class_status_and_dedupes(self):
        text = "\n".join([
            rec(),
            rec(),  # duplicate
            rec(file="lib/libc/stdlib/abs.c"),
            rec(status="PROVED"),
            rec(function="sleepq_type", **{"class": "POINTER"}),
            rec(file="sys/kern/kern_synch.c", function="sleepq_type"),
            json.dumps({"_meta": True, "status": "TIMEOUT"}),
        ]) + "\n"
        with tempfile.TemporaryDirectory() as t:
            p = Path(t) / "cbmc.jsonl"
            p.write_text(text, encoding="utf-8")
            pairs, st = R.extract_pairs(p, "sys/", "SCALAR", "TIMEOUT")
        self.assertEqual(pairs, [
            ("sys/kern/subr_prf.c", "kprintf"),
            ("sys/kern/kern_synch.c", "sleepq_type"),
        ])
        self.assertEqual(st["TIMEOUT"], 5)
        self.assertEqual(st["PROVED"], 1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
