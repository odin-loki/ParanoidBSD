# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Consolidated per-file session: one context, tool loop, escalate-up-only.

System prompt + original source + stub + refusals are prepended once and never
rewritten. Fix-up turns append. That is what makes DeepSeek prefix cache
actually fire on retries.

DeepSeek-only: Flash → Pro, then NEEDS-REVIEW (no Kimi).
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from pbsd_passes.compile_db import default_flags, find_clang
from pbsd_passes.differential import differential
from pbsd_passes.ir_oracle import compare_ir, find_clangxx
from pbsd_passes.schema import PortRecord, StageEvidence

from .esbmc_check import esbmc_check
from .model_clients import (
    MAX_TIER,
    ChatClient,
    Usage,
    client_for_tier,
    estimate_cost_usd,
    model_name_for_tier,
)

ROOT = Path(__file__).resolve().parents[2]
CORPUS_OUT = ROOT / "docs" / "migration" / "clang_port" / "agent_corpus"
ASAN_CACHE = ROOT / "docs" / "migration" / "clang_port" / "asan_baseline"
COST_LOG = ROOT / "docs" / "migration" / "clang_port" / "agent_port_cost.jsonl"
FAILURES_LOG = ROOT / "docs" / "migration" / "clang_port" / "agent_port_failures.jsonl"

# Hard lineages start on Pro (never Flash alone).
FORCE_PRO_MARKERS = (
    "/sched",
    "kern_switch",
    "kern_malloc",
    "/uma/",
    "vm_page",
    "libcapsicum",
    "capsicum",
    "sys_capability",
    "/uda",
    "bifrost",
    "/vmm",
    "intentional_ub",
)

SYSTEM_PROMPT = """You are porting HardenedBSD C to C++23 for PBSD Stage F/G.

You receive one file's original C, its existing .cppm stub, refusal reasons from
the deterministic passes, and risk/envelope metadata. Produce a faithful port.

FAITHFUL means: preserve behaviour exactly, including bugs, integer signedness,
evaluation order and pointer arithmetic. Do not improve anything.

Reply with a single JSON object (no markdown wrapper) with keys:
  spec_notes     — informal Stage B notes as a comment block
  port_cppm      — full replacement text for the .cppm stub
  corpus_c       — optional Stage D corpus input (C, same format as tools/pbsd_passes/corpus/*.c)
  corpus_expected — optional companion .expected text

If a construct cannot be ported faithfully, leave it out and say so in spec_notes.
Never emit Status::NotImplemented, stubs, or TODO bodies.
Never claim a file is correct — the compile / ASan / differential / ESBMC tools run after you.
"""

FIXUP_PROMPT = """The verification tools reported failures. Fix the port in the SAME JSON shape.
Do not reload context; only change what the tools rejected.

Tool report:
{report}
"""


@dataclass
class FileContext:
    source: str
    c_text: str
    stub_path: Path | None
    stub_text: str
    refusals: list[dict]
    risk_tier: int
    envelope_hint: str
    wave: str = "wave2"
    prior_record: dict | None = None


@dataclass
class ToolReport:
    compile: dict = field(default_factory=dict)
    asan: dict = field(default_factory=dict)
    differential: dict = field(default_factory=dict)
    ir: dict = field(default_factory=dict)
    esbmc: dict = field(default_factory=dict)

    def all_green(self, *, require_esbmc: bool) -> bool:
        if not self.compile.get("ok"):
            return False
        if self.asan.get("status") == "failed":
            return False
        if self.differential.get("status") in {"mismatch", "build_fail"}:
            return False
        if self.ir.get("status") == "mismatch":
            return False
        if require_esbmc and self.esbmc.get("status") == "failed":
            return False
        return True

    def as_feedback(self) -> str:
        return json.dumps(
            {
                "compile": self.compile,
                "asan": self.asan,
                "differential": self.differential,
                "ir": self.ir,
                "esbmc": {k: v for k, v in self.esbmc.items() if k != "detail"}
                | {"detail": (self.esbmc.get("detail") or "")[-1200:]},
            },
            indent=2,
        )[:8000]


def starting_tier(ctx: FileContext) -> int:
    path = ctx.source.replace("\\", "/").lower()
    if any(m in path for m in FORCE_PRO_MARKERS):
        return 2
    if ctx.risk_tier == 1:
        return 2
    return 1


def append_failure(record: PortRecord, *, detail: str = "") -> None:
    """Persist failed / NEEDS-REVIEW work so it can be resumed later."""
    FAILURES_LOG.parent.mkdir(parents=True, exist_ok=True)
    row = {
        "source": record.source,
        "status": record.status,
        "model_used": record.model_used,
        "escalation_trail": record.escalation_trail,
        "stage_evidence": record.stage_evidence,
        "est_cost_usd": record.est_cost_usd,
        "detail": detail,
        "ts": time.time(),
    }
    with FAILURES_LOG.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(row, sort_keys=True) + "\n")


def parse_agent_payload(text: str) -> dict:
    """Accept a raw JSON object or fenced ```json block."""
    blob = text.strip()
    fence = re.search(r"```(?:json)?\s*(\{.*?\})\s*```", blob, re.S)
    if fence:
        blob = fence.group(1)
    else:
        start = blob.find("{")
        end = blob.rfind("}")
        if start >= 0 and end > start:
            blob = blob[start : end + 1]
    try:
        data = json.loads(blob)
    except json.JSONDecodeError:
        return {"spec_notes": "", "port_cppm": text, "parse_error": "not json"}
    if not isinstance(data, dict):
        return {"spec_notes": "", "port_cppm": text, "parse_error": "not object"}
    return data


def _write_cost_line(record: dict) -> None:
    COST_LOG.parent.mkdir(parents=True, exist_ok=True)
    with COST_LOG.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(record, sort_keys=True) + "\n")


def _asan_cache_key(src: Path) -> Path:
    ASAN_CACHE.mkdir(parents=True, exist_ok=True)
    return ASAN_CACHE / (src.as_posix().replace("/", "_") + ".json")


def _run_sanitized(compiler: str, src: Path, lang: list[str], timeout: float) -> dict:
    flags = default_flags(ROOT)
    with tempfile.TemporaryDirectory(prefix="pbsd_asan_") as td:
        out = Path(td) / "prog"
        cmd = [
            compiler,
            *lang,
            "-O0",
            "-g",
            "-fsanitize=address,undefined",
            *flags,
            str(src),
            "-o",
            str(out),
        ]
        build = subprocess.run(cmd, capture_output=True, text=True, timeout=min(120, timeout))
        if build.returncode != 0:
            return {
                "status": "build_fail",
                "stderr": (build.stderr or "")[-1500:],
            }
        if b"int main" not in src.read_bytes() and b"main(" not in src.read_bytes():
            return {"status": "compile_only", "ok": True}
        env = os.environ.copy()
        env["ASAN_OPTIONS"] = "abort_on_error=1:detect_leaks=0"
        run = subprocess.run(
            [str(out)],
            capture_output=True,
            timeout=15,
            env=env,
        )
        if run.returncode != 0:
            return {
                "status": "failed",
                "exit": run.returncode,
                "stderr": (run.stderr or b"").decode("utf-8", errors="replace")[-1500:],
            }
        return {"status": "ok", "ok": True, "exit": 0}


class FileSession:
    """One file, one append-only message list, escalate-up-only."""

    def __init__(
        self,
        ctx: FileContext,
        *,
        max_retries: int = 2,
        file_timeout: float = 300.0,
        do_diff: bool = True,
        do_ir: bool = True,
        do_asan: bool = True,
        do_esbmc: bool = True,
        start_tier: int | None = None,
    ) -> None:
        self.ctx = ctx
        self.max_retries = max_retries
        self.file_timeout = file_timeout
        self.do_diff = do_diff
        self.do_ir = do_ir
        self.do_asan = do_asan
        self.do_esbmc = do_esbmc
        self.tier = start_tier or starting_tier(ctx)
        self.messages: list[dict[str, Any]] = []
        self.usage_total = Usage()
        self.est_cost = 0.0
        self.trail: list[dict] = []
        self.model_used = model_name_for_tier(self.tier)
        self._frozen_prefix = 0
        self._build_prefix()

    def _build_prefix(self) -> None:
        refusals = json.dumps(self.ctx.refusals[:40], indent=2)[:6000]
        prior = ""
        if self.ctx.prior_record:
            prior = "\nPrior NEEDS-REVIEW state:\n" + json.dumps(self.ctx.prior_record)[:2000]
        user = (
            f"SOURCE: {self.ctx.source}\n"
            f"RISK_TIER: {self.ctx.risk_tier}  ENVELOPE: {self.ctx.envelope_hint}\n"
            f"STUB: {self.ctx.stub_path or '(none)'}\n\n"
            f"--- original C ---\n{self.ctx.c_text}\n\n"
            f"--- existing stub ---\n{self.ctx.stub_text}\n\n"
            f"--- refusals.jsonl ---\n{refusals}\n"
            f"{prior}"
        )
        self.messages = [
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": user},
        ]
        self._frozen_prefix = len(self.messages)

    def prefix_is_frozen(self) -> bool:
        return self.messages[: self._frozen_prefix] == [
            {"role": "system", "content": SYSTEM_PROMPT},
            self.messages[1],
        ]

    def _call(self, client: ChatClient) -> str:
        result = client.complete(self.messages)
        # DeepSeek multi-turn thinking: pass reasoning_content back (API ignores it
        # for billing but requires the field shape on later turns).
        assistant: dict[str, Any] = {"role": "assistant", "content": result.text}
        if result.reasoning_content:
            assistant["reasoning_content"] = result.reasoning_content
        self.messages.append(assistant)
        self.usage_total.prompt_tokens += result.usage.prompt_tokens
        self.usage_total.completion_tokens += result.usage.completion_tokens
        self.usage_total.cache_hit_tokens += result.usage.cache_hit_tokens
        self.usage_total.cache_miss_tokens += result.usage.cache_miss_tokens
        self.est_cost += estimate_cost_usd(result.model, result.usage)
        self.model_used = result.model
        return result.text

    def _apply_payload(self, payload: dict) -> Path | None:
        port = payload.get("port_cppm") or ""
        dest = self.ctx.stub_path
        if dest and port:
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_text(port, encoding="utf-8")
        notes = payload.get("spec_notes") or ""
        if dest and notes:
            spec = dest.with_suffix(".cppm.spec.txt")
            spec.write_text(notes, encoding="utf-8")
        corpus_c = payload.get("corpus_c") or ""
        if corpus_c:
            CORPUS_OUT.mkdir(parents=True, exist_ok=True)
            stem = Path(self.ctx.source).stem
            (CORPUS_OUT / f"agent_{stem}.c").write_text(corpus_c, encoding="utf-8")
            expected = payload.get("corpus_expected") or ""
            if expected:
                (CORPUS_OUT / f"agent_{stem}.expected").write_text(expected, encoding="utf-8")
        return dest if dest and dest.is_file() else None

    def _compile(self, cxx: Path) -> dict:
        clangxx = find_clangxx()
        flags = ["-std=c++23", "-fsyntax-only", "-Wno-everything"]
        if cxx.suffix == ".cppm":
            flags += ["-fmodules-ts"]
        try:
            proc = subprocess.run(
                [clangxx, *flags, str(cxx)],
                capture_output=True,
                text=True,
                timeout=min(120, self.file_timeout),
            )
        except subprocess.TimeoutExpired:
            return {"ok": False, "status": "timeout"}
        except FileNotFoundError:
            return {"ok": False, "status": "no_clangxx"}
        return {
            "ok": proc.returncode == 0,
            "status": "ok" if proc.returncode == 0 else "error",
            "stderr": (proc.stderr or "")[-1500:],
        }

    def _tools(self, cxx: Path | None) -> ToolReport:
        report = ToolReport()
        c_src = ROOT / self.ctx.source
        if cxx is None:
            report.compile = {"ok": False, "status": "no_output"}
            return report
        report.compile = self._compile(cxx)
        if self.do_asan:
            cache = _asan_cache_key(c_src)
            if cache.is_file():
                baseline = json.loads(cache.read_text(encoding="utf-8"))
            else:
                try:
                    baseline = _run_sanitized(
                        find_clang(), c_src, ["-x", "c", "-std=c17"], self.file_timeout
                    )
                except (subprocess.TimeoutExpired, OSError) as e:
                    baseline = {"status": "error", "detail": str(e)}
                cache.write_text(json.dumps(baseline), encoding="utf-8")
            try:
                ported = _run_sanitized(
                    find_clangxx(), cxx, ["-x", "c++", "-std=c++23"], self.file_timeout
                )
            except (subprocess.TimeoutExpired, OSError) as e:
                ported = {"status": "error", "detail": str(e)}
            report.asan = {"baseline": baseline, **ported}
        if self.do_diff and c_src.is_file():
            try:
                report.differential = differential(c_src, cxx)
            except Exception as e:
                report.differential = {"status": "error", "equal": False, "detail": str(e)}
        if self.do_ir and c_src.is_file():
            try:
                report.ir = compare_ir(c_src, cxx)
            except Exception as e:
                report.ir = {"status": "error", "equal": False, "detail": str(e)}
        if self.do_esbmc:
            es = esbmc_check(cxx, timeout=min(60.0, self.file_timeout / 4))
            report.esbmc = {
                "status": es.status,
                "sat": es.sat,
                "detail": es.detail,
                "counterexample": es.counterexample,
            }
        return report

    def run(self) -> PortRecord:
        deadline = time.monotonic() + self.file_timeout
        evidence = StageEvidence()
        last_report = ToolReport()
        while self.tier <= MAX_TIER:
            if time.monotonic() > deadline:
                self.trail.append(
                    {
                        "tier": self.tier,
                        "model": model_name_for_tier(self.tier),
                        "reason": "file_timeout",
                        "retries": 0,
                    }
                )
                break
            remaining = max(30.0, deadline - time.monotonic())
            try:
                client = client_for_tier(self.tier, timeout=min(300.0, remaining))
            except ValueError as e:
                self.trail.append(
                    {
                        "tier": self.tier,
                        "model": model_name_for_tier(self.tier),
                        "reason": str(e),
                        "retries": 0,
                    }
                )
                self.tier += 1
                continue

            retries = 0
            reason = "draft"
            while retries <= self.max_retries:
                if time.monotonic() > deadline:
                    reason = "file_timeout"
                    break
                try:
                    text = self._call(client)
                except Exception as e:
                    reason = f"api_error: {e}"
                    retries += 1
                    if retries > self.max_retries:
                        break
                    continue
                payload = parse_agent_payload(text)
                dest = self._apply_payload(payload)
                evidence.B = "pass" if payload.get("spec_notes") else "fail"
                evidence.F = "pass" if dest else "fail"
                evidence.D = "pass" if payload.get("corpus_c") else "skip"
                last_report = self._tools(dest)
                evidence.C = last_report.asan.get("status", "skip")
                evidence.G = (
                    "pass"
                    if last_report.differential.get("equal")
                    or last_report.esbmc.get("status") == "ok"
                    else last_report.differential.get("status", "skip")
                )
                if last_report.all_green(require_esbmc=self.do_esbmc and last_report.esbmc.get("status") != "skipped"):
                    rec = self._record("converted", evidence)
                    self._log_cost(rec, "converted")
                    return rec
                retries += 1
                reason = "tool_failure"
                self.messages.append(
                    {
                        "role": "user",
                        "content": FIXUP_PROMPT.format(report=last_report.as_feedback()),
                    }
                )
            self.trail.append(
                {
                    "tier": self.tier,
                    "model": model_name_for_tier(self.tier),
                    "reason": reason,
                    "retries": retries,
                }
            )
            self.tier += 1
            # Keep the same append-only list when escalating — prefix stays cached.

        rec = self._record("NEEDS-REVIEW", evidence)
        self._log_cost(rec, "NEEDS-REVIEW")
        append_failure(rec)
        return rec

    def _record(self, status: str, evidence: StageEvidence) -> PortRecord:
        return PortRecord(
            source=self.ctx.source,
            model_used=self.model_used,
            escalation_trail=list(self.trail),
            stage_evidence=evidence.to_dict(),
            tokens_in=self.usage_total.tokens_in,
            tokens_out=self.usage_total.tokens_out,
            est_cost_usd=round(self.est_cost, 6),
            status=status,
            cache_hit_tokens=self.usage_total.cache_hit_tokens,
            cache_miss_tokens=self.usage_total.cache_miss_tokens,
        )

    def _log_cost(self, rec: PortRecord, status: str) -> None:
        _write_cost_line(
            {
                "source": rec.source,
                "model_used": rec.model_used,
                "status": status,
                "tokens_in": rec.tokens_in,
                "tokens_out": rec.tokens_out,
                "cache_hit_tokens": rec.cache_hit_tokens,
                "cache_miss_tokens": rec.cache_miss_tokens,
                "est_cost_usd": rec.est_cost_usd,
                "escalation_trail": rec.escalation_trail,
            }
        )
