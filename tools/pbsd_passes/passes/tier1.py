"""Tier 1 — pure syntax mechanical rewrites / refusals (todo.md)."""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

_propose_pol_OUT = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"


def _propose_pol(unit, kind: str, payload: dict) -> None:
    _propose_pol_OUT.mkdir(parents=True, exist_ok=True)
    with (_propose_pol_OUT / "proposals.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps({"file": unit.path, "kind": kind, **payload}) + "\n")


CPP_KEYWORDS = [
    "new",
    "class",
    "template",
    "this",
    "private",
    "public",
    "delete",
    "operator",
    "namespace",
    "try",
    "typename",
    "export",
    "and",
    "or",
    "not",
    "xor",
    "virtual",
    "friend",
    "mutable",
    "explicit",
    "wchar_t",
    "char8_t",
    "char16_t",
    "char32_t",
    "constexpr",
    "consteval",
    "constinit",
    "concept",
    "requires",
    "co_await",
    "co_return",
    "co_yield",
]


def _ref(
    unit: TranslationUnit, pass_name: str, reason: str, idx: int, snippet: str | None = None
) -> Refusal:
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


class RegisterRemovePass(Pass):
    name = "register_remove"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        text = unit.text
        # Work on masked positions; rebuild from original with substitutions.
        pattern = re.compile(r"\bregister\b")
        pieces: list[str] = []
        last = 0
        for m in pattern.finditer(masked):
            if masked[m.start() : m.end()] != "register":
                continue
            # confirm original also has register at same span
            if text[m.start() : m.end()] != "register":
                continue
            pieces.append(text[last : m.start()])
            pieces.append("")  # remove
            line, _ = unit.line_col(m.start())
            edits.append(
                Edit(self.name, "remove register", line, "register", "")
            )
            last = m.end()
        if not edits:
            return PassResult.unchanged(text)
        pieces.append(text[last:])
        # Clean double spaces left by removal
        new_text = re.sub(r"  +", " ", "".join(pieces))
        return PassResult(text=new_text, refusals=[], edits=edits)


class RestrictPass(Pass):
    name = "restrict_to_underscore"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        masked = unit.mask_strings_comments()
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        pieces: list[str] = []
        last = 0
        for m in re.finditer(r"\brestrict\b", masked):
            if text[m.start() : m.end()] != "restrict":
                continue
            pieces.append(text[last : m.start()])
            pieces.append("__restrict")
            line, _ = unit.line_col(m.start())
            edits.append(Edit(self.name, "restrict→__restrict", line, "restrict", "__restrict"))
            last = m.end()
        if not edits:
            return PassResult.unchanged(text)
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=[], edits=edits)


class C11ToCxxPass(Pass):
    name = "c11_to_cxx"
    tier = 1

    REPL = [
        (r"\b_Bool\b", "bool", "C11_TYPE"),
        (r"\b_Static_assert\b", "static_assert", "C11_TYPE"),
        (r"\b_Atomic\b", "std::atomic", "C11_TYPE"),
    ]

    def apply(self, unit: TranslationUnit) -> PassResult:
        masked = unit.mask_strings_comments()
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # Collect all replacements as (start,end,new,reason,old)
        reps: list[tuple[int, int, str, str, str]] = []
        for pat, repl, reason in self.REPL:
            for m in re.finditer(pat, masked):
                old = text[m.start() : m.end()]
                if not re.fullmatch(pat, old):
                    # masked matched; trust span
                    old = text[m.start() : m.end()]
                reps.append((m.start(), m.end(), repl, reason, old))
        if not reps:
            return PassResult.unchanged(text)
        reps.sort(key=lambda x: x[0])
        pieces: list[str] = []
        last = 0
        need_atomic = False
        need_cstdbool = False
        for start, end, repl, reason, old in reps:
            pieces.append(text[last:start])
            pieces.append(repl)
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, f"{old}→{repl}", line, old, repl))
            if repl == "std::atomic":
                need_atomic = True
            if repl == "bool":
                need_cstdbool = True
            last = end
        pieces.append(text[last:])
        new_text = "".join(pieces)
        headers = []
        if need_cstdbool and "#include <cstdbool>" not in new_text and "#include <stdbool.h>" not in new_text:
            headers.append("#include <cstdbool>")
        if need_atomic and "#include <atomic>" not in new_text:
            headers.append("#include <atomic>")
        if headers:
            banner = "\n".join(headers) + "\n"
            # Insert after last #include block start
            m = list(re.finditer(r"(?m)^#include\b.*$", new_text))
            if m:
                pos = m[-1].end()
                new_text = new_text[:pos] + "\n" + banner + new_text[pos:]
            else:
                new_text = banner + new_text
        return PassResult(text=new_text, refusals=[], edits=edits)


class NullToNullptrPass(Pass):
    name = "null_to_nullptr"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        masked = unit.mask_strings_comments()
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        pieces: list[str] = []
        last = 0
        for m in re.finditer(r"\bNULL\b", masked):
            if text[m.start() : m.end()] != "NULL":
                continue
            pieces.append(text[last : m.start()])
            pieces.append("nullptr")
            line, _ = unit.line_col(m.start())
            edits.append(Edit(self.name, "NULL→nullptr", line, "NULL", "nullptr"))
            last = m.end()
        if not edits:
            return PassResult.unchanged(text)
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=[], edits=edits)


class TypedefToUsingPass(Pass):
    name = "typedef_to_using"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # Simple: typedef <type-tokens> <name>;
        pattern = re.compile(
            r"typedef\s+((?:unsigned\s+|signed\s+|long\s+|short\s+|const\s+|volatile\s+|struct\s+|enum\s+|union\s+)*[\w:]+(?:\s*\*)*)\s+(\w+)\s*;"
        )
        pieces: list[str] = []
        last = 0
        for m in pattern.finditer(masked):
            # Use original text spans
            full = text[m.start() : m.end()]
            om = pattern.match(full)
            if not om:
                _propose_pol(unit, "TYPEDEF_COMPLEX", {"line": unit.line_col(m.start())[0], "snippet": full[:80]})
                continue
            if "[" in full:
                _propose_pol(unit, "TYPEDEF_COMPLEX", {"line": unit.line_col(m.start())[0], "snippet": full[:80]})
                continue
            # typedef ret (*name)(args); → using name = ret (*)(args);
            fp = re.match(
                r"typedef\s+(.+?)\s*\(\s*\*\s*(\w+)\s*\)\s*\((.*)\)\s*;",
                full,
                re.S,
            )
            if fp:
                ret, name, args = fp.group(1).strip(), fp.group(2), fp.group(3).strip()
                repl = f"using {name} = {ret} (*)({args});"
                pieces.append(text[last : m.start()])
                pieces.append(repl)
                line, _ = unit.line_col(m.start())
                edits.append(Edit(self.name, "typedef-fnptr→using", line, full[:60], repl[:60]))
                last = m.end()
                continue
            if "(" in full:
                _propose_pol(unit, "TYPEDEF_COMPLEX", {"line": unit.line_col(m.start())[0], "snippet": full[:80]})
                continue
            typ, name = om.group(1).strip(), om.group(2)
            repl = f"using {name} = {typ};"
            pieces.append(text[last : m.start()])
            pieces.append(repl)
            line, _ = unit.line_col(m.start())
            edits.append(Edit(self.name, "typedef→using", line, full, repl))
            last = m.end()
        if not edits:
            # Still scan complex typedefs to refuse
            for m in re.finditer(r"\btypedef\b", masked):
                line_end = text.find(";", m.start())
                snip = text[m.start() : line_end + 1 if line_end != -1 else m.start() + 40]
                if "(" in snip or "[" in snip:
                    _propose_pol(unit, "TYPEDEF_COMPLEX", {"line": unit.line_col(m.start())[0], "snippet": snip[:80]})
            if not refusals:
                return PassResult.unchanged(text)
            return PassResult(text=text, refusals=refusals, edits=[])
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=refusals, edits=edits)


class CppKeywordRenamePass(Pass):
    name = "cpp_keyword_rename"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        masked = unit.mask_strings_comments()
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # Avoid renaming common false friends used as macros/types in BSD carefully.
        # Only rename when used as identifier token not after `#define` / `#` line.
        reps: list[tuple[int, int, str, str]] = []
        for kw in CPP_KEYWORDS:
            for m in re.finditer(rf"\b{kw}\b", masked):
                # Skip preprocessor lines
                line_start = text.rfind("\n", 0, m.start()) + 1
                line = text[line_start : text.find("\n", m.start())]
                if line.lstrip().startswith("#"):
                    continue
                # Skip if looks like a label of struct member access via ->kw rare; still rename
                old = text[m.start() : m.end()]
                if old != kw:
                    continue
                new = f"pbsd_kw_{kw}"
                reps.append((m.start(), m.end(), new, old))
        if not reps:
            return PassResult.unchanged(text)
        reps.sort(key=lambda x: x[0])
        pieces: list[str] = []
        last = 0
        for start, end, new, old in reps:
            pieces.append(text[last:start])
            pieces.append(new)
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, f"rename {old}", line, old, new))
            last = end
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=[], edits=edits)


class StringLiteralConstPass(Pass):
    name = "string_literal_const"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # Match on real text; skip spans inside comments via mask check on 'char'
        pattern = re.compile(r"\bchar\s*\*\s*([A-Za-z_]\w*)\s*=\s*\"")
        pieces: list[str] = []
        last = 0
        masked = unit.mask_strings_comments()
        for m in pattern.finditer(text):
            # If 'char' was masked away, we're inside comment/string — skip
            if masked[m.start() : m.start() + 4] != "char":
                continue
            span = text[m.start() : m.end()]
            new_span = re.sub(r"\bchar\s*\*", "const char *", span, count=1)
            if new_span == span:
                continue
            pieces.append(text[last : m.start()])
            pieces.append(new_span)
            line, _ = unit.line_col(m.start())
            edits.append(Edit(self.name, "const char*", line, span, new_span))
            last = m.end()
        if not edits:
            return PassResult.unchanged(text)
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=[], edits=edits)


class VoidPtrCastPass(Pass):
    name = "void_ptr_cast"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        """Insert static_cast for `T *p = malloc(...)` style (heuristic)."""
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        pattern = re.compile(
            r"((\w[\w\s\*]*?)\*\s*)([A-Za-z_]\w*)\s*=\s*((?:\(.*?\)\s*)?(?:malloc|calloc|realloc|reallocf)\s*\([^;]+);"
        )
        pieces: list[str] = []
        last = 0
        for m in pattern.finditer(text):
            lhs_type = m.group(1)
            name = m.group(3)
            rhs = m.group(4)
            # Skip if already cast
            if "static_cast" in m.group(0) or re.search(r"\)\s*(malloc|calloc)", m.group(0)):
                # classic C cast might exist
                if re.search(r"=\s*\([^)]+\*\)\s*(malloc|calloc|realloc)", m.group(0)):
                    continue
            # Build typed pointer type string
            type_str = lhs_type.strip()
            if not type_str.endswith("*"):
                continue
            cast_rhs = f"static_cast<{type_str}>({rhs[:-1]});"  # drop trailing ;
            # Actually rhs includes trailing ;
            rhs_expr = rhs.rstrip().rstrip(";")
            new = f"{lhs_type}{name} = static_cast<{type_str}>({rhs_expr});"
            pieces.append(text[last : m.start()])
            pieces.append(new)
            line, _ = unit.line_col(m.start())
            edits.append(Edit(self.name, "void* static_cast", line, m.group(0)[:80], new[:80]))
            last = m.end()
        if not edits:
            return PassResult.unchanged(text)
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=[], edits=edits)


class RefusePatternPass(Pass):
    """Generic refuse-and-log pass (some codes demoted to proposals)."""

    PROPOSAL_CODES = {
        "FLEXIBLE_ARRAY",
        "NESTED_STRUCT_TAG",
        "GENERIC",
        "KR_DEFINITION",
        "IMPLICIT_INT",
    }

    def __init__(self, name: str, reason: str, pattern: str, flags: int = 0):
        self.name = name
        self.tier = 1
        self.reason = reason
        self.rx = re.compile(pattern, flags)

    def apply(self, unit: TranslationUnit) -> PassResult:
        masked = unit.mask_strings_comments()
        refusals: list[Refusal] = []
        for m in self.rx.finditer(masked):
            snip = unit.text[m.start() : m.start() + 80]
            if self.reason in self.PROPOSAL_CODES:
                _propose_pol(
                    unit,
                    self.reason,
                    {"line": unit.line_col(m.start())[0], "snippet": snip},
                )
            else:
                refusals.append(_ref(unit, self.name, self.reason, m.start(), snip))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class GotoRefusePass(Pass):
    """Superseded by goto_cleanup_scope_guard — no longer refuse every goto."""

    name = "goto_cross_init"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        return PassResult.unchanged(unit.text)


TIER1_PASSES: list[Pass] = [
    RegisterRemovePass(),
    RestrictPass(),
    C11ToCxxPass(),
    RefusePatternPass("knr_reject", "KR_DEFINITION", r"\)\s*\n(?:\s*[\w\s\*]+\s+[A-Za-z_]\w*\s*;\s*\n)+\s*\{"),
    RefusePatternPass("generic_refuse", "GENERIC", r"\b_Generic\b"),
    # compound / designated / VLA / goto handled by promote_refusals.py
    RefusePatternPass(
        "flexible_array_refuse",
        "FLEXIBLE_ARRAY",
        r"(?:char|unsigned\s+char|uint8_t|u_char)\s+\w+\s*\[\s*\]\s*;",
    ),
    RefusePatternPass(
        "nested_struct_tag_refuse",
        "NESTED_STRUCT_TAG",
        r"struct\s+\w+\s*\{[^}]*struct\s+\w+\s*\{",
    ),
    StringLiteralConstPass(),
    VoidPtrCastPass(),
    TypedefToUsingPass(),
    NullToNullptrPass(),
    CppKeywordRenamePass(),
]
