"""Enhanced malloc-RAII and span-signature rewrites (staged ports).

Registered after REFUSAL_PROMOTE_PASSES. Handles MALLOC_ESCAPE / SPAN_CANDIDATE
promotion with broader preconditions than the original tier2/promote stubs.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

_OUT = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"


def _propose_ms(unit, kind: str, payload: dict) -> None:
    _OUT.mkdir(parents=True, exist_ok=True)
    with (_OUT / "proposals.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps({"file": unit.path, "kind": kind, **payload}) + "\n")


# Non-retaining libc / BSD helpers — pointer not stored beyond the call.
NON_RETAINING_FNS = frozenset(
    {
        "free",
        "memset",
        "memcpy",
        "memmove",
        "memcmp",
        "bzero",
        "bcopy",
        "strlen",
        "strcmp",
        "strncmp",
        "strcpy",
        "strncpy",
        "strcat",
        "strncat",
        "snprintf",
        "vsnprintf",
        "sprintf",
        "printf",
        "fprintf",
        "dprintf",
        "asprintf",  # allocates itself; arg format only
        "puts",
        "fputs",
        "fputc",
        "putc",
        "putchar",
        "fwrite",
        "fread",
        "read",
        "write",
        "readv",
        "writev",
        "pread",
        "pwrite",
        "send",
        "recv",
        "sendto",
        "recvfrom",
        "err",
        "errx",
        "errc",
        "warn",
        "warnx",
        "exit",
        "_exit",
        "abort",
        "assert",
        "strlcpy",
        "strlcat",
        "sysctl",
        "sysctlbyname",
        "open",
        "close",
        "fstat",
        "stat",
        "lstat",
        "access",
        "chmod",
        "chown",
        "unlink",
        "rename",
        "qsort",
        "bsearch",
        "heapsort",
        "mergesort",
        "ptrace",
        "ioctl",
        "fcntl",
        "poll",
        "select",
        "kevent",
        "EV_SET",
        "htonl",
        "htons",
        "ntohl",
        "ntohs",
        "le16enc",
        "le32enc",
        "be16enc",
        "be32enc",
        "strchr",
        "strrchr",
        "strstr",
        "strvis",
        "strvisx",
        "strunvis",
        "atoi",
        "atol",
        "strtol",
        "strtoul",
        "strtoimax",
        "basename",
        "dirname",
        "getopt",
        "isdigit",
        "isalpha",
        "isspace",
        "tolower",
        "toupper",
        "wprintf",
        "fwprintf",
        "swprintf",
        "sbuf_bcat",
        "sbuf_cat",
        "sbuf_printf",
        "sbuf_finish",
        "ATF_REQUIRE",
        "ATF_CHECK",
        "ATF_REQUIRE_MSG",
        "bcmp",
        "kvm_read",
        "kvm_write",
        "mbsrtowcs",
        "wcsrtombs",
        "fgets",
        "fgetln",
        "getline",
        "bsdar_warnc",
        "bsdar_errc",
        "maybe_errx",
        "printname",
        "badfmt",
    }
)


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


def _functions(text: str) -> list[tuple[str, str, int, int, str]]:
    """Return (qualifiers, name, start, end, body) for function definitions."""
    out: list[tuple[str, str, int, int, str]] = []
    for m in re.finditer(
        r"(?:^|\n)([\w\s\*]+?)\b([A-Za-z_]\w*)\s*\(([^;]*?)\)\s*\{",
        text,
    ):
        quals = m.group(1)
        name = m.group(2)
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
                    out.append((quals, name, m.start(), i + 1, text[body_start : i + 1]))
                    break
            i += 1
    return out


def _stmt_end(text: str, start: int) -> int:
    """End index (exclusive) of a simple statement or `{...}` block from start."""
    i = start
    while i < len(text) and text[i] in " \t\r\n":
        i += 1
    if i < len(text) and text[i] == "{":
        depth = 0
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    return i + 1
            i += 1
        return len(text)
    while i < len(text) and text[i] != ";":
        i += 1
    return min(i + 1, len(text))


def _call_takes_var(body: str, var: str) -> list[str]:
    """Return callee names where `var` appears as an argument."""
    callees: list[str] = []
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", body):
        fn = m.group(1)
        if fn in ("if", "while", "for", "switch", "return", "sizeof"):
            continue
        i = m.end()
        depth = 1
        args_start = i
        while i < len(body) and depth:
            if body[i] == "(":
                depth += 1
            elif body[i] == ")":
                depth -= 1
            i += 1
        args = body[args_start : i - 1]
        if re.search(rf"\b{re.escape(var)}\b", args):
            callees.append(fn)
    return callees


def _malloc_escapes(body: str, var: str) -> str | None:
    """Return escape reason or None if safe for unique_ptr ownership."""
    if re.search(rf"&{re.escape(var)}\b", body):
        return "address-taken"
    if re.search(rf"(?:->|\.)\s*\w+\s*=\s*{re.escape(var)}\b", body):
        return "stored-to-field"
    if re.search(rf"\breturn\s+{re.escape(var)}\b", body):
        return "returned"
    for m in re.finditer(rf"(?<![=!<>])=(?!=)\s*{re.escape(var)}\b", body):
        left = body[max(0, m.start() - 60) : m.start()]
        if re.search(rf"\b{re.escape(var)}\s*$", left):
            continue
        if re.search(r"(?:malloc|calloc|realloc|reallocf)\s*\([^)]*$", left):
            continue
        if re.search(rf"(?:\*|{re.escape(var)}\s*\[)\s*$", left):
            continue
        # Only count pointer copies: `other = var` (not comparisons leftovers)
        lm = re.search(r"([A-Za-z_]\w*)\s*$", left)
        if not lm:
            continue
        if lm.group(1) == var:
            continue
        # Skip if looks like part of `==` already excluded; skip struct init noise
        if re.search(r"(?:return|case|sizeof)\s*$", left):
            continue
        return "copied-out"
    for fn in _call_takes_var(body, var):
        if fn in NON_RETAINING_FNS:
            continue
        if fn in (
            "malloc",
            "calloc",
            "realloc",
            "reallocf",
            "sizeof",
            "typeof",
            "if",
            "while",
            "for",
            "switch",
        ):
            continue
        return f"passed-to-{fn}"
    return None

def _guard_insert_offset(body: str, alloc_end: int, var: str) -> int:
    """Insert unique_ptr after null-check + err/errx when present; else after alloc."""
    rest = body[alloc_end:]
    m = re.match(
        rf"\s*if\s*\(\s*(?:{re.escape(var)}\s*==\s*(?:NULL|nullptr|0)|!\s*{re.escape(var)})\s*\)\s*",
        rest,
    )
    if not m:
        return alloc_end
    after_cond = alloc_end + m.end()
    peek = rest[m.end() :].lstrip()
    if re.match(r"(?:errx?|errc|warnx?|return|goto|exit|_exit|abort)\b", peek) or peek.startswith(
        "{"
    ):
        return _stmt_end(body, after_cond)
    return alloc_end


def _oom_err_exit(body: str, var: str) -> bool:
    """True when OOM path is noreturn err/exit (0 free sites are OK)."""
    v = re.escape(var)
    brace = r"(?:\{[^}]{0,200}\})?"
    if re.search(
        rf"if\s*\(\s*\(\s*{v}\s*=\s*(?:\(.*?\)\s*)?(?:malloc|calloc)\s*\([^;]+?\)\s*\)\s*"
        rf"==\s*(?:NULL|nullptr|0)\s*\)\s*{brace}(?:errx?|errc|exit|_exit|abort)\b",
        body,
        re.S,
    ):
        return True
    if re.search(
        rf"\b{v}\s*=\s*(?:\(.*?\)\s*)?(?:malloc|calloc)\s*\([^;]+;"
        rf"\s*if\s*\(\s*(?:{v}\s*==\s*(?:NULL|nullptr|0)|!\s*{v})\s*\)\s*"
        rf"{brace}(?:errx?|errc|exit|_exit|abort)\b",
        body,
        re.S,
    ):
        return True
    return False


def _iter_allocs(body: str) -> list[tuple[int, int, str, str]]:
    """Return (start, end, var, full_text) for each malloc/calloc assignment."""
    found: list[tuple[int, int, str, str]] = []
    for m in re.finditer(
        r"\b([A-Za-z_]\w*)\s*=\s*(?:\([^;]*?\)\s*)?(?:malloc|calloc)\s*\(",
        body,
        re.S,
    ):
        var = m.group(1)
        i = m.end() - 1
        depth = 0
        while i < len(body):
            if body[i] == "(":
                depth += 1
            elif body[i] == ")":
                depth -= 1
                if depth == 0:
                    i += 1
                    break
            i += 1
        found.append((m.start(), i, var, body[m.start() : i]))
    return found


class PromoteMallocRaiiPass(Pass):
    """malloc/calloc + free → unique_ptr; multi-pair; non-retaining calls OK."""

    name = "promote_malloc_raii"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        work: list[tuple[int, int, str]] = []

        for _quals, fname, fstart, fend, body in _functions(text):
            body_start = fend - len(body)
            allocs = _iter_allocs(body)
            seen_vars: set[str] = set()
            for a_start, a_end, var, a_text in allocs:
                if var in seen_vars:
                    _propose_ms(unit, "MALLOC_ESCAPE", {"line": unit.line_col(body_start + a_start)[0], "snippet": "re-alloc"})
                    continue
                seen_vars.add(var)
                if f"pbsd_guard_{var}" in body or f"pbsd_err_guard_{var}" in body:
                    continue
                frees = list(re.finditer(
                    rf"\bfree\s*\(\s*(?:\([^)]*\)\s*)?{re.escape(var)}\s*\)\s*;",
                    body,
                ))
                if len(frees) == 0:
                    # No free: still RAII if no escape (end-of-scope cleanup).
                    if _malloc_escapes(body, var):
                        _propose_ms(unit, "MALLOC_ESCAPE", {"line": unit.line_col(body_start + a_start)[0], "snippet": a_text[:80],})
                        continue
                    # fall through to guard insert
                elif len(frees) > 1:
                    body_wo_free = body
                    for fm in frees:
                        body_wo_free = (
                            body_wo_free[: fm.start()]
                            + " " * (fm.end() - fm.start())
                            + body_wo_free[fm.end() :]
                        )
                    if _malloc_escapes(body_wo_free, var):
                        _propose_ms(unit, "MALLOC_ESCAPE", {"line": unit.line_col(body_start + a_start)[0], "snippet": "multi-free",})
                        continue
                    for fm in frees:
                        work.append(
                            (
                                body_start + fm.start(),
                                body_start + fm.end(),
                                f"/* pbsd: free {var} via unique_ptr */",
                            )
                        )
                    frees = []
                free_m = frees[0] if frees else None
                body_wo_free = body
                if free_m is not None:
                    body_wo_free = (
                        body[: free_m.start()]
                        + " " * (free_m.end() - free_m.start())
                        + body[free_m.end() :]
                    )
                    esc = _malloc_escapes(body_wo_free, var)
                    if esc:
                        _propose_ms(unit, "MALLOC_ESCAPE", {"line": unit.line_col(body_start + a_start)[0], "snippet": esc})
                        continue
                # len(frees)==0 already checked escape above
                ins_rel = _guard_insert_offset(body, a_end, var)
                # Also handle if ((var = malloc()) == null) — insert after full if-stmt
                if ins_rel == a_end:
                    rest = body[a_end:]
                    mif = re.match(
                        rf"\s*\)\s*==\s*(?:NULL|nullptr|0)\s*\)\s*",
                        rest,
                    )
                    if mif:
                        after = a_end + mif.end()
                        peek = body[after:].lstrip()
                        if re.match(r"(?:errx?|errc|exit|_exit|abort)\b", peek) or peek.startswith(
                            "{"
                        ):
                            ins_rel = _stmt_end(body, after)
                guard = (
                    f"\n\tstd::unique_ptr<void, decltype(&std::free)> "
                    f"pbsd_guard_{var}({var}, &std::free);"
                )
                if free_m is not None:
                    work.append(
                        (
                            body_start + free_m.start(),
                            body_start + free_m.end(),
                            "/* pbsd: free via unique_ptr */",
                        )
                    )
                work.append((body_start + ins_rel, body_start + ins_rel, guard))
                edits.append(
                    Edit(
                        self.name,
                        f"RAII guard for {var} in {fname}",
                        unit.line_col(body_start + a_start)[0],
                        a_text[:60],
                        guard.strip(),
                    )
                )

        if not work:
            return PassResult(text=text, refusals=refusals, edits=edits)

        for start, end, payload in sorted(work, key=lambda x: x[0], reverse=True):
            text = text[:start] + payload + text[end:]
        if "#include <memory>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <memory>\n#include <cstdlib>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text
        return PassResult(text=text, refusals=refusals, edits=edits)


class PromoteSpanSignaturePass(Pass):
    """(const T*|T*|void*, size_t|int) → std::span; static in-file call-site updates."""

    name = "promote_span_signature"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        work: list[tuple[str, int, int, str]] = []
        need_span = False
        rx = re.compile(
            r"\b((?:const\s+)?(?:void|[\w:]+))\s*\*\s*(\w+)\s*,\s*"
            r"(size_t|int|u_int|unsigned|ssize_t)\s+(\w+)"
        )

        for m in re.finditer(
            r"(?:^|\n)([\w\s\*]+)\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            text,
        ):
            quals, fname, params = m.group(1), m.group(2), m.group(3)
            if "std::span" in params:
                continue
            pm = rx.search(params)
            if not pm:
                continue
            typ, ptr, _lnty, ln = pm.group(1).strip(), pm.group(2), pm.group(3), pm.group(4)
            is_const = bool(re.search(r"\bconst\b", typ))
            base = re.sub(r"\bconst\b", "", typ).strip()
            if base == "void":
                span_t = "std::span<const std::byte>" if is_const else "std::span<std::byte>"
            elif is_const:
                span_t = f"std::span<const {base}>"
            else:
                span_t = f"std::span<{base}>"

            body_start = m.end() - 1
            depth = 0
            i = body_start
            while i < len(text):
                if text[i] == "{":
                    depth += 1
                elif text[i] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            body_end = i + 1
            body = text[body_start:body_end]

            # Arithmetic / increment — but not `p->field`
            if re.search(
                rf"\b{re.escape(ptr)}\s*(?:\+\+|--|\+(?![=+])|-(?!>))",
                body,
            ):
                _propose_ms(unit, "SPAN_CANDIDATE", {"line": unit.line_col(m.start())[0], "snippet": f"{ptr},{ln}"})
                continue

            idx_uses = re.findall(rf"\b{re.escape(ptr)}\s*\[\s*([^\]]+)\s*\]", body)
            if not idx_uses:
                # Not a ptr/len buffer pair (e.g. main argv) — skip quietly.
                continue

            unbounded = False
            for idx_expr in idx_uses:
                idx_expr = idx_expr.strip()
                if re.fullmatch(r"\d+", idx_expr):
                    continue
                idents = re.findall(r"\b([A-Za-z_]\w*)\b", idx_expr)
                ok = False
                for ident in idents:
                    if ident == ln:
                        ok = True
                        break
                    if re.search(
                        rf"\b{re.escape(ident)}\s*(?:<|<=)\s*{re.escape(ln)}\b", body
                    ) or re.search(
                        rf"\b{re.escape(ln)}\s*(?:>|>=)\s*{re.escape(ident)}\b", body
                    ):
                        ok = True
                        break
                if not ok and idents:
                    _propose_ms(unit, "SPAN_CANDIDATE", {"line": unit.line_col(m.start())[0], "snippet": f"{ptr},{ln}:unbounded-{idx_expr[:40]}",})
                    unbounded = True
                    break
            if unbounded:
                continue

            new_params = rx.sub(f"{span_t} {ptr}", params, count=1)
            open_paren = text.find("(", m.start())
            close_paren = text.find(")", open_paren)
            work.append(("repl", open_paren + 1, close_paren, new_params))
            if re.search(rf"\b{re.escape(ln)}\b", body):
                work.append(
                    (
                        "ins",
                        body_start + 1,
                        body_start + 1,
                        f"\n\tsize_t {ln} = {ptr}.size();\n",
                    )
                )
            edits.append(
                Edit(
                    self.name,
                    f"span {ptr}",
                    unit.line_col(m.start())[0],
                    params[:40],
                    new_params[:40],
                )
            )
            need_span = True

            is_static = bool(re.search(r"\bstatic\b", quals))
            if is_static:
                self._rewrite_static_calls(
                    text, fname, body_start, work, edits, unit, refusals
                )
            else:
                # Non-static: definition rewritten; callers out of scope for staged ports.
                _propose_ms(unit, "CALL_SITE_SPAN", {"line": unit.line_col(m.start())[0], "snippet": fname})

        for kind, start, end, payload in sorted(work, key=lambda x: x[1], reverse=True):
            if kind == "repl":
                text = text[:start] + payload + text[end:]
            else:
                text = text[:start] + payload + text[start:]

        if need_span and "#include <span>" not in text:
            hm = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <span>\n#include <cstddef>\n"
            if hm:
                pos = hm[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text
        return PassResult(text=text, refusals=refusals, edits=edits)

    def _rewrite_static_calls(
        self,
        text: str,
        fname: str,
        body_start: int,
        work: list,
        edits: list[Edit],
        unit: TranslationUnit,
        refusals: list[Refusal],
    ) -> None:
        """foo(buf, n, ...) → foo(std::span(buf, n), ...) for in-file call sites."""
        for cm in re.finditer(rf"\b{re.escape(fname)}\s*\(", text):
            close = text.find(")", cm.end())
            if close < 0:
                continue
            args = text[cm.end() : close]
            parts = [p.strip() for p in args.split(",") if p.strip() != ""]
            if len(parts) < 2:
                continue
            # Skip the definition parameter list (typed args).
            if cm.start() < body_start and (
                "*" in parts[0]
                or re.search(r"\b(?:size_t|int|u_int|unsigned|ssize_t)\b", parts[1])
            ):
                continue
            # Already rewritten
            if args.strip().startswith("std::span"):
                continue
            a0, a1 = parts[0], parts[1]
            new_args = f"std::span({a0}, {a1})"
            if len(parts) > 2:
                new_args += ", " + ", ".join(parts[2:])
            work.append(("repl", cm.end(), close, new_args))
            edits.append(
                Edit(
                    self.name,
                    f"call-site span {fname}",
                    unit.line_col(cm.start())[0],
                    args[:40],
                    new_args[:40],
                )
            )


# Span runs via SpanSignatureRewritePass (promote_refusals) which delegates here.
# Only register malloc here — after REFUSAL_PROMOTE_PASSES in __init__.py.
PROMOTE_MALLOC_SPAN_PASSES: list[Pass] = [
    PromoteMallocRaiiPass(),
]
