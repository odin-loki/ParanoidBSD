#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Carry a sweep's results off the machine that ran it, without the bulk.

A whole-tree run leaves about 85 MB in its output directory, and roughly
83 MB of that is REGENERABLE in under fifteen seconds:

    universe.jsonl     inventory.py rebuilds it from the port ledger
    matrix.jsonl       matrix.py rebuilds it from universe + the stage
                       results, which is the whole point of its design
    *.partial.jsonl    a driver's resume file, meaningless once it finished
    classes.json       classify.py rebuilds it, and only cbmc reads it

What is NOT regenerable is the evidence: the per-stage JSONL that each
instrument actually produced, and `state.json', which says what ran and
what did NOT. Those are what a second reader needs, and they are small.

Two sizes, because two situations
---------------------------------
    --pack      everything irreplaceable, compressed. Attach it, or push
                it to a branch. Tens of MB for a whole-tree run.
    --digest    a few hundred KB: the counts, the stages that did not
                run, the findings nobody has triaged, and the queue for
                a reading pass. Paste-able. It is a SUMMARY and it says
                so - a digest is not evidence and cannot be re-analysed.

The rule the digest obeys
-------------------------
It carries the ABSENCES first. A summary that lists findings and omits
the four instruments that never ran is the same lie in a smaller file,
and the smaller file is the one people actually read.
"""
from __future__ import annotations

import argparse
import collections
import json
import sys
import tarfile
from pathlib import Path

# Regenerable, and how. Printed into the digest so the reader on the far
# end knows the command rather than the file is missing.
REGENERABLE = {
    "universe.jsonl": "python3 tools/verify/inventory.py --out universe.jsonl",
    "matrix.jsonl": "python3 tools/verify/matrix.py --universe universe.jsonl "
                    "--ingest <engine>=<file>.jsonl ... --out matrix.jsonl",
    "classes.json": "python3 tools/verify/classify.py --scope <s> "
                    "--out classes.json",
}
STAGE_FILES = ("cbmc.jsonl", "esbmc.jsonl", "fusebmc.jsonl", "analyze.jsonl",
               "tidy.jsonl", "cppcheck.jsonl", "cocci.jsonl",
               "warnings.jsonl", "cxx_analyze.jsonl", "cxx_tidy.jsonl")
KEEP_ALSO = ("state.json", "rates.json")


def _load(p: Path):
    for line in p.open():
        line = line.strip()
        if line:
            yield json.loads(line)


def digest(d: Path) -> dict:
    """The small thing. Absences first, on purpose."""
    out = {"dir": str(d), "stages": {}, "did_not_run": [], "totals": {}}

    st = d / "state.json"
    if st.is_file():
        state = json.loads(st.read_text())
        for name, r in state.get("stages", {}).items():
            s = r.get("status")
            if s == "ok":
                out["stages"][name] = {"status": s,
                                       "seconds": r.get("seconds")}
            else:
                out["did_not_run"].append({
                    "stage": name, "status": s,
                    "why": r.get("why", ""),
                    "kind_mismatch": bool(r.get("kind_mismatch")),
                })

    verdicts = collections.Counter()
    checkers = collections.Counter()
    tu = collections.Counter()
    untriaged = []
    for f in STAGE_FILES:
        p = d / f
        if not p.is_file():
            continue
        n = 0
        for rec in _load(p):
            if rec.get("_meta"):
                continue
            n += 1
            if "status" in rec and "function" in rec:          # per function
                verdicts[rec["status"]] += 1
                if rec["status"] == "FAILED":
                    for fail in (rec.get("failures") or [])[:1]:
                        untriaged.append(
                            f"{rec['file']}:{rec['function']}  "
                            f"{fail.get('desc', '')[:100]}")
            elif "status" in rec:                               # per TU
                tu[rec["status"]] += 1
                for g in rec.get("findings") or ():
                    checkers[g.get("checker", "?")] += 1
        out["totals"][f] = n

    out["verdicts"] = dict(verdicts)
    out["tu_status"] = dict(tu)
    out["top_checkers"] = dict(checkers.most_common(40))
    out["failed_sample"] = untriaged[:400]
    out["_note"] = (
        "A SUMMARY, not evidence. It cannot be re-analysed and it cannot "
        "be ingested by matrix.py. `did_not_run' is listed FIRST and in "
        "full, because a digest that lists findings and omits the "
        "instruments that never ran is the same lie in a smaller file - "
        "and the smaller file is the one people actually read.")
    out["_regenerable"] = REGENERABLE
    return out


def pack(d: Path, out: Path) -> tuple[int, int, list[str]]:
    kept, dropped = [], []
    with tarfile.open(out, "w:gz") as tf:
        for p in sorted(d.iterdir()):
            if not p.is_file():
                continue
            if p.name in REGENERABLE or p.name.endswith(".partial.jsonl"):
                dropped.append(p.name)
                continue
            tf.add(p, arcname=f"sweep/{p.name}")
            kept.append(p.name)
        # The regeneration recipe travels WITH the tarball, so the far
        # end is not guessing which command rebuilds what.
        note = d / ".pack-note.json"
        note.write_text(json.dumps(
            {"dropped_because_regenerable": {k: REGENERABLE.get(k, "resume "
              "file, meaningless once the driver finished") for k in dropped},
             "kept": kept}, indent=1))
        tf.add(note, arcname="sweep/HOW-TO-REBUILD.json")
        note.unlink()
    return out.stat().st_size, sum((d / k).stat().st_size for k in kept), dropped


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("sweep_dir", help="the --out directory of a sweep_all run")
    ap.add_argument("--pack", metavar="FILE.tar.gz",
                    help="everything irreplaceable, compressed")
    ap.add_argument("--digest", metavar="FILE.json",
                    help="a few hundred KB, paste-able")
    args = ap.parse_args(argv)

    d = Path(args.sweep_dir)
    if not d.is_dir():
        print(f"pack_results: {d} is not a directory", file=sys.stderr)
        return 2
    if not (args.pack or args.digest):
        ap.error("give --pack, --digest, or both")

    if args.digest:
        g = digest(d)
        Path(args.digest).write_text(json.dumps(g, indent=1))
        kb = Path(args.digest).stat().st_size / 1024
        print(f"digest  -> {args.digest}  ({kb:.0f} KB)")
        if g["did_not_run"]:
            print(f"  {len(g['did_not_run'])} stage(s) produced no data:")
            for s in g["did_not_run"]:
                tag = " (no source of its kind - not a gap)" \
                    if s["kind_mismatch"] else ""
                print(f"    {s['stage']:14} {s['status']}{tag}")
        else:
            print("  every stage ran.")

    if args.pack:
        size, raw, dropped = pack(d, Path(args.pack))
        print(f"pack    -> {args.pack}  "
              f"({size / 1e6:.1f} MB, from {raw / 1e6:.1f} MB raw)")
        print(f"  dropped {len(dropped)} regenerable file(s): "
              f"{' '.join(dropped)}")
        print("  the rebuild commands travel inside, as HOW-TO-REBUILD.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
