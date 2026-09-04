#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""CLI: Stage F/G agent port over the deterministic-pass refusal queue.

Same flag conventions as tools/run_todo_passes.py. Consumes refusals.jsonl,
existing stubs, differential.py, ir_oracle.py, and (if installed) ESBMC.
Does not replace those tools.

DeepSeek-only (Flash -> Pro). Failures are saved to progress + agent_port_failures.jsonl.
"""
from __future__ import annotations

import argparse
import csv
import json
import sys
import time
import traceback
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from threading import Semaphore

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
if str(ROOT / "tools") not in sys.path:
    sys.path.insert(0, str(ROOT / "tools"))

from convert_c_batch import load_progress, recompute_wave_stats, save_progress  # noqa: E402
from pbsd_secrets import load_secrets, parse_secrets_text  # noqa: E402
from pbsd_agent.esbmc_check import parse_esbmc_output  # noqa: E402
from pbsd_agent.model_clients import (  # noqa: E402
    MAX_TIER,
    MODEL_FLASH,
    MODEL_PRO,
    Usage,
    deepseek_extra_body,
    estimate_cost_usd,
    model_name_for_tier,
)
from pbsd_agent.session import (  # noqa: E402
    FileContext,
    FileSession,
    append_failure,
    parse_agent_payload,
    starting_tier,
)
from pbsd_passes.schema import PortRecord, StageEvidence  # noqa: E402

REFUSALS = ROOT / "docs" / "migration" / "clang_port" / "refusals.jsonl"
QUEUE = ROOT / "docs" / "migration" / "clang_port" / "queue.json"
CENSUS = ROOT / "docs" / "migration" / "clang_port" / "ast_census_slim.json"
INVENTORY = ROOT / "docs" / "migration" / "c_inventory.csv"


def expand_scope(scope: str) -> list[str]:
    prefixes: list[str] = []
    for raw in scope.split(","):
        s = raw.strip().replace("\\", "/")
        if not s:
            continue
        if s.startswith("hbsd/"):
            prefixes.append(s.rstrip("/"))
        else:
            prefixes.append(f"hbsd/src/{s}".rstrip("/"))
    return prefixes


def matches_prefix(path: str, prefixes: list[str]) -> bool:
    p = path.replace("\\", "/")
    return any(p == pref or p.startswith(pref + "/") for pref in prefixes)


def load_jsonl(path: Path) -> list[dict]:
    if not path.is_file():
        return []
    rows: list[dict] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line.strip():
            continue
        try:
            rows.append(json.loads(line))
        except json.JSONDecodeError:
            continue
    return rows


def refusals_by_file() -> dict[str, list[dict]]:
    grouped: dict[str, list[dict]] = {}
    for row in load_jsonl(REFUSALS):
        key = (row.get("file") or "").replace("\\", "/")
        if not key:
            continue
        grouped.setdefault(key, []).append(row)
    return grouped


def load_queue_index() -> dict[str, dict]:
    if not QUEUE.is_file():
        return {}
    data = json.loads(QUEUE.read_text(encoding="utf-8"))
    return {item["path"].replace("\\", "/"): item for item in data.get("items", [])}


def load_census_index() -> dict[str, dict]:
    if not CENSUS.is_file():
        return {}
    data = json.loads(CENSUS.read_text(encoding="utf-8"))
    items = data if isinstance(data, list) else data.get("files") or data.get("items") or []
    out: dict[str, dict] = {}
    for item in items:
        p = (item.get("path") or "").replace("\\", "/")
        if p:
            out[p] = item
    return out


def load_inventory_waves() -> dict[str, str]:
    if not INVENTORY.is_file():
        return {}
    with INVENTORY.open(encoding="utf-8", newline="") as fh:
        return {r["path"].replace("\\", "/"): r.get("wave", "wave2") for r in csv.DictReader(fh)}


def stubbed_candidates(entries: dict[str, dict]) -> list[str]:
    """Work queue when refusals/queue are empty: stubbed + NEEDS-REVIEW."""
    out: list[str] = []
    for src, entry in entries.items():
        status = entry.get("status")
        if status in {"stubbed", "NEEDS-REVIEW", "pending", "PENDING"}:
            out.append(src)
    return sorted(out)


def build_work(
    prefixes: list[str],
    files: list[str] | None,
    *,
    include_converted: bool = False,
) -> list[FileContext]:
    progress = load_progress()
    entries = {e["source"].replace("\\", "/"): e for e in progress.get("entries", [])}
    refusals = refusals_by_file()
    queue = load_queue_index()
    census = load_census_index()
    waves = load_inventory_waves()

    candidates: list[str] = []
    if files:
        candidates = [f.replace("\\", "/") for f in files]
    elif refusals:
        candidates = sorted(refusals)
    else:
        stubbed = stubbed_candidates(entries)
        candidates = stubbed if stubbed else sorted(queue)

    work: list[FileContext] = []
    for src in candidates:
        if prefixes and not matches_prefix(src, prefixes):
            continue
        entry = entries.get(src, {})
        if not include_converted and entry.get("status") == "converted":
            continue
        c_path = ROOT / src
        if not c_path.is_file():
            continue
        stub_rel = entry.get("stub")
        stub_path = (ROOT / stub_rel) if stub_rel else None
        q = queue.get(src, {})
        c = census.get(src, {})
        risk = int(q.get("risk_tier") or c.get("risk_tier") or 3)
        env = str(q.get("envelope_hint") or c.get("envelope_hint") or "C")
        work.append(
            FileContext(
                source=src,
                c_text=c_path.read_text(encoding="utf-8", errors="replace"),
                stub_path=stub_path if stub_path and stub_path.is_file() else stub_path,
                stub_text=stub_path.read_text(encoding="utf-8", errors="replace")
                if stub_path and stub_path.is_file()
                else "",
                refusals=refusals.get(src, []),
                risk_tier=risk,
                envelope_hint=env,
                wave=entry.get("wave") or waves.get(src, "wave2"),
                prior_record=entry.get("port_record") if entry.get("status") == "NEEDS-REVIEW" else None,
            )
        )
    return work


def update_progress(ctx: FileContext, record) -> None:
    progress = load_progress()
    found = False
    payload = record.to_dict()
    for entry in progress.get("entries", []):
        if entry.get("source") == ctx.source:
            entry["status"] = "converted" if record.status == "converted" else "NEEDS-REVIEW"
            entry["port_record"] = payload
            entry["model_used"] = record.model_used
            entry["escalation_trail"] = record.escalation_trail
            entry["stage_evidence"] = record.stage_evidence
            found = True
            break
    if not found:
        progress.setdefault("entries", []).append(
            {
                "source": ctx.source,
                "wave": ctx.wave,
                "status": record.status if record.status != "converted" else "converted",
                "stub": ctx.stub_path.relative_to(ROOT).as_posix() if ctx.stub_path else "",
                "port_record": payload,
                "model_used": record.model_used,
                "escalation_trail": record.escalation_trail,
                "stage_evidence": record.stage_evidence,
            }
        )
    progress["waves"] = recompute_wave_stats(progress["entries"])
    save_progress(progress)


def crash_record(ctx: FileContext, err: BaseException) -> PortRecord:
    """Build a NEEDS-REVIEW record when a worker dies unexpectedly."""
    rec = PortRecord(
        source=ctx.source,
        model_used=model_name_for_tier(starting_tier(ctx)),
        escalation_trail=[
            {
                "tier": starting_tier(ctx),
                "model": model_name_for_tier(starting_tier(ctx)),
                "reason": f"worker_crash: {err}",
                "retries": 0,
            }
        ],
        stage_evidence=StageEvidence().to_dict(),
        tokens_in=0,
        tokens_out=0,
        est_cost_usd=0.0,
        status="NEEDS-REVIEW",
    )
    append_failure(rec, detail=traceback.format_exc()[-4000:])
    return rec


def run_self_test() -> int:
    failures: list[str] = []

    ctx = FileContext(
        source="hbsd/src/bin/echo/echo.c",
        c_text="int main(){return 0;}",
        stub_path=None,
        stub_text="",
        refusals=[],
        risk_tier=3,
        envelope_hint="C",
    )
    if starting_tier(ctx) != 1:
        failures.append("tier-3-risk should start at Flash")
    ctx.risk_tier = 1
    if starting_tier(ctx) != 2:
        failures.append("risk-1 should start at Pro")
    ctx.source = "hbsd/src/sys/kern/sched_ule.c"
    if starting_tier(ctx) != 2:
        failures.append("scheduler path should force Pro")

    payload = parse_agent_payload('```json\n{"spec_notes":"n","port_cppm":"x"}\n```')
    if payload.get("port_cppm") != "x":
        failures.append("fenced json parse failed")

    status, sat, _ = parse_esbmc_output("VERIFICATION SUCCESSFUL\nUNSATISFIABLE")
    if status != "ok" or sat != "UNSAT":
        failures.append("esbmc success parse failed")
    status, sat, cex = parse_esbmc_output("VERIFICATION FAILED\nCounterexample\n  x=1")
    if status != "failed" or sat != "SAT" or "Counterexample" not in cex:
        failures.append("esbmc fail parse failed")

    cost = estimate_cost_usd("deepseek-v4-flash", Usage(prompt_tokens=1_000_000, completion_tokens=0))
    if cost <= 0:
        failures.append("cost estimate should be positive")

    if model_name_for_tier(1) != MODEL_FLASH or model_name_for_tier(2) != MODEL_PRO:
        failures.append("tiers must map to Flash then Pro")
    if model_name_for_tier(3) != MODEL_PRO:
        failures.append("tier>2 must clamp to Pro (no Kimi)")
    if MAX_TIER != 2:
        failures.append("MAX_TIER must be 2 (DeepSeek-only)")

    extra = deepseek_extra_body()
    if extra.get("reasoning_effort") != "max" or (extra.get("thinking") or {}).get("type") != "enabled":
        failures.append("DeepSeek default must be thinking+max effort")

    parsed = parse_secrets_text(
        "DEEPSEEK_API_KEY=sk-test\n# comment\nexport OTHER_KEY='abc'\nEMPTY=\n"
    )
    if parsed.get("DEEPSEEK_API_KEY") != "sk-test" or parsed.get("OTHER_KEY") != "abc":
        failures.append("secrets parser failed")
    if "EMPTY" in parsed:
        failures.append("empty secret values must be skipped")

    if failures:
        print("self-test FAILED:")
        for f in failures:
            print(f"  {f}")
        return 1
    print("self-test OK (DeepSeek-only Flash->Pro, effort=max)")
    return 0


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--scope",
        default="bin/echo",
        help="Comma-separated under hbsd/src (default: bin/echo smoke target)",
    )
    ap.add_argument("--limit", type=int, default=None)
    ap.add_argument("--skip", type=int, default=0, help="Skip first N work items (resume)")
    ap.add_argument("--file", action="append", default=[], help="Specific file(s) relative to repo root")
    ap.add_argument("--max-retries", type=int, default=2, help="Fix-up turns per tier before escalate")
    ap.add_argument(
        "--file-timeout",
        type=float,
        default=600.0,
        help="Per-file wall-clock timeout seconds (default 600; max-effort needs headroom)",
    )
    ap.add_argument(
        "--jobs",
        type=int,
        default=48,
        help="Flash concurrent slots (default 48)",
    )
    ap.add_argument(
        "--pro-jobs",
        type=int,
        default=24,
        help="Pro concurrent slots (default 24)",
    )
    ap.add_argument("--no-ir", action="store_true")
    ap.add_argument("--no-diff", action="store_true")
    ap.add_argument("--no-asan", action="store_true")
    ap.add_argument("--no-esbmc", action="store_true")
    ap.add_argument("--include-converted", action="store_true")
    ap.add_argument("--dry-run", action="store_true", help="Print routing, do not call APIs")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)

    if args.self_test:
        return run_self_test()

    used = load_secrets(ROOT)
    if used:
        print(f"secrets: loaded {used}")
    if not __import__("os").environ.get("DEEPSEEK_API_KEY"):
        print("ERROR: DEEPSEEK_API_KEY missing (secrets/api-keys or env)", file=sys.stderr)
        return 2

    prefixes = expand_scope(args.scope)
    files = args.file or None
    work = build_work(prefixes, files, include_converted=args.include_converted)
    if args.skip:
        work = work[args.skip :]
    if args.limit is not None:
        work = work[: args.limit]

    flash_n = max(1, args.jobs)
    pro_n = max(1, args.pro_jobs)
    print(
        f"agent-port files={len(work)} scope={prefixes} "
        f"retries={args.max_retries} timeout={args.file_timeout}s "
        f"jobs={flash_n} pro_jobs={pro_n} effort={deepseek_extra_body().get('reasoning_effort')} "
        f"models={MODEL_FLASH} -> {MODEL_PRO}"
    )
    if not work:
        print("nothing to do — refusals.jsonl empty or all converted in scope")
        return 0

    if args.dry_run:
        for ctx in work:
            print(
                f"  {ctx.source}  risk={ctx.risk_tier} env={ctx.envelope_hint} "
                f"start={model_name_for_tier(starting_tier(ctx))} "
                f"refusals={len(ctx.refusals)} stub={ctx.stub_path or '-'}"
            )
        return 0

    flash_sem = Semaphore(flash_n)
    pro_sem = Semaphore(pro_n)

    def sem_for(tier: int) -> Semaphore:
        return flash_sem if tier <= 1 else pro_sem

    def run_one(ctx: FileContext):
        tier = starting_tier(ctx)
        sem = sem_for(tier)
        with sem:
            try:
                session = FileSession(
                    ctx,
                    max_retries=args.max_retries,
                    file_timeout=args.file_timeout,
                    do_diff=not args.no_diff,
                    do_ir=not args.no_ir,
                    do_asan=not args.no_asan,
                    do_esbmc=not args.no_esbmc,
                    start_tier=tier,
                )
                return ctx, session.run()
            except Exception as e:
                return ctx, crash_record(ctx, e)

    converted = 0
    review = 0
    # Workers = flash + pro so Pro slots are not starved by the Flash pool size.
    pool = flash_n + pro_n
    t0 = time.monotonic()
    with ThreadPoolExecutor(max_workers=pool) as ex:
        futs = [ex.submit(run_one, ctx) for ctx in work]
        for fut in as_completed(futs):
            try:
                ctx, rec = fut.result()
            except Exception as e:
                print(f"  UNHANDLED future error: {e}", file=sys.stderr)
                continue
            update_progress(ctx, rec)
            if rec.status == "converted":
                converted += 1
                mark = "converted"
            else:
                review += 1
                mark = "NEEDS-REVIEW"
            print(
                f"  {mark} {ctx.source} model={rec.model_used} "
                f"in={rec.tokens_in} out={rec.tokens_out} "
                f"${rec.est_cost_usd} trail={rec.escalation_trail}"
            )

    elapsed = time.monotonic() - t0
    print(
        f"done converted={converted} needs-review={review} elapsed={elapsed:.1f}s "
        f"cost_log=docs/migration/clang_port/agent_port_cost.jsonl "
        f"failures=docs/migration/clang_port/agent_port_failures.jsonl"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
