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
            _propose(
                unit,
                "STR_FORMAT_CANDIDATE",
                {
                    "line": unit.line_col(m.start())[0],
                    "snippet": m.group(1),
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


class MacroAntiUnificationPass(Pass):
    """Identical-modulo-args macros → constexpr/inline; else divergent proposal."""

    name = "macro_anti_unification"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        macros = unit.meta.get("macros") or []
        if not macros:
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
                _propose(
                    unit,
                    "MACRO_OBJECT",
                    {
                        "name": mac["name"],
                        "line": mac.get("line", 0),
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
