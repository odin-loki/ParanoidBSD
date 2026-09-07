#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Hold docs/migration/freebsd_oracle_removed.txt to what git can settle.

Every entry in that file adds one to a floor's sum, which makes it the one
place where a number can be held up by a claim rather than by a
measurement. So the claim is checked, and it is checked against the
repository rather than against itself:

  * the path is not in the tree now      - or it is not removed, and the
                                           oracle is counting it twice
  * it was there at the commit's parent  - or the commit is not the one
                                           that removed it
  * it was gone at the commit itself     - same, from the other side
  * the commit is an ancestor of HEAD    - or it names a branch this tree
                                           does not contain

Those four make an entry unforgeable in the direction that matters: you
cannot keep a floor up over a file that still exists, and you cannot blame
a deletion on a commit that did not do it.
"""
from __future__ import annotations
import subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RECORD = ROOT / "docs" / "migration" / "freebsd_oracle_removed.txt"
LEGAL_COUNTED = {"ir", "ir,abi"}


def parse(text: str) -> list[dict]:
    """The record's lines.

    run_freebsd_oracle.sh imports this rather than parsing the file again,
    so the gate below cannot end up checking a file the oracle is reading
    differently.
    """
    out = []
    for n, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split(None, 3)
        out.append({
            "line": n,
            "path": parts[0] if parts else "",
            "commit": parts[1] if len(parts) > 1 else "",
            "counted": parts[2] if len(parts) > 2 else "",
            "reason": parts[3] if len(parts) > 3 else "",
        })
    return out


def entries() -> list[dict]:
    """Every recorded removal, or nothing if the record does not exist."""
    return parse(RECORD.read_text()) if RECORD.exists() else []


def counted_for(scope: str) -> tuple[list[str], list[str]]:
    """The removals under `scope', split by which floor had counted them.

    Returns (ir, abi) as lists of paths rather than counts, because a
    number added to a ratchet that nobody can see the makeup of is exactly
    the thing this file exists to avoid.
    """
    ir, abi = [], []
    pre = scope.rstrip("/") + "/"
    for e in entries():
        if not e["path"].startswith(pre) or e["counted"] not in LEGAL_COUNTED:
            continue
        ir.append(e["path"])
        if e["counted"] == "ir,abi":
            abi.append(e["path"])
    return ir, abi


def git(*args: str) -> tuple[int, str]:
    p = subprocess.run(("git", "-C", str(ROOT)) + args,
                       capture_output=True, text=True)
    return p.returncode, (p.stdout + p.stderr).strip()


_deepened = False


def have(commit: str) -> bool:
    """Is this commit in the local object store, deepening if it is not?

    actions/checkout clones at depth 1, so in CI the commit an entry names
    is simply absent and every check below would fail for a reason that has
    nothing to do with the entry. Deepen once, on demand, rather than
    making every job pay for a full history: the record's commits are
    recent by construction, since a source cannot be removed before it was
    counted.
    """
    global _deepened
    if git("rev-parse", "--verify", "--quiet", f"{commit}^{{commit}}")[0] == 0:
        return True
    if not _deepened:
        _deepened = True
        # Two steps rather than --unshallow: the tree is large and the
        # second only runs if the first was not enough.
        for n in (250, 2000):
            git("fetch", "--quiet", f"--deepen={n}")
            if git("rev-parse", "--verify", "--quiet",
                   f"{commit}^{{commit}}")[0] == 0:
                break
    return git("rev-parse", "--verify", "--quiet",
               f"{commit}^{{commit}}")[0] == 0


def at_rev(rev: str, path: str) -> bool:
    """Is `path' present in `rev''s tree?

    rev-parse rather than cat-file -e: it resolves the path to an object id
    without needing the blob itself, which a partial clone would otherwise
    go and fetch one file at a time.
    """
    return git("rev-parse", "--verify", "--quiet", f"{rev}:{path}")[0] == 0


def main() -> int:
    if not RECORD.exists():
        print(f"{RECORD.relative_to(ROOT)}: absent, nothing to check")
        return 0
    recorded = entries()
    fails: list[str] = []

    def check(what: str, ok: bool, why: str = "") -> None:
        if ok:
            print(f"  ok   {what}")
        else:
            print(f"  FAIL {what}{'  ' + why if why else ''}")
            fails.append(what)

    seen: dict[str, int] = {}
    for e in recorded:
        p, c, tag, why = e["path"], e["commit"], e["counted"], e["reason"]
        at = f"line {e['line']}"

        check(f"{at}: {p} has all four fields",
              bool(p and c and tag and why),
              "path, commit, counted and a reason are all required")
        if not (p and c and tag):
            continue

        check(f"{at}: counted={tag} is legal", tag in LEGAL_COUNTED,
              f"expected one of {sorted(LEGAL_COUNTED)} - ABI-equal implies "
              f"IR-equal, so `abi' alone is not a thing the report can say")

        first = seen.setdefault(p, e["line"])
        check(f"{at}: {p} is listed once", first == e["line"],
              f"already listed at line {first}; it would be counted twice")

        check(f"{at}: {p} is not in the tree", not (ROOT / p).exists(),
              "it is back, so the oracle discovers it again and this entry "
              "adds a second count for it")

        if not have(c):
            check(f"{at}: {c} is in this clone", False,
                  "not found even after deepening the history - the entry "
                  "names a commit this repository does not have")
            continue

        rc, _ = git("merge-base", "--is-ancestor", c, "HEAD")
        check(f"{at}: {c} is an ancestor of HEAD", rc == 0,
              "this tree does not contain the commit the entry blames")

        check(f"{at}: {p} existed at {c}^", at_rev(f"{c}^", p),
              "it was not there before the commit that is said to have "
              "removed it")

        check(f"{at}: {p} was gone at {c}", not at_rev(c, p),
              "that commit did not remove it - the entry names the wrong "
              "commit")

    print()
    if fails:
        print(f"{len(fails)} check(s) failed")
        return 1
    ir = sum(1 for e in recorded if e["counted"] in LEGAL_COUNTED)
    abi = sum(1 for e in recorded if e["counted"] == "ir,abi")
    print(f"all checks passed - {len(recorded)} removed source(s), "
          f"{ir} counted toward an IR floor, {abi} toward an ABI floor")
    return 0


if __name__ == "__main__":
    sys.exit(main())
