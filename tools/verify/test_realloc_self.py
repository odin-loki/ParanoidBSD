#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""realloc_self.py has to DISTINGUISH before any of its 148 are worth reading.

A rule that reports `p = realloc(p, n)' is only useful if it also stays
quiet on the four things that look like it and are not: an assignment to
a different name, reallocf(3), the kernel's M_WAITOK spelling, and the
same text inside a comment or a string.
"""

import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from realloc_self import scan  # noqa: E402

FAILED = 0


def check(name: str, ok: bool, detail: str = "") -> None:
    global FAILED
    print(("ok   " if ok else "FAIL ") + name + (("  " + detail) if detail
                                                 and not ok else ""))
    if not ok:
        FAILED += 1


def hits(body: str) -> list[str]:
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "t.c"
        p.write_text(body)
        return [t for _, t, _ in scan(p)]


# --- the shapes it must report -------------------------------------------
check("the plain one",
      hits("void f(void){ p = realloc(p, n); }") == ["p"])
check("through a dereference",
      hits("void f(void){ *iov = realloc(*iov, n); }") == ["*iov"])
check("through a member",
      hits("void f(void){ s->buf = realloc(s->buf, n); }") == ["s->buf"])
check("inside an if, the shape six of the tree's were in",
      hits("void f(void){ if ((d = realloc(d, n + 1)) == NULL) return; }")
      == ["d"])
check("wrapped across lines, anchored on the assignment",
      hits("void f(void){\n\tkeys = realloc(keys,\n\t    n + 1);\n}")
      == ["keys"])
check("the kernel's four-argument form with M_NOWAIT",
      hits("void f(void){ p = realloc(p, n, M_X, M_NOWAIT); }") == ["p"])
check("...and with a flags variable, which says nothing",
      hits("void f(void){ p = realloc(p, n, M_X, flags); }") == ["p"])
check("whitespace does not hide it",
      hits("void f(void){ s -> buf = realloc( s->buf , n ); }")
      == ["s->buf"])

# --- and the ones it must not --------------------------------------------
check("a different name is the FIX, not the defect",
      hits("void f(void){ q = realloc(p, n); }") == [])
check("reallocf(3) frees the old block itself",
      hits("void f(void){ p = reallocf(p, n); }") == [])
check("the kernel's M_WAITOK cannot return NULL",
      hits("void f(void){ p = realloc(p, n, M_X, M_WAITOK); }") == [])
check("...M_WAITOK | M_ZERO is still M_WAITOK",
      hits("void f(void){ p = realloc(p, n, M_X, M_WAITOK | M_ZERO); }")
      == [])
check("but M_NOWAIT | M_ZERO is not",
      hits("void f(void){ p = realloc(p, n, M_X, M_NOWAIT | M_ZERO); }")
      == ["p"])
check("a block comment is not code",
      hits("void f(void){ /* p = realloc(p, n); */ }") == [])
check("a line comment is not code",
      hits("void f(void){ // p = realloc(p, n);\n}") == [])
check("a string literal is not code",
      hits('void f(void){ warnx("p = realloc(p, n)"); }') == [])
check("a comparison is not an assignment",
      hits("void f(void){ if (p == realloc(p, n)) return; }") == [])
check("a compound assignment is not one either",
      hits("void f(void){ n += realloc(p, n); }") == [])
check("a one-argument call is not realloc's shape",
      hits("void f(void){ p = realloc(p); }") == [])
check("a different member of the same struct",
      hits("void f(void){ s->a = realloc(s->b, n); }") == [])

# --- reallocarray(3): same contract, same defect -------------------------
check("reallocarray is the same shape",
      hits("void f(void){ p = reallocarray(p, n, sz); }") == ["p"])
check("...through a member too",
      hits("void f(void){ s->sb = reallocarray(s->sb, n, sz); }")
      == ["s->sb"])
check("...and through a dereference",
      hits("void f(void){ *items = reallocarray(*items, n, sz); }")
      == ["*items"])
check("reallocarray to a different name is still the fix",
      hits("void f(void){ q = reallocarray(p, n, sz); }") == [])
check("reallocf is still not reallocarray",
      hits("void f(void){ p = reallocf(p, n); }") == [])

# --- and it must find the ones the tree was fixed for --------------------
SRC = Path(__file__).resolve().parents[2] / "hbsd" / "src"
for rel in ("lib/libfetch/http.c", "lib/libutil/mntopts.c",
            "lib/libfigpar/figpar.c"):
    p = SRC / rel
    check("%s is fixed and stays quiet" % rel,
          p.exists() and [t for _, t, _ in scan(p)] == [],
          str([t for _, t, _ in scan(p)]) if p.exists() else "missing")

print("\n%s" % ("all checks passed" if not FAILED
                else "%d check(s) FAILED" % FAILED))
sys.exit(1 if FAILED else 0)
