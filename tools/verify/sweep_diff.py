#!/usr/bin/env python3
"""Diff two sweeps without being fooled by line numbers.

Every fix in this work is measured as a before/after at the same scope
on the same tree, and the natural way to diff the two jsonl files is by
the finding's `where' -- file and line.  That is wrong whenever the fix
inserted a comment, which every fix here does: ctladm's two-line change
carried a thirteen-line comment, and all forty-eight findings below it
moved, so a diff by `where' showed 48 gone and 48 new around a real
delta of 2.  It has been hand-checked three times; this does it.

A finding is identified by (file, checker, function, message) instead,
which is stable under insertion and still distinguishes the cases that
matter -- two findings of the same checker in the same function differ
in their message often enough, and where they do not, the count does.
"""

import argparse
import collections
import json
import sys


def load(path):
    c = collections.Counter()
    ok = err = 0
    with open(path) as fh:
        for i, line in enumerate(fh):
            if i == 0:
                continue
            try:
                r = json.loads(line)
            except ValueError:
                continue
            ok += r.get("status") == "OK"
            err += r.get("status") == "ERROR"
            for f in r.get("findings", []):
                c[(r.get("file", ""), f.get("checker", ""),
                   f.get("fn", ""), f.get("msg", ""))] += 1
    return c, ok, err


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("before")
    ap.add_argument("after")
    ap.add_argument("--quiet", action="store_true",
                    help="print only the counts")
    args = ap.parse_args()

    b, bok, berr = load(args.before)
    a, aok, aerr = load(args.after)
    nb, na = sum(b.values()), sum(a.values())

    print(f"before  {nb:>4} finding(s)  OK {bok}  ERROR {berr}")
    print(f"after   {na:>4} finding(s)  OK {aok}  ERROR {aerr}")
    if bok != aok or berr != aerr:
        print("\n!! the translation-unit counts moved: the two sides are not "
              "the same measurement", file=sys.stderr)

    if not args.quiet:
        gone, new = b - a, a - b
        if gone:
            print("\n--- closed ---")
            for k, n in sorted(gone.items()):
                print(f"  {n}x  {k[0]}  {k[1]}  {k[2]}(): {k[3][:70]}")
        if new:
            print("\n--- appeared ---")
            for k, n in sorted(new.items()):
                print(f"  {n}x  {k[0]}  {k[1]}  {k[2]}(): {k[3][:70]}")
        if not gone and not new:
            print("\nno change")
    return 0


if __name__ == "__main__":
    sys.exit(main())
