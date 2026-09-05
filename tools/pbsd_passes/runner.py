# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Pass-runner framework: apply passes, record diffs/refusals, stage outputs."""
from __future__ import annotations

import json
import shutil
import platform
import tempfile
from dataclasses import asdict
from datetime import datetime, timezone
from pathlib import Path

from .compile_db import coverage_report, default_flags, generate_compile_commands
from .differential import differential
from .ir_oracle import compare_ir
from .shard import merge_shards
from .passes import passes_for_tiers
from .proposals import flush as flush_proposals
from .proposals import reset as clear_proposal_buffer
from .schema import PassResult, Refusal
from .unit import TranslationUnit

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "migration" / "clang_port"
STAGED = OUT / "staged"
CORPUS = Path(__file__).resolve().parent / "corpus"


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


_ARCH_DIR = {"x86_64": "amd64", "aarch64": "aarch64"}.get(
    platform.machine(), platform.machine()
)

_SHIM_DIR: Path | None = None


def _machine_include_shim() -> Path | None:
    """A directory where <machine/...> and <x86/...> resolve.

    FreeBSD's build makes machine/ a symlink to sys/<arch>/include; nothing
    in a plain checkout does, so every source reaching for
    <machine/_types.h> failed to compile and the IR oracle could not judge
    it. Built once per process in a temp dir - the repo is left alone.
    """
    global _SHIM_DIR
    if _SHIM_DIR is not None:
        return _SHIM_DIR if _SHIM_DIR.is_dir() else None
    sys_dir = ROOT / "hbsd" / "src" / "sys"
    arch = platform.machine()
    candidates = {"x86_64": "amd64", "aarch64": "arm64"}.get(arch, arch)
    inc = sys_dir / candidates / "include"
    if not inc.is_dir():
        return None
    d = Path(tempfile.mkdtemp(prefix="pbsd_machine_shim_"))
    try:
        (d / "machine").symlink_to(inc, target_is_directory=True)
        x86 = sys_dir / "x86" / "include"
        if x86.is_dir():
            (d / "x86").symlink_to(x86, target_is_directory=True)
    except OSError:
        return None
    _SHIM_DIR = d
    return d


def oracle_include_flags(src: Path) -> list[str]:
    """Include path for compiling a source and its staged port side by side.

    Two things were missing, and between them they made every IR check on real
    sources report compile_fail:

      * the C side needs the vendored FreeBSD headers - msun's math.h pulls in
        <sys/_types.h>, which lives under hbsd/src/sys;
      * the C++ side is compiled from docs/migration/clang_port/staged/..., so
        it loses the siblings it used to include by quoted path -
        "math_private.h" sits next to the original, not next to the copy.

    Both directories are added for both sides, so the port is compared against
    the original rather than against a missing header.
    """
    flags = ["-Wno-everything", f"-I{src.parent}"]
    shim = _machine_include_shim()
    if shim is not None:
        flags.append(f"-idirafter{shim}")
    libc = ROOT / "hbsd" / "src" / "lib" / "libc"
    extras = [
        ROOT / "hbsd" / "src" / "sys",
        libc / "include",
        # <_fpmath.h> and union IEEEl2bits are per-architecture.
        libc / _ARCH_DIR,
    ]
    # msun keeps its long-double helpers (invtrig.h) in a sibling of src/.
    if "lib/msun/" in src.as_posix():
        msun = ROOT / "hbsd" / "src" / "lib" / "msun"
        extras += [msun / "src", msun / "ld80", msun / "ld128"]
    # -idirafter, not -I: these directories go *after* the system ones.
    # With -I, libstdc++'s <cstdlib> does #include_next <stdlib.h> and lands
    # on FreeBSD's instead of glibc's, so `using ::atoll;` fails and every
    # C++ side died with "no member named 'atoll' in namespace '__gnu_cxx'".
    # Appending instead lets glibc win the standard headers and leaves these
    # to supply only what it has no answer for - machine/, sys/_types.h,
    # _fpmath.h, invtrig.h. This is the shadowing trap the Linux build
    # exception list warns about, avoided rather than walked into.
    for extra in extras:
        if extra.is_dir():
            flags.append(f"-idirafter{extra}")
    return flags


def discover_sources(
    scopes: list[str],
    limit: int | None = None,
    skip: int = 0,
) -> list[Path]:
    files: list[Path] = []
    for scope in scopes:
        base = ROOT / "hbsd" / "src" / scope
        if not base.exists():
            continue
        for p in sorted(base.rglob("*.c")):
            if ".git" in p.parts:
                continue
            files.append(p)
    if skip:
        files = files[skip:]
    if limit is not None:
        files = files[:limit]
    return files


BANNER = """/* PBSD automated port from {orig}. Do not mark DONE without differential / IR oracle. */
/* Generated by tools/run_todo_passes.py — deterministic passes from docs/plans/todo-passes.md */
#if defined(__cplusplus)
#include <cstddef>
#include <cstdlib>
#endif

"""


def run_passes_on_unit(
    unit: TranslationUnit,
    tiers: set[int] | None = None,
    safe_only: bool = False,
) -> tuple[TranslationUnit, list[Refusal], list[dict]]:
    refusals: list[Refusal] = []
    edit_log: list[dict] = []
    for p in passes_for_tiers(tiers, safe_only=safe_only):
        if not p.precondition(unit):
            refusals.append(
                Refusal(
                    file=unit.path,
                    line=1,
                    col=1,
                    pass_name=p.name,
                    reason_code="PRECONDITION_FAIL",
                    enclosing_function="<file>",
                    snippet="precondition failed",
                )
            )
            continue
        result: PassResult = p.apply(unit)
        unit.text = result.text
        refusals.extend(result.refusals)
        for e in result.edits:
            edit_log.append(asdict(e))
    return unit, refusals, edit_log


def stage_path_for(src: Path) -> Path:
    rel = src.relative_to(ROOT / "hbsd" / "src")
    return STAGED / rel.with_suffix(".cpp")


def process_file(
    src: Path,
    tiers: set[int] | None = None,
    do_ir: bool = False,
    do_diff: bool = False,
    max_bytes: int = 2_000_000,
    safe_only: bool = False,
) -> dict:
    size = src.stat().st_size
    if size > max_bytes:
        flush_proposals()
        return {
            "source": src.relative_to(ROOT).as_posix(),
            "staged": "",
            "edits": 0,
            "refusals": 0,
            "edit_list": [],
            "refusal_list": [],
            "ir": {"status": "skipped_huge", "equal": False},
            "diff": {"status": "skipped_huge", "equal": False},
            "ir_eligible": False,
            "diff_eligible": False,
            "skipped_huge": True,
        }
    text = src.read_text(encoding="utf-8", errors="replace")
    rel = src.relative_to(ROOT).as_posix()
    unit = TranslationUnit(path=rel, text=text)
    unit, refusals, edits = run_passes_on_unit(unit, tiers, safe_only=safe_only)

    out_text = BANNER.format(orig=rel) + unit.text
    dest = stage_path_for(src)
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(out_text, encoding="utf-8")

    # Side-by-side original copy for oracle
    orig_stage = dest.with_suffix(".c.orig")
    shutil.copy2(src, orig_stage)

    record = {
        "source": rel,
        "staged": dest.relative_to(ROOT).as_posix(),
        "edits": len(edits),
        "refusals": len(refusals),
        "edit_list": edits,
        "refusal_list": [r.to_dict() for r in refusals],
        "ir": None,
        "diff": None,
        "ir_eligible": False,
        "diff_eligible": False,
    }

    # For IR/diff, prefer freestanding-ish files without heavy BSD headers.
    # <err.h> is ubiquitous in userland and available on many hosts — do not
    # treat it as heavy. Skip kernel/capsicum-heavy TUs.
    heavy = any(
        x in text
        for x in (
            "capsicum",
            "#include <sys/",
            '#include "sys/',
            "libcasper",
            "caph_",
        )
    )
    record["ir_eligible"] = bool(do_ir and not heavy and src.stat().st_size < 12_000)
    record["diff_eligible"] = bool(
        do_diff and not heavy and "main" in text and src.stat().st_size < 4000
    )

    if record["ir_eligible"]:
        record["ir"] = compare_ir(src, dest,
                                  include_flags=oracle_include_flags(src))
    elif do_ir:
        record["ir"] = {"status": "skipped_heavy", "equal": False}

    if record["diff_eligible"]:
        # Only argv-only utilities; avoid programs that read stdin forever.
        record["diff"] = differential(
            src,
            dest,
            inputs=[[]],
            include_flags=oracle_include_flags(src),
        )
    elif do_diff:
        record["diff"] = {"status": "skipped_heavy", "equal": False}

    flush_proposals()
    return record


def process_file_timed(
    src: Path,
    tiers: set[int] | None = None,
    safe_only: bool = False,
    do_ir: bool = False,
    do_diff: bool = False,
    timeout_s: float = 90.0,
) -> dict:
    """Run process_file with SIGALRM wall-clock timeout (WSL/Linux)."""
    import signal

    def _alarm(_signum, _frame):
        raise TimeoutError(f"process_file timeout {timeout_s}s on {src.name}")

    old = signal.signal(signal.SIGALRM, _alarm)
    signal.setitimer(signal.ITIMER_REAL, timeout_s)
    try:
        return process_file(
            src, tiers=tiers, do_ir=do_ir, do_diff=do_diff, safe_only=safe_only
        )
    except TimeoutError:
        flush_proposals()
        print(f"  TIMEOUT {src.name} after {timeout_s}s — skipped", flush=True)
        return {
            "source": src.relative_to(ROOT).as_posix(),
            "staged": "",
            "edits": 0,
            "refusals": 0,
            "edit_list": [],
            "refusal_list": [],
            "ir": {"status": "timeout", "equal": False},
            "diff": {"status": "timeout", "equal": False},
            "ir_eligible": False,
            "diff_eligible": False,
            "timeout": True,
        }
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)
        signal.signal(signal.SIGALRM, old)


def run_corpus_tests() -> dict:
    """Golden-file smoke tests for Tier 1 transforms + optional IR oracle."""
    results = []
    ir_results = []
    corpus_dir = CORPUS
    if not corpus_dir.exists():
        return {"ok": False, "error": "corpus missing", "cases": []}
    for inp in sorted(corpus_dir.glob("*.c")):
        expected = inp.with_suffix(".expected")
        text = inp.read_text(encoding="utf-8")
        unit = TranslationUnit(path=inp.name, text=text)
        unit, refusals, edits = run_passes_on_unit(unit, tiers={0, 1, 2, 3})
        got = unit.text
        if expected.exists():
            exp = expected.read_text(encoding="utf-8")
            # Compare ignoring trailing whitespace
            ok = got.strip() == exp.strip() or exp.strip() in got.strip()
            # Also accept if all expected markers present
            if not ok and expected.suffix == ".expected":
                markers = [
                    ln[2:].strip()
                    for ln in exp.splitlines()
                    if ln.startswith("@@")
                ]
                if markers:
                    ok = all(m in got for m in markers)
            results.append(
                {
                    "case": inp.name,
                    "ok": ok,
                    "edits": len(edits),
                    "refusals": len(refusals),
                }
            )
        else:
            # Write expected draft for first run
            expected.write_text(got, encoding="utf-8")
            results.append({"case": inp.name, "ok": True, "drafted_expected": True})

        # IR oracle on self-contained corpus mains (no system includes)
        if (
            "int main" in text
            and "#include" not in text
            and "fprintf" not in text
        ):
            with tempfile.TemporaryDirectory(prefix="pbsd_corpus_ir_") as td:
                td_path = Path(td)
                c_src = td_path / "orig.c"
                cxx_src = td_path / "port.cpp"
                c_src.write_text(text, encoding="utf-8")
                cxx_src.write_text(got, encoding="utf-8")
                ir = compare_ir(c_src, cxx_src, include_flags=["-Wno-everything"])
                ir_results.append(
                    {
                        "case": inp.name,
                        "status": ir.get("status"),
                        "equal": bool(ir.get("equal")),
                        "abi_equal": bool(ir.get("abi_equal")),
                        "abi_only_in_c": ir.get("abi_only_in_c") or [],
                        "abi_only_in_cxx": ir.get("abi_only_in_cxx") or [],
                    }
                )

    golden_ok = all(r.get("ok") for r in results) if results else False

    # The IR oracle is described as the highest-leverage Tier 0 item, but its
    # verdict used to be reported and then dropped: `ok` came from the golden
    # files alone, so a transform that changed the emitted code could still be
    # reported as passing. It gates now. A case that will not compile counts
    # as a failure too - the corpus cases take no #include, so a compile error
    # means the fixture is self-inconsistent and nothing was verified.
    ir_ok = all(r.get("equal") for r in ir_results) if ir_results else True
    ok = golden_ok and ir_ok
    from .proposals import flush as flush_proposals

    flush_proposals()
    ir_ran = len(ir_results)
    ir_equal = sum(1 for r in ir_results if r.get("equal"))
    # A port is only committable if it is *both* semantically equal and
    # exports the same symbols. See exported_symbols() in ir_oracle.
    abi_equal = sum(1 for r in ir_results
                    if r.get("equal") and r.get("abi_equal"))
    return {
        "ok": ok,
        "golden_ok": golden_ok,
        "ir_ok": ir_ok,
        "cases": results,
        "ir_ran": ir_ran,
        "ir_equal": ir_equal,
        "abi_equal": abi_equal,
        "ir_cases": ir_results,
    }


def run_clang_tidy_on_staged(limit: int | None = 80, fix: bool = True) -> dict:
    """Run clang-tidy-18 on staged .cpp (best-effort; no compile DB required)."""
    import shutil
    import subprocess

    tidy = shutil.which("clang-tidy-18") or shutil.which("clang-tidy")
    if not tidy:
        return {"skipped": True, "reason": "clang-tidy not found"}
    targets = sorted(STAGED.rglob("*.cpp"))
    if limit is not None:
        targets = targets[:limit]
    cfg = ROOT / "tools" / "clang_port" / "clang-tidy-port"
    cfg.parent.mkdir(parents=True, exist_ok=True)
    if not cfg.exists():
        cfg.write_text(
            "---\n"
            "Checks: 'modernize-use-nullptr,modernize-use-override,modernize-use-using,"
            "cppcoreguidelines-no-malloc,bugprone-use-after-move'\n"
            "WarningsAsErrors: ''\n",
            encoding="utf-8",
        )
    results = []
    warning_sum = 0
    for t in targets:
        cmd = [tidy, f"-config-file={cfg}", str(t)]
        if fix:
            cmd.insert(1, "-fix")
        cmd.extend(
            [
                "--",
                "-std=c++23",
                "-Wno-everything",
                f"-I{ROOT / 'pbsd'}",
                f"-I{ROOT / 'hbsd' / 'src' / 'include'}",
            ]
        )
        try:
            proc = subprocess.run(cmd, capture_output=True, text=True, timeout=90)
            w = (proc.stdout + proc.stderr).count("warning:")
            warning_sum += w
            results.append(
                {
                    "path": t.relative_to(ROOT).as_posix(),
                    "rc": proc.returncode,
                    "warnings": w,
                }
            )
        except (subprocess.TimeoutExpired, OSError) as e:
            results.append({"path": str(t), "error": str(e)})
    out = {
        "skipped": False,
        "targets": len(targets),
        "warning_sum": warning_sum,
        "fix": fix,
        "results": results,
    }
    (OUT / "tidy_staged.json").write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")
    return out


def _eligibility(src: Path, do_ir: bool, do_diff: bool) -> tuple[bool, bool]:
    """Cheap pre-scan mirroring process_file, so budgets can be assigned
    before any work is dispatched.

    The serial loop spent its IR and differential budget as it went. A pool
    finishes files out of order, so deciding there would make the result
    depend on scheduling. Deciding here keeps a --jobs run reproducible and
    identical to the serial one.
    """
    try:
        text = src.read_text(encoding='utf-8', errors='replace')
        size = src.stat().st_size
    except OSError:
        return False, False
    heavy = any(
        x in text
        for x in ('capsicum', '#include <sys/', '#include "sys/',
                  'libcasper', 'caph_')
    )
    ir_ok = bool(do_ir and not heavy and size < 12_000)
    diff_ok = bool(do_diff and not heavy and 'main' in text and size < 4000)
    return ir_ok, diff_ok


def _worker(args: tuple) -> dict:
    """Pool entry point. Must be module level so it can be pickled."""
    src, tiers, want_ir, want_diff, timeout_s, safe_only = args
    rec = process_file_timed(
        src, tiers=tiers, do_ir=want_ir, do_diff=want_diff,
        timeout_s=timeout_s, safe_only=safe_only
    )
    flush_proposals()
    return rec


def run_pipeline(
    scopes: list[str],
    limit: int | None = None,
    tiers: set[int] | None = None,
    do_ir: bool = True,
    do_diff: bool = True,
    files: list[Path] | None = None,
    do_tidy: bool = False,
    tidy_limit: int | None = 100,
    ir_limit: int | None = 25,
    diff_limit: int | None = 10,
    skip: int = 0,
    append_proposals: bool = False,
    skip_corpus: bool = False,
    reset_proposals: bool = True,
    file_timeout: float = 90.0,
    jobs: int = 1,
    safe_only: bool = False,
) -> dict:
    OUT.mkdir(parents=True, exist_ok=True)
    clear_proposal_buffer()
    for side in ("pointer_kinds.jsonl", "global_clusters.jsonl"):
        p = OUT / side
        if p.exists():
            p.unlink()
    prop = OUT / "proposals.jsonl"
    if prop.exists() and reset_proposals and not append_proposals:
        prop.unlink()

    if skip_corpus:
        corpus = {"ok": True, "skipped": True, "cases": []}
    else:
        corpus = run_corpus_tests()
    sources = (
        files
        if files is not None
        else discover_sources(scopes, limit=limit, skip=skip)
    )
    compile_commands = OUT / "compile_commands.json"
    generate_compile_commands(sources, compile_commands, ROOT)
    cov = coverage_report(sources, compile_commands)

    records = []
    all_refusals: list[dict] = []
    ir_budget = ir_limit if do_ir else 0
    diff_budget = diff_limit if do_diff else 0
    total = len(sources)

    # Assign the IR and differential budgets up front. Both are expensive and
    # capped, and a pool completes files out of order, so spending the budget
    # as results arrive would make the outcome depend on scheduling.
    plan: list[tuple] = []
    for src in sources:
        el_ir, el_diff = _eligibility(src, do_ir, do_diff)
        want_ir = bool(el_ir and (ir_budget is None or ir_budget > 0))
        want_diff = bool(el_diff and (diff_budget is None or diff_budget > 0))
        if want_ir and ir_budget is not None:
            ir_budget -= 1
        if want_diff and diff_budget is not None:
            diff_budget -= 1
        plan.append((src, tiers, want_ir, want_diff, file_timeout, safe_only))

    def _finish(rec: dict, src: Path, want_ir: bool, want_diff: bool) -> dict:
        if do_ir and not want_ir and "ir" not in rec:
            rec["ir"] = {"status": "skipped_budget", "equal": False}
        if do_diff and not want_diff and "diff" not in rec:
            rec["diff"] = {"status": "skipped_budget", "equal": False}
        return rec

    if jobs > 1 and total > 1:
        import concurrent.futures as _cf
        import os as _os

        # Each worker appends to its own shard of proposals.jsonl and the
        # tier-3 side files; the parent folds them back in below.
        _os.environ["PBSD_SHARD"] = "1"
        print(f"  running {total} file(s) across {jobs} workers", flush=True)
        results: list[dict | None] = [None] * total
        done = 0
        try:
            with _cf.ProcessPoolExecutor(max_workers=jobs) as pool:
                futures = {
                    pool.submit(_worker, item): idx
                    for idx, item in enumerate(plan)
                }
                for fut in _cf.as_completed(futures):
                    idx = futures[fut]
                    src, _t, w_ir, w_diff, _to, _s = plan[idx]
                    try:
                        rec = fut.result()
                    except Exception as exc:  # a worker died; do not lose the run
                        rec = {
                            "source": src.relative_to(ROOT).as_posix(),
                            "staged": "", "edits": 0, "refusals": 0,
                            "edit_list": [], "refusal_list": [],
                            "ir": {"status": "worker_error", "equal": False},
                            "diff": {"status": "worker_error", "equal": False},
                            "ir_eligible": False, "diff_eligible": False,
                            "error": str(exc),
                        }
                        print(f"  WORKER ERROR {src.name}: {exc}", flush=True)
                    results[idx] = _finish(rec, src, w_ir, w_diff)
                    done += 1
                    if done % 50 == 0 or done == total:
                        got = [r for r in results if r]
                        print(
                            f"  progress {done}/{total} "
                            f"edits={sum(r['edits'] for r in got)} "
                            f"refusals={sum(len(r['refusal_list']) for r in got)}",
                            flush=True,
                        )
        finally:
            _os.environ.pop("PBSD_SHARD", None)
            for side in ("proposals.jsonl", "pointer_kinds.jsonl",
                         "global_clusters.jsonl"):
                merge_shards(OUT / side)
        records = [r for r in results if r]
        for r in records:
            all_refusals.extend(r["refusal_list"])
    else:
        for i, (src, _t, want_ir, want_diff, _to, _s) in enumerate(plan, 1):
            rec = _finish(
                process_file_timed(
                    src,
                    tiers=tiers,
                    do_ir=want_ir,
                    do_diff=want_diff,
                    timeout_s=file_timeout,
                    safe_only=safe_only,
                ),
                src, want_ir, want_diff,
            )
            records.append(rec)
            all_refusals.extend(rec["refusal_list"])
            if i % 10 == 0 or i == total:
                print(
                    f"  progress {i}/{total} edits={sum(r['edits'] for r in records)} "
                    f"refusals={len(all_refusals)} last={src.name}",
                    flush=True,
                )

    flush_proposals()

    # Dedupe refusals: same file/line/reason from stacked passes
    deduped: list[dict] = []
    seen_r: set[tuple] = set()
    for r in all_refusals:
        key = (r.get("file"), r.get("line"), r.get("reason_code"), (r.get("snippet") or "")[:60])
        if key in seen_r:
            continue
        seen_r.add(key)
        deduped.append(r)
    all_refusals = deduped

    tidy_report = None
    if do_tidy:
        tidy_report = run_clang_tidy_on_staged(limit=tidy_limit, fix=True)

    refusals_path = OUT / "refusals.jsonl"
    with refusals_path.open("w", encoding="utf-8") as f:
        for r in all_refusals:
            f.write(json.dumps(r) + "\n")

    # Reason histogram
    hist: dict[str, int] = {}
    for r in all_refusals:
        hist[r["reason_code"]] = hist.get(r["reason_code"], 0) + 1

    report = {
        "generated": utc_now(),
        "scopes": scopes,
        "files": len(sources),
        "staged_dir": STAGED.relative_to(ROOT).as_posix(),
        "compile_commands_coverage": cov,
        "corpus": corpus,
        "edits_total": sum(r["edits"] for r in records),
        "refusals_total": len(all_refusals),
        "reason_histogram": dict(sorted(hist.items(), key=lambda x: -x[1])),
        "ir_equal": sum(1 for r in records if (r.get("ir") or {}).get("equal")),
        "abi_equal": sum(1 for r in records
                         if (r.get("ir") or {}).get("equal")
                         and (r.get("ir") or {}).get("abi_equal")),
        "ir_ran": sum(
            1
            for r in records
            if (r.get("ir") or {}).get("status") in ("ok", "mismatch", "compile_fail")
        ),
        "diff_equal": sum(1 for r in records if (r.get("diff") or {}).get("equal")),
        "clang_tidy": tidy_report,
        "records": records,
    }
    (OUT / "pass_report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    # Markdown summary
    lines = [
        "# PBSD todo.md Pass Report",
        "",
        f"Generated: `{report['generated']}`",
        "",
        f"- Files processed: **{report['files']}**",
        f"- Edits applied: **{report['edits_total']}**",
        f"- Refusals (model queue only): **{report['refusals_total']}**",
        f"- compile_commands coverage: **{cov['coverage_pct']}%**",
        f"- Corpus OK: **{corpus.get('ok')}**",
        f"- Corpus IR equal: **{corpus.get('ir_equal', 0)}** / ran **{corpus.get('ir_ran', 0)}**",
        f"- IR equal: **{report['ir_equal']}** / ran **{report['ir_ran']}**",
        f"- ABI equal (same exported symbols too): "
        f"**{report['abi_equal']}** / ran **{report['ir_ran']}**",
        f"- Diff equal: **{report['diff_equal']}**",
    ]
    if tidy_report:
        if tidy_report.get("skipped"):
            lines.append(f"- clang-tidy: skipped ({tidy_report.get('reason')})")
        else:
            lines.append(
                f"- clang-tidy: targets={tidy_report.get('targets')} "
                f"warnings={tidy_report.get('warning_sum')} fix={tidy_report.get('fix')}"
            )
    lines += [
        "",
        "## Reason histogram (true refusals — not successful edits)",
        "",
        "| Reason | Count |",
        "|---|---:|",
    ]
    for k, v in report["reason_histogram"].items():
        lines.append(f"| `{k}` | {v} |")
    lines += [
        "",
        "## Top edited files",
        "",
        "| Edits | Refusals | File |",
        "|---:|---:|---|",
    ]
    top = sorted(records, key=lambda r: -r["edits"])[:30]
    for r in top:
        lines.append(f"| {r['edits']} | {r['refusals']} | `{r['source']}` |")
    lines += [
        "",
        "## Outputs",
        "",
        f"- Staged C++: `{STAGED}`",
        f"- Refusals: `{refusals_path}`",
        f"- Full JSON: `docs/migration/clang_port/pass_report.json`",
        "",
    ]
    # Proposal histogram
    prop_path = OUT / "proposals.jsonl"
    if prop_path.exists():
        ph: dict[str, int] = {}
        nprop = 0
        with prop_path.open(encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    kind = json.loads(line).get("kind", "?")
                except json.JSONDecodeError:
                    continue
                ph[kind] = ph.get(kind, 0) + 1
                nprop += 1
        report["proposals_total"] = nprop
        report["proposal_histogram"] = dict(sorted(ph.items(), key=lambda x: -x[1]))
        lines += [
            f"## Proposal histogram (`proposals.jsonl`, {nprop})",
            "",
            "| Kind | Count |",
            "|---|---:|",
        ]
        for k, v in list(report["proposal_histogram"].items())[:40]:
            lines.append(f"| `{k}` | {v} |")
        lines.append("")
    md = "\n".join(lines) + "\n"
    (OUT / "todo_pass_report.md").write_text(md, encoding="utf-8")
    return report
