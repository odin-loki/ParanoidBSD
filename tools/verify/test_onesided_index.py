#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tests for onesided_index.py.

The first three cases are the three defects this checker was written
after, in the form they had before they were fixed.  A lint that cannot
find the bugs it was written for is worth nothing, and the only way to
know is to keep them.

The rest are the shapes it must NOT report, each one a real function in
this tree that the first draft got wrong.
"""
from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import onesided_index  # noqa: E402


class Base(unittest.TestCase):
    def hits(self, body: str):
        d = Path(tempfile.mkdtemp(prefix="onesided-"))
        p = d / "t.c"
        p.write_text(body)
        return list(onesided_index.scan(p))

    def kinds(self, body):
        return sorted({h[0] for h in self.hits(body)})


class TheThreeItWasWrittenFor(Base):
    def test_systat_get_tbl_ptr(self):
        # `idx = scale < SC_AUTO ? scale : SC_AUTO' bounds the top and
        # leaves the bottom, and &convtbl[idx] is then a wild pointer
        # both callers dereference at once.
        r = self.hits(
            "struct convtbl *\n"
            "get_tbl_ptr(const uintmax_t size, const int scale)\n"
            "{\n"
            "\tuintmax_t tmp;\n"
            "\tint idx;\n"
            "\n"
            "\tidx = scale < SC_AUTO ? scale : SC_AUTO;\n"
            "\treturn (&convtbl[idx]);\n"
            "}\n")
        self.assertTrue(any(h[0] == "ONESIDED" and h[3] == "idx" for h in r),
                        r)

    def test_libc_querylocale(self):
        # ffs() answers 0 when no bit is set, so type is -1 and the
        # `>= XLC_LAST' test passes it straight through.
        r = self.hits(
            "const char *\n"
            "querylocale(int mask, locale_t loc)\n"
            "{\n"
            "\tint type = ffs(mask & ~LC_VERSION_MASK) - 1;\n"
            "\tif (type >= XLC_LAST)\n"
            "\t\treturn (NULL);\n"
            "\tif (loc->components[type])\n"
            "\t\treturn (loc->components[type]->locale);\n"
            "\treturn (\"C\");\n"
            "}\n")
        self.assertTrue(any(h[0] == "ONESIDED" and h[3] == "type" for h in r),
                        r)

    def test_libcalendar_easterodn(self):
        # C's % keeps the sign of the dividend.
        r = self.hits(
            "static int\n"
            "easterodn(int y)\n"
            "{\n"
            "\tint mc[] = {5, 25, 13};\n"
            "\tdate dt;\n"
            "\n"
            "\tdt.d = mc[y % 19];\n"
            "\treturn (dt.d);\n"
            "}\n")
        self.assertTrue(any(h[0] == "MOD" and h[3] == "y" for h in r), r)


class ShapesItMustNotReport(Base):
    def test_a_loop_counter(self):
        # The most common subscript in the tree. The initialiser pins it
        # at zero and nothing else assigns it.
        self.assertEqual(self.kinds(
            "void\nf(int n)\n{\n\tint i;\n\n"
            "\tfor (i = 0; i < n; i++)\n\t\ta[i] = 0;\n}\n"), [])

    def test_an_unsigned_index(self):
        # The whole defect is the sign. bin/pax hashes on a uid_t.
        self.assertEqual(self.kinds(
            "void\nf(u_int uid)\n{\n\tptr = uidtb[uid % UID_SZ];\n}\n"), [])

    def test_a_floor_written_with_a_name(self):
        # setlocale() spells its floor as `category < LC_ALL', and
        # LC_ALL is 0. A rule that took `V < NAME' for a floor would
        # swallow every upper-bound test in the tree, so what is
        # recognised is the SHAPE: one guard, two bounds, opposite ways.
        self.assertEqual(self.kinds(
            "char *\nsetlocale(int category, const char *locale)\n{\n"
            "\tif (category < LC_ALL || category >= _LC_LAST) {\n"
            "\t\terrno = EINVAL;\n\t\treturn (NULL);\n\t}\n"
            "\treturn (current_categories[category]);\n}\n"), [])

    def test_an_explicit_floor(self):
        self.assertEqual(self.kinds(
            "int\nf(int i)\n{\n\tif (i < 0)\n\t\treturn (-1);\n"
            "\tif (i >= N)\n\t\treturn (-1);\n\treturn (a[i]);\n}\n"), [])

    def test_the_sentinel_rejected(self):
        # ifcmds.c: `if ((scale = get_scale(args)) != -1)'. Rejecting
        # the sentinel is how most of this tree spells the floor.
        self.assertEqual(self.kinds(
            "void\nf(void)\n{\n\tint scale;\n\n"
            "\tif ((scale = get_scale(args)) != -1) {\n"
            "\t\tif (scale < SC_AUTO)\n\t\t\tuse(tbl[scale]);\n\t}\n}\n"), [])

    def test_bounded_nowhere_is_not_this_defect(self):
        # An index with no test at all is a different report to write.
        # This one is about a bound that guards one end of two.
        self.assertEqual(self.kinds(
            "int\nf(int i)\n{\n\treturn (a[i]);\n}\n"), [])

    def test_a_type_it_does_not_know(self):
        # Guessing at signedness would put the tree's commonest shapes
        # in the report and the report would stop being read.
        self.assertEqual(self.kinds(
            "int\nf(myindex_t i)\n{\n\tif (i >= N)\n\t\treturn (0);\n"
            "\treturn (a[i]);\n}\n"), [])


class TheGateBites(Base):
    """A gate that has only ever been observed printing `ok' is not a
    gate.  This runs the real thing over a real directory with one
    record removed, and again with it back."""

    def gate_exit(self, source: str):
        import subprocess
        tool = Path(onesided_index.__file__)
        keep = tool.read_text()
        try:
            tool.write_text(source)
            r = subprocess.run(
                [sys.executable, str(tool), "--gate", "--scope", "usr.bin/pr"],
                capture_output=True, text=True)
            return r.returncode
        finally:
            tool.write_text(keep)

    def test_a_pointer_parameter_is_a_parameter(self):
        """declared() wanted `<type><space><name>', so `struct foo *p'
        had a `*' where the space should be and the parameter set came
        back EMPTY -- for most of this tree, and for rbootd's
        SendFileNo(struct rmp_packet *, RMPCONN *, char *[]) exactly.
        from_outside() then had nothing to match and the rule dropped
        the worst instance of its own shape in the tree: a 32-bit wire
        value decremented into `filelist[i]'."""
        for decl, want in [
            ("SendFileNo(struct rmp_packet *req, RMPCONN *rconn, "
             "char *filelist[])", {"req", "rconn", "filelist"}),
            ("f(int a, char *b)", {"a", "b"}),
            ("g(void)", set()),
            # A function-pointer parameter's last identifier is a type in
            # the inner list, not a name. Skipping it loses a name;
            # guessing one invents a wrong one.
            ("h(int x, int (*cmp)(const void *, const void *))", {"x"}),
        ]:
            self.assertEqual(onesided_index.param_names(decl), want, decl)

    def test_an_out_parameter_macro_counts_as_outside(self):
        """GETWORD(w, i) is `(i) = ntohl(w)': the assignment is inside
        the macro, so there is no `i =' in the source at all. V passed
        to a call whose argument list also names a parameter, and never
        assigned a value itself, came from outside."""
        text = ("\tGETWORD(req->r_brpl.rmp_seqno, i);\n"
                "\tPUTWORD(i, rpl->r_brpl.rmp_seqno);\n"
                "\ti--;\n"
                "\tif (i < C_MAXFILE && filelist[i] != NULL) {\n")
        self.assertTrue(onesided_index.from_outside("i", {"req"}, text))
        # `i--' must not disqualify it -- it modifies a value that was
        # already there. A real assignment does.
        self.assertFalse(
            onesided_index.from_outside("i", {"req"},
                                        "\ti = 0;\n" + text))
        # And a TRACE macro taking a loop counter by value is not an
        # out-parameter, however many parameters sit beside it.
        self.assertFalse(onesided_index.from_outside(
            "index", {"table"},
            "\tfor (index = 0; index < N; index++) {\n"
            "\t\tEFSYS_PROBE2(table, int, index, uint32_t, byte);\n"))

    def test_an_unsigned_cast_is_a_floor(self):
        """`(u_int)fd >= fdt->fdt_nfiles' is the kernel's idiom for
        bounding a descriptor and it bounds BOTH ends in one
        comparison: the cast turns a negative fd into a value above any
        real limit, so the same `>=' rejects it. kern_descrip.c spells
        it that way and the rule called it one-sided."""
        self.assertTrue(onesided_index._floor_re("fd").search(
            "if (__predict_false((u_int)fd >= fdt->fdt_nfiles))"))
        self.assertTrue(onesided_index._floor_re("i").search(
            "if ((size_t)i >= n)"))
        # The cast has to be to an unsigned type, and to V itself.
        self.assertFalse(onesided_index._floor_re("fd").search(
            "if ((int)fd >= fdt->fdt_nfiles)"))
        self.assertFalse(onesided_index._floor_re("fd").search(
            "if (fd >= (u_int)fdt->fdt_nfiles)"))

    def test_a_site_off_the_record_fails(self):
        src = Path(onesided_index.__file__).read_text()
        holed = src.replace('"usr.bin/pr/pr.c:1420":',
                            '"usr.bin/pr/pr.c:99999":', 1)
        self.assertNotEqual(src, holed, "the record moved; update this test")
        self.assertEqual(self.gate_exit(holed), 1)

    def test_the_record_as_it_stands_passes(self):
        self.assertEqual(
            self.gate_exit(Path(onesided_index.__file__).read_text()), 0)


if __name__ == "__main__":
    unittest.main()
