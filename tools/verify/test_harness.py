#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""String harness vs scalar: PROVED and PROVED-ASSUMING stay distinct.

cbmc_driver.py's first POINTER run of strcat reported the absence of a
precondition as a defect. --min-null-tree-depth 3 is the right assumption
when the contract is "valid objects". It is the wrong assumption when the
contract is "NUL-terminated buffers", and calling either success PROVED
would be a theorem about a domain the run did not explore.

These tests do not need a real CBMC. They check the decision that would
make that false theorem cheap: strcat-shaped signatures emit a harness
and a mocked SUCCESS is PROVED-ASSUMING with the path and the text;
scalar-shaped signatures emit nothing and an unguarded SUCCESS is PROVED.
"""
from __future__ import annotations

import unittest
from unittest.mock import patch

import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import classify  # noqa: E402
import cbmc_driver as D  # noqa: E402
import harness as H  # noqa: E402

STRCAT_TY = "char * (char *s, const char *append)"
FFS_TY = "signed int (signed int i)"
STRNCPY_TY = "char * (char *d, const char *s, unsigned long n)"
MEMCPY_TY = "void * (void *d, const void *s, unsigned long n)"
STRCHR_TY = "char * (const char *s, signed int c)"

SUCCESS_OUT = "[harness.pointer.1] dereference failure: pointer NULL: SUCCESS\n"


def _ok(returncode=0, stdout=SUCCESS_OUT):
    class P:
        pass
    p = P()
    p.returncode = returncode
    p.stdout = stdout
    p.stderr = ""
    return p


class ClassifySplit(unittest.TestCase):
    def test_strcat_is_pointer(self):
        self.assertEqual(classify.classify_type(STRCAT_TY), "POINTER")

    def test_ffs_is_scalar(self):
        self.assertEqual(classify.classify_type(FFS_TY), "SCALAR")

    def test_void_is_void(self):
        self.assertEqual(classify.classify_type("signed int (void)"), "VOID")


class StringLike(unittest.TestCase):
    def test_strcat_type_is_string_like(self):
        self.assertTrue(H.is_string_like("strcat", STRCAT_TY))

    def test_strcat_name_alone_is_string_like(self):
        self.assertTrue(H.is_string_like("strcat"))

    def test_scalar_ffs_is_not_string_like(self):
        self.assertFalse(H.is_string_like("ffs", FFS_TY))
        self.assertFalse(H.is_string_like("ffs"))

    def test_strncpy_has_size_so_not_string_like(self):
        self.assertFalse(H.is_string_like("strncpy", STRNCPY_TY))
        self.assertFalse(H.is_string_like("strncpy"))

    def test_memcpy_is_not_a_c_string(self):
        self.assertFalse(H.is_string_like("memcpy", MEMCPY_TY))

    def test_strchr_int_is_not_a_length(self):
        self.assertTrue(H.is_string_like("strchr", STRCHR_TY))

    def test_wcswidth_n_is_a_length(self):
        ty = "signed int (const wchar_t *s, signed int n)"
        self.assertFalse(H.is_string_like("wcswidth", ty))


class Emit(unittest.TestCase):
    def test_strcat_harness_assumes_bounded_nul_buffers(self):
        c = H.emit_harness("strcat", STRCAT_TY, bound=32)
        self.assertIsNotNone(c)
        assert c is not None
        self.assertIn("__CPROVER_assume", c)
        self.assertIn("__CPROVER_r_ok", c)
        self.assertIn("__CPROVER_w_ok", c)
        self.assertIn("strcat(", c)
        self.assertIn("'\\0'", c)
        self.assertIn("p0[0] = '\\0'", c)
        self.assertIn("void harness(void)", c)
        self.assertNotIn("--min-null-tree-depth", c)

    def test_strlen_does_not_empty_the_buffer(self):
        self.assertTrue(H.is_string_like("strlen"))
        c = H.emit_harness("strlen", "unsigned long (const char *s)")
        self.assertIsNotNone(c)
        assert c is not None
        self.assertIn("strlen(", c)
        self.assertNotIn("[0] = '\\0'", c)

    def test_scalar_emits_nothing(self):
        self.assertIsNone(H.emit_harness("ffs", FFS_TY))

    def test_strncpy_emits_nothing(self):
        self.assertIsNone(H.emit_harness("strncpy", STRNCPY_TY))

    def test_assumption_text_names_the_precondition(self):
        shape = H.shape_for("strcat", STRCAT_TY)
        text = H.assumption_text("strcat", shape, 32)
        self.assertIn("NUL-terminated", text)
        self.assertIn("32", text)
        self.assertIn("strcat", text)


class ProvedVsAssuming(unittest.TestCase):
    def test_clean_scalar_is_proved(self):
        task = {"class": "SCALAR", "null_depth": 0}
        self.assertEqual(D.decide_status(task, [], []), "PROVED")
        self.assertFalse(D.is_assumed_run(task))

    def test_pointer_success_is_never_proved(self):
        task = {"class": "POINTER", "null_depth": 0}
        self.assertEqual(D.decide_status(task, [], []), "PROVED-ASSUMING")

    def test_null_depth_success_is_assuming(self):
        task = {"class": "POINTER", "null_depth": 3}
        self.assertEqual(D.decide_status(task, [], []), "PROVED-ASSUMING")

    def test_harness_success_is_assuming(self):
        task = {"class": "POINTER", "harness": "/tmp/strcat.c", "null_depth": 0}
        self.assertEqual(D.decide_status(task, [], []), "PROVED-ASSUMING")

    def test_failure_stays_failed_under_a_harness(self):
        task = {"class": "POINTER", "harness": "/tmp/strcat.c"}
        self.assertEqual(D.decide_status(task, [{"desc": "x"}], []), "FAILED")

    def test_unwind_stays_bounded_under_null_depth(self):
        task = {"class": "POINTER", "null_depth": 3}
        self.assertEqual(D.decide_status(task, [], [{"desc": "unwinding"}]),
                         "BOUNDED")

    def test_rec_carries_harness_path_and_assumption(self):
        task = {
            "file": "lib/libc/string/strcat.c",
            "function": "harness",
            "orig_function": "strcat",
            "tier": "ub",
            "class": "POINTER",
            "linkage": "exported",
            "null_depth": 0,
            "unwind": 33,
            "harness": "/tmp/pbsd_harness/strcat.c",
            "assuming": "bounded NUL-terminated buffers of 32 bytes",
        }
        r = D._rec(task, "PROVED-ASSUMING")
        self.assertEqual(r["status"], "PROVED-ASSUMING")
        self.assertNotEqual(r["status"], "PROVED")
        self.assertEqual(r["function"], "strcat")
        self.assertEqual(r["harness"], "/tmp/pbsd_harness/strcat.c")
        self.assertIn("NUL-terminated", r["assuming"])

    def test_null_depth_rec_states_the_flag(self):
        task = {
            "file": "sys/kern/subr_scanf.c",
            "function": "foo",
            "tier": "ub",
            "class": "POINTER",
            "linkage": "exported",
            "null_depth": 3,
            "unwind": 16,
        }
        r = D._rec(task, "PROVED-ASSUMING")
        self.assertEqual(r["status"], "PROVED-ASSUMING")
        self.assertIn("min-null-tree-depth 3", r["assuming"])
        self.assertNotIn("harness", r)


class MockedCbmc(unittest.TestCase):
    def _task(self, **kw):
        t = {
            "file": "lib/libc/string/strcat.c",
            "src": "dummy.gb",
            "function": "strcat",
            "class": "POINTER",
            "linkage": "exported",
            "unwind": 16,
            "timeout": 5,
            "tier": "ub",
            "null_depth": 0,
            "mem_mb": 0,
        }
        t.update(kw)
        return t

    @patch("cbmc_driver.subprocess.run")
    def test_null_depth_passes_the_flag_and_does_not_say_proved(self, run):
        run.return_value = _ok()
        r = D.verify_one(self._task(null_depth=3, function="strcat"))
        cmd = run.call_args[0][0]
        self.assertIn("--min-null-tree-depth", cmd)
        self.assertEqual(cmd[cmd.index("--min-null-tree-depth") + 1], "3")
        self.assertEqual(r["status"], "PROVED-ASSUMING")
        self.assertNotEqual(r["status"], "PROVED")
        self.assertIn("min-null-tree-depth", r["assuming"])

    @patch("cbmc_driver.subprocess.run")
    def test_harness_entry_records_callee_and_path(self, run):
        run.return_value = _ok()
        r = D.verify_one(self._task(
            function="harness",
            orig_function="strcat",
            harness="/tmp/h.c",
            assuming="bounded NUL-terminated buffers of 32 bytes",
            null_depth=0,
        ))
        cmd = run.call_args[0][0]
        self.assertIn("--function", cmd)
        self.assertEqual(cmd[cmd.index("--function") + 1], "harness")
        self.assertNotIn("--min-null-tree-depth", cmd)
        self.assertEqual(r["status"], "PROVED-ASSUMING")
        self.assertEqual(r["function"], "strcat")
        self.assertEqual(r["harness"], "/tmp/h.c")
        self.assertIn("NUL-terminated", r["assuming"])

    @patch("cbmc_driver.subprocess.run")
    def test_scalar_unguarded_success_is_proved(self, run):
        run.return_value = _ok()
        t = self._task(
            function="ffs",
            file="lib/libc/string/ffs.c",
            null_depth=0,
        )
        t["class"] = "SCALAR"
        r = D.verify_one(t)
        self.assertEqual(r["status"], "PROVED")
        self.assertNotIn("harness", r)
        self.assertNotIn("assuming", r)

    @patch("cbmc_driver.subprocess.run")
    def test_pointer_without_null_depth_still_not_proved(self, run):
        run.return_value = _ok()
        r = D.verify_one(self._task(null_depth=0))
        self.assertEqual(r["status"], "PROVED-ASSUMING")


class TypeFromSource(unittest.TestCase):
    def test_strcat_source_shape(self):
        src = (
            "char *\n"
            "strcat(char * __restrict s, const char * __restrict append)\n"
            "{\n    return s;\n}\n"
        )
        ty = H.type_from_source(src, "strcat")
        self.assertIsNotNone(ty)
        self.assertTrue(H.is_string_like("strcat", ty))
        self.assertEqual(H.shape_for("strcat", ty), ("dst", "src"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
