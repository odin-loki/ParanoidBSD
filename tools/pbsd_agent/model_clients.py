"""Thin OpenAI-compatible chat clients for DeepSeek and Moonshot.

One class; only base_url, model, and API key differ per tier.
Message lists are append-only — callers must not rebuild prompts between turns
or prefix-cache hits are lost (see docs/plans/agent-port-master-plan.md §5.1).
"""
from __future__ import annotations

import json
import os
import urllib.error
import urllib.request
from dataclasses import dataclass, field
from typing import Any

DEEPSEEK_BASE = "https://api.deepseek.com"
MOONSHOT_BASE = "https://api.moonshot.ai/v1"

MODEL_FLASH = "deepseek-v4-flash"
MODEL_PRO = "deepseek-v4-pro"
MODEL_KIMI = "kimi-k3"

# USD per million tokens. Estimates for the cost log — override via env.
# Kimi K3 output is billed at $15/M including reasoning tokens (cannot disable).
RATES_USD_PER_M: dict[str, dict[str, float]] = {
    MODEL_FLASH: {"in": 0.14, "out": 0.28, "cache_hit": 0.014},
    MODEL_PRO: {"in": 0.55, "out": 2.19, "cache_hit": 0.055},
    MODEL_KIMI: {"in": 1.20, "out": 15.00, "cache_hit": 0.12},
}


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


class ChatClient:
    """Shared Chat Completions client. Swap base_url + model + key per tier."""

    def __init__(
        self,
        base_url: str,
        model: str,
        api_key: str,
        *,
        timeout: float = 180.0,
        extra_body: dict[str, Any] | None = None,
    ) -> None:
        if not api_key:
            raise ValueError(f"missing API key for model {model}")
        self.base_url = base_url
        self.model = model
        self.api_key = api_key
        self.timeout = timeout
        self.extra_body = extra_body or {}

    def complete(self, messages: list[dict[str, str]]) -> ChatResult:
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
        # Kimi K3 always emits reasoning; keep only the visible answer for the port.
        if not text and msg.get("reasoning_content"):
            text = str(msg["reasoning_content"])
        return ChatResult(
            text=text,
            model=self.model,
            usage=_parse_usage(raw.get("usage")),
            raw=raw,
        )


def client_for_tier(tier: int, *, timeout: float = 180.0) -> ChatClient:
    try:
        from pbsd_secrets import load_secrets
        load_secrets()
    except Exception:
        pass
    if tier <= 1:
        return ChatClient(
            os.environ.get("DEEPSEEK_BASE_URL", DEEPSEEK_BASE),
            MODEL_FLASH,
            os.environ.get("DEEPSEEK_API_KEY", ""),
            timeout=timeout,
        )
    if tier == 2:
        return ChatClient(
            os.environ.get("DEEPSEEK_BASE_URL", DEEPSEEK_BASE),
            MODEL_PRO,
            os.environ.get("DEEPSEEK_API_KEY", ""),
            timeout=timeout,
        )
    return ChatClient(
        os.environ.get("MOONSHOT_BASE_URL", MOONSHOT_BASE),
        MODEL_KIMI,
        os.environ.get("MOONSHOT_API_KEY", ""),
        timeout=timeout,
        extra_body={"reasoning_effort": os.environ.get("KIMI_REASONING_EFFORT", "low")},
    )


def model_name_for_tier(tier: int) -> str:
    return {1: MODEL_FLASH, 2: MODEL_PRO, 3: MODEL_KIMI}.get(tier, MODEL_KIMI)
