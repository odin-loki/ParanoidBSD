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

    def test_a_function_that_returns(self):
        p = self.write("int\nf(int s)\n{\n\tif (s)\n\t\texit(1);\n"
                       "\treturn (s);\n}\n")
        self.assertFalse(fusebmc.terminates_process(p, "f"))


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
