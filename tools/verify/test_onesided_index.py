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

    def test_an_unsigned_operand_is_a_floor(self):
        """`alg < nitems(alg_types)' is the same bound with the
        unsignedness on the other side of the operator: nitems() is
        size_t, so alg converts to size_t and a negative alg compares
        above the limit. opencrypto's alg_type() is written that way
        and the rule called it one-sided. 35 sites in the tree are."""
        u = onesided_index._unsigned_operand_re
        self.assertTrue(u("alg").search(
            "return (alg < nitems(alg_types) ? alg_types[alg] : ALG_NONE);"))
        self.assertTrue(u("ipi_idx").search(
            "if (ipi_idx >= nitems(xen_ipis))"))
        self.assertTrue(u("len").search("if (len > sizeof(buf) - 1)"))
        self.assertTrue(u("i").search("if (nitems(tab) <= i)"))
        # A comparison against something that is not size_t is not it.
        self.assertFalse(u("alg").search("if (alg < ALG_LAST)"))
        self.assertFalse(u("i").search("if (i < sc->sc_ntab)"))
        # Nor is a sizeof that is not the operand being compared to V.
        self.assertFalse(u("i").search("memcpy(a, b, sizeof(x));"))

    def test_a_file_local_nitems_alias(self):
        """hccontrol's util.c spells nitems() itself --
        `#define SIZE(x) (sizeof((x))/sizeof((x)[0]))' -- and uses it
        fifteen times. What makes the comparison unsigned is the sizeof
        quotient, not the name, so the file is asked for its aliases.
        68 files in the tree define one: SIZE, N, ARRAY_SIZE, AS,
        UCODE, NUM_ELEMENTS, X."""
        self.assertEqual(
            onesided_index.sizeof_macros(
                "#define SIZE(x) (sizeof((x))/sizeof((x)[0]))\n"),
            frozenset({"SIZE"}))
        self.assertEqual(
            onesided_index.sizeof_macros(
                "#define\tARRAY_SIZE(x)\t(sizeof(x) / sizeof((x)[0]))\n"),
            frozenset({"ARRAY_SIZE"}))
        # A macro that merely mentions sizeof promises no type.
        self.assertEqual(
            onesided_index.sizeof_macros(
                "#define ZERO(x) memset((x), 0, sizeof(*(x)))\n"),
            frozenset())
        # And an object-like macro is not a nitems alias either.
        self.assertEqual(
            onesided_index.sizeof_macros("#define NT sizeof(t)/sizeof(t[0])\n"),
            frozenset())
        body = ("#define SIZE(x) (sizeof((x))/sizeof((x)[0]))\n"
                "\n"
                "char const *\n"
                "hci_role2str(int role)\n"
                "{\n"
                "\tstatic char const * const roles[] = { \"a\", \"b\" };\n"
                "\n"
                "\treturn (role >= SIZE(roles)? \"Unknown role\" : roles[role]);\n"
                "}\n")
        self.assertFalse([h for h in self.hits(body) if h[0] == "ONESIDED"],
                         self.hits(body))

    def test_long_long_is_excluded_from_the_unsigned_operand_floor(self):
        """On a 32-bit target size_t is unsigned int and a long long
        can represent all of it, so the size_t converts to long long
        and the index stays signed. The tree targets i386, armv7 and
        32-bit powerpc, so that case is not hypothetical."""
        body = ("static int\n"
                "f(long long idx, int n)\n"
                "{\n"
                "\tif (idx < nitems(tab))\n"
                "\t\treturn (tab[idx]);\n"
                "\treturn (0);\n"
                "}\n")
        r = self.hits(body)
        self.assertTrue(any(h[0] == "ONESIDED" and h[3] == "idx"
                            for h in r), r)
        self.assertTrue(any(ty in onesided_index._LONG_LONG
                            for ty in ("long long", "int64_t")))
        self.assertNotIn("int", onesided_index._LONG_LONG)

    def test_an_unsigned_variable_is_a_floor(self):
        """`if (fd > highfd)' with `int fd' and `u_int highfd' is the
        nitems() conversion with the unsignedness in a declared type
        instead of an operator, and close_range_flags() writes exactly
        that. The rule reads the BOUND's declared type, so DECL_RE had
        to learn the u_* family first -- `u_int' has no leading
        underscore and does not end in `_t', so declared() did not know
        the type existed."""
        d = {"fd": "int", "highfd": "u_int", "n": "size_t",
             "big": "long long", "l": "long"}
        u = onesided_index.unsigned_bound
        self.assertTrue(u("fd", "int", d, "if (fd > highfd)"))
        self.assertTrue(u("fd", "int", d, "while (fd <= n)"))
        # Rank decides, not signedness alone: on LP64 a long can
        # represent every unsigned int, so the unsigned int converts and
        # the index stays signed.
        self.assertFalse(u("l", "long", d, "if (l > highfd)"))
        self.assertTrue(u("l", "long", d, "if (l > n)"))
        # And a size_t cannot convert a long long, for the same reason
        # the nitems() clause excludes it.
        self.assertFalse(u("big", "long long", d, "if (big > n)"))
        # A signed bound is not a floor at all.
        self.assertFalse(u("fd", "int", d, "if (fd > l)"))
        self.assertFalse(u("fd", "int", d, "if (fd > unknown)"))

    def test_decl_re_knows_the_u_family(self):
        """An index declared u_int was skipped before because its type
        was unknown, which was the right answer by luck. Now it is the
        right answer by knowledge -- and the same change is what lets a
        u_int BOUND be read."""
        got = dict(m.groups()[::-1] for m in onesided_index.DECL_RE.finditer(
            "\tu_int highfd;\n\tu_long n;\n\tu_char c;\n\tint fd;\n"))
        self.assertEqual(got.get("highfd"), "u_int")
        self.assertEqual(got.get("n"), "u_long")
        self.assertEqual(got.get("c"), "u_char")
        self.assertEqual(got.get("fd"), "int")
        self.assertNotIn("u_int", onesided_index.SIGNED)
        self.assertNotIn("u_long", onesided_index.SIGNED)

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
