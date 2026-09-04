# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tier 2 — local dataflow / intra-procedural (docs/plans/todo-passes.md)."""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

_OUT = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"


def _propose_soft(unit, kind: str, payload: dict) -> None:
    from ..proposals import propose

    propose(unit.path, kind, payload)



def _ref(unit: TranslationUnit, pass_name: str, reason: str, idx: int, snippet: str = "") -> Refusal:
    line, col = unit.line_col(idx)
    return Refusal(
        file=unit.path,
        line=line,
        col=col,
        pass_name=pass_name,
        reason_code=reason,
        enclosing_function=unit.enclosing_function(idx),
        snippet=(snippet or unit.snippet_at(idx))[:160],
    )


def _functions(text: str) -> list[tuple[str, int, int, str]]:
    """Return (name, start, end, body) for top-level-ish function bodies."""
    out: list[tuple[str, int, int, str]] = []
    for m in re.finditer(
        r"(?:^|\n)([\w\s\*]+?)\b([A-Za-z_]\w*)\s*\(([^;]*?)\)\s*\{",
        text,
    ):
        name = m.group(2)
        body_start = m.end() - 1  # at '{'
        depth = 0
        i = body_start
        while i < len(text):
            c = text[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    out.append((name, m.start(), i + 1, text[body_start : i + 1]))
                    break
            i += 1
    return out


class LocalMallocRaiiPass(Pass):
    """Superseded by promote_malloc_span.PromoteMallocRaiiPass (runs earlier)."""

    name = "local_malloc_raii"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        return PassResult.unchanged(unit.text)


class GotoCleanupLogPass(Pass):
    """Superseded by goto_cleanup_scope_guard / goto_structured (refuse there only)."""

    name = "goto_cleanup_log"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        return PassResult.unchanged(unit.text)


class StackBufArrayPass(Pass):
    name = "stack_buf_array"
    tier = 2

    DATA_APIS = frozenset(
        {
            "strlen",
            "strcpy",
            "strncpy",
            "strlcpy",
            "strlcat",
            "strcmp",
            "strncmp",
            "snprintf",
            "sprintf",
            "printf",
            "fprintf",
            "puts",
            "fputs",
            "memcpy",
            "memmove",
            "memset",
            "bzero",
            "read",
            "write",
            "pread",
            "pwrite",
            "fread",
            "fwrite",
            "open",
            "stat",
            "fstat",
            "lstat",
            "access",
            "atoi",
            "atol",
            "strtol",
            "strtoul",
            "strchr",
            "strrchr",
            "strstr",
            "basename",
            "dirname",
        }
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        arr_ops: list[tuple[int, int, str, str]] = []
        call_ops: list[tuple[int, int, str]] = []

        for fname, fstart, fend, body in _functions(text):
            # body is text[body_start:fend]; fstart is match start (not '{')
            body_start = fend - len(body)
            for m in re.finditer(r"\bchar\s+(\w+)\s*\[\s*(\d+)\s*\]\s*;", body):
                var, n = m.group(1), m.group(2)
                abs_i = body_start + m.start()
                if re.search(rf"&{re.escape(var)}\b", body):
                    _propose_soft(unit, "STACK_BUF_ARRAY", {"line": unit.line_col(abs_i)[0], "snippet": "address-taken"})
                    continue
                pending_calls: list[tuple[int, int, str]] = []
                bad = False
                for cm in re.finditer(
                    rf"\b([A-Za-z_]\w*)\s*\(([^)]*\b{re.escape(var)}\b[^)]*)\)",
                    body,
                ):
                    callee, args = cm.group(1), cm.group(2)
                    # Skip if already .data() or only subscripted in args text
                    if not re.search(rf"\b{re.escape(var)}\b(?!\s*(?:\.data\s*\(|\[))", args):
                        continue
                    if callee in self.DATA_APIS:
                        new_args = re.sub(
                            rf"\b{re.escape(var)}\b(?!\s*(?:\.data\s*\(|\[))",
                            f"{var}.data()",
                            args,
                        )
                        pending_calls.append(
                            (body_start + cm.start(2), body_start + cm.end(2), new_args)
                        )
                    elif callee in ("if", "while", "for", "switch", "sizeof"):
                        continue
                    else:
                        bad = True
                        break
                if bad:
                    _propose_soft(unit, "STACK_BUF_ARRAY", {"line": unit.line_col(abs_i)[0], "snippet": "passed-to-fn"})
                    continue
                call_ops.extend(pending_calls)
                old = m.group(0)
                new = f"std::array<char, {n}> {var}{{}};"
                arr_ops.append((body_start + m.start(), body_start + m.end(), new, old))

        if not arr_ops and not call_ops:
            return PassResult(text=text, refusals=refusals, edits=edits)

        # Apply all text ops from end so earlier offsets stay valid.
        unified: list[tuple[int, int, str, str, str]] = []
        for start, end, new_args in call_ops:
            unified.append((start, end, new_args, "buf.data()", ""))
        for start, end, new, old in arr_ops:
            unified.append((start, end, new, "char[]->std::array", old))
        for start, end, payload, desc, old in sorted(unified, key=lambda x: x[0], reverse=True):
            text = text[:start] + payload + text[end:]
            edits.append(Edit(self.name, desc, unit.line_col(start)[0], old, payload[:60]))

        if arr_ops and "#include <array>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <array>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text
        return PassResult(text=text, refusals=refusals, edits=edits)


class ConstInferenceLogPass(Pass):
    name = "const_inference_log"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        refusals = []
        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            unit.mask_strings_comments(),
        ):
            params = m.group(2)
            for p in params.split(","):
                p = p.strip()
                if not p or p == "void" or "const" in p:
                    continue
                # pointer params that look unread-for-write
                if "*" in p:
                    name_m = re.search(r"([A-Za-z_]\w*)\s*$", p.replace("[]", ""))
                    if not name_m:
                        continue
                    pname = name_m.group(1)
                    # look ahead in a window for writes through pname
                    window = unit.text[m.end() : m.end() + 4000]
                    if re.search(rf"\*{re.escape(pname)}\s*=", window):
                        continue
                    if re.search(rf"{re.escape(pname)}\s*\[.*?\]\s*=", window):
                        continue
                    refusals.append(
                        _ref(unit, self.name, "CONST_CANDIDATE", m.start(), p[:80])
                    )
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class DeadStoreLogPass(Pass):
    name = "dead_store_log"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        # Log only — never rewrite (ABI).
        refusals = []
        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*[^;]+;\s*(?:/\*.*?\*/\s*)?\1\s*=", unit.text):
            refusals.append(_ref(unit, self.name, "DEAD_STORE", m.start(), m.group(0)[:80]))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class RangeForLogPass(Pass):
    name = "range_for_log"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        refusals = []
        rx = re.compile(
            r"for\s*\(\s*(?:unsigned\s+|size_t\s+|int\s+)?(\w+)\s*=\s*0\s*;\s*\1\s*<\s*(\w+)\s*;\s*\+\+\1\s*\)"
        )
        for m in rx.finditer(unit.mask_strings_comments()):
            refusals.append(_ref(unit, self.name, "RANGE_FOR_CANDIDATE", m.start(), m.group(0)[:80]))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


TIER2_PASSES: list[Pass] = [
    # LocalMallocRaiiPass superseded by promote_malloc_span.PromoteMallocRaiiPass
    StackBufArrayPass(),
]
