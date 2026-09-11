#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""capacity_first.py still tells the one from the six.

A rule with a six-entry EXPECTED table and one real find is a rule that
can stop distinguishing without anyone noticing: widen it and the gate
still passes, narrow it and the gate still passes.  So the shapes are
written out here and checked directly, and the real find is checked as
a FIXED file that must stay quiet.
"""
from __future__ import annotations

import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from capacity_first import EXPECTED, SRC, scan  # noqa: E402

fails: list[str] = []


def check(what: str, ok: bool, why: str = "") -> None:
    print(f"  {'ok  ' if ok else 'FAIL'}   {what}" + (f"  -- {why}"
                                                      if not ok and why
                                                      else ""))
    if not ok:
        fails.append(what)


def hits(body: str) -> list[tuple[int, str, str]]:
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "t.c"
        p.write_text(body)
        return list(scan(p))


print("the shape it is FOR")

check("a doubled field, a temporary, a checked failure, no restore",
      len(hits('''
void f(void) {
	if (p->n >= p->cap) {
		p->cap *= 2;
		tmp = reallocarray(p->arr, p->cap, sizeof(*p->arr));
		if (tmp == NULL)
			return (-1);
		p->arr = tmp;
	}
}
''')) == 1)

check("the same with += and a goto",
      len(hits('''
void f(void) {
	s->size += 4;
	s->idx = realloc(s->idx, s->size * sizeof(x));
	if (s->idx == NULL)
		goto out;
}
''')) == 1)

check("a post-increment counts too",
      len(hits('''
void f(void) {
	sc->npins++;
	sc->pins = malloc(sizeof(*sc->pins) * sc->npins, M_DEVBUF, M_NOWAIT);
	if (sc->pins == NULL)
		return (ENOMEM);
}
''')) == 1)

print()
print("the shapes it must NOT report")

check("a constant size is the ordinary idiom, not a running capacity",
      hits('''
void f(void) {
	sc->size = 8 * 1024;
	sc->buf = malloc(sc->size);
	if (sc->buf == NULL)
		return (-1);
}
''') == [])

check("a bare local is re-derived on the next call",
      hits('''
void f(void) {
	size *= 2;
	tmp = realloc(arr, size);
	if (tmp == NULL)
		return (-1);
	arr = tmp;
}
''') == [])

check("the restore is the other correct answer",
      hits('''
void f(void) {
	p->cap *= 2;
	tmp = realloc(p->arr, p->cap);
	if (tmp == NULL) {
		p->cap /= 2;
		return (-1);
	}
	p->arr = tmp;
}
''') == [])

check("a failure that exits loses nothing",
      hits('''
void f(void) {
	p->cap *= 2;
	p->arr = realloc(p->arr, p->cap);
	if (p->arr == NULL)
		err(1, "realloc");
}
''') == [])

check("no failure arm at all is realloc_self.py's finding, not this one",
      hits('''
void f(void) {
	p->cap *= 2;
	p->arr = realloc(p->arr, p->cap);
	p->arr[0] = 1;
}
''') == [])

check("a grow with no allocation after it is not this rule",
      hits('''
void f(void) {
	p->cap *= 2;
	do_something(p);
	if (p->arr == NULL)
		return (-1);
}
''') == [])

check("an allocation whose size does not mention the grown field",
      hits('''
void f(void) {
	p->cap *= 2;
	tmp = realloc(p->arr, other_size);
	if (tmp == NULL)
		return (-1);
	p->arr = tmp;
}
''') == [])

print()
print("the real tree")

seen = set()
for rel_pre in sorted({k.rsplit(":", 1)[0] for k in EXPECTED}):
    p = SRC / rel_pre
    if not p.exists():
        fails.append(f"{rel_pre} is gone")
        print(f"  FAIL   {rel_pre} is gone")
        continue
    for line, _var, _src in scan(p):
        seen.add(f"{rel_pre}:{line}")

stale = sorted(set(EXPECTED) - seen)
check("no EXPECTED entry has gone stale", not stale, f"stale: {stale}")

check("lib/libproc/proc_rtld.c is fixed and stays quiet",
      not list(scan(SRC / "lib/libproc/proc_rtld.c")))

check("...and the fix is actually there, not just the report gone",
      "phdl->maparrsz * 2,"
      in (SRC / "lib/libproc/proc_rtld.c").read_text())

print()
if fails:
    print(f"{len(fails)} check(s) failed")
    sys.exit(1)
print(f"all checks passed — {len(EXPECTED)} site(s) on the record")
