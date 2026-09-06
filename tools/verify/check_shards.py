#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every directory the sweep should analyse is in a shard of BOTH matrices.

Two checks. Under sys/, every directory must be covered. Above sys/, every
top-level directory of the tree must be EITHER covered OR listed in
UNANALYSED below with a reason -- so a directory nobody analyses is a
decision somebody wrote down, not a gap nobody noticed.

The second check exists because of libexec/rtld-elf. The run-time linker
is the first C code every dynamically linked process runs, and it was in
no shard at all: the scope list was sys/ plus lib/libc plus lib/msun, and
that was never stated as a choice anywhere. rtld.c held a read of an
uninitialised automatic that clang deletes the enclosing function over,
and it cost thirty-seven boot runs. The analyser finds it in under a
minute -- once the file is in a scope, resolves its headers, and is
compiled with the -DHARDENEDBSD the real build uses. All three were
missing, and any one of them alone was enough to hide it.

The sweep runs as one script on a workstation and as a sharded pipeline on
a 4-core runner, and the shards are written out by hand because analyze.py
takes scopes and not exclusions. Hand-written means a directory can be left
out, and a directory left out is a scope that reports ZERO FINDINGS while
looking exactly like one that is clean.

The first version of the shard list missed fourteen, including
sys/hardenedbsd - which is PBSD's own code, and the last place a silent
gap belongs. So this is a gate rather than a comment.

BOTH matrices, since the model checker was sharded too. That job used to
take `--scope sys` and could not have a gap; now it has four shards and
can, which is a new instance of the old hazard rather than a new hazard.
Checking one matrix and not the other would be the same mistake this
tree keeps writing up: the guard on one of a pair.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WF = ROOT / ".github/workflows/pbsd-verify.yml"
SRC = ROOT / "hbsd/src"
SYS = SRC / "sys"

# Top-level directories that are deliberately NOT in a shard, and why.
# A directory here is a decision on the record; a directory in neither
# this table nor a shard fails the gate.
UNANALYSED = {
    "contrib":    "third-party source PBSD does not maintain",
    "crypto":     "third-party source PBSD does not maintain",
    "cddl":       "third-party source PBSD does not maintain",
    "gnu":        "third-party source PBSD does not maintain",
    "kerberos5":  "third-party source PBSD does not maintain",
    "krb5":       "third-party source PBSD does not maintain",
    "secure":     "third-party source PBSD does not maintain",
    "sys":        "checked directory-by-directory above, not as a whole",
    "lib":        "lib/libc and lib/msun are sharded; the rest is not yet",
    "bin":        "not yet - userland utilities, after libexec",
    "sbin":       "not yet - userland utilities, after libexec",
    "usr.bin":    "not yet - userland utilities, after libexec",
    "usr.sbin":   "not yet - userland utilities, after libexec",
    "stand":      "not yet - the loader is its own header universe",
    "tests":      "not yet - exercises the tree rather than being it",
    "tools":      "build tooling, not shipped code",
    "release":    "build tooling, not shipped code",
    "share":      "makefiles and data, no C worth checking",
    "targets":    "build tooling, not shipped code",
    "etc":        "configuration, not C",
    "include":    "headers only; they are checked through their users",
    "rescue":     "no C at all - a Makefile that crunchgens bin/ and sbin/",
}


def main() -> int:
    if not WF.is_file() or not SYS.is_dir():
        print(f"FAIL  cannot find {WF} or {SYS}")
        return 1
    text = WF.read_text()
    top = {f"sys/{p.name}" for p in SYS.iterdir() if p.is_dir()}
    # (job name, the slice of the file its matrix lives in)
    blocks = {
        "analyse": text.split("\n  analyze:", 1)[-1].split("\n  lints:", 1)[0],
        "model-check": text.split("\n  sweep:", 1)[-1].split("\n  analyze:", 1)[0],
    }
    bad = False
    for job, block in blocks.items():
        # Not \S+: the last scope on a quoted YAML line is `sys/cddl' }`
        # and \S+ takes the quote with it, so three directories that were
        # covered read as missing on this script's first run.
        covered = set(re.findall(r"--scope\s+([^\s'\"}]+)", block))
        missing = sorted(
            d for d in top
            if d not in covered and "sys" not in covered
            and not any(d.startswith(c + "/") for c in covered))
        if missing:
            bad = True
            print(f"FAIL  {len(missing)} directory(ies) under sys/ are in no "
                  f"{job} shard:")
            for d in missing:
                print(f"      {d}")
        else:
            print(f"ok    all {len(top)} directories under sys/ are in a "
                  f"{job} shard")
    # And above sys/: covered, or on the record as not covered.
    tree = sorted(p.name for p in SRC.iterdir() if p.is_dir()
                  and not p.name.startswith("."))
    for job, block in blocks.items():
        covered = set(re.findall(r"--scope\s+([^\s'\"}]+)", block))
        roots = {c.split("/", 1)[0] for c in covered}
        undecided = [d for d in tree
                     if d not in roots and d not in UNANALYSED]
        if undecided:
            bad = True
            print(f"FAIL  {len(undecided)} top-level directory(ies) are in "
                  f"no {job} shard and not in UNANALYSED:")
            for d in undecided:
                print(f"      {d}")
        else:
            print(f"ok    every top-level directory is in a {job} shard or "
                  f"on the record ({len(UNANALYSED)} listed)")

    # A stale UNANALYSED entry is its own kind of lie.
    gone = sorted(d for d in UNANALYSED if not (SRC / d).is_dir())
    if gone:
        bad = True
        print(f"FAIL  {len(gone)} UNANALYSED entry(ies) name a directory "
              f"that does not exist: {', '.join(gone)}")

    if bad:
        print("\n      A scope nobody checks reports zero findings and")
        print("      looks exactly like one that is clean. Add it to the")
        print("      right shard in .github/workflows/pbsd-verify.yml, or")
        print("      to UNANALYSED here with the reason it is skipped.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
