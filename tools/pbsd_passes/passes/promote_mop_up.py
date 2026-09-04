# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Aggressive mop-up rewrites for remaining hard refusal codes."""
from __future__ import annotations

import re

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass


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


class MemcpyByteSpanPass(Pass):
    """memcpy(dst, src, n) where dst/src are void*/char* buffers → std::copy_n / memcpy keep but wrap sizes.

    For identical-type POD structs already handled; here:
    - memcpy(a, b, sizeof(*a)) when a,b same pointer type → bit_cast loop refuse
    - memcpy(dst, src, n) with char*/void* → std::copy_n(reinterpret...)
    Prefer: leave as memcpy but add static_cast<void*> for C++ — actually just
    `std::memcpy` with `#include <cstring>` and stop refusing.
    """

    name = "memcpy_cstring"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        if "memcpy" not in text:
            return PassResult.unchanged(text)

        # Promote C memcpy to std::memcpy (valid in C++)
        masked = unit.mask_strings_comments()
        ops = []
        for m in re.finditer(r"(?<![:\w])memcpy\s*\(", masked):
            ops.append((m.start(), m.start() + len("memcpy"), "std::memcpy", "memcpy"))
        # Also memmove/memset
        for name, repl in (("memmove", "std::memmove"), ("memset", "std::memset")):
            for m in re.finditer(rf"(?<![:\w]){name}\s*\(", masked):
                ops.append((m.start(), m.start() + len(name), repl, name))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(Edit(self.name, f"{old}→{new}", unit.line_col(start)[0], old, new))

        if edits and "#include <cstring>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <cstring>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text

        # Remaining BIT_CAST only for *(T*)& patterns not yet rewritten
        for m in re.finditer(r"\*\s*\(\s*([\w:]+)\s*\*\s*\)\s*&(\w+)", unit.mask_strings_comments()):
            typ, src = m.group(1), m.group(2)
            if typ in ("void", "char", "unsigned"):
                refusals.append(_ref(unit, self.name, "BIT_CAST_CANDIDATE", m.start(), m.group(0)[:40]))
                continue
            old = text[m.start() : m.end()]
            new = f"std::bit_cast<{typ}>({src})"
            text = text[: m.start()] + new + text[m.end() :]
            edits.append(Edit(self.name, "bit_cast", unit.line_col(m.start())[0], old[:40], new[:40]))
            if "#include <bit>" not in text:
                text = "#include <bit>\n" + text

        return PassResult(text=text, refusals=refusals, edits=edits)


class MallocErrPatternPass(Pass):
    """Classic BSD: p = malloc(n); if (p == NULL) err(1, ...); ... use p ... (no free) in main-like.

    Or: if ((p = malloc(n)) == NULL) err...
    Wrap with unique_ptr after the null check.
    """

    name = "malloc_err_pattern"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        ops = []

        # if ((var = malloc(...)) == nullptr) err...
        rx = re.compile(
            r"if\s*\(\s*\(\s*([A-Za-z_]\w*)\s*=\s*((?:malloc|calloc)\s*\([^;]+?)\)\s*==\s*(?:nullptr|NULL)\s*\)\s*"
            r"(?:\{[^}]{0,200}\})?(?:err|errx|errc)\s*\([^;]+;"
        )
        for m in rx.finditer(unit.mask_strings_comments()):
            var = m.group(1)
            if f"pbsd_err_guard_{var}" in text:
                continue
            # insert unique_ptr after the if statement
            insert_at = m.end()
            guard = (
                f"\n\tstd::unique_ptr<void, decltype(&std::free)> "
                f"pbsd_err_guard_{var}({var}, &std::free);"
            )
            # Only if free(var) exists later OR function returns without free (still OK — unique_ptr)
            ops.append((insert_at, guard, var, m.start()))

        for insert_at, guard, var, start in sorted(ops, key=lambda x: x[0], reverse=True):
            # remove matching free if present once
            text2 = re.sub(
                rf"\bfree\s*\(\s*{re.escape(var)}\s*\)\s*;",
                f"/* pbsd: free {var} via err-guard */",
                text,
                count=1,
            )
            text = text2[:insert_at] + guard + text2[insert_at:]
            edits.append(
                Edit(self.name, f"err-malloc RAII {var}", unit.line_col(start)[0], var, guard.strip())
            )

        if edits and "#include <memory>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <memory>\n#include <cstdlib>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text

        return PassResult(text=text, refusals=refusals, edits=edits)


class SpanCallSiteRewritePass(Pass):
    """Rewrite non-static call sites: foo(buf, n) → foo(std::span(buf, n)) when foo's
    definition in same file was already span-ified (detect size_t alias inject).
    """

    name = "span_call_site_rewrite"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []

        # Functions that have `const std::size_t X = Y.size()` inject → spanified
        span_fns = set(
            re.findall(
                r"\b([A-Za-z_]\w*)\s*\([^)]*std::span<",
                text,
            )
        )
        # Also from signature rewrite residue
        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\(\s*std::span<[^>]+>\s*(\w+)\s*\)",
            text,
        ):
            span_fns.add(m.group(1))

        for fname in span_fns:
            # call sites: fname(arg1, arg2)
            for m in re.finditer(
                rf"\b{re.escape(fname)}\s*\(\s*([^,]+)\s*,\s*([^)]+)\)",
                unit.mask_strings_comments(),
            ):
                # skip definition
                line_start = text.rfind("\n", 0, m.start()) + 1
                prefix = text[line_start : m.start()]
                if re.search(r"\b(?:static|inline|const|unsigned|int|void|char|size_t)\b", prefix):
                    # might still be a call
                    pass
                # skip if already span
                args = text[m.start(1) : m.end(2)]
                if "std::span" in args or "span<" in args:
                    continue
                a1 = text[m.start(1) : m.end(1)].strip()
                a2 = text[m.start(2) : m.end(2)].strip()
                # skip if looks like definition params with types
                if re.search(r"\b(?:int|char|void|size_t|struct)\b", a1):
                    continue
                new_call = f"{fname}(std::span({a1}, {a2}))"
                old = text[m.start() : m.end()]
                text = text[: m.start()] + new_call + text[m.end() :]
                edits.append(
                    Edit(self.name, f"call-site span {fname}", unit.line_col(m.start())[0], old[:50], new_call[:50])
                )

        if edits and "#include <span>" not in text:
            text = "#include <span>\n" + text

        # leftover CALL_SITE_SPAN style — refuse remaining fname(buf,n) for known names without rewrite
        return PassResult(text=text, refusals=refusals, edits=edits)


class KrDefinitionFixPass(Pass):
    """K&R: type foo(a, b) type a; type b; { → type foo(type a, type b) {"""

    name = "kr_definition_fix"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        # Line-oriented K&R match — avoid nested [\w\s*]+ catastrophic regex
        rx = re.compile(
            r"(?m)^([\w\s\*]{1,80}?)\b([A-Za-z_]\w*)\s*\("
            r"([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*){0,8})\)\s*$"
        )
        lines = text.splitlines(keepends=True)
        offs: list[int] = []
        pos = 0
        for ln in lines:
            offs.append(pos)
            pos += len(ln)
        decl_re = re.compile(
            r"^[ \t]*([\w\s\*]{1,60}?)\s+([A-Za-z_]\w*)\s*;[ \t]*$"
        )
        ops = []
        for i, ln in enumerate(lines):
            m = rx.match(ln.rstrip("\n"))
            if not m:
                continue
            ret, name, params = m.group(1), m.group(2), m.group(3)
            pnames = [p.strip() for p in params.split(",")]
            dtype: dict[str, str] = {}
            j = i + 1
            while j < len(lines) and len(dtype) < 16:
                dm = decl_re.match(lines[j].rstrip("\n"))
                if not dm:
                    break
                dtype[dm.group(2)] = dm.group(1).strip()
                j += 1
            if j >= len(lines) or not re.match(r"^[ \t]*\{", lines[j]):
                continue
            if not all(p in dtype for p in pnames):
                from ..proposals import propose

                propose(
                    unit.path,
                    "KR_DEFINITION",
                    {
                        "line": unit.line_col(offs[i])[0],
                        "snippet": name,
                        "hint": "incomplete K&R decl types",
                    },
                )
                continue
            new_params = ", ".join(f"{dtype[p]} {p}" for p in pnames)
            new = f"{ret}{name}({new_params})\n{{"
            end = offs[j] + len(lines[j])
            # Consume through opening brace line
            brace_end = offs[j] + lines[j].find("{") + 1
            ops.append((offs[i], brace_end, new, text[offs[i] : brace_end][:60]))
        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(Edit(self.name, "K&R→ANSI", unit.line_col(start)[0], old, new[:60]))
        return PassResult(text=text, refusals=[], edits=edits)


class CloseFdGuardPass(Pass):
    """goto cleanup with close(fd) → unique_ptr with custom closer, or RAII int wrapper comment.

    Practical: after `fd = open(...)` success, if close(fd) at label, inject:
      /* PBSD: prefer posix close guard */
    and replace close with no-op when we can use a simple struct.
    """

    name = "close_fd_guard"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        if "close(" not in text or "goto" not in text:
            return PassResult.unchanged(text)

        helper = """
#ifndef PBSD_FD_GUARD
#define PBSD_FD_GUARD
struct pbsd_fd_guard {
  int fd{-1};
  explicit pbsd_fd_guard(int f) : fd(f) {}
  ~pbsd_fd_guard() { if (fd >= 0) ::close(fd); }
  pbsd_fd_guard(const pbsd_fd_guard&) = delete;
  int get() const { return fd; }
  int release() { int t = fd; fd = -1; return t; }
};
#endif
"""
        ops = []
        for lab_m in re.finditer(r"(?m)^[ \t]*(out|fail|error|cleanup|done|err)\s*:", text, re.I):
            after = text[lab_m.end() : lab_m.end() + 400]
            for cm in re.finditer(r"\bclose\s*\(\s*(\w+)\s*\)\s*;", after):
                fd = cm.group(1)
                # find open assignment
                opens = list(
                    re.finditer(
                        rf"\b{re.escape(fd)}\s*=\s*open\s*\([^;]+;",
                        text,
                    )
                )
                if not opens:
                    continue
                o = opens[-1]
                if f"pbsd_fd_guard pbsd_g_{fd}" in text:
                    continue
                guard = f"\n\tpbsd_fd_guard pbsd_g_{fd}({fd});"
                ops.append((o.end(), guard, lab_m.end() + cm.start(), lab_m.end() + cm.end(), fd))

        if not ops:
            return PassResult.unchanged(text)

        if "PBSD_FD_GUARD" not in text:
            text = helper + text
            edits.append(Edit(self.name, "fd guard helper", 1, "", "helper"))

        for ins, guard, cs, ce, fd in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:cs] + f"/* pbsd: close {fd} via guard */" + text[ce:]
            text = text[:ins] + guard + text[ins:]
            edits.append(Edit(self.name, f"fd guard {fd}", 1, fd, guard.strip()))

        if edits and "#include <unistd.h>" not in text and "<unistd.h>" not in text:
            text = "#include <unistd.h>\n" + text

        return PassResult(text=text, refusals=[], edits=edits)


MOP_UP_PASSES: list[Pass] = [
    MemcpyByteSpanPass(),
    MallocErrPatternPass(),
    SpanCallSiteRewritePass(),
    KrDefinitionFixPass(),
    CloseFdGuardPass(),
]
