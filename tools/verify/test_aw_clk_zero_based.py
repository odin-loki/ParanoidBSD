#!/usr/bin/env python3
"""The six ways this check was broken on purpose before it was believed.

Each is an edit that was made to the real tree, run, and reverted while
the check was being written. Two of them found defects in the CHECK
rather than in the tree, and both are asserted here so they cannot come
back:

  - the first draft collapsed multi-line comments to a single space, so
    a definition at ccu_a83t.c:397 was reported at :368. A wrong line
    number is worse than none, because it looks like an answer.
  - the first draft trusted an empty divisor set. aw_clk_nm.c has a
    `cur = aw_clk_nm_find_best(...);' line that matches the frequency
    expression pattern and contains no division, so a driver whose
    arithmetic moved into a helper would have reported "divides by
    (nothing)" and waved through every zero-based factor it had.
"""

import contextlib
import io
import re
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import aw_clk_zero_based as az                              # noqa: E402


class Denominators(unittest.TestCase):
    """Which side of the bar an identifier is on."""

    def test_a_parenthesised_denominator_covers_all_of_it(self):
        self.assertEqual(az.denominator_names("(fparent * n * k) / (m * p)"),
                         {"m", "p"})

    def test_repeated_division_accumulates(self):
        self.assertEqual(az.denominator_names("fparent / n / m"), {"n", "m"})

    def test_a_leading_deref_star_is_not_a_multiply_that_matters(self):
        self.assertEqual(az.denominator_names("*freq * n / m0 / m1"),
                         {"m0", "m1"})

    def test_a_multiplied_factor_is_not_a_divisor(self):
        self.assertEqual(az.denominator_names("fparent * n / p"), {"p"})

    def test_dividing_by_a_parenthesised_quotient_inverts_twice(self):
        self.assertEqual(az.denominator_names("a / (b / c)"), {"b"})


class CommentStripping(unittest.TestCase):
    def test_a_multiline_comment_keeps_its_lines(self):
        """Every clock definition line ends in a `/* n factor */', and the
        definitions run to twelve lines. Losing a newline here moves every
        answer after it."""
        text = "one\n/* two\n   three */\nfour\n"
        self.assertEqual(az.strip_comments(text).count("\n"), text.count("\n"))

    def test_the_comment_body_is_gone(self):
        self.assertNotIn("ZERO", az.strip_comments("a /* ZERO_BASED */ b"))


class MacroReading(unittest.TestCase):
    def setUp(self):
        self.macs = az.macros(az._read(az.HEADER))

    def test_the_macros_are_found_and_name_their_driver(self):
        self.assertIn("NKMP_CLK", self.macs)
        self.assertEqual(self.macs["NKMP_CLK"][0], "nkmp")
        self.assertEqual(self.macs["NM_CLK"][0], "nm")

    def test_the_flags_argument_index_is_read_from_the_body(self):
        """NKMP_CLK's parameters are positional; only `.n.flags = _n_flags'
        says which one is the n factor's."""
        fields = self.macs["NKMP_CLK"][1]
        self.assertEqual(sorted(fields.values()), ["k", "m", "n", "p"])

    def test_a_macro_body_that_sets_flags_from_a_non_parameter(self):
        with self.assertRaises(az.Fault):
            az.macros("#define X_CLK(_a, _b) \\\n"
                      "  static struct aw_clk_nm_def _a = { .n.flags = _zz, };\n")

    def test_a_header_with_no_clock_macros_at_all(self):
        with self.assertRaises(az.Fault):
            az.macros("#define NOTHING(x) (x)\n")


class TheRealTree(unittest.TestCase):
    def test_n_multiplies_in_nkmp_and_divides_in_nm(self):
        """The whole reason the check is per-driver rather than per-name.
        A rule about the letter `n' would be wrong for one of these two."""
        self.assertNotIn("n", az.divisors("nkmp"))
        self.assertIn("n", az.divisors("nm"))

    def test_every_driver_divides_by_something(self):
        for kind in sorted({k for k, _ in az.macros(az._read(az.HEADER)).values()}):
            with self.subTest(kind):
                self.assertTrue(az.divisors(kind))

    def test_thirteen_zero_based_factors_all_of_them_nkmp_n(self):
        faults, lines = az.check()
        self.assertEqual(faults, [])
        used = [ln for ln in lines if "ccu_" in ln]
        self.assertEqual(len(used), 13)
        for ln in used:
            self.assertIn("NKMP_CLK n factor", ln)
            self.assertIn("(multiplies)", ln)

    def test_the_gate_passes(self):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = az.main(["--gate"])
        self.assertEqual(rc, 0)
        self.assertIn("no zero-based factor is divided by", buf.getvalue())


class Breakage(unittest.TestCase):
    """The real tree, with one thing changed. `_read' is redirected rather
    than the files edited, so a failed test cannot leave the tree dirty."""

    def patched(self, **files):
        real = az._read

        def fake(rel):
            return files[rel] if rel in files else real(rel)

        az._read = fake
        try:
            buf = io.StringIO()
            with contextlib.redirect_stderr(buf), contextlib.redirect_stdout(
                    io.StringIO()):
                rc = az.main([])          # NB: no --gate
            return rc, buf.getvalue()
        finally:
            az._read = real

    def faults_with(self, **files):
        real = az._read

        def fake(rel):
            return files[rel] if rel in files else real(rel)

        az._read = fake
        try:
            return az.check()[0]
        finally:
            az._read = real

    A83T = f"{az.CLKDIR}/ccu_a83t.c"
    NM = f"{az.CLKDIR}/aw_clk_nm.c"

    def test_the_flag_moved_from_nkmp_n_to_nkmp_m(self):
        src = az._read(self.A83T).replace(
            "    8, 8, 0, AW_CLK_FACTOR_ZERO_BASED,\t\t/* n factor */\n"
            "    0, 0, 1, AW_CLK_FACTOR_FIXED,\t\t/* k factor (fake) */\n"
            "    0, 0, 1, AW_CLK_FACTOR_FIXED,\t\t/* m factor */",
            "    8, 8, 0, 0,\t\t/* n factor */\n"
            "    0, 0, 1, AW_CLK_FACTOR_FIXED,\t\t/* k factor (fake) */\n"
            "    0, 0, 1, AW_CLK_FACTOR_ZERO_BASED,\t\t/* m factor */", 1)
        faults = self.faults_with(**{self.A83T: src})
        self.assertEqual(len(faults), 1, faults)
        self.assertIn("NKMP_CLK m factor", faults[0])
        self.assertIn("aw_clk_nkmp.c DIVIDES by m", faults[0])

    def test_the_flag_on_an_nm_clk_n_which_divides(self):
        lines = az._read(self.A83T).splitlines(keepends=True)
        i = next(n for n, ln in enumerate(lines)
                 if "16, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO," in ln)
        lines[i] = lines[i].replace("AW_CLK_FACTOR_POWER_OF_TWO",
                                    "AW_CLK_FACTOR_ZERO_BASED")
        faults = self.faults_with(**{self.A83T: "".join(lines)})
        self.assertEqual(len(faults), 1, faults)
        self.assertIn("NM_CLK n factor", faults[0])
        self.assertIn("aw_clk_nm.c DIVIDES by n", faults[0])
        # The line the FLAG is on, not the line the macro name is on.
        self.assertIn(f":{i + 1} ", faults[0])

    def test_get_min_no_longer_returning_zero_for_the_flag(self):
        src = re.sub(r"(" + az.FLAG + r"\)\s*\n\s*min = )0;", r"\g<1>1;",
                     az._read(az.HEADER))
        rc, err = self.patched(**{az.HEADER: src})
        self.assertEqual(rc, 1)
        self.assertIn("the premise this check enforces has changed", err)

    def test_the_flag_gone_from_every_definition(self):
        files = {rel: az._read(rel).replace(az.FLAG + ",", "0,")
                 for rel in (self.A83T, f"{az.CLKDIR}/ccu_a10.c")}
        rc, err = self.patched(**files)
        self.assertEqual(rc, 1)
        self.assertIn("this reader stopped finding it", err)

    def test_a_divisor_the_reader_cannot_trace_to_a_factor(self):
        src = az._read(self.NM).replace(
            "\tm = aw_clk_get_factor(val, &sc->m);",
            "\tm = some_other_source(val);")
        rc, err = self.patched(**{self.NM: src})
        self.assertEqual(rc, 1)
        self.assertIn("the reader cannot say which factor that is", err)

    def test_a_driver_whose_division_moved_into_a_helper(self):
        """aw_clk_nm.c keeps a `cur = aw_clk_nm_find_best(...)' line that
        matches the expression pattern and divides by nothing. An empty
        divisor set must not read as a clean bill of health."""
        src = (az._read(self.NM)
               .replace("cur = fparent / n / m;", "cand = fparent / n / m;")
               .replace("*freq = *freq / prediv / n / m;",
                        "out = *freq / prediv / n / m;"))
        rc, err = self.patched(**{self.NM: src})
        self.assertEqual(rc, 1)
        self.assertIn("this reader now passes everything in this driver", err)


if __name__ == "__main__":
    unittest.main()
