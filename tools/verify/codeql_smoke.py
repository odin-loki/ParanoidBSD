#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""One-TU CodeQL database + query. Presence is not a tree taint run.

taxonomy.py rates codeql FINDS for TRUST-UNVALIDATED-INPUT when the
binary is on PATH. This script is the cheaper honesty check: can the
extractor actually see a FreeBSD TU on this Linux host, using the same
include_flags() the rest of the harness uses.

Default source is lib/libc/stdlib/abs.c (already CBMC PROVED). Output
is JSONL under --out. Do not quote a one-TU smoke as kernel coverage.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
SRC = ROOT / "hbsd" / "src"
sys.path.insert(0, str(HERE))
import includes  # noqa: E402

SMOKE_REL = "lib/libc/stdlib/abs.c"


def smoke_ql(symbol: str, rel: str) -> str:
    base = Path(rel).name
    return (
        "import cpp\n\n"
        "from Function f\n"
        f'where f.getName() = "{symbol}" '
        f'and f.getFile().getRelativePath().matches("%{base}")\n'
        f'select f, "smoke: CodeQL sees {symbol}"\n'
    )


def build_script(rel: str, flags: list[str], clang: str,
                 extra: list[str] | None = None) -> str:
    """Compile the TU from hbsd/src. Do not copy it into an empty tree.

    include_flags() already assume the real source root. Extra flags are
    for a one-TU option that opt_shim() leaves off (e.g. -DFFCLOCK):
    kern_ffclock.c is `standard` in sys/conf/files but HARDENEDBSD does
    not set FFCLOCK, so the extracted body is `return (ENOSYS)` unless
    the option is forced.
    """
    import shlex
    cmd = [clang, "-c", rel, "-o", "/tmp/codeql-smoke.o", *flags, *(extra or [])]
    quoted = " ".join(shlex.quote(x) for x in cmd)
    return (
        "#!/bin/sh\nset -e\n"
        f"cd {shlex.quote(str(SRC))}\n"
        f"{quoted}\n"
    )


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    print("+", " ".join(cmd), flush=True)
    return subprocess.run(cmd, text=True, capture_output=True, **kw)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--rel", default=SMOKE_REL)
    ap.add_argument("--symbol", default="abs")
    ap.add_argument("--db", default=str(Path.home() / "pbsd-sweep" / "codeql-smoke-db"))
    ap.add_argument("--out", default=str(Path.home() / "pbsd-sweep" / "codeql-smoke.jsonl"))
    ap.add_argument("--arch", default="amd64")
    ap.add_argument("--extra-cflag", action="append", default=[],
                    help="extra clang flags; write --extra-cflag=-DFFCLOCK")
    args = ap.parse_args()

    expect = f"smoke: CodeQL sees {args.symbol}"
    ql_text = smoke_ql(args.symbol, args.rel)

    codeql = shutil.which("codeql")
    rec = {
        "v": 1,
        "file": args.rel,
        "symbol": args.symbol,
        "instrument": "codeql",
        "codeql": codeql,
    }
    if not codeql:
        rec["status"] = "ERROR"
        rec["detail"] = "codeql not on PATH"
        Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
        print(json.dumps(rec), flush=True)
        return 1

    src = SRC / args.rel
    if not src.is_file():
        rec["status"] = "ERROR"
        rec["detail"] = f"missing {src}"
        Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
        return 1

    flags = includes.include_flags(src, arch=args.arch, cc="clang")
    clang = shutil.which("clang") or "clang"
    db = Path(args.db)
    if db.exists():
        shutil.rmtree(db)
    db.parent.mkdir(parents=True, exist_ok=True)
    rec["extra_cflag"] = list(args.extra_cflag)
    wrapper = db.parent / "codeql-smoke-build.sh"
    wrapper.write_text(
        build_script(args.rel, flags, clang, extra=args.extra_cflag),
        encoding="utf-8")
    wrapper.chmod(0o755)

    p = run([codeql, "database", "create", str(db),
             "--language=c-cpp", "--source-root", str(SRC),
             "--command", str(wrapper)])
    rec["create_rc"] = p.returncode
    rec["create_err"] = ((p.stderr or "") + (p.stdout or ""))[-800:]
    if p.returncode != 0:
        rec["status"] = "ERROR"
        rec["detail"] = "database create failed"
        Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
        print(json.dumps({k: rec[k] for k in rec if k != "create_err"}), flush=True)
        print(rec["create_err"], flush=True)
        return 1

    qdir = Path(args.out).with_suffix(".ql")
    qdir.mkdir(parents=True, exist_ok=True)
    (qdir / "qlpack.yml").write_text(
        "name: pbsd-codeql-smoke\nversion: 0.0.1\ndependencies:\n"
        "  codeql/cpp-all: \"*\"\n",
        encoding="utf-8")
    (qdir / "smoke.ql").write_text(ql_text, encoding="utf-8")
    pack = run([codeql, "pack", "install", str(qdir)])
    rec["pack_rc"] = pack.returncode
    rec["pack_err"] = ((pack.stderr or "") + (pack.stdout or ""))[-400:]
    q = run([codeql, "query", "run", str(qdir / "smoke.ql"),
             "--database", str(db)])
    rec["query_rc"] = q.returncode
    rec["query_out"] = ((q.stdout or "") + (q.stderr or ""))[-800:]
    if q.returncode == 0 and expect in (q.stdout or ""):
        rec["status"] = "SMOKE-OK"
    elif q.returncode == 0:
        rec["status"] = "SMOKE-EMPTY"
    else:
        rec["status"] = "ERROR"
        rec["detail"] = "query run failed"
    Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
    print("status", rec["status"], flush=True)
    print(rec.get("query_out", "")[-400:], flush=True)
    return 0 if rec["status"].startswith("SMOKE") else 1


if __name__ == "__main__":
    os.chdir(ROOT)
    raise SystemExit(main())
