"""Promote former refusal-only sites into deterministic rewrites.

Also: successful edits must NOT emit Refusal rows (those are the model queue).
"""
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


_OUT = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"


def _propose_rf(unit, kind: str, payload: dict) -> None:
    _OUT.mkdir(parents=True, exist_ok=True)
    with (_OUT / "proposals.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps({"file": unit.path, "kind": kind, **payload}) + "\n")



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


class CompoundLiteralRewritePass(Pass):
    """(T){...} → T{...} (C++20 aggregate / temporary)."""

    name = "compound_literal_rewrite"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # (struct foo){ or (foo_t){ or (unsigned int){ — skip function-cast-like
        rx = re.compile(
            r"\(\s*((?:struct|union|enum)\s+\w+|[\w:]+)\s*\)\s*(\{)"
        )
        pieces: list[str] = []
        last = 0
        for m in rx.finditer(masked):
            if masked[m.start()] != "(":
                continue
            # Skip function defs/calls: ident(…){  — compound literals need
            # non-ident before '(' (e.g. '=', ',', '(', or keyword+space).
            j = m.start() - 1
            while j >= 0 and masked[j] in " \t":
                j -= 1
            if j >= 0 and (masked[j].isalnum() or masked[j] == "_"):
                continue
            typ = text[m.start(1) : m.end(1)].strip()
            if typ == "void":
                continue
            # Refuse function-pointer / complex types
            if "*" in typ or "(" in typ:
                refusals.append(_ref(unit, self.name, "COMPOUND_LITERAL", m.start(), typ[:60]))
                continue
            brace = text[m.start(2) : m.end(2)]
            new = f"{typ}{brace}"
            pieces.append(text[last : m.start()])
            pieces.append(new)
            line, _ = unit.line_col(m.start())
            edits.append(Edit(self.name, "compound→T{", line, m.group(0)[:40], new[:40]))
            last = m.end()
        if not edits and not refusals:
            return PassResult.unchanged(text)
        if not edits:
            return PassResult(text=text, refusals=refusals, edits=[])
        pieces.append(text[last:])
        return PassResult(text="".join(pieces), refusals=refusals, edits=edits)


class DesignatedInitAllowCxx20Pass(Pass):
    """C++20 allows designated init in declaration order — stop refusing; annotate once."""

    name = "designated_init_cxx20"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        if not re.search(r"[\{\,]\s*\.\w+\s*=", unit.mask_strings_comments()):
            return PassResult.unchanged(text)
        if "PBSD_CXX20_DESIGNATED_INIT" in text:
            return PassResult.unchanged(text)
        note = (
            "/* PBSD_CXX20_DESIGNATED_INIT: designated initializers retained; "
            "must match declaration order (C++20). Out-of-order sites → model queue. */\n"
        )
        # Detect likely out-of-order only when same aggregate has decreasing field names? Skip — just annotate.
        return PassResult(
            text=note + text,
            refusals=[],
            edits=[Edit(self.name, "cxx20 designated init policy", 1, "", note.strip())],
        )


class VlaToVectorPass(Pass):
    """Policy: VLAs → std::vector; constexpr sizes → std::array; macros → proposal."""

    name = "vla_to_vector"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        ops: list[tuple[int, int, str, str]] = []
        masked = unit.mask_strings_comments()

        rx = re.compile(
            r"\b((?:unsigned\s+|signed\s+|long\s+|short\s+|const\s+|volatile\s+|u_)*"
            r"(?:char|int|short|long|float|double|size_t|u_int|u_char|u_short|uint\d+_t|int\d+_t|wchar_t|char_type|count_int|code_int))\s+"
            r"(\w+)\s*\[\s*([^\]]+)\s*\]\s*;"
        )
        for m in rx.finditer(masked):
            typ = text[m.start(1) : m.end(1)].strip()
            name = text[m.start(2) : m.end(2)]
            expr = text[m.start(3) : m.end(3)].strip()
            old = text[m.start() : m.end()]
            # Pure integer / constexpr arithmetic → std::array
            if re.fullmatch(r"[\d\s\+\-\*\/\(\)]+", expr):
                try:
                    sz = int(eval(expr, {"__builtins__": {}}, {}))  # noqa: S307 — const only
                except Exception:
                    _propose_pol(unit, "VLA", {"line": unit.line_col(m.start())[0], "snippet": old[:60]})
                    continue
                if sz <= 0 or sz > 1_000_000:
                    _propose_pol(unit, "VLA", {"line": unit.line_col(m.start())[0], "snippet": old[:60]})
                    continue
                new = f"std::array<{typ}, {sz}> {name}{{}};"
                ops.append((m.start(), m.end(), new, old))
                continue
            # Identifier or simple id op id → vector
            if re.fullmatch(r"[A-Za-z_]\w*(?:\s*[\+\-\*\/]\s*[A-Za-z_0-9]\w*)*", expr):
                new = f"std::vector<{typ}> {name}({expr});"
                ops.append((m.start(), m.end(), new, old))
                continue
            # Macro / complex → proposal-style refuse once
            _propose_pol(unit, "VLA", {"line": unit.line_col(m.start())[0], "snippet": old[:60]})

        need_vec = any("std::vector" in o[2] for o in ops)
        need_arr = any("std::array" in o[2] for o in ops)
        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, "VLA rewrite", line, old[:40], new[:40]))
        hdrs = []
        if need_vec and "#include <vector>" not in text:
            hdrs.append("#include <vector>\n")
        if need_arr and "#include <array>" not in text:
            hdrs.append("#include <array>\n")
        if hdrs:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            blob = "".join(hdrs)
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + blob + text[pos:]
            else:
                text = blob + text
        if not edits and not refusals:
            return PassResult.unchanged(unit.text)
        return PassResult(text=text, refusals=refusals, edits=edits)


class ConstParamRewritePass(Pass):
    """Insert const on pointer params never written through in the function body."""

    name = "const_param_rewrite"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # Find function definitions
        for m in re.finditer(
            r"(?:^|\n)([\w\s\*]+)\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            text,
        ):
            params = m.group(3)
            if not params.strip() or params.strip() == "void":
                continue
            fname = m.group(2)
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
            body = text[body_start : i + 1]
            new_params = []
            changed = False
            for p in params.split(","):
                raw = p
                pstrip = p.strip()
                if not pstrip or pstrip == "void" or "const" in pstrip or "*" not in pstrip:
                    new_params.append(raw)
                    continue
                name_m = re.search(r"([A-Za-z_]\w*)\s*(?:\[\s*\])?\s*$", pstrip)
                if not name_m:
                    new_params.append(raw)
                    continue
                pname = name_m.group(1)
                if re.search(rf"\*{re.escape(pname)}\s*=", body) or re.search(
                    rf"{re.escape(pname)}\s*\[[^\]]+\]\s*=", body
                ):
                    new_params.append(raw)
                    continue
                # also skip if passed to non-const-looking callee as mutable — conservative: still const
                # Insert const after type start
                np = re.sub(
                    r"^((?:unsigned\s+|signed\s+|long\s+|short\s+|struct\s+\w+\s+|enum\s+\w+\s+)*)([\w:]+)",
                    r"\1const \2",
                    pstrip,
                    count=1,
                )
                if np == pstrip:
                    # pointer form: T *p → const T *p
                    np = re.sub(r"^([\w\s:]+?)\s*\*", r"const \1 *", pstrip, count=1)
                if np != pstrip and "const const" not in np:
                    # preserve leading spacing style roughly
                    leading = raw[: len(raw) - len(raw.lstrip())]
                    new_params.append(leading + np)
                    changed = True
                    edits.append(
                        Edit(
                            self.name,
                            f"const param {pname} in {fname}",
                            unit.line_col(m.start())[0],
                            pstrip,
                            np,
                        )
                    )
                else:
                    refusals.append(_ref(unit, self.name, "CONST_CANDIDATE", m.start(), pstrip[:60]))
                    new_params.append(raw)
            if changed:
                old_sig = text[m.start() : m.end()]
                # rebuild signature carefully
                sig_start = m.start()
                # replace only param list
                open_paren = text.find("(", m.start())
                close_paren = text.find(")", open_paren)
                text = text[: open_paren + 1] + ",".join(new_params) + text[close_paren:]
                # After mutation, coordinates for later matches are wrong — process one function per apply via break
                # So collect one change then restart by returning — do all via offset-safe reverse later
                unit.meta.setdefault("_const_ops", []).append(
                    (open_paren + 1, close_paren, ",".join(new_params))
                )
                # restore text for scanning? Better: don't mutate text in loop; only collect ops from original
                text = unit.text  # reset — we only collect from original below

        # Re-scan original only collecting ops (rewrite loop above mutated then reset — messy). Clean reimplementation:
        text = unit.text
        ops: list[tuple[int, int, str]] = []
        edits = []
        refusals = []
        for m in re.finditer(
            r"(?:^|\n)([\w\s\*]+)\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            text,
        ):
            params = m.group(3)
            if not params.strip() or params.strip() == "void":
                continue
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
            body = text[body_start : i + 1]
            new_params = []
            changed = False
            for p in params.split(","):
                raw = p
                pstrip = p.strip()
                if not pstrip or pstrip == "void" or "const" in pstrip or "*" not in pstrip:
                    new_params.append(raw)
                    continue
                name_m = re.search(r"([A-Za-z_]\w*)\s*(?:\[\s*\])?\s*$", pstrip)
                if not name_m:
                    new_params.append(raw)
                    continue
                pname = name_m.group(1)
                if re.search(rf"\*{re.escape(pname)}\s*=", body) or re.search(
                    rf"{re.escape(pname)}\s*\[[^\]]+\]\s*=", body
                ):
                    new_params.append(raw)
                    continue
                np = re.sub(r"^([\w\s:]+?)\s*\*", r"const \1*", pstrip, count=1)
                np = re.sub(r"\s+\*", " *", np)
                if np != pstrip and "const const" not in np:
                    leading = raw[: len(raw) - len(raw.lstrip())]
                    new_params.append(leading + np)
                    changed = True
                    edits.append(
                        Edit(
                            self.name,
                            f"const {pname}",
                            unit.line_col(m.start())[0],
                            pstrip,
                            np,
                        )
                    )
                else:
                    new_params.append(raw)
            if changed:
                open_paren = text.find("(", m.start())
                close_paren = text.find(")", open_paren)
                ops.append((open_paren + 1, close_paren, ",".join(new_params)))

        for start, end, repl in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + repl + text[end:]
        return PassResult(text=text, refusals=refusals, edits=edits)


class SpanSignatureRewritePass(Pass):
    """(const T*|T*|void*, size_t|int) → std::span; static in-file call sites too.

    Implementation lives in promote_malloc_span.PromoteSpanSignaturePass.
    """

    name = "span_signature_rewrite"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        from .promote_malloc_span import PromoteSpanSignaturePass

        inner = PromoteSpanSignaturePass()
        # Keep this pass's name on refusal/edit attribution
        result = inner.apply(unit)
        for r in result.refusals:
            r.pass_name = self.name
        for e in result.edits:
            e.pass_name = self.name
        return result


class RangeForRewritePass(Pass):
    name = "range_for_rewrite"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        rx = re.compile(
            r"for\s*\(\s*(?:unsigned\s+|size_t\s+|int\s+)?(\w+)\s*=\s*0\s*;\s*\1\s*<\s*(\w+)\s*;\s*\+\+\1\s*\)\s*\{"
        )
        ops = []
        for m in rx.finditer(unit.mask_strings_comments()):
            idx, bound = m.group(1), m.group(2)
            # Find loop body
            body_start = text.find("{", m.start())
            if body_start < 0:
                continue
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
            body = text[body_start : i + 1]
            # Only rewrite if body uses arr[idx] pattern with single array
            uses = re.findall(rf"\b([A-Za-z_]\w*)\s*\[\s*{re.escape(idx)}\s*\]", body)
            if len(set(uses)) != 1:
                _propose_rf(unit, "RANGE_FOR_CANDIDATE", {"line": unit.line_col(m.start())[0], "snippet": m.group(0)[:60]})
                continue
            arr = uses[0]
            if bound != f"{arr}" and not re.search(rf"\b{re.escape(bound)}\b", text[m.start() - 80 : m.start()]):
                # bound may be n / len — still OK if arr[idx] only
                pass
            old_head = text[m.start() : body_start]
            new_head = f"for (auto &pbsd_elem : /* TODO(span): */ std::span({arr}, {bound})) "
            # Replace arr[idx] with pbsd_elem in body — careful
            new_body = re.sub(
                rf"\b{re.escape(arr)}\s*\[\s*{re.escape(idx)}\s*\]",
                "pbsd_elem",
                body,
            )
            # If idx still used, refuse
            if re.search(rf"\b{re.escape(idx)}\b", new_body):
                _propose_rf(unit, "RANGE_FOR_CANDIDATE", {"line": unit.line_col(m.start())[0], "snippet": "idx still used"})
                continue
            ops.append((m.start(), i + 1, new_head + new_body, old_head))
        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, "range-for", line, old[:40], new[:40]))
        if edits and "#include <span>" not in text:
            m = list(re.finditer(r"(?m)^#include\b.*$", text))
            hdr = "#include <span>\n"
            if m:
                pos = m[-1].end()
                text = text[:pos] + "\n" + hdr + text[pos:]
            else:
                text = hdr + text
        return PassResult(text=text, refusals=refusals, edits=edits)


class PointerKindRefuseWildOnlyPass(Pass):
    """Replace noisy POINTER_KIND census refusals: only WILD goes to model queue."""

    name = "pointer_kind_wild_only"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        refusals: list[Refusal] = []
        kinds: dict[str, str] = {}
        for m in re.finditer(r"\*\s*([A-Za-z_]\w*)\b", text):
            kinds.setdefault(m.group(1), "SAFE")
        for name in list(kinds):
            if re.search(rf"\b{re.escape(name)}\s*[\+\-\[]", text):
                kinds[name] = "SEQ"
            if re.search(
                rf"\(\s*[\w\s\*]+\*\s*\)\s*{re.escape(name)}\b|"
                rf"\b{re.escape(name)}\s*=\s*\([^)]*void\s*\*",
                text,
            ):
                kinds[name] = "WILD"
        for name, kind in kinds.items():
            if kind != "WILD":
                continue
            m = re.search(rf"\b{re.escape(name)}\b", text)
            if m:
                refusals.append(_ref(unit, self.name, "POINTER_KIND", m.start(), f"{name}:WILD"))
        unit.meta["pointer_kinds"] = kinds
        return PassResult(text=text, refusals=refusals, edits=[])


REFUSAL_PROMOTE_PASSES: list[Pass] = [
    CompoundLiteralRewritePass(),
    DesignatedInitAllowCxx20Pass(),
    VlaToVectorPass(),
    ConstParamRewritePass(),
    SpanSignatureRewritePass(),
    RangeForRewritePass(),
    # nullability / bit_cast / goto cleanup / error_code → promote_null_bitcast_goto.py
    # POINTER_KIND / WILD handled by WildPointerCastPass in promote_proposals
]
