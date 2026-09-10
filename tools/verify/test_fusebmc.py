#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tests for fusebmc.py's parameter parser, harness and task loader.

Every case here is a mistake this engine actually made, kept so it
cannot make it twice.  The engine's own claim is that ERROR is the
measurement of what it could not reach -- which is only worth anything
if ERROR means what it says, so the cases that turn a true statement
about the TABLE into a false one about the CODE are the ones that
matter most.
"""
from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fusebmc  # noqa: E402


class ParseParams(unittest.TestCase):
    def parse(self, text, fn="f"):
        return fusebmc.parse_params(text, fn)

    def test_no_definition_is_none_not_a_reason(self):
        # "this file does not define it" and "this takes a pointer" are
        # different answers and the caller reports them differently.
        self.assertIsNone(self.parse("int g(int a)\n{\n}\n"))

    def test_void_is_an_empty_list(self):
        self.assertEqual(self.parse("int f(void)\n{\n}\n"), [])

    def test_scalars(self):
        self.assertEqual(self.parse("int f(int a, unsigned long b)\n{\n}\n"),
                         [("int", "a"), ("unsigned long", "b")])

    def test_return_type_on_its_own_line(self):
        # KNF puts the return type on its own line.  The character class
        # before the name did not admit a newline, and ffs() -- the first
        # function this was ever run on -- came back as if it took a
        # pointer.
        self.assertEqual(self.parse("int\nf(int mask)\n{\n}\n"),
                         [("int", "mask")])

    def test_a_pointer_is_a_reason_not_a_guess(self):
        r = self.parse("int f(char *s)\n{\n}\n")
        self.assertIsInstance(r, str)
        self.assertIn("pointer", r)

    def test_varargs_is_a_reason(self):
        self.assertIsInstance(self.parse("int f(int a, ...)\n{\n}\n"), str)

    def test_trailing_attribute_is_not_part_of_the_type(self):
        # `int signo __unused' is an int named signo.  Taking the last
        # token for the name made the type "int signo", and 148
        # parameters came back as "parameter type 'int signo' is not a
        # known scalar" -- true of the table, false of the parameter.
        self.assertEqual(self.parse("void f(int signo __unused)\n{\n}\n"),
                         [("int", "signo")])

    def test_const_is_stripped(self):
        self.assertEqual(self.parse("int f(const int a)\n{\n}\n"),
                         [("int", "a")])

    def test_a_pointer_typedef_reports_as_a_pointer(self):
        # device_t is `struct _device *'.  Reporting 9,559 of these as
        # "not a known scalar" says nothing about why they cannot be
        # reached; saying "a pointer" does.
        r = self.parse("int f(device_t dev)\n{\n}\n")
        self.assertIsInstance(r, str)
        self.assertIn("pointer", r)

    def test_a_parameter_list_macro_is_named(self):
        r = self.parse("int f(SYSCTL_HANDLER_ARGS)\n{\n}\n")
        self.assertIsInstance(r, str)
        self.assertIn("macro", r)

    def test_an_address_sized_integer_is_a_scalar(self):
        # vm_offset_t is __uint64_t, not one of the opaque pointers whose
        # names sit beside it in the ERROR histogram.
        self.assertEqual(self.parse("int f(vm_offset_t a)\n{\n}\n"),
                         [("vm_offset_t", "a")])

    def test_bool_and_double_are_scalars(self):
        self.assertEqual(self.parse("int f(bool a, double b)\n{\n}\n"),
                         [("bool", "a"), ("double", "b")])


class TypeShapes(unittest.TestCase):
    """Answers the ERROR histogram named, once it was bucketed."""

    def parse(self, text, fn="f"):
        return fusebmc.parse_params(text, fn)

    def test_an_enum_is_an_int(self):
        # An enum is an integer type in C whatever its tag is. 23 came
        # back "not a known scalar", which is true of the table and
        # false of the language.
        self.assertEqual(self.parse("int f(enum ev_type t)\n{\n}\n"),
                         [("int", "t")])

    def test_a_struct_by_value_says_which_it_is(self):
        # Not an unknown type: a known one this cannot make. `struct
        # in_addr' is 21 of them, and inventing an aggregate is the same
        # dishonesty as inventing a buffer behind a pointer.
        r = self.parse("int f(struct in_addr a)\n{\n}\n")
        self.assertIsInstance(r, str)
        self.assertIn("aggregate", r)

    def test_a_union_by_value_too(self):
        self.assertIn("aggregate", self.parse("int f(union key k)\n{\n}\n"))

    def test_a_complex_is_a_scalar(self):
        self.assertEqual(self.parse("int f(double complex z)\n{\n}\n"),
                         [("double complex", "z")])

    def test_the_harness_can_name_a_complex_type(self):
        # <complex.h> for the `complex' keyword: without it the harness
        # for cpow() does not compile, and a type this CAN synthesise
        # would be reported as one it cannot.
        h = fusebmc.build_harness(Path("x.c"), "f", [("double complex", "z")])
        self.assertIn("#include <complex.h>", h)

    def test_quad_t_is_an_int64(self):
        self.assertEqual(self.parse("int f(quad_t q)\n{\n}\n"),
                         [("quad_t", "q")])


class Layout(unittest.TestCase):
    def test_natural_alignment(self):
        slots, total = fusebmc.layout([("char", "a"), ("uint64_t", "b")])
        self.assertEqual([s[0] for s in slots], [0, 8])
        self.assertEqual(total, 16)

    def test_empty_reserves_one_byte(self):
        # A zero-byte read would make fread() succeed on empty input and
        # the harness would run on nothing.
        _slots, total = fusebmc.layout([])
        self.assertEqual(total, 1)


class Harness(unittest.TestCase):
    def test_reads_at_the_offset_the_packer_writes(self):
        h = fusebmc.build_harness(Path("x.c"), "f",
                                  [("int32_t", "a"), ("uint64_t", "b")])
        self.assertIn("memcpy(&v0, buf + 0, 4)", h)
        self.assertIn("memcpy(&v1, buf + 8, 8)", h)

    def test_a_bool_is_normalised_not_memcpyd(self):
        # An arbitrary byte in a _Bool can be a trap representation, and
        # reading it back is undefined -- the harness would be the bug.
        h = fusebmc.build_harness(Path("x.c"), "f", [("bool", "a")])
        self.assertNotIn("memcpy(&v0", h)
        self.assertIn("(buf[0] & 1) != 0", h)

    def test_void_return_has_no_sink(self):
        h = fusebmc.build_harness(Path("x.c"), "f", [("int", "a")], "void")
        self.assertNotIn("volatile", h)
        self.assertIn("\tf(v0);", h)

    def test_a_value_return_goes_to_a_volatile_sink(self):
        # Discarded, the optimiser deletes the call and a whole fuzzing
        # budget is spent on nothing.
        h = fusebmc.build_harness(Path("x.c"), "f", [("int", "a")], "int")
        self.assertIn("volatile int sink", h)


class CppFlags(unittest.TestCase):
    """cbmc is not a compiler driver, and neither is a list comprehension.

    Both halves of this were real: passing cbmc a flag it does not take
    made it print its usage and exit, which the engine recorded as
    NOSEED with a page of help text as the reason -- the tool reporting
    its own misuse as a property of the code.
    """

    def test_a_separated_pair_is_kept_whole(self):
        self.assertEqual(fusebmc.cpp_flags(["-I", "/a", "-O2"]),
                         ["-I", "/a"])

    def test_a_dropped_pair_takes_its_argument_with_it(self):
        # Keeping `-include' and dropping the header is worse than
        # dropping both: the next flag silently becomes its argument.
        got = fusebmc.cpp_flags(["-include", "x.h", "-I/b"])
        self.assertEqual(got, ["-I/b"])

    def test_the_cbmc_set_refuses_minus_u(self):
        # cbmc 5.95.1: "Unknown option: -U__linux__", then usage, exit.
        got = fusebmc.cpp_flags(["-I/a", "-U__linux__", "-DX=1"],
                                accept=("-I", "-D"))
        self.assertEqual(got, ["-I/a", "-DX=1"])

    def test_nostdinc_is_not_a_preprocessor_flag_here(self):
        # cbmc models the C library itself rather than reading a host's
        # headers for it, so -nostdinc is neither needed nor accepted.
        self.assertEqual(fusebmc.cpp_flags(["-nostdinc", "-I/a"]), ["-I/a"])


class TerminatesProcess(unittest.TestCase):
    """A function that exits is not a function that crashed.

    AFL's child does not come back to the fork server either way, so
    _Exit() and quick_exit() were both reported CRASH -- two of the four
    the first run at scale produced.
    """

    def write(self, body):
        d = Path(tempfile.mkdtemp(prefix="fusebmc-test-"))
        p = d / "t.c"
        p.write_text(body)
        return p

    def test_a_libc_terminator_is_recognised_by_name(self):
        self.assertTrue(fusebmc.terminates_process(self.write(""), "_exit"))

    def test_a_function_ending_in_exit(self):
        p = self.write("void\nf(int s)\n{\n\tcleanup();\n\texit(s);\n}\n")
        self.assertTrue(fusebmc.terminates_process(p, "f"))

    def test_a_function_that_kills_its_own_process(self):
        # bsdinstall's reproduce_signal_death(): SIG_DFL, no core, then
        # kill(getpid(), sig). Reported CRASH on SIGILL, which is the
        # function doing precisely what its name says. Neither raise()
        # nor kill() is __dead2 -- both return when the signal is
        # blocked -- so noreturn_check.py cannot know this on its own.
        p = self.write("void\nf(int sig)\n{\n\tsignal(sig, SIG_DFL);\n"
                       "\tkill(getpid(), sig);\n}\n")
        self.assertTrue(fusebmc.terminates_process(p, "f"))

    def test_killing_another_process_is_not_the_same(self):
        p = self.write("void\nf(int sig)\n{\n\tkill(child, sig);\n}\n")
        self.assertFalse(fusebmc.terminates_process(p, "f"))

    def test_a_function_that_returns(self):
        p = self.write("int\nf(int s)\n{\n\tif (s)\n\t\texit(1);\n"
                       "\treturn (s);\n}\n")
        self.assertFalse(fusebmc.terminates_process(p, "f"))


class SanitizerBroke(unittest.TestCase):
    """The sanitizer aborting on ITSELF is not a finding about the code."""

    ASAN = ('AddressSanitizer: CHECK failed: asan_allocator.cpp:601 '
            '"((user_end)) <= ((alloc_end))" (0x8000000000000001, 0x50)\n'
            + "".join("    #%d 0x5632 (/tmp/t+0x%x)\n" % (i, i)
                      for i in range(40)))

    def test_an_internal_check_is_recognised(self):
        # memalign(1 << 63, 0): the C library answers NULL with EINVAL,
        # ASan's own arithmetic overflows. Reported CRASH once.
        self.assertTrue(fusebmc.sanitizer_broke(self.ASAN))

    def test_a_real_report_is_not(self):
        self.assertFalse(fusebmc.sanitizer_broke(
            "SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior "
            "abs.c:37:17 in"))

    def test_empty_is_not(self):
        self.assertFalse(fusebmc.sanitizer_broke(""))

    def test_the_evidence_is_the_head_not_the_tail(self):
        # A sanitizer puts its verdict FIRST and the stack after it, so
        # a 600-character tail is exactly the part with no answer in it
        # -- memalign came back CRASH a second time with the line that
        # classifies it three screens above the cut.
        self.assertIn("CHECK failed", fusebmc.evidence(self.ASAN))

    def test_evidence_keeps_the_verdict_lines_out_of_the_frames(self):
        # A real report buries its verdict among forty stack frames, and
        # the first thing recorded for the two convtbl crashes was the
        # frames. The lines that name the defect are the ones to keep.
        report = ("==1==ERROR: AddressSanitizer: SEGV on unknown address\n"
                  + "    #%d 0x00 (/tmp/t+0x0)\n" * 60 % tuple(range(60))
                  + "SUMMARY: AddressSanitizer: SEGV convtbl.c:99 in convert\n")
        got = fusebmc.evidence(report)
        self.assertIn("SUMMARY: AddressSanitizer: SEGV convtbl.c:99", got)
        self.assertNotIn("#59", got)

    def test_evidence_falls_back_to_the_tail(self):
        # A compiler puts its verdict last, which is what _tail is for.
        self.assertIn("2 errors generated.",
                      fusebmc.evidence("noise\n" * 400 +
                                       "2 errors generated."))


class LoadTasks(unittest.TestCase):
    def plan(self, records):
        d = Path(tempfile.mkdtemp(prefix="fusebmc-plan-"))
        p = d / "plan.json"
        p.write_text(json.dumps({"records": records}))
        return p

    def test_one_task_per_file_and_function(self):
        p = self.plan([{"path": "lib/a.c", "functions": ["f", "g"]},
                       {"path": "lib/b.c", "functions": ["h"]}])
        self.assertEqual(len(fusebmc.load_tasks(p, [])), 3)

    def test_scope_is_a_path_prefix(self):
        p = self.plan([{"path": "lib/a.c", "functions": ["f"]},
                       {"path": "sys/b.c", "functions": ["h"]}])
        got = fusebmc.load_tasks(p, ["sys"])
        self.assertEqual([t["function"] for t in got], ["h"])

    def test_a_record_with_no_functions_is_not_a_task(self):
        p = self.plan([{"path": "lib/a.c"}, {"path": "lib/b.c",
                                             "functions": []}])
        self.assertEqual(fusebmc.load_tasks(p, []), [])

    def test_limit_stops_early(self):
        p = self.plan([{"path": "lib/a.c", "functions": ["f", "g", "h"]}])
        self.assertEqual(len(fusebmc.load_tasks(p, [], limit=2)), 2)


if __name__ == "__main__":
    unittest.main()
