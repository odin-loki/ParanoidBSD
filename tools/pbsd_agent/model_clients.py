# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Thin OpenAI-compatible chat client for DeepSeek Flash / Pro.

One class; only model (and key/base) differ per tier.
Message lists are append-only — callers must not rebuild prompts between turns
or prefix-cache hits are lost (see docs/plans/agent-port-master-plan.md §5.1).

DeepSeek-only: no Moonshot / Kimi. Thinking mode + max reasoning effort by default.
"""
from __future__ import annotations

import json
import os
import urllib.error
import urllib.request
from dataclasses import dataclass, field
from typing import Any

DEEPSEEK_BASE = "https://api.deepseek.com"

MODEL_FLASH = "deepseek-v4-flash"
MODEL_PRO = "deepseek-v4-pro"

# Highest tier is Pro (2). Kept as MAX_TIER for session loops / CLI.
MAX_TIER = 2

# USD per million tokens. Estimates for the cost log — override via env.
RATES_USD_PER_M: dict[str, dict[str, float]] = {
    MODEL_FLASH: {"in": 0.14, "out": 0.28, "cache_hit": 0.014},
    MODEL_PRO: {"in": 0.55, "out": 2.19, "cache_hit": 0.055},
}

DEFAULT_REASONING_EFFORT = "max"


@dataclass
class Usage:
    prompt_tokens: int = 0
    completion_tokens: int = 0
    cache_hit_tokens: int = 0
    cache_miss_tokens: int = 0

    @property
    def tokens_in(self) -> int:
        return self.prompt_tokens

    @property
    def tokens_out(self) -> int:
        return self.completion_tokens


@dataclass
class ChatResult:
    text: str
    model: str
    usage: Usage = field(default_factory=Usage)
    raw: dict[str, Any] = field(default_factory=dict)
    reasoning_content: str | None = None


def estimate_cost_usd(model: str, usage: Usage) -> float:
    rates = RATES_USD_PER_M.get(model, {"in": 1.0, "out": 1.0, "cache_hit": 0.1})
    hit = usage.cache_hit_tokens
    miss = usage.cache_miss_tokens
    if miss == 0 and hit == 0:
        miss = usage.prompt_tokens
    inp = (miss * rates["in"] + hit * rates["cache_hit"]) / 1_000_000
    out = usage.completion_tokens * rates["out"] / 1_000_000
    return round(inp + out, 6)


def _endpoint(base_url: str) -> str:
    base = base_url.rstrip("/")
    if base.endswith("/chat/completions"):
        return base
    return f"{base}/chat/completions"


def _parse_usage(block: dict[str, Any] | None) -> Usage:
    if not block:
        return Usage()
    hit = int(
        block.get("prompt_cache_hit_tokens")
        or block.get("cache_creation_input_tokens")
        or 0
    )
    miss = int(block.get("prompt_cache_miss_tokens") or 0)
    prompt = int(block.get("prompt_tokens") or 0)
    if miss == 0 and hit and prompt:
        miss = max(0, prompt - hit)
    return Usage(
        prompt_tokens=prompt,
        completion_tokens=int(block.get("completion_tokens") or 0),
        cache_hit_tokens=hit,
        cache_miss_tokens=miss,
    )


def deepseek_extra_body() -> dict[str, Any]:
    """Thinking on + max effort (override with DEEPSEEK_REASONING_EFFORT)."""
    effort = os.environ.get("DEEPSEEK_REASONING_EFFORT", DEFAULT_REASONING_EFFORT).strip() or "max"
    return {
        "thinking": {"type": "enabled"},
        "reasoning_effort": effort,
    }


class ChatClient:
    """Shared Chat Completions client. Swap model (+ key/base) per tier."""

    def __init__(
        self,
        base_url: str,
        model: str,
        api_key: str,
        *,
        timeout: float = 300.0,
        extra_body: dict[str, Any] | None = None,
    ) -> None:
        if not api_key:
            raise ValueError(f"missing API key for model {model}")
        self.base_url = base_url
        self.model = model
        self.api_key = api_key
        self.timeout = timeout
        self.extra_body = extra_body or {}

    def complete(self, messages: list[dict[str, Any]]) -> ChatResult:
        body: dict[str, Any] = {
            "model": self.model,
            "messages": messages,
            **self.extra_body,
        }
        data = json.dumps(body).encode("utf-8")
        req = urllib.request.Request(
            _endpoint(self.base_url),
            data=data,
            method="POST",
            headers={
                "Authorization": f"Bearer {self.api_key}",
                "Content-Type": "application/json",
            },
        )
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as resp:
                raw = json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            detail = e.read().decode("utf-8", errors="replace")[-1500:]
            raise RuntimeError(f"{self.model} HTTP {e.code}: {detail}") from e
        choices = raw.get("choices") or []
        if not choices:
            raise RuntimeError(f"{self.model} returned no choices: {raw!r}"[:800])
        msg = choices[0].get("message") or {}
        text = msg.get("content") or ""
        reasoning = msg.get("reasoning_content")
        # Prefer visible answer; fall back to reasoning only if content empty.
        if not text and reasoning:
            text = str(reasoning)
        return ChatResult(
            text=text,
            model=self.model,
            usage=_parse_usage(raw.get("usage")),
            raw=raw,
            reasoning_content=str(reasoning) if reasoning else None,
        )


def client_for_tier(tier: int, *, timeout: float = 300.0) -> ChatClient:
    try:
        from pbsd_secrets import load_secrets

        load_secrets()
    except Exception:
        pass
    base = os.environ.get("DEEPSEEK_BASE_URL", DEEPSEEK_BASE)
    key = os.environ.get("DEEPSEEK_API_KEY", "")
    extra = deepseek_extra_body()
    if tier <= 1:
        return ChatClient(base, MODEL_FLASH, key, timeout=timeout, extra_body=extra)
    return ChatClient(base, MODEL_PRO, key, timeout=timeout, extra_body=extra)


def model_name_for_tier(tier: int) -> str:
    if tier <= 1:
        return MODEL_FLASH
    return MODEL_PRO
