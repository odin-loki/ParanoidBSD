#!/usr/bin/env python3
"""CLI: run todo.md automated C→C++23 port passes."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
if str(ROOT / "tools") not in sys.path:
    sys.path.insert(0, str(ROOT / "tools"))

from pbsd_passes.runner import run_corpus_tests, run_pipeline  # noqa: E402


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--scope",
        default="bin,usr.bin,sbin,lib,usr.sbin,libexec",
        help="Comma-separated under hbsd/src",
    )
    ap.add_argument("--limit", type=int, default=None)
    ap.add_argument("--file", action="append", default=[], help="Specific file(s) relative to repo root")
    ap.add_argument("--tiers", default="0,1,2,3,4", help="Comma-separated tier numbers")
    ap.add_argument("--corpus-only", action="store_true")
    ap.add_argument("--no-ir", action="store_true")
    ap.add_argument("--no-diff", action="store_true")
    ap.add_argument("--all-passes", action="store_true", help="Alias: all tiers")
    ap.add_argument("--tidy", action="store_true", help="Run clang-tidy -fix on staged outputs")
    ap.add_argument("--tidy-limit", type=int, default=120)
    args = ap.parse_args(argv)

    if args.corpus_only:
        result = run_corpus_tests()
        print(json.dumps(result, indent=2))
        return 0 if result.get("ok") else 1

    tiers = {int(x) for x in args.tiers.split(",") if x.strip() != ""}
    if args.all_passes:
        tiers = {0, 1, 2, 3, 4}

    files = None
    if args.file:
        files = [(ROOT / f).resolve() for f in args.file]

    scopes = [s.strip() for s in args.scope.split(",") if s.strip()]
    report = run_pipeline(
        scopes=scopes,
        limit=args.limit,
        tiers=tiers,
        do_ir=not args.no_ir,
        do_diff=not args.no_diff,
        files=files,
        do_tidy=args.tidy,
        tidy_limit=args.tidy_limit,
    )
    print(
        f"OK files={report['files']} edits={report['edits_total']} "
        f"refusals={report['refusals_total']} "
        f"report=docs/migration/clang_port/todo_pass_report.md"
    )
    # Print top reasons
    for reason, count in list(report["reason_histogram"].items())[:12]:
        print(f"  {reason}: {count}")
    if report.get("clang_tidy"):
        ct = report["clang_tidy"]
        print(f"  clang-tidy: targets={ct.get('targets')} warnings={ct.get('warning_sum')} fix={ct.get('fix')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
