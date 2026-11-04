# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Remaining Tier-1/2/3/4 scripting: tentative defs, str→format, Andersen,
region/lifetime, macro anti-unification, fn-ptr structs, callback+ctx,
PPCallbacks-style macro ranges, Alive2/fuzz/syscall stubs.
"""
from __future__ import annotations

import json
import re
from collections import defaultdict
from pathlib import Path

from ..schema import Edit, PassResult
from ..unit import TranslationUnit
from .base import Pass

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "docs" / "migration" / "clang_port"


def _propose(unit: TranslationUnit, kind: str, payload: dict) -> None:
    from ..proposals import propose

    propose(unit.path, kind, payload)


# ---------------------------------------------------------------------------
# Tier 0 — macro expansion ranges (PPCallbacks stand-in)
# ---------------------------------------------------------------------------


class MacroRangeMaskPass(Pass):
    """Record #define ranges for later passes (Clang PPCallbacks stand-in)."""

    name = "macro_range_mask"
    tier = 0

    def apply(self, unit: TranslationUnit) -> PassResult:
        macros: list[dict] = []
        for m in re.finditer(
            r"(?m)^#\s*define\s+([A-Za-z_]\w*)(\([^\)]*\))?\s+(.*)$",
            unit.text,
        ):
            name, params, body = m.group(1), m.group(2), m.group(3).strip()
            macros.append(
                {
                    "name": name,
                    "function_like": bool(params),
                    "params": params or "",
                    "body": body[:200],
                    "line": unit.line_col(m.start())[0],
                    "index": m.start(),
                }
            )
        unit.meta["macros"] = macros
        unit.meta["macro_line_indexes"] = {m["index"] for m in macros}
        return PassResult.unchanged(unit.text)


# ---------------------------------------------------------------------------
# Tier 1 — tentative definitions
# ---------------------------------------------------------------------------


class TentativeDefinitionPass(Pass):
    """File-scope duplicate `T x;` → keep one, others `extern T x;`."""

    name = "tentative_definition"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        rx = re.compile(
            r"(?m)^((?:unsigned\s+|signed\s+|long\s+|short\s+|const\s+)*)"
            r"(int|char|long|short|float|double|size_t|ssize_t|u_int|uid_t|gid_t|off_t|mode_t)\s+"
            r"([A-Za-z_]\w*)\s*;"
        )
        hits: dict[str, list[re.Match[str]]] = defaultdict(list)
        depth = 0
        # Also try BOF (no leading newline)
        m0 = rx.match(text, 0)
        if m0:
            line = text[0 : text.find("\n") if "\n" in text else len(text)]
            if not re.search(r"\b(?:typedef|extern|static|inline)\b", line):
                hits[m0.group(3)].append(m0)
        i = 0
        while i < len(text):
            c = text[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth = max(0, depth - 1)
            elif c == "\n" and depth == 0:
                m = rx.match(text, i + 1)
                if m:
                    line_start = i + 1
                    nl = text.find("\n", line_start)
                    line = text[line_start:nl if nl >= 0 else len(text)]
                    if not re.search(r"\b(?:typedef|extern|static|inline)\b", line):
                        hits[m.group(3)].append(m)
                    i = m.end()
                    continue
            i += 1

        ops: list[tuple[int, int, str, str]] = []
        for name, ms in hits.items():
            if len(ms) < 2:
                continue
            for m in ms[1:]:
                old = text[m.start() : m.end()]
                typ = (m.group(1) + m.group(2)).strip()
                new = f"extern {typ} {name};"
                ops.append((m.start(), m.end(), new, old))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(
                Edit(self.name, "tentative→extern", unit.line_col(start)[0], old, new)
            )
        if not edits:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=[], edits=edits)


# ---------------------------------------------------------------------------
# Tier 2 — str*/snprintf → string_view / format
# ---------------------------------------------------------------------------


class StrToStringViewPass(Pass):
    """Rewrite strlen(s)==0; propose snprintf/strcpy → format/string_view."""

    name = "str_to_string_view"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        masked = unit.mask_strings_comments()
        ops: list[tuple[int, int, str, str]] = []

        for m in re.finditer(
            r"\bstrlen\s*\(\s*([A-Za-z_]\w*)\s*\)\s*==\s*0\b",
            masked,
        ):
            s = m.group(1)
            ops.append(
                (
                    m.start(),
                    m.end(),
                    f"std::string_view({s}).empty()",
                    text[m.start() : m.end()],
                )
            )
        for m in re.finditer(
            r"\bstrlen\s*\(\s*([A-Za-z_]\w*)\s*\)\s*!=\s*0\b",
            masked,
        ):
            s = m.group(1)
            ops.append(
                (
                    m.start(),
                    m.end(),
                    f"!std::string_view({s}).empty()",
                    text[m.start() : m.end()],
                )
            )

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(
                Edit(self.name, "strlen→string_view", unit.line_col(start)[0], old, new)
            )

        for m in re.finditer(
            r"\b(snprintf|sprintf|strcpy|strncpy|strcat|strncat|strlcpy|strlcat)\s*\(",
            unit.mask_strings_comments(),
        ):
            # Skip shapes rewritten by snprintf_literal / strcpy_literal
            window = unit.text[m.start() : m.start() + 160]
            fn = m.group(1)
            if fn in ("strlcpy", "strlcat") and re.search(
                r"sizeof\s*\(\s*[A-Za-z_]\w*\s*\)", window
            ):
                continue
            if fn == "snprintf" and '"%s"' in window and "sizeof" in window:
                continue
            if fn == "sprintf" and '"%s"' in window:
                continue
            if fn == "strcpy" and re.search(r'strcpy\s*\(\s*\w+\s*,\s*"', window):
                continue
            _propose(
                unit,
                "STR_FORMAT_CANDIDATE",
                {
                    "line": unit.line_col(m.start())[0],
                    "snippet": fn,
                    "hint": "std::format / string_view when dest not aliased",
                },
            )

        if edits and "#include <string_view>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <string_view>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text

        if not edits:
            return PassResult(text=unit.text, refusals=[], edits=[])
        return PassResult(text=text, refusals=[], edits=edits)


# ---------------------------------------------------------------------------
# Tier 3 — Andersen-lite
# ---------------------------------------------------------------------------


class AndersenEscapePass(Pass):
    """File-local Andersen inclusion points-to → escape / owned proposals."""

    name = "andersen_escape"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        pts: dict[str, set[str]] = defaultdict(set)

        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*=\s*(?:\([^)]{0,200}\)\s*)?(?:malloc|calloc|realloc)\s*\(",
            text,
        ):
            v = m.group(1)
            pts[v].add(f"heap:{v}@{m.start()}")

        # Cap work on huge TUs
        if len(text) > 200_000:
            return PassResult.unchanged(unit.text)

        assigns = list(re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s*;", text))
        addrs = list(re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*&([A-Za-z_]\w*)\b", text))
        changed = True
        rounds = 0
        while changed and rounds < 16:
            changed = False
            rounds += 1
            for m in assigns:
                p, q = m.group(1), m.group(2)
                before = len(pts[p])
                pts[p] |= pts[q]
                if len(pts[p]) != before:
                    changed = True
            for m in addrs:
                p, q = m.group(1), m.group(2)
                before = len(pts[p])
                pts[p].add(f"stack:{q}")
                if len(pts[p]) != before:
                    changed = True

        escaped: set[str] = set()
        for m in re.finditer(r"\breturn\s+([A-Za-z_]\w*)\s*;", text):
            escaped |= pts[m.group(1)]
        for m in re.finditer(r"(?:->|\.)\s*\w+\s*=\s*([A-Za-z_]\w*)\b", text):
            escaped |= pts[m.group(1)]

        for var, locs in pts.items():
            if locs & escaped:
                _propose(
                    unit,
                    "ANDERSEN_ESCAPE",
                    {
                        "var": var,
                        "locs": sorted(locs)[:8],
                        "hint": "shared_ptr or raw observer — ownership crosses",
                    },
                )
            elif locs and all(x.startswith("heap:") for x in locs):
                _propose(
                    unit,
                    "ANDERSEN_OWNED",
                    {"var": var, "locs": sorted(locs)[:8], "hint": "unique_ptr candidate"},
                )

        unit.meta["andersen_pts"] = {k: sorted(v) for k, v in pts.items() if v}
        return PassResult.unchanged(unit.text)


# ---------------------------------------------------------------------------
# Tier 3 — region / lifetime
# ---------------------------------------------------------------------------


class RegionLifetimePass(Pass):
    """Cyclone-lite region vars + outlives constraints → LIFETIME_* proposals."""

    name = "region_lifetime"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        regions: dict[str, str] = {}
        outlives: list[tuple[str, str]] = []

        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*&([A-Za-z_]\w*)\b", unit.text):
            p, q = m.group(1), m.group(2)
            regions[p] = regions.get(p, f"R_{p}")
            regions[q] = f"stack_{q}"
            outlives.append((regions[q], regions[p]))

        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*=\s*(?:\([^)]{0,200}\)\s*)?(?:malloc|calloc)\s*\(",
            unit.text,
        ):
            regions[m.group(1)] = f"heap_{m.group(1)}"

        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s*;", unit.text):
            p, q = m.group(1), m.group(2)
            if p in regions and q in regions:
                outlives.append((regions[q], regions[p]))

        bad: list[tuple[str, str]] = []
        for longer, shorter in outlives:
            if longer.startswith("stack_") and shorter.startswith("heap_"):
                bad.append((longer, shorter))
            if longer.startswith("stack_") and shorter.startswith("R_"):
                bad.append((longer, shorter))

        for longer, shorter in bad[:40]:
            _propose(
                unit,
                "LIFETIME_FAIL",
                {
                    "outlives": f"{longer} ⪰ {shorter}",
                    "hint": "stack address escapes — must copy or heap-allocate",
                },
            )
        for name, reg in list(regions.items())[:80]:
            if reg.startswith("heap_"):
                _propose(
                    unit,
                    "LIFETIME_OWN",
                    {"var": name, "region": reg, "hint": "owning region → unique_ptr"},
                )

        unit.meta["regions"] = regions
        return PassResult.unchanged(unit.text)


# ---------------------------------------------------------------------------
# Tier 3 — macro anti-unification
# ---------------------------------------------------------------------------


class MacroObjectConstexprPass(Pass):
    """Rewrite simple object-like macros: `#define N 1` → `inline constexpr auto N = 1;`.

    Skips names used in `#if`/`#ifdef` in the same TU, non-literal bodies, and
    string macros used in adjacent string-literal concatenation.
    """

    name = "macro_object_constexpr"
    tier = 1

    _LIT = re.compile(
        r"^(?:"
        r"0[xX][0-9A-Fa-f]+[uUlL]*|"
        r"0[bB][01]+[uUlL]*|"
        r"[0-9]+(?:\.[0-9]*)?(?:[eE][+-]?[0-9]+)?[fFlLuU]*|"
        r"'(?:\\.|[^\\'])'|"
        r'"(?:\\.|[^\\"])*"|'
        r"nullptr|NULL|true|false"
        r")$"
    )
    # Integer/float arithmetic of literals only — e.g. (32 * 1024), (1+2)
    _SAFE_ARITH = re.compile(
        r"^(?:"
        r"\s*(?:"
        r"0[xX][0-9A-Fa-f]+[uUlL]*|"
        r"0[bB][01]+[uUlL]*|"
        r"[0-9]+(?:\.[0-9]*)?(?:[eE][+-]?[0-9]+)?[fFlLuU]*|"
        r"[+\-*/%()]"
        r")\s*"
        r")+$"
    )

    @classmethod
    def _literal_expr(cls, body: str) -> str | None:
        """Accept lit, (lit), ((lit)), (type)lit, or literal-only arithmetic."""
        s = body.strip()
        cast_re = re.compile(
            r"^\(\s*((?:unsigned\s+|signed\s+|long\s+|short\s+|const\s+)*"
            r"(?:int|char|long|short|size_t|ssize_t|u_int|uint\d+_t|int\d+_t))\s*\)\s*(.+)$"
        )
        for _ in range(4):
            cm = cast_re.match(s)
            if cm and cls._LIT.match(cm.group(2).strip()):
                return s
            if len(s) >= 2 and s[0] == "(" and s[-1] == ")":
                inner = s[1:-1].strip()
                if cls._LIT.match(inner):
                    return inner
                if cls._SAFE_ARITH.fullmatch(inner) and re.search(r"\d", inner):
                    return s  # keep outer parens
                s = inner
                continue
            break
        if cls._LIT.match(s):
            return "nullptr" if s == "NULL" else s
        if cls._SAFE_ARITH.fullmatch(s) and re.search(r"\d", s):
            return s
        return None

    @staticmethod
    def _used_in_string_concat(text: str, name: str) -> bool:
        """C adjacent string concat: \"x\" NAME or NAME \"x\" — constexpr breaks this."""
        # Ignore the #define line for this name (body is often a string literal).
        cleaned = re.sub(
            rf"(?m)^#\s*define\s+{re.escape(name)}\b.*$",
            "",
            text,
        )
        return bool(
            re.search(
                rf'(?<!\\)"\s*{re.escape(name)}\b|\b{re.escape(name)}\s*"',
                cleaned,
            )
        )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        # Names appearing in preprocessor conditions must stay macros
        pp_names: set[str] = set()
        for m in re.finditer(
            r"(?m)^#\s*if(?:n?def)?\s+([A-Za-z_]\w*)|"
            r"^#\s*if\b([^\n]*)|"
            r"^#\s*elif\b([^\n]*)",
            text,
        ):
            if m.group(1):
                pp_names.add(m.group(1))
            for g in m.groups()[1:]:
                if g:
                    pp_names.update(re.findall(r"\b([A-Za-z_]\w*)\b", g))

        edits: list[Edit] = []
        ops: list[tuple[int, int, str, str]] = []
        for m in re.finditer(
            r"(?m)^(#\s*define\s+)([A-Za-z_]\w*)(\s+)(\S.*)$",
            text,
        ):
            name = m.group(2)
            line = m.group(0)
            # Function-like macros have '(' immediately after the name (no space).
            if re.search(rf"#\s*define\s+{re.escape(name)}\(", line):
                continue
            body = m.group(4).strip()
            if body.endswith("\\"):
                continue
            body_code = re.split(r"/\*|//", body, maxsplit=1)[0].strip()
            lit = self._literal_expr(body_code)
            if not lit:
                continue
            if name in pp_names:
                continue
            if name.endswith("_H") or name.endswith("_H_") or name.startswith("HAVE_"):
                continue
            if lit.startswith('"') and self._used_in_string_concat(text, name):
                continue
            new = f"inline constexpr auto {name} = {lit};"
            ops.append((m.start(), m.end(), new, m.group(0)))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(
                Edit(self.name, "define→constexpr", unit.line_col(start)[0], old[:80], new[:80])
            )
        if not edits:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=[], edits=edits)


class SnprintfLiteralPass(Pass):
    """Safe snprintf patterns → format/strncpy for simple formats."""

    name = "snprintf_literal"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        ops: list[tuple[int, int, str, str]] = []

        # snprintf(buf, sizeof(buf), "%s", expr)
        for m in re.finditer(
            r"\bsnprintf\s*\(\s*([A-Za-z_]\w*)\s*,\s*sizeof\s*\(\s*\1\s*\)\s*,\s*"
            r'"%s"\s*,\s*([^)]+)\)',
            masked,
        ):
            buf = m.group(1)
            orig = text[m.start() : m.end()]
            em = re.search(
                r"\bsnprintf\s*\(\s*"
                + re.escape(buf)
                + r"\s*,\s*sizeof\s*\(\s*"
                + re.escape(buf)
                + r"\s*\)\s*,\s*\"%s\"\s*,\s*([^)]+)\)",
                orig,
            )
            if not em:
                continue
            expr = em.group(1).strip()
            new = (
                f"{{ auto _pbsd_s = std::format(\"{{}}\", {expr}); "
                f"std::strncpy({buf}, _pbsd_s.c_str(), sizeof({buf}) - 1); "
                f"{buf}[sizeof({buf}) - 1] = '\\0'; }}"
            )
            ops.append((m.start(), m.end(), new, orig))

        # sprintf(buf, "%s", expr) — same shape, assume buf is array
        for m in re.finditer(
            r"\bsprintf\s*\(\s*([A-Za-z_]\w*)\s*,\s*\"%s\"\s*,\s*([^)]+)\)",
            masked,
        ):
            buf = m.group(1)
            if not re.search(r"(buf|path|tmp|name|str|line|file|dir|cmd)", buf, re.I):
                continue
            orig = text[m.start() : m.end()]
            em = re.search(
                r"\bsprintf\s*\(\s*"
                + re.escape(buf)
                + r"\s*,\s*\"%s\"\s*,\s*([^)]+)\)",
                orig,
            )
            if not em:
                continue
            expr = em.group(1).strip()
            new = (
                f"{{ auto _pbsd_s = std::format(\"{{}}\", {expr}); "
                f"std::strncpy({buf}, _pbsd_s.c_str(), sizeof({buf}) - 1); "
                f"{buf}[sizeof({buf}) - 1] = '\\0'; }}"
            )
            ops.append((m.start(), m.end(), new, orig))

        # strlcpy(buf, expr, sizeof(buf)) / strlcat(buf, expr, sizeof(buf))
        for fn in ("strlcpy", "strlcat"):
            for m in re.finditer(
                rf"\b{fn}\s*\(\s*([A-Za-z_]\w*)\s*,\s*([^,]+)\s*,\s*sizeof\s*\(\s*\1\s*\)\s*\)",
                masked,
            ):
                buf = m.group(1)
                if not re.search(r"(buf|path|tmp|name|str|line|file|dir|cmd|dst|dest)", buf, re.I):
                    continue
                orig = text[m.start() : m.end()]
                em = re.search(
                    rf"\b{fn}\s*\(\s*"
                    + re.escape(buf)
                    + r"\s*,\s*([^,]+)\s*,\s*sizeof\s*\(\s*"
                    + re.escape(buf)
                    + r"\s*\)\s*\)",
                    orig,
                )
                if not em:
                    continue
                expr = em.group(1).strip()
                if fn == "strlcpy":
                    new = (
                        f"{{ auto _pbsd_s = std::string_view({expr}); "
                        f"auto _pbsd_n = _pbsd_s.size() < sizeof({buf}) - 1 "
                        f"? _pbsd_s.size() : sizeof({buf}) - 1; "
                        f"std::memcpy({buf}, _pbsd_s.data(), _pbsd_n); "
                        f"{buf}[_pbsd_n] = '\\0'; }}"
                    )
                else:
                    new = (
                        f"{{ auto _pbsd_s = std::string_view({expr}); "
                        f"auto _pbsd_len = std::string_view({buf}).size(); "
                        f"auto _pbsd_room = sizeof({buf}) > _pbsd_len + 1 "
                        f"? sizeof({buf}) - _pbsd_len - 1 : 0; "
                        f"auto _pbsd_n = _pbsd_s.size() < _pbsd_room "
                        f"? _pbsd_s.size() : _pbsd_room; "
                        f"std::memcpy({buf} + _pbsd_len, _pbsd_s.data(), _pbsd_n); "
                        f"{buf}[_pbsd_len + _pbsd_n] = '\\0'; }}"
                    )
                ops.append((m.start(), m.end(), new, orig))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(
                Edit(self.name, "sprintf→format", unit.line_col(start)[0], old[:80], new[:80])
            )

        if edits:
            need = []
            if "std::format" in text and "#include <format>" not in text:
                need.append("#include <format>\n")
            if "std::string_view" in text and "#include <string_view>" not in text:
                need.append("#include <string_view>\n")
            if ("std::strncpy" in text or "std::memcpy" in text) and "#include <cstring>" not in text:
                need.append("#include <cstring>\n")
            if need:
                incs = list(re.finditer(r"(?m)^#include\b.*$", text))
                hdr = "".join(need)
                if incs:
                    pos = incs[-1].end()
                    text = text[:pos] + "\n" + hdr + text[pos:]
                else:
                    text = hdr + text

        if not edits:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=[], edits=edits)


class MacroAntiUnificationPass(Pass):
    """Identical-modulo-args macros → constexpr/inline; else divergent proposal.

    Scans *remaining* `#define`s after earlier rewrite passes (not stale meta),
    so successful object-like constexpr promotions are not re-proposed.
    """

    name = "macro_anti_unification"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        # Always re-scan current text — MacroObjectConstexpr may have rewritten
        # many object-like macros already.
        macros: list[dict] = []
        for m in re.finditer(
            r"(?m)^#\s*define\s+([A-Za-z_]\w*)(\([^\)]*\))?\s+(.*)$",
            unit.text,
        ):
            macros.append(
                {
                    "name": m.group(1),
                    "function_like": bool(m.group(2)),
                    "params": m.group(2) or "",
                    "body": m.group(3).strip()[:200],
                    "line": unit.line_col(m.start())[0],
                }
            )

        # Normalize body: replace param tokens with $0,$1,...
        templates: dict[str, list[dict]] = defaultdict(list)
        for mac in macros:
            if not mac.get("function_like"):
                body = mac.get("body", "")
                # Skip header guards / feature tests — not constexpr material
                name = mac["name"]
                if name.endswith("_H") or name.endswith("_H_") or name.startswith("HAVE_"):
                    continue
                _propose(
                    unit,
                    "MACRO_OBJECT",
                    {
                        "name": name,
                        "line": mac.get("line", 0),
                        "body": body[:80],
                        "hint": "constexpr / inline constexpr variable",
                    },
                )
                continue
            params = re.findall(r"[A-Za-z_]\w*", mac.get("params", ""))
            body = mac.get("body", "")
            norm = body
            for i, p in enumerate(params):
                norm = re.sub(rf"\b{re.escape(p)}\b", f"${i}", norm)
            templates[norm].append(mac)

        for norm, group in templates.items():
            names = [g["name"] for g in group]
            simple = bool(
                norm
                and re.fullmatch(
                    r"[\w\s\$\(\)\+\-\*/<>!=&|?:.,\[\]]{1,120}",
                    norm,
                )
                and "$" in norm
            )
            if len(group) >= 2 or simple:
                _propose(
                    unit,
                    "MACRO_CONSTEXPR",
                    {
                        "names": names,
                        "template": norm[:120],
                        "hint": "inline constexpr / template function",
                    },
                )
            else:
                _propose(
                    unit,
                    "MACRO_DIVERGENT",
                    {
                        "names": names,
                        "template": norm[:120],
                        "hint": "keep as macro or hand-port — anti-unification failed",
                    },
                )

        return PassResult.unchanged(unit.text)


class MacroFunctionConstexprPass(Pass):
    """Rewrite simple function-like macros to constexpr templates.

    Example: `#define ADD(a, b) ((a)+(b))` →
      `template<class T0, class T1> constexpr auto ADD(T0 a, T1 b) { return ((a)+(b)); }`
    """

    name = "macro_function_constexpr"
    tier = 1

    _SAFE_BODY = re.compile(
        r"^[\w\s\(\)\+\-\*/%<>=!&|?:.,\[\]]{1,120}$"
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        ops: list[tuple[int, int, str, str]] = []

        for m in re.finditer(
            r"(?m)^#\s*define\s+([A-Za-z_]\w*)\(([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*){0,3})\)\s+(\S.*)$",
            text,
        ):
            name, params_s, body = m.group(1), m.group(2), m.group(3).strip()
            if body.endswith("\\"):
                continue
            body_code = re.split(r"/\*|//", body, maxsplit=1)[0].strip()
            if not body_code or not self._SAFE_BODY.match(body_code):
                continue
            # Reject statement-like / control flow
            if re.search(r"\b(do|while|for|if|switch|return|goto)\b", body_code):
                continue
            params = [p.strip() for p in params_s.split(",")]
            if not params or any(not p.isidentifier() for p in params):
                continue
            # Body must reference only identifiers that are params or literals-ish
            ids = set(re.findall(r"\b([A-Za-z_]\w*)\b", body_code))
            allowed = set(params) | {
                "sizeof",
                "NULL",
                "true",
                "false",
                "uint8_t",
                "uint16_t",
                "uint32_t",
                "uint64_t",
                "int8_t",
                "int16_t",
                "int32_t",
                "int64_t",
                "size_t",
                "ssize_t",
            }
            if ids - allowed:
                continue
            tparams = ", ".join(f"class T{i}" for i in range(len(params)))
            fparams = ", ".join(f"T{i} {p}" for i, p in enumerate(params))
            new = (
                f"template<{tparams}>\n"
                f"constexpr auto {name}({fparams}) {{\n"
                f"  return {body_code};\n"
                f"}}"
            )
            ops.append((m.start(), m.end(), new, m.group(0)))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(
                Edit(
                    self.name,
                    "macro→constexpr fn",
                    unit.line_col(start)[0],
                    old[:80],
                    new[:80],
                )
            )
        if not edits:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=[], edits=edits)


class StrcpyLiteralPass(Pass):
    """strcpy(dst, \"lit\") / strncpy(dst, \"lit\", n) → bounded copy helpers."""

    name = "strcpy_literal"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        ops: list[tuple[int, int, str, str]] = []

        for m in re.finditer(
            r"\bstrcpy\s*\(\s*([A-Za-z_]\w*)\s*,\s*(\"([^\"\\]|\\.)*\")\s*\)",
            text,
        ):
            # Only if match region isn't all spaces in mask (i.e. not inside comment) —
            # approximate: require masked has strcpy at same index
            if masked[m.start() : m.start() + 6] != "strcpy":
                continue
            dst, lit = m.group(1), m.group(2)
            new = (
                f"{{ constexpr auto _pbsd_lit = {lit}; "
                f"std::strncpy({dst}, _pbsd_lit, sizeof({dst}) - 1); "
                f"{dst}[sizeof({dst}) - 1] = '\\0'; }}"
            )
            # sizeof(dst) wrong if dst is char* — only when dst looks like array use:
            # require sizeof pattern elsewhere or skip pointers: heuristic — name is buf/path/tmp
            if not re.search(r"(buf|path|tmp|name|str|line|file|dir|cmd)", dst, re.I):
                _propose(
                    unit,
                    "STR_FORMAT_CANDIDATE",
                    {
                        "line": unit.line_col(m.start())[0],
                        "snippet": "strcpy-lit",
                        "hint": "string_view assign if destination is array",
                    },
                )
                continue
            ops.append((m.start(), m.end(), new, m.group(0)))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(
                Edit(self.name, "strcpy→strncpy", unit.line_col(start)[0], old[:80], new[:80])
            )
        if edits and "#include <cstring>" not in text:
            incs = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <cstring>\n"
            if incs:
                pos = incs[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text
        if not edits:
            return PassResult(text=unit.text, refusals=[], edits=[])
        return PassResult(text=text, refusals=[], edits=edits)


# ---------------------------------------------------------------------------
# Tier 3 — fn-ptr struct → virtuals; callback + void* ctx
# ---------------------------------------------------------------------------


class FnPtrStructPass(Pass):
    """Structs of function pointers → propose virtual interface / std::function."""

    name = "fn_ptr_struct"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        # struct Foo { ... (*op)(...); ... };
        for m in re.finditer(
            r"\bstruct\s+([A-Za-z_]\w*)\s*\{([^}]{0,4000})\}",
            unit.text,
            re.S,
        ):
            name, body = m.group(1), m.group(2)
            fps = re.findall(
                r"\([\s*]*([A-Za-z_]\w*)\s*\)\s*\([^;]*\)\s*;",
                body,
            )
            if len(fps) >= 2:
                _propose(
                    unit,
                    "FN_PTR_STRUCT",
                    {
                        "struct": name,
                        "methods": fps[:12],
                        "line": unit.line_col(m.start())[0],
                        "hint": "abstract class with virtual methods",
                    },
                )
        return PassResult.unchanged(unit.text)


class CallbackCtxPass(Pass):
    """Functions taking (T *cb)(…, void *ctx) or void *arg → propose typed ctx."""

    name = "callback_void_ctx"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\([^;]{0,200}?"
            r"(?:\(\s*\*\s*([A-Za-z_]\w*)\s*\)|([A-Za-z_]\w*)\s*)\s*"
            r"\([^)]*void\s*\*\s*(?:ctx|arg|cookie|userdata)[^)]*\)",
            unit.text,
            re.I,
        ):
            _propose(
                unit,
                "CALLBACK_CTX",
                {
                    "function": m.group(1),
                    "line": unit.line_col(m.start())[0],
                    "hint": "template Ctx or std::function; erase void*",
                },
            )
        # Simpler: (void *arg) near function pointer param
        for m in re.finditer(
            r"\([^)]*\(\s*\*\s*[A-Za-z_]\w*\s*\)\s*\([^)]*\)\s*,\s*void\s*\*\s*\w+",
            unit.text,
        ):
            _propose(
                unit,
                "CALLBACK_CTX",
                {
                    "line": unit.line_col(m.start())[0],
                    "snippet": unit.snippet_at(m.start())[:80],
                    "hint": "typed context instead of void*",
                },
            )
        return PassResult.unchanged(unit.text)


# ---------------------------------------------------------------------------
# Tier 4 — Alive2 / fuzz / syscall-trace stubs
# ---------------------------------------------------------------------------


class Alive2OracleStubPass(Pass):
    """Emit Alive2 / litmus stub commands into reports (no Alive2 binary required)."""

    name = "alive2_oracle_stub"
    tier = 4

    def apply(self, unit: TranslationUnit) -> PassResult:
        OUT.mkdir(parents=True, exist_ok=True)
        stub_dir = OUT / "stubs" / "alive2"
        stub_dir.mkdir(parents=True, exist_ok=True)
        # Only for small TUs that look like pure functions
        if unit.text.count("\n") > 200:
            return PassResult.unchanged(unit.text)
        if "malloc" in unit.text or "syscall" in unit.text:
            return PassResult.unchanged(unit.text)
        marker = stub_dir / (Path(unit.path).stem + ".alive2.txt")
        marker.write_text(
            f"# Alive2 hook for {unit.path}\n"
            f"# Run when alive-tv available:\n"
            f"#   clang -emit-llvm -O2 -S {unit.path} -o before.ll\n"
            f"#   clang -emit-llvm -O2 -S <rewritten.cpp> -o after.ll\n"
            f"#   alive-tv before.ll after.ll\n",
            encoding="utf-8",
        )
        unit.meta["alive2_stub"] = str(marker)
        return PassResult.unchanged(unit.text)


class FuzzOracleStubPass(Pass):
    """Record libFuzzer / AFL entry stub path for differential inputs."""

    name = "fuzz_oracle_stub"
    tier = 4

    def apply(self, unit: TranslationUnit) -> PassResult:
        if "main(" not in unit.text and "LLVMFuzzerTestOneInput" not in unit.text:
            return PassResult.unchanged(unit.text)
        OUT.mkdir(parents=True, exist_ok=True)
        stub = OUT / "stubs" / "fuzz" / (Path(unit.path).stem + "_fuzz.md")
        stub.parent.mkdir(parents=True, exist_ok=True)
        stub.write_text(
            f"# Fuzz differential for `{unit.path}`\n\n"
            f"1. Build C and C++23 ports with `-fsanitize=fuzzer,address`.\n"
            f"2. Share corpus; assert same exit / no new ASan diffs.\n"
            f"3. Prefer harness around pure helpers, not full main.\n",
            encoding="utf-8",
        )
        unit.meta["fuzz_stub"] = str(stub)
        return PassResult.unchanged(unit.text)


class SyscallTraceStubPass(Pass):
    """Propose ktrace/truss/strace compare for I/O-bound userland."""

    name = "syscall_trace_stub"
    tier = 4

    def apply(self, unit: TranslationUnit) -> PassResult:
        io_hits = len(
            re.findall(
                r"\b(read|write|open|close|ioctl|socket|connect|send|recv)\s*\(",
                unit.text,
            )
        )
        if io_hits < 2:
            return PassResult.unchanged(unit.text)
        _propose(
            unit,
            "SYSCALL_TRACE",
            {
                "io_calls": io_hits,
                "hint": "ktrace/truss both binaries; diff syscall seq + errno",
            },
        )
        return PassResult.unchanged(unit.text)
