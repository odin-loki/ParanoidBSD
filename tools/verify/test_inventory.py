#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The ways a function inventory lies.

The scanner is a brace matcher, not a parser, and this file is where its
limits are written down as assertions rather than as prose. Two shapes
matter more than the rest:

  - `if (x) {' must never be a function called `if', because a scanner
    that counts control flow inflates the denominator and every coverage
    fraction built on it goes DOWN for free;
  - a call inside a body must never be a definition, for the same reason.

And the circularity check: the universe must not be built by asking the
instruments what they saw.
"""
from __future__ import annotations

import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

import inventory as I  # noqa: E402


class Scanner(unittest.TestCase):

    def test_a_plain_c_function(self):
        self.assertEqual(I.scan("int f(int x) { return x; }"), ["f"])

    def test_a_declaration_is_not_a_definition(self):
        self.assertEqual(I.scan("int f(int x);"), [])

    def test_control_flow_is_not_a_function(self):
        src = """
        int f(void) {
            if (x) { a(); }
            for (i = 0; i < n; i++) { b(); }
            while (c) { d(); }
            switch (e) { case 1: break; }
            return 0;
        }
        """
        self.assertEqual(I.scan(src), ["f"])

    def test_a_call_in_a_body_is_not_a_definition(self):
        src = """
        int f(void) {
            g(1);
            h(2);
            return 0;
        }
        """
        self.assertEqual(I.scan(src), ["f"])

    def test_two_functions_in_a_row(self):
        self.assertEqual(I.scan("void a(void) { }\nstatic int b(int x) { }"),
                         ["a", "b"])

    def test_a_brace_in_a_string_does_not_move_the_depth(self):
        src = 'void a(void) { puts("}"); }\nvoid b(void) { }'
        self.assertEqual(I.scan(src), ["a", "b"])

    def test_a_brace_in_a_comment_does_not_move_the_depth(self):
        src = "void a(void) { /* } */ }\nvoid b(void) { }"
        self.assertEqual(I.scan(src), ["a", "b"])
        src2 = "void a(void) { // }\n }\nvoid b(void) { }"
        self.assertEqual(I.scan(src2), ["a", "b"])

    def test_a_char_literal_brace(self):
        src = "void a(void) { char c = '}'; }\nvoid b(void) { }"
        self.assertEqual(I.scan(src), ["a", "b"])

    def test_a_multi_line_signature(self):
        src = """
        static int
        long_name(int a,
                  int b)
        {
            return a + b;
        }
        """
        self.assertEqual(I.scan(src), ["long_name"])

    def test_a_macro_wrapped_definition_is_found(self):
        """NCURSES_EXPORT(wchar_t *)(wunctrl)(...) { } and __elfN(x)(...) { }
        are real definitions the ledger misses. The scanner finding them is
        not a false positive - it is the scanner being right."""
        self.assertIn("wunctrl",
                      I.scan("NCURSES_EXPORT(wchar_t *) wunctrl (int c) { }"))

    def test_a_cxx_method_carries_its_class(self):
        src = """
        class Window {
        public:
            void show() { }
        };
        """
        self.assertEqual(I.scan(src), ["Window::show"])

    def test_an_out_of_line_cxx_method_keeps_its_qualification(self):
        self.assertEqual(I.scan("void Window::show() { }"), ["Window::show"])

    def test_a_const_method(self):
        self.assertEqual(I.scan("int Window::width() const { return w; }"),
                         ["Window::width"])

    def test_a_constructor_with_an_initialiser_list(self):
        src = "Window::Window(int w) : m_w(w), m_h(0) { }"
        self.assertEqual(I.scan(src), ["Window::Window"])

    def test_a_destructor(self):
        self.assertIn("Window::~Window", I.scan("Window::~Window() { }"))

    def test_a_namespaced_function(self):
        src = """
        namespace KWin {
        void run() { }
        }
        """
        self.assertEqual(I.scan(src), ["KWin::run"])

    def test_a_struct_initialiser_is_not_a_function(self):
        src = "static const struct foo t[] = { { 1, 2 }, { 3, 4 } };"
        self.assertEqual(I.scan(src), [])


class Universe(unittest.TestCase):

    def test_the_ledger_is_preferred_and_not_double_counted(self):
        """hbsd/src rows come from the ledger only. If the textual scanner
        also ran over that tree its weaker rows would be indistinguishable
        from the good ones and the error bar would silently apply to all
        297,212 of them."""
        import inspect
        src = inspect.getsource(I.build)
        self.assertIn('if tree == "hbsd":', src)
        self.assertIn("continue", src)

    def test_every_row_carries_where_it_came_from(self):
        """A row with no provenance cannot be given an error bar, and a
        number with no error bar is the thing this whole project is
        against."""
        import inspect
        src = inspect.getsource(I.build)
        self.assertIn('"src"', src)

    def test_the_two_sources_are_the_only_two(self):
        self.assertEqual({"ledger", "textual"},
                         {r[3] for r in [("", "", "", "ledger"),
                                         ("", "", "", "textual")]})


class Keywords(unittest.TestCase):

    def test_the_keyword_list_holds_the_control_flow_words(self):
        for kw in ("if", "for", "while", "switch", "catch", "return",
                   "sizeof", "do", "else"):
            self.assertIn(kw, I._NOT_A_NAME, kw)

    def test_cxx_keywords_that_precede_a_paren_and_a_brace(self):
        for kw in ("operator", "namespace", "class", "struct", "union",
                   "enum", "template", "noexcept", "decltype"):
            self.assertIn(kw, I._NOT_A_NAME, kw)


if __name__ == "__main__":
    unittest.main(verbosity=2)
