#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A ledger of the tree must be a property of the tree, not of the machine.

`tools/port_plan.py` used to be whatever `rglob` found under `hbsd/src`. The
machine that generates it here also carries an ignored checkout of
`hbsd/src/contrib/llvm-project` - the analyser wants LLVM's headers and the
repository does not carry its source - and 8,914 of its files have a source
suffix. So the ledger counted 50,057 files, a fresh clone counted 41,143, and
the CI gate added to compare the two could never pass: the runner regenerated
a correct ledger, found it differed from the committed one, and went red
while nothing was actually stale.

Ask git which files the repository has. The checks here hold that:

  - `tracked()` names the tracked files and not the untracked ones,
  - a file git does not track is left out of the walk,
  - and, off a repository, `tracked()` says None rather than guessing,
    so the tool still runs in a tarball even though the gate needs git.
"""
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import port_plan  # noqa: E402

FAILURES: list[str] = []


def check(ok: bool, what: str) -> None:
    print(("  ok    " if ok else "  FAIL  ") + what)
    if not ok:
        FAILURES.append(what)


def git(cwd: Path, *args: str) -> None:
    subprocess.run(["git", "-C", str(cwd), *args],
                   check=True, capture_output=True)


def main() -> int:
    print("tracked() on a small repository of its own")
    with tempfile.TemporaryDirectory() as td:
        d = Path(td)
        git(d, "init", "-q")
        git(d, "config", "user.email", "t@example.invalid")
        git(d, "config", "user.name", "t")
        (d / "sub").mkdir()
        (d / "sub/kept.c").write_text("int kept(void) { return 0; }\n")
        (d / "sub/never.c").write_text("int never(void) { return 0; }\n")
        (d / ".gitignore").write_text("sub/never.c\n")
        git(d, "add", "sub/kept.c", ".gitignore")
        git(d, "commit", "-qm", "one file")

        old, port_plan.ROOT = port_plan.ROOT, d
        try:
            keep = port_plan.tracked()
        finally:
            port_plan.ROOT = old
        check(keep is not None, "a repository answers")
        check("sub/kept.c" in (keep or ()), "the tracked file is named")
        check("sub/never.c" not in (keep or ()),
              "the ignored file is not, though it is on disk")

    print()
    print("tracked() off a repository")
    with tempfile.TemporaryDirectory() as td:
        # A directory with no .git above it either - /tmp is not a checkout.
        old, port_plan.ROOT = port_plan.ROOT, Path(td)
        try:
            keep = port_plan.tracked()
        finally:
            port_plan.ROOT = old
        check(keep is None, "says None rather than an empty set, which would "
                            "silently empty the ledger")

    print()
    print("the walk, against this checkout")
    root = port_plan.ROOT
    keep = port_plan.tracked()
    if keep is None:
        print("  skip  not a git checkout here")
    else:
        check("tools/port_plan.py" in keep, "the generator is tracked")
        src = root / "hbsd/src/contrib"
        if not src.is_dir():
            print("  skip  no hbsd/src/contrib checkout here")
        else:
            on_disk = [p for p in src.rglob("*")
                       if p.is_file() and p.suffix in port_plan.SOURCE]
            tracked_here = [p for p in on_disk
                            if p.relative_to(root).as_posix() in keep]
            with tempfile.TemporaryDirectory() as td:
                out = Path(td) / "p.md"
                subprocess.run(
                    [sys.executable, str(root / "tools/port_plan.py"),
                     "--areas", "contrib", "--out", str(out),
                     "--json", str(Path(td) / "p.json")],
                    check=True, capture_output=True, cwd=root)
                said = 0
                for line in out.read_text().splitlines():
                    if line.endswith("directories."):
                        said = int(line.split()[0].replace(",", ""))
                        break
            check(said == len(tracked_here),
                  f"contrib is ledgered at its tracked file count "
                  f"({said:,} == {len(tracked_here):,})")
            if len(on_disk) == len(tracked_here):
                print("  skip  nothing untracked under contrib here, so the "
                      "filter cannot be shown to bite")
            else:
                check(said < len(on_disk),
                      f"and not at its on-disk count, which is "
                      f"{len(on_disk):,} - {len(on_disk) - len(tracked_here):,} "
                      f"files git does not track")

    print()
    if FAILURES:
        print(f"{len(FAILURES)} FAILED")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
