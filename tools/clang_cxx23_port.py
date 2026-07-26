#!/usr/bin/env python3
"""
PBSD Clang-first C → C++23 port automation.

Deterministic pipeline (no LLM judgment):
  inventory  — refresh c_inventory.csv
  query      — AST / regex pattern census for risk + envelope hints
  tidy       — clang-tidy modernize / ownership checks (optional --fix)
  deps       — clang-scan-deps / include graph samples for queue ordering
  queue      — write leaf-ish queue.json from inventory + query scores
  report     — markdown + JSON summary under docs/migration/clang_port/

Usage (repo root, preferably WSL with Clang 18):
  python3 tools/clang_cxx23_port.py --phase all
  python3 tools/clang_cxx23_port.py --phase query --limit 100
  python3 tools/clang_cxx23_port.py --phase tidy --fix --limit 40
"""
from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import sys
from collections import Counter, defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "docs" / "migration" / "clang_port"
INVENTORY = ROOT / "docs" / "migration" / "c_inventory.csv"
QUERY_JSON = OUT / "ast_census.json"
QUEUE_JSON = OUT / "queue.json"
REPORT_MD = OUT / "REPORT.md"
TIDY_JSON = OUT / "tidy_results.json"
DEPS_JSON = OUT / "deps_sample.json"
CONFIG_DIR = ROOT / "tools" / "clang_port"

# Patterns that are cheap, deterministic, and high-signal for port triage.
# Full clang-query is used when available; these regexes always run as fallback.
PATTERN_RULES: list[tuple[str, str, int, str]] = [
    # name, regex, risk_weight, envelope_hint
    ("malloc_family", r"\b(malloc|calloc|realloc|free|reallocf)\s*\(", 2, "n/a"),
    ("goto", r"\bgoto\b", 1, "n/a"),
    ("void_star", r"\bvoid\s*\*", 1, "n/a"),
    ("tailq", r"\b(TAILQ_|LIST_|STAILQ_|SLIST_|RB_|SPLAY_)\w*", 2, "B"),
    ("mutex_lock", r"\b(mtx_lock|mtx_unlock|rw_rlock|rw_wlock|sx_xlock|sx_slock)\b", 3, "C"),
    ("atomic", r"\b(atomic_|__atomic_|__sync_)\w*", 2, "A"),
    ("copyin_out", r"\b(copyin|copyout|copyinstr|fuiword|suword)\b", 3, "n/a"),
    ("intr_handler", r"\b(intr_|bus_setup_intr|DRIVER_MODULE)\b", 3, "A"),
    ("syscall_impl", r"\b(SYS[A-Z0-9_]*|sys_[a-z0-9_]+)\b", 2, "n/a"),
    ("flexible_array", r"\[\s*\]\s*;", 2, "n/a"),
    ("typeof_gnu", r"\b(__typeof__|typeof)\b", 1, "n/a"),
    ("statement_expr", r"\(\s*\{", 2, "n/a"),
    ("vl_array", r"\[[^\]]+\]\s*;", 1, "n/a"),  # noisy; scored lightly
    ("union_pun", r"\bunion\b", 1, "n/a"),
    ("static_global", r"^\s*static\s+(?!inline\b)(?!const\b)", 1, "C"),
]

CLANG_QUERY_MATCHERS = [
    ("callExpr(callee(functionDecl(hasName(\"malloc\"))))", "malloc_call"),
    ("gotoStmt()", "goto_stmt"),
    ("varDecl(hasType(pointerType(pointee(voidType()))))", "void_ptr_var"),
]


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def which_tool(*names: str) -> str | None:
    for n in names:
        p = shutil.which(n)
        if p:
            return p
    return None


def find_clang_tools() -> dict[str, str | None]:
    return {
        "clang": which_tool("clang-18", "clang"),
        "clangxx": which_tool("clang++-18", "clang++"),
        "clang_tidy": which_tool("clang-tidy-18", "clang-tidy"),
        "clang_query": which_tool("clang-query-18", "clang-query"),
        "clang_format": which_tool("clang-format-18", "clang-format"),
        "clang_scan_deps": which_tool("clang-scan-deps-18", "clang-scan-deps"),
        "clang_apply": which_tool(
            "clang-apply-replacements-18", "clang-apply-replacements"
        ),
    }


def ensure_out() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    CONFIG_DIR.mkdir(parents=True, exist_ok=True)


def load_inventory(limit: int | None = None) -> list[dict[str, str]]:
    if not INVENTORY.exists():
        run_inventory()
    with INVENTORY.open(encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f))
    if limit is not None:
        rows = rows[:limit]
    return rows


def run_inventory() -> None:
    script = ROOT / "tools" / "inventory_c_sources.py"
    subprocess.run([sys.executable, str(script)], cwd=ROOT, check=True)


def score_file(path: Path) -> dict:
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        return {"path": str(path), "error": str(e), "score": 0, "hits": {}}

    hits: dict[str, int] = {}
    score = 0
    envelope_votes: Counter[str] = Counter()
    for name, pattern, weight, env in PATTERN_RULES:
        # multiline for static_global
        flags = re.M if name == "static_global" else 0
        n = len(re.findall(pattern, text, flags))
        if n:
            hits[name] = n
            score += n * weight
            if env != "n/a":
                envelope_votes[env] += n * weight

    risk = 3
    if score >= 40 or any(k in hits for k in ("mutex_lock", "copyin_out", "intr_handler")):
        risk = 1
    elif score >= 12:
        risk = 2

    envelope = "C"
    if envelope_votes:
        envelope = envelope_votes.most_common(1)[0][0]

    return {
        "path": path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else path.as_posix(),
        "score": score,
        "risk_tier": risk,
        "envelope_hint": envelope,
        "hits": hits,
        "loc": text.count("\n") + 1,
    }


def phase_query(limit: int | None, jobs: int) -> dict:
    rows = load_inventory(limit)
    results: list[dict] = []
    paths = []
    for r in rows:
        p = ROOT / r["path"]
        if p.exists() and p.suffix.lower() in {".c", ".cc", ".cxx", ".cpp"}:
            paths.append(p)

    with ThreadPoolExecutor(max_workers=max(1, jobs)) as ex:
        futs = {ex.submit(score_file, p): p for p in paths}
        for fut in as_completed(futs):
            results.append(fut.result())

    results.sort(key=lambda x: (-x.get("score", 0), x.get("path", "")))

    # Optional clang-query sample on top-N hot files (expensive).
    tools = find_clang_tools()
    query_samples: list[dict] = []
    cq = tools["clang_query"]
    clang = tools["clang"]
    if cq and clang and results:
        sample = [r for r in results if r.get("hits")][: min(8, len(results))]
        qfile = CONFIG_DIR / "port_matchers.cquery"
        qfile.write_text(
            "set bind-root false\n"
            + "\n".join(f"match {m}" for m, _ in CLANG_QUERY_MATCHERS)
            + "\n",
            encoding="utf-8",
        )
        for r in sample:
            src = ROOT / r["path"]
            # Non-interactive: -c commands; parse as C with best-effort includes.
            cmd = [cq, str(src), "-c", "set bind-root false"]
            for m, _ in CLANG_QUERY_MATCHERS:
                cmd.extend(["-c", f"match {m}"])
            cmd.extend(
                [
                    "--",
                    "-std=c17",
                    "-ferror-limit=0",
                    "-Wno-everything",
                    f"-I{ROOT / 'hbsd' / 'src' / 'sys'}",
                    f"-I{ROOT / 'hbsd' / 'src' / 'include'}",
                ]
            )
            try:
                proc = subprocess.run(
                    cmd,
                    cwd=ROOT,
                    capture_output=True,
                    text=True,
                    timeout=60,
                )
                query_samples.append(
                    {
                        "path": r["path"],
                        "returncode": proc.returncode,
                        "stdout_tail": (proc.stdout or "")[-2000:],
                        "stderr_tail": (proc.stderr or "")[-1000:],
                    }
                )
            except (subprocess.TimeoutExpired, OSError) as e:
                query_samples.append({"path": r["path"], "error": str(e)})

    payload = {
        "generated": utc_now(),
        "files_scored": len(results),
        "tools": {k: (v is not None) for k, v in tools.items()},
        "top_hot": results[:50],
        "by_risk": {
            "1": sum(1 for r in results if r.get("risk_tier") == 1),
            "2": sum(1 for r in results if r.get("risk_tier") == 2),
            "3": sum(1 for r in results if r.get("risk_tier") == 3),
        },
        "envelope_hints": dict(
            Counter(r.get("envelope_hint", "C") for r in results)
        ),
        "pattern_totals": dict(
            Counter(
                name
                for r in results
                for name, n in (r.get("hits") or {}).items()
                for _ in range(min(n, 50))  # cap contribution
            )
        ),
        "clang_query_samples": query_samples,
        "all": results,
    }
    QUERY_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    # Slim index for queue without dumping every hit detail twice
    slim = [{k: r[k] for k in ("path", "score", "risk_tier", "envelope_hint", "loc") if k in r} | {"hits": r.get("hits", {})} for r in results]
    (OUT / "ast_census_slim.json").write_text(
        json.dumps({"generated": utc_now(), "files": slim}, indent=2) + "\n",
        encoding="utf-8",
    )
    return payload


def find_compile_commands() -> Path | None:
    candidates = [
        ROOT / "compile_commands.json",
        ROOT / "build" / "compile_commands.json",
        ROOT / "pbsd" / "build" / "compile_commands.json",
        ROOT / "build-userland" / "compile_commands.json",
    ]
    for c in candidates:
        if c.exists():
            return c
    return None


def collect_tidy_targets(limit: int | None) -> list[Path]:
    targets: list[Path] = []
    for base in (ROOT / "pbsd",):
        if not base.exists():
            continue
        for ext in ("*.cpp", "*.cc", "*.cxx", "*.cppm"):
            targets.extend(base.rglob(ext))
    # Prefer nucleus over mass stubs
    def rank(p: Path) -> tuple[int, str]:
        s = p.as_posix()
        if "/ports/" in s:
            return (2, s)
        if any(x in s for x in ("/handles/", "/core/", "/lineage/", "/analyser/")):
            return (0, s)
        return (1, s)

    targets = sorted(set(targets), key=rank)
    if limit is not None:
        targets = targets[:limit]
    return targets


def phase_tidy(limit: int | None, fix: bool, jobs: int) -> dict:
    tools = find_clang_tools()
    tidy = tools["clang_tidy"]
    if not tidy:
        payload = {
            "generated": utc_now(),
            "skipped": True,
            "reason": "clang-tidy not found (install clang-tidy-18)",
            "results": [],
        }
        TIDY_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        return payload

    config = ROOT / ".clang-tidy"
    # Port-oriented extra checks config
    port_tidy = CONFIG_DIR / "clang-tidy-port"
    port_tidy.write_text(
        """\
---
Checks: >
  -*,
  modernize-use-nullptr,
  modernize-use-override,
  modernize-use-using,
  modernize-use-equals-default,
  modernize-use-equals-delete,
  modernize-pass-by-value,
  modernize-avoid-c-arrays,
  cppcoreguidelines-no-malloc,
  cppcoreguidelines-owning-memory,
  cppcoreguidelines-pro-type-reinterpret-cast,
  bugprone-use-after-move,
  readability-identifier-naming,
  clang-analyzer-core.*,
  clang-analyzer-cplusplus.*
WarningsAsErrors: ''
HeaderFilterRegex: '.*'
""",
        encoding="utf-8",
    )

    db = find_compile_commands()
    targets = collect_tidy_targets(limit)
    results: list[dict] = []

    def run_one(path: Path) -> dict:
        cmd = [tidy, f"-config-file={port_tidy}", str(path)]
        if db:
            cmd.extend(["-p", str(db.parent)])
        else:
            # Fallback compile flags for modules / freestanding-ish TUs
            cmd.extend(
                [
                    "--",
                    "-std=c++23",
                    "-fno-exceptions",
                    "-fno-rtti",
                    f"-I{ROOT / 'pbsd'}",
                    "-Wno-everything",
                ]
            )
        if fix:
            cmd.insert(1, "-fix")
            cmd.insert(2, "-format-style=file")
        try:
            proc = subprocess.run(
                cmd, cwd=ROOT, capture_output=True, text=True, timeout=120
            )
            return {
                "path": path.relative_to(ROOT).as_posix(),
                "returncode": proc.returncode,
                "warnings": len(re.findall(r"warning:", proc.stdout + proc.stderr)),
                "stdout_tail": (proc.stdout or "")[-1500:],
                "stderr_tail": (proc.stderr or "")[-800:],
            }
        except (subprocess.TimeoutExpired, OSError) as e:
            return {"path": path.relative_to(ROOT).as_posix(), "error": str(e)}

    with ThreadPoolExecutor(max_workers=max(1, min(jobs, 4))) as ex:
        futs = [ex.submit(run_one, t) for t in targets]
        for fut in as_completed(futs):
            results.append(fut.result())

    results.sort(key=lambda r: r.get("path", ""))
    payload = {
        "generated": utc_now(),
        "skipped": False,
        "compile_commands": str(db) if db else None,
        "fix": fix,
        "targets": len(targets),
        "warning_sum": sum(r.get("warnings", 0) for r in results),
        "results": results,
        "base_config": str(config) if config.exists() else None,
    }
    TIDY_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    return payload


def phase_deps(limit: int | None) -> dict:
    tools = find_clang_tools()
    scan = tools["clang_scan_deps"]
    rows = load_inventory(limit or 40)
    samples: list[dict] = []
    if not scan:
        # Fallback: regex #include graph
        graph: dict[str, list[str]] = {}
        for r in rows[: limit or 40]:
            p = ROOT / r["path"]
            if not p.exists():
                continue
            try:
                text = p.read_text(encoding="utf-8", errors="replace")
            except OSError:
                continue
            incs = re.findall(r'#\s*include\s*[<"]([^>"]+)[>"]', text)
            graph[r["path"]] = incs[:40]
        payload = {
            "generated": utc_now(),
            "mode": "regex_includes",
            "files": len(graph),
            "graph": graph,
        }
        DEPS_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        return payload

    for r in rows[: limit or 20]:
        p = ROOT / r["path"]
        if not p.exists():
            continue
        cmd = [
            scan,
            "--format=make",
            "--",
            tools["clang"] or "clang",
            "-std=c17",
            "-Wno-everything",
            f"-I{ROOT / 'hbsd' / 'src' / 'sys'}",
            str(p),
        ]
        try:
            proc = subprocess.run(
                cmd, cwd=ROOT, capture_output=True, text=True, timeout=45
            )
            samples.append(
                {
                    "path": r["path"],
                    "returncode": proc.returncode,
                    "make_tail": (proc.stdout or "")[-1500:],
                }
            )
        except (subprocess.TimeoutExpired, OSError) as e:
            samples.append({"path": r["path"], "error": str(e)})

    payload = {
        "generated": utc_now(),
        "mode": "clang-scan-deps",
        "samples": samples,
    }
    DEPS_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    return payload


def phase_queue(limit: int | None) -> dict:
    if not QUERY_JSON.exists():
        phase_query(limit, jobs=os.cpu_count() or 4)
    census = json.loads(QUERY_JSON.read_text(encoding="utf-8"))
    files = census.get("all") or []
    # Leaves-first approximation: lower score + higher wave leaf paths first among tier 3
    inv = {r["path"]: r for r in load_inventory()}
    wave_rank = {
        "wave2": 2,
        "wave3": 3,
        "wave4": 0,
        "wave5": 1,
        "wave6": 1,
        "wave7": 2,
        "wave8": 3,
        "wave9": 4,
    }

    def sort_key(f: dict) -> tuple:
        path = f.get("path", "")
        wave = inv.get(path, {}).get("wave", "wave9")
        risk = f.get("risk_tier", 3)
        # Prefer tier 3, then lower AST score, then preferred waves
        return (risk if risk else 3, -1 if risk == 3 else 0, f.get("score", 0), wave_rank.get(wave, 9), path)

    ordered = sorted(files, key=sort_key)
    if limit is not None:
        ordered = ordered[:limit]

    queue = {
        "generated": utc_now(),
        "policy": "tier_asc then score_asc then wave; foundations remain manual first",
        "count": len(ordered),
        "items": [
            {
                "path": f["path"],
                "risk_tier": f.get("risk_tier"),
                "envelope_hint": f.get("envelope_hint"),
                "score": f.get("score"),
                "wave": inv.get(f["path"], {}).get("wave"),
                "status": "PENDING",
            }
            for f in ordered
        ],
    }
    QUEUE_JSON.write_text(json.dumps(queue, indent=2) + "\n", encoding="utf-8")
    # Also publish under pbsd/ if conversion layout expects it
    pbsd_queue = ROOT / "pbsd" / "queue.json"
    try:
        pbsd_queue.write_text(json.dumps(queue, indent=2) + "\n", encoding="utf-8")
    except OSError:
        pass
    return queue


def phase_report() -> str:
    tools = find_clang_tools()
    census = {}
    tidy = {}
    queue = {}
    deps = {}
    if QUERY_JSON.exists():
        census = json.loads(QUERY_JSON.read_text(encoding="utf-8"))
    if TIDY_JSON.exists():
        tidy = json.loads(TIDY_JSON.read_text(encoding="utf-8"))
    if QUEUE_JSON.exists():
        queue = json.loads(QUEUE_JSON.read_text(encoding="utf-8"))
    if DEPS_JSON.exists():
        deps = json.loads(DEPS_JSON.read_text(encoding="utf-8"))

    inv_lines = 0
    if INVENTORY.exists():
        inv_lines = max(0, sum(1 for _ in INVENTORY.open(encoding="utf-8")) - 1)

    lines = [
        "# PBSD Clang C→C++23 Port Report",
        "",
        f"Generated: `{utc_now()}`",
        "",
        "## Tool availability",
        "",
        "| Tool | Found |",
        "|---|---|",
    ]
    for k, v in tools.items():
        lines.append(f"| `{k}` | `{v or 'MISSING'}` |")

    lines += [
        "",
        "## Inventory",
        "",
        f"- Rows in `c_inventory.csv`: **{inv_lines}**",
        f"- Files scored in AST census: **{census.get('files_scored', 0)}**",
        "",
        "### Risk tier distribution",
        "",
    ]
    by_risk = census.get("by_risk") or {}
    for t in ("1", "2", "3"):
        lines.append(f"- Tier {t}: **{by_risk.get(t, 0)}**")

    lines += ["", "### Envelope hints", ""]
    for k, v in sorted((census.get("envelope_hints") or {}).items()):
        lines.append(f"- Envelope {k}: **{v}**")

    lines += ["", "### Hottest files (port with care)", "", "| Score | Tier | Env | Path |", "|---:|---|---|---|"]
    for r in (census.get("top_hot") or [])[:25]:
        lines.append(
            f"| {r.get('score')} | {r.get('risk_tier')} | {r.get('envelope_hint')} | `{r.get('path')}` |"
        )

    lines += [
        "",
        "## clang-tidy",
        "",
    ]
    if tidy.get("skipped"):
        lines.append(f"- Skipped: {tidy.get('reason')}")
    else:
        lines.append(f"- Targets: **{tidy.get('targets', 0)}**")
        lines.append(f"- Warning sum: **{tidy.get('warning_sum', 0)}**")
        lines.append(f"- compile_commands: `{tidy.get('compile_commands')}`")
        lines.append(f"- fix mode: **{tidy.get('fix')}**")

    lines += [
        "",
        "## Queue",
        "",
        f"- Items: **{queue.get('count', 0)}** → `docs/migration/clang_port/queue.json`",
        "",
        "## Deps sample",
        "",
        f"- Mode: `{deps.get('mode', 'n/a')}`",
        "",
        "## Next deterministic steps",
        "",
        "1. Drain `queue.json` leaves (tier 3) through Stages A–H in the master plan.",
        "2. Install/fix `compile_commands.json` (`cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) then re-run `--phase tidy --fix`.",
        "3. Wire LibTooling `pbsd-ownership` plugin for L2 as handle usage grows.",
        "4. Keep `python3 tools/wave_purge_c_check.py --gate` in CI until owned C hits zero.",
        "5. Use clang-query matchers in `tools/clang_port/` to expand ownership-candidate hunts.",
        "",
        "## Other deterministic ideas (not yet automated here)",
        "",
        "- `clang-include-cleaner` / IWYU on each DONE file",
        "- `clang-diff` structural compare original vs port",
        "- Sanitizer+replay differential runner as Stage G oracle",
        "- TLA+/SPIN for epoch substrate only",
        "- Reproducible-build hash gate + SBOM per release",
        "",
    ]
    text = "\n".join(lines)
    REPORT_MD.write_text(text + "\n", encoding="utf-8")
    return text


def write_helper_configs() -> None:
    ensure_out()
    (CONFIG_DIR / "README.md").write_text(
        """# Clang port helpers

Configs consumed by `tools/clang_cxx23_port.py`.

- `clang-tidy-port` — modernize + ownership-oriented checks for PBSD C++23 TUs
- `port_matchers.cquery` — regenerated clang-query matcher batch

See `pbsd-cxx23-port-master-plan.md` §8.
""",
        encoding="utf-8",
    )


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--phase",
        choices=("all", "inventory", "query", "tidy", "deps", "queue", "report"),
        default="all",
    )
    ap.add_argument("--limit", type=int, default=None, help="Cap files processed")
    ap.add_argument("--fix", action="store_true", help="Apply clang-tidy fixits")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    args = ap.parse_args()

    ensure_out()
    write_helper_configs()
    os.chdir(ROOT)

    # Default sampling for interactive runs; full tree for --limit 0 meaning unlimited
    limit = args.limit
    if args.phase == "all" and limit is None:
        # Full census can be huge; default to a substantial but bounded pass
        limit = 2000

    if args.phase in ("all", "inventory"):
        print("== inventory ==")
        run_inventory()

    if args.phase in ("all", "query"):
        print("== query (AST/pattern census) ==")
        q = phase_query(limit, args.jobs)
        print(f"scored {q.get('files_scored')} files → {QUERY_JSON}")

    if args.phase in ("all", "tidy"):
        print("== tidy ==")
        tidy_limit = 40 if args.phase == "all" and args.limit is None else limit
        t = phase_tidy(tidy_limit, args.fix, args.jobs)
        if t.get("skipped"):
            print(f"SKIP: {t.get('reason')}")
        else:
            print(f"tidy targets={t.get('targets')} warnings={t.get('warning_sum')} → {TIDY_JSON}")

    if args.phase in ("all", "deps"):
        print("== deps ==")
        d = phase_deps(min(limit or 40, 40))
        print(f"deps mode={d.get('mode')} → {DEPS_JSON}")

    if args.phase in ("all", "queue"):
        print("== queue ==")
        # Full ordered queue from census (may be large)
        qlimit = None if args.phase == "queue" else limit
        queue = phase_queue(qlimit)
        print(f"queue items={queue.get('count')} → {QUEUE_JSON}")

    if args.phase in ("all", "report"):
        print("== report ==")
        phase_report()
        print(f"wrote {REPORT_MD}")

    print("OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
