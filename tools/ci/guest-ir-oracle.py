#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Run the IR oracle on every hbsd_cpp twin in ir-transfer-queue.jsonl.

Invoked inside the FreeBSD QEMU guest. Writes one JSONL record per file so a
killed run can resume. Forces compare_ir even when the pipeline would mark a
TU skipped_heavy: on FreeBSD the sys headers are native.
"""
from __future__ import annotations

import json
import os
import sys
import traceback
from pathlib import Path

ROOT = Path(os.environ.get("PBSD_ROOT", "/mnt/pbsd")).resolve()
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "tools"))
os.environ.setdefault("PYTHONHASHSEED", "0")

from pbsd_passes.ir_oracle import compare_ir  # noqa: E402
from pbsd_passes.runner import oracle_include_flags, process_file, stage_path_for  # noqa: E402

# After encodings is loaded. chdir before import breaks FreeBSD python on 9p.
os.chdir(ROOT)

QUEUE = ROOT / "docs/verify/sweep-2026-09-18/queue/ir-transfer-queue.jsonl"
OUT = Path(os.environ.get("PBSD_ORACLE_OUT", "/mnt/sweep/ir-oracle-qemu.jsonl"))


def load_queue() -> list[dict]:
    rows = []
    for line in QUEUE.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        rec = json.loads(line)
        if rec.get("_meta"):
            continue
        if rec.get("kind") != "hbsd_cpp":
            continue
        rows.append(rec)
    return rows


def already_done(path: Path) -> set[str]:
    done: set[str] = set()
    if not path.exists():
        return done
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            rec = json.loads(line)
        except json.JSONDecodeError:
            continue
        if rec.get("file") and rec.get("status") not in (
            "error",
            "missing",
            "missing_source",
            "compile_fail",
        ):
            done.add(rec["file"])
    return done


def main() -> int:
    print("== toolchain", flush=True)
    os.system("clang --version | head -1")
    os.system("clang++ --version | head -1")
    os.system("python3 --version")
    os.system("uname -a")
    print(f"ROOT={ROOT}", flush=True)
    print(f"QUEUE={QUEUE} exists={QUEUE.is_file()}", flush=True)
    print(f"OUT={OUT}", flush=True)

    rows = load_queue()
    print(f"== {len(rows)} hbsd_cpp twins", flush=True)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    done = already_done(OUT)
    print(f"== already recorded: {len(done)}", flush=True)

    equal_n = abi_n = 0
    with OUT.open("a", encoding="utf-8") as fh:
        for rec in rows:
            rel_file = rec["file"]
            if rel_file in done:
                print(f"  skip {rel_file} (already in jsonl)", flush=True)
                continue
            src_rel = rec.get("twin", {}).get("hbsd_cpp") or f"hbsd/src/{rel_file}"
            src = ROOT / src_rel
            row = {
                "file": rel_file,
                "source": src_rel,
                "kind": rec.get("kind"),
                "equal": False,
                "abi_equal": False,
                "status": "missing",
            }
            try:
                if not src.is_file():
                    row["status"] = "missing_source"
                    row["error"] = str(src)
                else:
                    result = process_file(
                        src,
                        tiers={0, 1, 2, 3, 4},
                        do_ir=True,
                        do_diff=False,
                    )
                    ir = result.get("ir") or {}
                    staged = result.get("staged") or ""
                    dest = (ROOT / staged) if staged else stage_path_for(src)
                    # On FreeBSD, force the oracle even if the Linux-oriented
                    # heavy/sys-header skip fired.
                    if ir.get("status") not in ("ok", "mismatch", "compile_fail", "timeout"):
                        if dest.is_file():
                            ir = compare_ir(
                                src,
                                dest,
                                include_flags=oracle_include_flags(src),
                            )
                            result["ir_forced"] = True
                    row.update(
                        {
                            "equal": bool(ir.get("equal")),
                            "abi_equal": bool(ir.get("abi_equal")),
                            "status": ir.get("status") or "unknown",
                            "edits": result.get("edits", 0),
                            "staged": staged,
                            "ir_forced": bool(result.get("ir_forced")),
                            "ir": {key: ir[key] for key in (
                                    "equal",
                                    "abi_equal",
                                    "status",
                                    "c_ok",
                                    "cxx_ok",
                                    "c_err",
                                    "cxx_err",
                                    "abi_only_in_c",
                                    "abi_only_in_cxx",
                                    "c_hash",
                                    "cxx_hash",
                                    "c_lines",
                                    "cxx_lines",
                                    "diff",
                                    "target_flags",
                                ) if key in ir},
                        }
                    )
            except Exception as exc:  # keep going; one TU must not kill the run
                row["status"] = "error"
                row["error"] = f"{type(exc).__name__}: {exc}"
                row["traceback"] = traceback.format_exc()[-2000:]
            fh.write(json.dumps(row) + "\n")
            fh.flush()
            if row["equal"]:
                equal_n += 1
            if row["abi_equal"]:
                abi_n += 1
            print(
                f"  {row['status']:14} equal={int(row['equal'])} "
                f"abi={int(row['abi_equal'])}  {rel_file}",
                flush=True,
            )

    print(f"== done equal={equal_n} abi_equal={abi_n} of {len(rows)}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
