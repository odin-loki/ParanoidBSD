"""Demote proposal-only / false-positive refusals; strengthen queue.h + WILD casts.

Proposal codes go to docs/migration/clang_port/proposals.jsonl — NOT the model refusal queue.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "docs" / "migration" / "clang_port"
CXX_KEYWORDS = frozenset(
    {
        "if",
        "for",
        "while",
        "switch",
        "return",
        "sizeof",
        "typeof",
        "else",
        "do",
        "case",
        "default",
        "goto",
        "break",
        "continue",
        "static",
        "extern",
        "inline",
        "const",
        "volatile",
        "struct",
        "union",
        "enum",
        "typedef",
        "void",
        "int",
        "char",
        "long",
        "short",
        "unsigned",
        "signed",
        "float",
        "double",
        "bool",
        "true",
        "false",
        "nullptr",
        "main",
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


def _propose(unit: TranslationUnit, kind: str, payload: dict) -> None:
    from ..proposals import propose

    propose(unit.path, kind, payload)


class PurityProposeOnlyPass(Pass):
    """Replace noisy PURITY refusals — only real function defs, proposals only."""

    name = "purity_propose_only"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        for m in re.finditer(
            r"(?:^|\n)((?:static\s+|inline\s+|const\s+)*)([\w\s\*]+)\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            unit.text,
        ):
            name = m.group(3)
            if name in CXX_KEYWORDS or name.startswith("pbsd_"):
                continue
            start = m.end() - 1
            depth = 0
            i = start
            while i < len(unit.text):
                if unit.text[i] == "{":
                    depth += 1
                elif unit.text[i] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            body = unit.text[start : i + 1]
            # has calls? skip first `{`
            if re.search(r"\b[A-Za-z_]\w*\s*\(", body[1:]):
                continue
            if re.search(r"\b(errno|stdout|stderr)\b", body):
                continue
            _propose(unit, "PURITY", {"function": name, "line": unit.line_col(m.start())[0]})
        return PassResult.unchanged(unit.text)


class GlobalClusterProposeOnlyPass(Pass):
    name = "global_cluster_propose_only"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        gnames = []
        for m in re.finditer(
            r"(?m)^(?:static\s+)?(?:const\s+)?(?:unsigned\s+|signed\s+|long\s+|short\s+|int\s+|char\s+|size_t\s+|bool\s+|struct\s+\w+\s+\*?\s*)([A-Za-z_]\w*)\s*(?:=|;|\[)",
            unit.text,
        ):
            if m.group(1) not in CXX_KEYWORDS:
                gnames.append(m.group(1))
        if gnames:
            _propose(unit, "GLOBAL_CLUSTER", {"globals": gnames[:40], "count": len(gnames)})
        return PassResult.unchanged(unit.text)


class DeadStoreProposeOnlyPass(Pass):
    name = "dead_store_propose_only"
    tier = 2

    def apply(self, unit: TranslationUnit) -> PassResult:
        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*=\s*[^;]+;\s*(?:/\*.*?\*/\s*)?\1\s*=",
            unit.text,
        ):
            _propose(
                unit,
                "DEAD_STORE",
                {"line": unit.line_col(m.start())[0], "snippet": m.group(0)[:80]},
            )
        return PassResult.unchanged(unit.text)


class EnumArithmeticStrictPass(Pass):
    """Only refuse actual enum arithmetic, not pointer stars / attributes."""

    name = "enum_arithmetic_strict"
    tier = 1

    def apply(self, unit: TranslationUnit) -> PassResult:
        # ++/--/+=/-= on vars declared `enum Tag name` — propose (design: enum class).
        for m in re.finditer(
            r"\b([A-Za-z_]\w*)\s*(\+\+|--|\+=|-=)\s*",
            unit.mask_strings_comments(),
        ):
            name = m.group(1)
            if name in CXX_KEYWORDS or name.startswith("__"):
                continue
            decl = re.search(rf"\benum\s+\w+\s+{re.escape(name)}\b", unit.text)
            if decl:
                _propose(
                    unit,
                    "ENUM_ARITHMETIC",
                    {"line": unit.line_col(m.start())[0], "snippet": m.group(0)[:40]},
                )
        return PassResult.unchanged(unit.text)


class QueueHTemplateStubPass(Pass):
    """Emit real C++ intrusive-list stub include + map LIST_/TAILQ_ macros to comments with types."""

    name = "queue_h_template_stub"
    tier = 3

    MACROS = (
        "LIST_ENTRY",
        "LIST_HEAD",
        "TAILQ_ENTRY",
        "TAILQ_HEAD",
        "STAILQ_ENTRY",
        "STAILQ_HEAD",
        "SLIST_ENTRY",
        "SLIST_HEAD",
        "RB_ENTRY",
        "RB_HEAD",
        "SPLAY_ENTRY",
        "SPLAY_HEAD",
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        hits = []
        for mac in self.MACROS:
            for m in re.finditer(rf"\b{mac}\s*\(\s*(\w+)\s*\)", text):
                hits.append((mac, m.group(1), m.start()))
            for m in re.finditer(rf"\b{mac}\b", text):
                if not any(abs(m.start() - h[2]) < 3 for h in hits):
                    hits.append((mac, "?", m.start()))
        if not hits:
            return PassResult.unchanged(text)

        stub_dir = OUT / "stubs"
        stub_dir.mkdir(parents=True, exist_ok=True)
        stub = stub_dir / "pbsd_intrusive_queue.hpp"
        if not stub.exists():
            stub.write_text(
                """#pragma once
// PBSD staged intrusive container stubs — replace queue.h / tree.h macros.
#include <cstddef>
namespace pbsd::intrusive {
template<class T> struct list_node { T* next{nullptr}; T** prev{nullptr}; };
template<class T> struct list_head { T* first{nullptr}; };
template<class T> struct tailq_node { T* next{nullptr}; T** prev{nullptr}; };
template<class T> struct tailq_head { T* first{nullptr}; T** last{nullptr}; };
template<class T> struct slist_node { T* next{nullptr}; };
template<class T> struct slist_head { T* first{nullptr}; };
template<class T> struct stailq_node { T* next{nullptr}; };
template<class T> struct stailq_head { T* first{nullptr}; T** last{nullptr}; };
template<class T, class Key> struct rb_node { T* left{}; T* right{}; T* parent{}; };
template<class Head> inline void list_init(Head* h) { h->first = nullptr; }
template<class Head> inline void slist_init(Head* h) { h->first = nullptr; }
template<class Head> inline void tailq_init(Head* h) {
  h->first = nullptr; h->last = &h->first; }
template<class Head> inline void stailq_init(Head* h) {
  h->first = nullptr; h->last = &h->first; }
} // namespace pbsd::intrusive
""",
                encoding="utf-8",
            )

        edits: list[Edit] = []
        refusals: list[Refusal] = []
        if '#include "pbsd_intrusive_queue.hpp"' not in text and "pbsd_intrusive_queue.hpp" not in text:
            inc = (
                f'#include "{stub.as_posix()}" /* PBSD queue.h → intrusive stubs */\n'
            )
            # Prefer after sys/queue.h
            q = re.search(r'#include\s*[<"]sys/queue\.h[>"]', text)
            if q:
                text = text[: q.end()] + "\n" + inc + text[q.end() :]
            else:
                text = inc + text
            edits.append(Edit(self.name, "include intrusive stub", 1, "", inc.strip()))

        # Rewrite simple ENTRY/HEAD macros to stub types when arg present
        def repl_entry(m: re.Match) -> str:
            mac, typ = m.group(1), m.group(2)
            mapping = {
                "LIST_ENTRY": f"pbsd::intrusive::list_node<{typ}>",
                "TAILQ_ENTRY": f"pbsd::intrusive::tailq_node<{typ}>",
                "STAILQ_ENTRY": f"pbsd::intrusive::stailq_node<{typ}>",
                "SLIST_ENTRY": f"pbsd::intrusive::slist_node<{typ}>",
                "RB_ENTRY": f"pbsd::intrusive::rb_node<{typ}, void>",
                "LIST_HEAD": f"pbsd::intrusive::list_head<{typ}>",
                "TAILQ_HEAD": f"pbsd::intrusive::tailq_head<{typ}>",
                "STAILQ_HEAD": f"pbsd::intrusive::stailq_head<{typ}>",
                "SLIST_HEAD": f"pbsd::intrusive::slist_head<{typ}>",
                "RB_HEAD": f"pbsd::intrusive::rb_node<{typ}, void>",
            }
            return mapping.get(mac, m.group(0))

        new_text, n = re.subn(
            r"\b(LIST_ENTRY|TAILQ_ENTRY|STAILQ_ENTRY|SLIST_ENTRY|RB_ENTRY|"
            r"LIST_HEAD|TAILQ_HEAD|STAILQ_HEAD|SLIST_HEAD|RB_HEAD)\s*\(\s*(\w+)\s*\)",
            repl_entry,
            text,
        )
        if n:
            edits.append(Edit(self.name, f"macro→stub x{n}", 1, "queue macros", "intrusive stubs"))
            text = new_text
        else:
            # ENTRY/HEAD not rewritten — leave FOREACH/INSERT to dedicated passes.
            pass

        # Only tree macros (RB/SPLAY) remain on the model queue here.
        for m in re.finditer(
            r"\b(RB|SPLAY)_(FOREACH|INSERT|REMOVE|INIT)\w*\b",
            text,
        ):
            _propose(unit, "QUEUE_H_SITE", {"line": unit.line_col(m.start())[0], "snippet": m.group(0)})

        return PassResult(text=text, refusals=refusals, edits=edits)


class WildPointerCastPass(Pass):
    """For WILD pointers from void* assignment, insert static_cast on the assign."""

    name = "wild_pointer_cast"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        # T *p = (void*)... or p = (void*)...
        rx = re.compile(
            r"\b([A-Za-z_]\w*)\s*=\s*\(\s*void\s*\*\s*\)\s*([^;]+);"
        )
        ops = []
        for m in rx.finditer(unit.mask_strings_comments()):
            var = m.group(1)
            # find declared type of var
            decl = re.search(
                rf"((?:[\w:]+\s+)+\*)\s*{re.escape(var)}\b",
                text,
            )
            if not decl:
                _propose(unit, "POINTER_KIND", {"line": unit.line_col(m.start())[0], "snippet": f"{var}:WILD"})
                continue
            typ = decl.group(1).strip()
            rhs = text[m.start(2) : m.end(2)].strip()
            old = text[m.start() : m.end()]
            new = f"{var} = static_cast<{typ}>({rhs});"
            ops.append((m.start(), m.end(), new, old))
        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, "WILD static_cast", line, old[:60], new[:60]))
        # Leftover (void*) casts → proposals (not model-queue refusals).
        for m in re.finditer(r"\(\s*void\s*\*\s*\)", unit.mask_strings_comments()):
            if any(m.start() >= o[0] and m.start() < o[1] for o in ops):
                continue
            _propose(
                unit,
                "POINTER_KIND",
                {"line": unit.line_col(m.start())[0], "snippet": "void*:WILD"},
            )
        return PassResult(text=text, refusals=refusals, edits=edits)


PROPOSAL_PASSES: list[Pass] = [
    PurityProposeOnlyPass(),
    GlobalClusterProposeOnlyPass(),
    DeadStoreProposeOnlyPass(),
    EnumArithmeticStrictPass(),
    QueueHTemplateStubPass(),
    WildPointerCastPass(),
]
