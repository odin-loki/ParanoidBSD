# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Promote NULLABILITY / BIT_CAST / GOTO cleanup / ERROR_CODE_EXPECTED sites.

Goal: keep refusals only for true failures (unchecked deref, opaque punning,
multi-resource goto cleanup, mixed error conventions).
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

_OUT = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"


def _propose_null(unit, kind: str, payload: dict) -> None:
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


def _functions(text: str) -> list[tuple[str, int, int, str, str]]:
    """Return (name, abs_fn_start, abs_fn_end, body, ret_type)."""
    out: list[tuple[str, int, int, str, str]] = []
    for m in re.finditer(
        r"(?:^|\n)([\w\s\*]+?)\b([A-Za-z_]\w*)\s*\(([^;]*?)\)\s*\{",
        text,
    ):
        name = m.group(2)
        ret = m.group(1).strip()
        body_start = m.end() - 1
        depth = 0
        i = body_start
        while i < len(text):
            c = text[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    out.append((name, m.start(), i + 1, text[body_start : i + 1], ret))
                    break
            i += 1
    return out


def _ensure_includes(text: str, headers: list[str]) -> str:
    missing = [h for h in headers if h not in text]
    if not missing:
        return text
    hdr = "".join(missing)
    m = list(re.finditer(r"(?m)^#include\b.*$", text))
    if m:
        pos = m[-1].end()
        return text[:pos] + "\n" + hdr + text[pos:]
    return hdr + text


# ---------------------------------------------------------------------------
# 1. NULLABILITY
# ---------------------------------------------------------------------------


class NullabilityDominatingCheckPass(Pass):
    """Linear scan: refuse only when `p->` / `*p` appears before a dominating null check.

    Null-check sites themselves are never refused. Pointer universe = pointer params
    plus names that appear in null-check-with-exit (avoids prose/`*NOTREACHED*` FPs).
    Also: assert(p != NULL) → assert(p != nullptr).
    """

    name = "nullability_rewrite"
    tier = 3

    _NULL_CHECK = re.compile(
        r"\bif\s*\(\s*(?:"
        r"!(?P<bang>\w+)"
        r"|(?P<eq>\w+)\s*==\s*(?:nullptr|NULL|0)"
        r"|(?:nullptr|NULL|0)\s*==\s*(?P<eq2>\w+)"
        r")\s*\)\s*"
        r"(?:"
        r"(?:return|goto|continue|break)\b"
        r"|\{[^}]{0,400}?\b(?:return|goto|continue|break)\b"
        r")",
        re.S,
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        ops: list[tuple[int, int, str, str]] = []

        for m in re.finditer(
            r"\bassert\s*\(\s*(\w+)\s*!=\s*NULL\s*\)",
            unit.mask_strings_comments(),
        ):
            old = text[m.start() : m.end()]
            new = f"assert({m.group(1)} != nullptr)"
            ops.append((m.start(), m.end(), new, old))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(Edit(self.name, "assert nullptr", unit.line_col(start)[0], old, new))

        for _fname, fstart, fend, body, _ret in _functions(text):
            abs_body = fend - len(body)
            # Pointer params from signature
            head = text[fstart : abs_body + 1]
            ptrs: set[str] = set()
            open_p, close_p = head.rfind("("), head.rfind(")")
            if 0 <= open_p < close_p:
                for p in head[open_p + 1 : close_p].split(","):
                    if "*" not in p:
                        continue
                    nm = re.search(r"([A-Za-z_]\w*)\s*(?:\[\s*\])?\s*$", p.strip())
                    if nm:
                        ptrs.add(nm.group(1))

            events: list[tuple[int, str, str]] = []
            for m in self._NULL_CHECK.finditer(body):
                name = m.group("bang") or m.group("eq") or m.group("eq2")
                if name:
                    ptrs.add(name)
                    events.append((m.start(), "check", name))
            for m in re.finditer(r"\bassert\s*\(\s*(\w+)\s*(?:!=\s*(?:nullptr|NULL))?\s*\)", body):
                ptrs.add(m.group(1))
                events.append((m.start(), "check", m.group(1)))

            if not ptrs:
                continue

            for pname in ptrs:
                for m in re.finditer(rf"\b{re.escape(pname)}\s*->", body):
                    events.append((m.start(), "deref", pname))
                for m in re.finditer(rf"(?<![\w.])\*\s*{re.escape(pname)}\b", body):
                    before = body[max(0, m.start() - 40) : m.start()]
                    if re.search(
                        r"\b(?:char|int|void|long|short|unsigned|signed|const|struct|FILE|[\w]+_t)\s*$",
                        before,
                    ):
                        continue
                    events.append((m.start(), "deref", pname))

            # Only names that have at least one null-check-with-exit are candidates.
            # Never-checked params are treated as non-null-by-contract (no refusal).
            checked_names = {n for off, kind, n in events if kind == "check"}
            if not checked_names:
                continue
            events = [e for e in events if e[2] in checked_names]
            events.sort(key=lambda e: e[0])
            checked: set[str] = set()
            refused: set[str] = set()
            for off, kind, name in events:
                if kind == "check":
                    checked.add(name)
                elif kind == "deref" and name not in checked and name not in refused:
                    refused.add(name)
                    _propose_null(
                        unit,
                        "NULLABILITY",
                        {
                            "line": unit.line_col(abs_body + off)[0],
                            "snippet": f"maybe-null:{name}",
                        },
                    )

        if not edits and not refusals:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=refusals, edits=edits)


# ---------------------------------------------------------------------------
# 2. BIT_CAST_CANDIDATE
# ---------------------------------------------------------------------------


class BitCastExpandRewritePass(Pass):
    """Expand memcpy / *(T*)&x → std::bit_cast; refuse opaque punning only."""

    name = "bit_cast_rewrite"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        ops: list[tuple[int, int, str, str]] = []
        covered: set[int] = set()

        rx_memcpy = re.compile(
            r"memcpy\s*\(\s*&(\w+)\s*,\s*&(\w+)\s*,\s*sizeof\s*\(\s*([^)]+?)\s*\)\s*\)\s*;"
        )
        for m in rx_memcpy.finditer(masked):
            dst, src, sz = m.group(1), m.group(2), m.group(3).strip()
            type_ok = False
            if sz == dst or sz == src:
                type_ok = True
            elif re.fullmatch(r"(?:struct\s+|union\s+|enum\s+)?[\w:]+", sz):
                # Type matches decltype(dst) when a nearby decl uses that type for dst
                type_ok = (
                    re.search(rf"\b{re.escape(sz)}\s+\*?{re.escape(dst)}\b", text) is not None
                )
            if not type_ok:
                continue
            old = text[m.start() : m.end()]
            new = f"{dst} = std::bit_cast<std::remove_reference_t<decltype({dst})>>({src});"
            ops.append((m.start(), m.end(), new, old))
            covered.add(m.start())

        rx_star = re.compile(
            r"\*\s*\(\s*((?:unsigned\s+|signed\s+|long\s+|short\s+|const\s+|volatile\s+)*"
            r"(?:char|int|short|long|float|double|size_t|uint\d+_t|int\d+_t|u_int|[\w]+_t|[\w:]+))"
            r"\s*\*\s*\)\s*&(\w+)\b"
        )
        star_rewritten: set[int] = set()
        for m in rx_star.finditer(masked):
            typ, src = m.group(1).strip(), m.group(2)
            old = text[m.start() : m.end()]
            new = f"std::bit_cast<{typ}>({src})"
            ops.append((m.start(), m.end(), new, old))
            star_rewritten.add(m.start())

        # Plain memcpy → mop_up MemcpyByteSpanPass (std::memcpy); only refuse
        # true type-pun patterns below.

        for m in re.finditer(r"\bunion\s*(?:\w+\s*)?\{[^}]{0,200}?\}\s*(\w+)", masked, re.S):
            uname = m.group(1)
            members = re.findall(r"^\s*([\w\s\*]+?)\s+(\w+)\s*;", m.group(0), re.M)
            if len(members) < 2:
                continue
            a, b = members[0][1], members[1][1]
            window = text[m.end() : m.end() + 400]
            if re.search(rf"\b{re.escape(uname)}\s*\.\s*{re.escape(a)}\b", window) and re.search(
                rf"\b{re.escape(uname)}\s*\.\s*{re.escape(b)}\b", window
            ):
                _propose_null(unit, "BIT_CAST_CANDIDATE", {"line": unit.line_col(m.start())[0], "snippet": "union-pun"})

        for m in re.finditer(r"\*\s*\(\s*[\w\s\*]+\s*\*\)\s*&", masked):
            if m.start() in star_rewritten:
                continue
            # Try rewrite: *(T*)&x / *(T*)&(x.y)
            mm = re.match(
                r"\*\s*\(\s*((?:struct\s+|union\s+|unsigned\s+|signed\s+|long\s+|short\s+|const\s+)*[\w:]+)\s*\*\s*\)\s*&(\w+(?:\.\w+|\[\w+\])*)",
                text[m.start() : m.start() + 120],
            )
            if mm:
                typ, src = mm.group(1).strip(), mm.group(2)
                old = text[m.start() : m.start() + mm.end()]
                new = f"std::bit_cast<{typ}>({src})"
                ops.append((m.start(), m.start() + mm.end(), new, old))
                star_rewritten.add(m.start())
                continue
            _propose_null(
                unit,
                "BIT_CAST_CANDIDATE",
                {
                    "line": unit.line_col(m.start())[0],
                    "snippet": text[m.start() : m.start() + 40],
                },
            )

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, "bit_cast", line, old[:60], new[:60]))

        if edits:
            text = _ensure_includes(text, ["#include <bit>\n", "#include <type_traits>\n"])
        if not edits and not refusals:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=refusals, edits=edits)


# ---------------------------------------------------------------------------
# 3. GOTO_CROSS_INIT / GOTO_CLEANUP_CANDIDATE
# ---------------------------------------------------------------------------


class GotoCleanupUniquePtrPass(Pass):
    """Convert simple fail:/out: free(p); return … into unique_ptr RAII.

    Refuse when >2 resources or cleanup is not a simple free+return.
    """

    name = "goto_cleanup_scope_guard"
    tier = 2
    _LABELS = ("fail", "error", "out", "cleanup", "done")

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        work: list[tuple[int, int, str]] = []
        need_mem = False
        label_alt = "|".join(self._LABELS)

        for fname, fstart, fend, body, _ret in _functions(text):
            abs_body = fend - len(body)
            label_ms = list(re.finditer(rf"(?m)^[ \t]*(?P<label>{label_alt})\s*:", body, re.I))
            # Only care about gotos to known cleanup labels (leave other gotos alone).
            cleanup_gotos = list(
                re.finditer(rf"\bgoto\s+(?P<label>{label_alt})\s*;", body, re.I)
            )
            if not cleanup_gotos:
                continue
            if not label_ms:
                # Body extractor often truncates around switch/getopt; don't
                # spam the model queue with false "missing label" refusals.
                continue

            # var -> (label_name, free_start, free_end)
            simple: dict[str, tuple[str, int, int]] = {}
            complex_labels: set[str] = set()

            for lab in label_ms:
                label_name = lab.group("label")
                after = body[lab.end() : lab.end() + 240]
                free_m = re.match(
                    r"\s*free\s*\(\s*(\w+)\s*\)\s*;\s*(?:return\b[^;]*;|)",
                    after,
                    re.S,
                )
                if not free_m:
                    if re.search(rf"\bgoto\s+{re.escape(label_name)}\s*;", body, re.I):
                        if re.search(r"\b(free|close|munmap)\s*\(", after):
                            complex_labels.add(label_name)
                        # bare return labels: no refusal (not a cleanup chain)
                    continue
                var = free_m.group(1)
                rel = re.search(rf"free\s*\(\s*{re.escape(var)}\s*\)\s*;", after)
                if not rel:
                    continue
                fs, fe = lab.end() + rel.start(), lab.end() + rel.end()
                simple[var] = (label_name, fs, fe)

            for label_name in sorted(complex_labels):
                gm = re.search(rf"\bgoto\s+{re.escape(label_name)}\s*;", body, re.I)
                if gm:
                    _propose_null(
                        unit,
                        "GOTO_CLEANUP_CANDIDATE",
                        {
                            "line": unit.line_col(abs_body + gm.start())[0],
                            "snippet": f"complex-cleanup:{label_name}",
                        },
                    )

            if complex_labels and not simple:
                continue

            if len(simple) > 2:
                for m in re.finditer(r"\bgoto\s+\w+\s*;", body):
                    _propose_null(
                        unit,
                        "GOTO_CLEANUP_CANDIDATE",
                        {
                            "line": unit.line_col(abs_body + m.start())[0],
                            "snippet": m.group(0) + ": >2 resources",
                        },
                    )
                continue

            if not simple:
                continue

            new_body = body
            # Remove frees from end to start, then insert guards from end to start
            free_ops = sorted(simple.items(), key=lambda kv: kv[1][1], reverse=True)
            for var, (_ln, fs, fe) in free_ops:
                free_old = new_body[fs:fe]
                new_body = new_body[:fs] + f"/* pbsd: {var} freed by unique_ptr */" + new_body[fe:]
                # Find last assignment to var
                assign = None
                for am in re.finditer(rf"\b{re.escape(var)}\s*=\s*[^;]+;", new_body):
                    assign = am
                if assign is None:
                    gm = re.search(r"\bgoto\s+\w+\s*;", new_body)
                    if gm:
                        _propose_null(unit, "GOTO_CLEANUP_CANDIDATE", {"line": unit.line_col(abs_body + gm.start())[0], "snippet": f"no-assign:{var}",})
                    continue
                label_name = simple[var][0]
                early = any(
                    gm.start() < assign.end()
                    for gm in re.finditer(
                        rf"\bgoto\s+{re.escape(label_name)}\s*;",
                        new_body,
                        re.I,
                    )
                )
                if early:
                    _propose_null(unit, "GOTO_CROSS_INIT", {"line": unit.line_col(abs_body + assign.start())[0], "snippet": f"goto-before-assign:{var}",})
                    continue
                guard = (
                    f"\n\tstd::unique_ptr<void, decltype(&::free)> "
                    f"_g_{var}({var}, &::free);"
                )
                new_body = new_body[: assign.end()] + guard + new_body[assign.end() :]
                edits.append(
                    Edit(
                        self.name,
                        f"unique_ptr cleanup {var} in {fname}",
                        unit.line_col(abs_body + assign.start())[0],
                        free_old,
                        guard.strip(),
                    )
                )
                need_mem = True

            if new_body != body:
                work.append((abs_body, fend, new_body))

        for start, end, repl in sorted(work, key=lambda x: x[0], reverse=True):
            text = text[:start] + repl + text[end:]

        if need_mem:
            text = _ensure_includes(text, ["#include <memory>\n", "#include <cstdlib>\n"])

        if not edits and not refusals:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=refusals, edits=edits)


# ---------------------------------------------------------------------------
# 4. ERROR_CODE_EXPECTED
# ---------------------------------------------------------------------------


class ErrorCodeExpectedAnnotatePass(Pass):
    """Annotate int/-1/0 errno style functions; refuse only mixed NULL & -1 conventions."""

    name = "error_code_expected_log"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        inserts: list[tuple[int, str]] = []
        marker = "/* PBSD: candidate std::expected<void, int> */"

        for fname, _fstart, fend, body, ret in _functions(text):
            if not re.search(r"\bint\b", ret) or "*" in ret:
                continue
            if marker in body:
                continue

            abs_body = fend - len(body)
            has_neg1 = bool(re.search(r"\breturn\s*-\s*1\s*;", body))
            has_errno = bool(re.search(r"\breturn\s+errno\b", body))
            has_null = bool(re.search(r"\breturn\s+(?:NULL|nullptr)\b", body))
            has_zero = bool(re.search(r"\breturn\s+0\s*;", body))

            if has_null and (has_neg1 or has_errno):
                m = re.search(r"\breturn\s+(?:NULL|nullptr|-?\s*1|errno)\b", body)
                if m:
                    _propose_null(
                        unit,
                        "ERROR_CODE_EXPECTED",
                        {
                            "line": unit.line_col(abs_body + m.start())[0],
                            "snippet": m.group(0),
                        },
                    )
                continue

            if not ((has_neg1 or has_errno) and has_zero):
                continue

            inserts.append((abs_body + 1, f"\n\t{marker}\n"))
            edits.append(
                Edit(
                    self.name,
                    f"expected annotation {fname}",
                    unit.line_col(abs_body)[0],
                    "",
                    marker,
                )
            )

        for pos, ins in sorted(inserts, key=lambda x: x[0], reverse=True):
            text = text[:pos] + ins + text[pos:]

        if not edits and not refusals:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=refusals, edits=edits)


PROMOTE_NULL_BITCAST_GOTO_PASSES: list[Pass] = [
    NullabilityDominatingCheckPass(),
    BitCastExpandRewritePass(),
    GotoCleanupUniquePtrPass(),
    ErrorCodeExpectedAnnotatePass(),
]

# Alias expected by passes/__init__.py optional import
NULL_BITCAST_GOTO_PASSES = PROMOTE_NULL_BITCAST_GOTO_PASSES
