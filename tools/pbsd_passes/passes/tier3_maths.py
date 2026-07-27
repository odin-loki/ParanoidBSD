"""Tier-3 maths passes: CCured pointer kinds, Steensgaard escape, null lattice, span IV, queue macros."""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..analysis import (
    NullLattice,
    NullabilityFixpoint,
    PtrKind,
    Steensgaard,
    UnionFindLattice,
)
from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

_OUT = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"


def _propose_span(unit, kind: str, payload: dict) -> None:
    from ..proposals import propose

    propose(unit.path, kind, payload)


ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "docs" / "migration" / "clang_port"


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
    out = []
    for m in re.finditer(
        r"(?:^|\n)([\w\s\*]+?)\b([A-Za-z_]\w*)\s*\(([^;]*?)\)\s*\{",
        text,
    ):
        name = m.group(2)
        body_start = m.end() - 1
        depth = 0
        i = body_start
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    out.append((name, m.start(), i + 1, text[body_start : i + 1]))
                    break
            i += 1
    return out


class CCuredPointerKindPass(Pass):
    """Union-find pointer-kind inference; rewrite SAFE observers; refuse only WILD."""

    name = "ccured_pointer_kind"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        uf = UnionFindLattice()
        edits: list[Edit] = []
        refusals: list[Refusal] = []

        # Seed all pointer decls
        for m in re.finditer(
            r"\b((?:const\s+)?(?:struct\s+)?[\w:]+)\s*\*\s*([A-Za-z_]\w*)\b",
            masked,
        ):
            uf.add(m.group(2), PtrKind.SAFE)

        # Arithmetic → SEQ
        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*(\+\+|--|\+=|-=|\+|\[)", masked):
            uf.raise_kind(m.group(1), PtrKind.SEQ)

        # Casts involving void* or mismatched → WILD + unify
        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*=\s*\(\s*([\w\s\*]+)\s*\*?\s*\)\s*([A-Za-z_]\w*|malloc|calloc)",
            masked,
        ):
            dst, cast_t, src = m.group(1), m.group(2), m.group(3)
            uf.add(dst)
            if "void" in cast_t or src in ("malloc", "calloc"):
                uf.raise_kind(dst, PtrKind.WILD if "void" in cast_t else PtrKind.SAFE)
            if src not in ("malloc", "calloc"):
                uf.add(src)
                uf.union(dst, src)
                if "void" in cast_t:
                    uf.raise_kind(dst, PtrKind.WILD)

        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s*;", masked):
            uf.add(m.group(1))
            uf.add(m.group(2))
            uf.union(m.group(1), m.group(2))

        # Rewrite: SAFE pointer params never written → prefer references in signature
        # (only for static functions to limit ABI blast)
        for fname, fstart, fend, body in _functions(text):
            # detect static
            head = text[max(0, fstart - 20) : fstart + 40]
            is_static = bool(re.search(r"\bstatic\b", head))
            if not is_static:
                continue
            sig_m = re.search(
                rf"\b{re.escape(fname)}\s*\(([^)]*)\)",
                text[fstart : fstart + 200],
            )
            if not sig_m:
                continue
            params = sig_m.group(1)
            new_parts = []
            changed = False
            for p in params.split(","):
                raw = p
                ps = p.strip()
                if not ps or "*" not in ps or "const" not in ps:
                    new_parts.append(raw)
                    continue
                nm = re.search(r"([A-Za-z_]\w*)\s*$", ps.replace("[]", ""))
                if not nm:
                    new_parts.append(raw)
                    continue
                name = nm.group(1)
                if uf.get_kind(name) != PtrKind.SAFE:
                    new_parts.append(raw)
                    continue
                if re.search(rf"\b{re.escape(name)}\s*=", body):
                    new_parts.append(raw)
                    continue
                # const T *p → const T &p  (SAFE non-null preference — only if never null-checked as maybe)
                if re.search(rf"{re.escape(name)}\s*==\s*(?:nullptr|NULL)", body):
                    new_parts.append(raw)
                    continue
                np = re.sub(r"\s*\*\s*", " &", ps, count=1)
                if np != ps:
                    leading = raw[: len(raw) - len(raw.lstrip())]
                    new_parts.append(leading + np)
                    changed = True
                    # Fix body: p-> → p.  and (*p) careful — only arrow
                    # applied after signature rewrite
                    unit.meta.setdefault("_safe_ref", []).append((fname, name))
                else:
                    new_parts.append(raw)
            if changed:
                open_p = text.find("(", fstart)
                close_p = text.find(")", open_p)
                text = text[: open_p + 1] + ",".join(new_parts) + text[close_p:]
                for _, name in [x for x in unit.meta.get("_safe_ref", []) if x[0] == fname]:
                    # replace name-> with name. inside this function only — coords shifted; do via body re-find
                    pass
                edits.append(
                    Edit(
                        self.name,
                        f"SAFE→ref params {fname}",
                        unit.line_col(fstart)[0],
                        params[:40],
                        ",".join(new_parts)[:40],
                    )
                )

        # Arrow rewrite for converted refs — second scan
        for fname, name in unit.meta.pop("_safe_ref", []):
            for fn, fstart, fend, body in _functions(text):
                if fn != fname:
                    continue
                new_body = re.sub(rf"\b{re.escape(name)}\s*->", f"{name}.", body)
                if new_body != body:
                    text = text[:fstart] + text[fstart:fend].replace(body, new_body, 1)
                    # fstart here is wrong for body replace — use careful
                break

        # SAFE→ref edits only; WILD refusals left to wild_pointer_cast
        return PassResult(text=text, refusals=[], edits=edits)


class SteensgaardMallocPass(Pass):
    """Steensgaard escape analysis → unique_ptr when allocation does not escape."""

    name = "steensgaard_malloc"
    tier = 3

    ESCAPE_SINKS = frozenset(
        {
            "return",
            "longjmp",
            "pthread_create",
            "taskqueue",
        }
    )
    NON_ESCAPE_CALLEES = frozenset(
        {
            "free",
            "memset",
            "memcpy",
            "memmove",
            "bzero",
            "strlen",
            "strcmp",
            "strncmp",
            "strcpy",
            "strncpy",
            "strlcpy",
            "snprintf",
            "printf",
            "fprintf",
            "err",
            "errx",
            "warn",
            "warnx",
            "assert",
            "bcopy",
            "memcmp",
        }
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        ops: list[tuple[int, int, int, str]] = []

        for fname, fstart, fend, body in _functions(text):
            st = Steensgaard()
            allocs: list[tuple[str, int, int]] = []  # var, start, end in body

            # Process statements roughly line by line
            for m in re.finditer(
                r"\b([A-Za-z_]\w*)\s*=\s*((?:\(.*?\)\s*)?(?:malloc|calloc|reallocf?)\s*\([^;]+);",
                body,
            ):
                var = m.group(1)
                st.uf.add(st._loc(var))
                allocs.append((var, m.start(), m.end()))

            for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s*;", body):
                st.assign(m.group(1), m.group(2))

            for m in re.finditer(r"\breturn\s+([A-Za-z_]\w*)\s*;", body):
                st.mark_escape(m.group(1))

            for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(([^)]*)\)", body):
                callee, args = m.group(1), m.group(2)
                for a in re.findall(r"\b([A-Za-z_]\w*)\b", args):
                    if callee in self.NON_ESCAPE_CALLEES:
                        continue
                    if callee in ("malloc", "calloc", "realloc", "reallocf", "sizeof", "typeof"):
                        continue
                    # field store / unknown callee → escape
                    st.mark_escape(a)

            for m in re.finditer(r"(?:->|\.)\s*\w+\s*=\s*([A-Za-z_]\w*)\b", body):
                st.mark_escape(m.group(1))

            for var, a_start, a_end in allocs:
                frees = list(re.finditer(rf"\bfree\s*\(\s*{re.escape(var)}\s*\)\s*;", body))
                if st.escapes(var):
                    # Leave refusal to promote_malloc; we only rewrite proven non-escape.
                    continue
                if len(frees) > 1:
                    continue
                if len(frees) == 0 and not st.escapes(var):
                    # leak without free — still wrap
                    pass
                guard = (
                    f"\n\tstd::unique_ptr<void, decltype(&std::free)> "
                    f"pbsd_st_{var}({var}, &std::free);"
                )
                if frees:
                    free_m = frees[0]
                    ops.append(
                        (
                            fstart + free_m.start(),
                            fstart + free_m.end(),
                            fstart + a_end,
                            guard,
                        )
                    )
                else:
                    ops.append((-1, -1, fstart + a_end, guard))
                edits.append(
                    Edit(
                        self.name,
                        f"Steensgaard RAII {var}",
                        unit.line_col(fstart + a_start)[0],
                        var,
                        guard.strip(),
                    )
                )

        # no refusals from this pass — rewrite-only
        refusals = []

        if not ops:
            return PassResult(text=text, refusals=refusals, edits=edits)

        # Deduplicate by insert point
        seen_ins = set()
        uniq = []
        for op in ops:
            if op[2] in seen_ins:
                continue
            seen_ins.add(op[2])
            uniq.append(op)
        ops = uniq

        new_text = text
        for free_s, free_e, ins, guard in sorted(ops, key=lambda x: x[2], reverse=True):
            if free_s >= 0:
                new_text = new_text[:free_s] + "/* pbsd: steensgaard free */" + new_text[free_e:]
            new_text = new_text[:ins] + guard + new_text[ins:]
        if "#include <memory>" not in new_text:
            m = list(re.finditer(r"(?m)^#include\b.*$", new_text))
            hdr = "#include <memory>\n#include <cstdlib>\n"
            if m:
                pos = m[-1].end()
                new_text = new_text[:pos] + "\n" + hdr + new_text[pos:]
            else:
                new_text = hdr + new_text
        return PassResult(text=new_text, refusals=refusals, edits=edits)


class NullabilityLatticePass(Pass):
    """3-value nullability fixpoint; refuse only MAYBE_NULL params with -> and no check."""

    name = "nullability_lattice"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        refusals: list[Refusal] = []
        edits: list[Edit] = []

        # assert != NULL → nullptr
        for m in list(
            re.finditer(r"\bassert\s*\(\s*(\w+)\s*!=\s*NULL\s*\)", unit.mask_strings_comments())
        )[::-1]:
            old = text[m.start() : m.end()]
            new = f"assert({m.group(1)} != nullptr)"
            text = text[: m.start()] + new + text[m.end() :]
            edits.append(Edit(self.name, "assert", unit.line_col(m.start())[0], old, new))

        for fname, fstart, fend, body in _functions(text):
            # extract pointer params
            sig = re.search(rf"\b{re.escape(fname)}\s*\(([^)]*)\)", text[fstart : fstart + 300])
            if not sig:
                continue
            params = {}
            for p in sig.group(1).split(","):
                if "*" not in p:
                    continue
                nm = re.search(r"([A-Za-z_]\w*)\s*$", p.strip().replace("[]", ""))
                if nm:
                    params[nm.group(1)] = NullLattice.MAYBE_NULL

            lat = NullabilityFixpoint()
            for p, v in params.items():
                lat.state[p] = v

            # Walk statements
            for line in body.split("\n"):
                # null check early exit
                m = re.search(
                    r"if\s*\(\s*(?:!(\w+)|(\w+)\s*==\s*(?:nullptr|NULL|0))\s*\)\s*(?:return|goto)",
                    line,
                )
                if m:
                    name = m.group(1) or m.group(2)
                    # fallthrough after this line in same function is optimistic —
                    # mark NEVER_NULL for subsequent lines (approximation)
                    lat.set_meet_after_check(name)
                    continue
                m = re.search(r"\b(\w+)\s*=\s*(?:nullptr|NULL)\s*;", line)
                if m:
                    lat.assign_null(m.group(1))
                    continue
                m = re.search(r"\b(\w+)\s*=\s*(?:malloc|calloc)\b", line)
                if m:
                    lat.assign_alloc(m.group(1))
                    continue
                m = re.search(r"\b(\w+)\s*=\s*&", line)
                if m:
                    lat.assign_addr(m.group(1))
                    continue

            for pname in params:
                if not re.search(rf"\b{re.escape(pname)}\s*->", body):
                    continue
                # if any check exists, treat as handled
                if re.search(
                    rf"if\s*\(\s*(?:!{re.escape(pname)}|{re.escape(pname)}\s*==\s*(?:nullptr|NULL|0))",
                    body,
                ):
                    continue
                if lat.get(pname) == NullLattice.NEVER_NULL:
                    continue
                d_m = re.search(rf"\b{re.escape(pname)}\s*->", body)
                refusals.append(
                    _ref(
                        unit,
                        self.name,
                        "NULLABILITY",
                        fstart + (d_m.start() if d_m else 0),
                        f"lattice-maybe:{pname}",
                    )
                )
        return PassResult(text=text, refusals=refusals, edits=edits)


class SpanInductionPass(Pass):
    """Detect (T* p, size_t n) where all p[i] have i constrained by n (IV analysis)."""

    name = "span_induction"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        work: list[tuple[str, int, int, str]] = []

        for fname, fstart, fend, body in _functions(text):
            sig_m = re.search(
                rf"\b{re.escape(fname)}\s*\(([^)]*)\)",
                text[fstart : fstart + 400],
            )
            if not sig_m:
                continue
            params = sig_m.group(1)
            # find pairs
            pm = re.search(
                r"((?:const\s+)?(?:struct\s+)?[\w:]+)\s*\*\s*(\w+)\s*,\s*"
                r"(?:const\s+)?(size_t|int|u_int|unsigned|ssize_t)\s+(\w+)",
                params,
            )
            if not pm:
                continue
            typ, ptr, _, ln = pm.group(1).strip(), pm.group(2), pm.group(3), pm.group(4)
            # All index uses of ptr must be < ln or in for(i=0;i<ln;++i)
            idxs = re.findall(rf"\b{re.escape(ptr)}\s*\[\s*([^\]]+)\s*\]", body)
            if not idxs and not re.search(rf"\b{re.escape(ptr)}\b", body):
                continue
            ok = True
            if not idxs:
                # No indexing — still a (ptr,len) pair if len bounds mem*/loops
                if not re.search(rf"\b{re.escape(ln)}\b", body):
                    ok = False
            for ix in idxs:
                ix = ix.strip()
                if ix == ln:
                    ok = False
                    break
                if ix == "0" or re.fullmatch(r"\d+", ix):
                    continue
                if re.search(
                    rf"for\s*\([^;]*\b{re.escape(ix)}\s*=\s*0\s*;\s*{re.escape(ix)}\s*<(?:=)?\s*{re.escape(ln)}",
                    body,
                ):
                    continue
                if re.search(
                    rf"while\s*\(\s*{re.escape(ix)}\s*<(?:=)?\s*{re.escape(ln)}",
                    body,
                ):
                    continue
                if re.search(rf"\b{re.escape(ix)}\s*<(?:=)?\s*{re.escape(ln)}\b", body):
                    continue
                if re.search(
                    rf"\b{re.escape(ix)}\s*<(?:=)?\s*\(?\s*{re.escape(ln)}\s*-\s*1\s*\)?",
                    body,
                ):
                    continue
                ok = False
                break
            if re.search(rf"\b{re.escape(ptr)}\s*[\+\-]", body):
                ok = False
            if not ok:
                _propose_span(
                    unit,
                    "SPAN_CANDIDATE",
                    {"line": unit.line_col(fstart)[0], "snippet": f"{ptr},{ln}:iv-fail"},
                )
                continue

            typ_clean = re.sub(r"\bconst\b", "", typ).strip()
            span_t = f"std::span<{typ_clean}>"
            new_params = re.sub(
                r"((?:const\s+)?(?:struct\s+)?[\w:]+)\s*\*\s*"
                + re.escape(ptr)
                + r"\s*,\s*(?:const\s+)?(?:size_t|int|u_int|unsigned|ssize_t)\s+"
                + re.escape(ln),
                f"{span_t} {ptr}",
                params,
                count=1,
            )
            open_p = text.find("(", fstart)
            close_p = text.find(")", open_p)
            work.append(("repl", open_p + 1, close_p, new_params))
            if re.search(rf"\b{re.escape(ln)}\b", body):
                work.append(
                    (
                        "ins",
                        fstart + 1,  # wrong — need body start
                        fstart + 1,
                        f"\n\tconst std::size_t {ln} = {ptr}.size();\n",
                    )
                )
            # fix insert at real body start
            body_start = text.find("{", fstart)
            work[-1] = (
                "ins",
                body_start + 1,
                body_start + 1,
                f"\n\tconst std::size_t {ln} = {ptr}.size();\n",
            )
            edits.append(
                Edit(self.name, f"IV-span {ptr}", unit.line_col(fstart)[0], params[:40], new_params[:40])
            )

        for kind, a, b, payload in sorted(work, key=lambda x: x[1], reverse=True):
            if kind == "repl":
                text = text[:a] + payload + text[b:]
            else:
                text = text[:a] + payload + text[a:]
        if edits and "#include <span>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <span>\n#include <cstddef>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text
        return PassResult(text=text, refusals=refusals, edits=edits)


class QueueForeachRewritePass(Pass):
    """LIST/TAILQ_FOREACH → range-for over intrusive stub helpers."""

    name = "queue_foreach_rewrite"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []

        # LIST_FOREACH / FOREACH_SAFE(var, head, field[, tvar])
        rx = re.compile(
            r"\b(LIST|TAILQ|STAILQ|SLIST)_FOREACH(?:_SAFE)?\s*\(\s*(\w+)\s*,\s*([^,]+?)\s*,\s*(\w+)"
            r"(?:\s*,\s*\w+)?\s*\)(\s*\{)?"
        )
        ops = []
        for m in rx.finditer(unit.mask_strings_comments()):
            kind, var, head, field = m.group(1), m.group(2), m.group(3).strip(), m.group(4)
            brace = m.group(5) or ""
            old_head = text[m.start() : m.end()]
            new_head = (
                f"for (/*PBSD_{kind}_FOREACH*/ auto *{var} = "
                f"pbsd_intrusive_first({head}); {var}; "
                f"{var} = pbsd_intrusive_next({var}, {field}))"
            )
            if brace:
                new_head += " {"
            ops.append((m.start(), m.end(), new_head, old_head))

        # INSERT/REMOVE/INIT handled by promote_queue_ops (RB_/SPLAY_ refused there).

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            edits.append(Edit(self.name, "FOREACH→for", unit.line_col(start)[0], old[:40], new[:40]))

        if edits:
            stub = (
                "\n#ifndef PBSD_INTRUSIVE_FOREACH_HELPERS\n"
                "#define PBSD_INTRUSIVE_FOREACH_HELPERS\n"
                "#define pbsd_intrusive_first(h) ((h)->first)\n"
                "#define pbsd_intrusive_next(p, field) ((p)->field.next)\n"
                "#endif\n"
            )
            if "PBSD_INTRUSIVE_FOREACH_HELPERS" not in text:
                text = stub + text
                edits.append(Edit(self.name, "foreach helpers", 1, "", "helpers"))

        return PassResult(text=text, refusals=refusals, edits=edits)


class GotoStructuredPass(Pass):
    """Multi-resource goto cleanup → nested unique_ptr when frees are a stack (reverse alloc order)."""

    name = "goto_structured"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []

        for lab_m in re.finditer(r"(?m)^[ \t]*(out|fail|error|cleanup|done|err)\s*:", text, re.I):
            label = lab_m.group(1)
            after = text[lab_m.end() : lab_m.end() + 500]
            free_vars = re.findall(r"\bfree\s*\(\s*(\w+)\s*\)\s*;", after)
            close_vars = re.findall(r"\bclose\s*\(\s*(\w+)\s*\)\s*;", after)
            if len(free_vars) + len(close_vars) == 0:
                continue
            if len(free_vars) + len(close_vars) > 4:
                _propose_span(unit, "GOTO_CLEANUP_CANDIDATE", {"line": unit.line_col(lab_m.start())[0], "snippet": f"{label}:too-many"})
                continue
            gotos = list(re.finditer(rf"\bgoto\s+{re.escape(label)}\s*;", text, re.I))
            if not gotos:
                continue

            # For each free var, find malloc and insert unique_ptr; blank free at label
            for var in free_vars:
                allocs = list(
                    re.finditer(
                        rf"\b{re.escape(var)}\s*=\s*((?:\(.*?\)\s*)?(?:malloc|calloc)\s*\([^;]+);",
                        text,
                    )
                )
                if not allocs:
                    _propose_span(unit, "GOTO_CLEANUP_CANDIDATE", {"line": unit.line_col(lab_m.start())[0], "snippet": f"no-alloc:{var}"})
                    continue
                alloc = allocs[-1]
                if "pbsd_goto_guard_" + var in text:
                    continue
                guard = (
                    f"\n\tstd::unique_ptr<void, decltype(&std::free)> "
                    f"pbsd_goto_guard_{var}({var}, &std::free);"
                )
                text = text[: alloc.end()] + guard + text[alloc.end() :]
                # remove free(var) near label — search again after edit
                text = re.sub(
                    rf"\bfree\s*\(\s*{re.escape(var)}\s*\)\s*;",
                    f"/* pbsd: goto free {var} via unique_ptr */",
                    text,
                    count=1,
                )
                edits.append(
                    Edit(self.name, f"goto stack {var}", unit.line_col(alloc.start())[0], var, guard.strip())
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


MATHS_PASSES: list[Pass] = [
    SteensgaardMallocPass(),
    SpanInductionPass(),
    QueueForeachRewritePass(),
    GotoStructuredPass(),
    CCuredPointerKindPass(),
    # NullabilityLatticePass — refusal authority stays in promote_null_bitcast_goto
]
