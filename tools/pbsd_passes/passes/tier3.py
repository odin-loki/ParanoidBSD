# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tier 3 — interprocedural inference (census + proposals; conservative rewrites)."""
from __future__ import annotations

import json
import re
from collections import defaultdict
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

def _propose_lock(unit, kind: str, payload: dict) -> None:
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


class PointerKindCensusPass(Pass):
    name = "pointer_kind_census"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        refusals: list[Refusal] = []
        kinds: dict[str, str] = {}
        # Heuristic lattice: arithmetic → SEQ, cast involving void*/int → WILD, else SAFE
        for m in re.finditer(r"\b([\w\s\*]+)\*(\s*)([A-Za-z_]\w*)\b", text):
            name = m.group(3)
            kinds.setdefault(name, "SAFE")
        for name in list(kinds):
            if re.search(rf"\b{re.escape(name)}\s*[\+\-\[]", text):
                kinds[name] = "SEQ"
            if re.search(rf"\(\s*[\w\s\*]+\*\s*\)\s*{re.escape(name)}\b|\b{re.escape(name)}\s*=\s*\([^)]*void\s*\*", text):
                kinds[name] = "WILD"
        unit.meta["pointer_kinds"] = kinds
        for name, kind in kinds.items():
            if kind != "WILD":
                continue
            m = re.search(rf"\b{re.escape(name)}\b", text)
            if m:
                _propose_lock(
                    unit,
                    "POINTER_KIND",
                    {"line": unit.line_col(m.start())[0], "snippet": f"{name}:{kind}"},
                )
        OUT.mkdir(parents=True, exist_ok=True)
        side = OUT / "pointer_kinds.jsonl"
        with side.open("a", encoding="utf-8") as f:
            f.write(json.dumps({"file": unit.path, "kinds": kinds}) + "\n")
        return PassResult(text=text, refusals=refusals, edits=[])


class PtrLenSpanLogPass(Pass):
    """Superseded by promote_span_signature / span_induction."""

    name = "ptr_len_span_log"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        return PassResult.unchanged(unit.text)


class QueueHTemplatesPass(Pass):
    name = "queue_h_templates"
    tier = 3

    # Catch-all for leftover tree macros; list/tailq ops handled earlier.
    MACROS = (
        "RB_ENTRY",
        "RB_HEAD",
        "RB_INSERT",
        "RB_REMOVE",
        "RB_FOREACH",
        "SPLAY_ENTRY",
        "SPLAY_HEAD",
        "SPLAY_INSERT",
        "SPLAY_REMOVE",
        "SPLAY_FOREACH",
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        refusals: list[Refusal] = []
        edits: list[Edit] = []
        # Do not expand macros; annotate and log.
        comment = "\n/* PBSD: queue.h/tree.h site — map to pbsd::intrusive::* templates */\n"
        first = None
        for mac in self.MACROS:
            for m in re.finditer(rf"\b{mac}\b", text):
                _propose_lock(unit, "QUEUE_H_SITE", {"line": unit.line_col(m.start())[0], "snippet": mac})
                if first is None:
                    first = m.start()
        if first is not None and "pbsd::intrusive" not in text:
            # Insert annotation near first include of sys/queue.h if present
            q = re.search(r'#include\s*[<"]sys/queue\.h[>"]', text)
            if q:
                pos = q.end()
                text = text[:pos] + comment + text[pos:]
                edits.append(
                    Edit(self.name, "queue.h annotation", unit.line_col(q.start())[0], "", comment.strip())
                )
            else:
                text = comment + text
                edits.append(Edit(self.name, "queue.h annotation", 1, "", comment.strip()))
        return PassResult(text=text, refusals=refusals, edits=edits)


class NullabilityLogPass(Pass):
    name = "nullability_log"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        refusals = []
        for m in re.finditer(r"\bif\s*\(\s*(\w+)\s*==\s*(?:nullptr|NULL|0)\s*\)", unit.text):
            refusals.append(_ref(unit, self.name, "NULLABILITY", m.start(), f"maybe-null:{m.group(1)}"))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class BitCastCandidatesPass(Pass):
    name = "bit_cast_candidates"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        refusals = []
        for m in re.finditer(r"\bmemcpy\s*\(", unit.mask_strings_comments()):
            refusals.append(_ref(unit, self.name, "BIT_CAST_CANDIDATE", m.start(), "memcpy"))
        for m in re.finditer(r"\*\s*\(\s*[\w\s\*]+\s*\*\)\s*&", unit.text):
            refusals.append(_ref(unit, self.name, "BIT_CAST_CANDIDATE", m.start(), m.group(0)[:40]))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class PurityLogPass(Pass):
    name = "purity_log"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        refusals = []
        # Functions with no obvious global write and no calls → pure candidate
        for m in re.finditer(
            r"(?:^|\n)([\w\s\*]+)\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            unit.text,
        ):
            name = m.group(2)
            # skip main
            if name == "main":
                continue
            # find body end roughly
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
            if re.search(r"\b\w+\s*\(", body[1:]):  # has calls
                continue
            if re.search(r"\b[a-z_][a-z0-9_]*\s*=", body) and not re.search(
                r"\b(?:int|char|size_t|long|unsigned)\s+\w+\s*=", body
            ):
                # might write globals — skip
                continue
            refusals.append(_ref(unit, self.name, "PURITY", m.start(), f"pure-candidate:{name}"))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class GlobalClusterPass(Pass):
    name = "global_cluster_proposal"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        # Bipartite-ish: functions × file-scope identifiers written
        globals_defs = [
            m.group(1)
            for m in re.finditer(
                r"(?m)^(static\s+)?(?:const\s+)?(?:unsigned\s+|int\s+|long\s+|char\s+|size_t\s+|struct\s+\w+\s+)([A-Za-z_]\w*)\s*(=|;|\[)",
                unit.text,
            )
        ]
        # fix regex groups
        gnames = []
        for m in re.finditer(
            r"(?m)^(?:static\s+)?(?:const\s+)?(?:unsigned\s+|signed\s+|long\s+|short\s+|int\s+|char\s+|size_t\s+|struct\s+\w+\s+\*?\s*)([A-Za-z_]\w*)\s*(?:=|;|\[)",
            unit.text,
        ):
            gnames.append(m.group(1))
        fn_to_g: dict[str, set[str]] = defaultdict(set)
        for m in re.finditer(
            r"(?:^|\n)([\w\s\*]+)\b([A-Za-z_]\w*)\s*\(([^)]*)\)\s*\{",
            unit.text,
        ):
            fname = m.group(2)
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
            for g in gnames:
                if re.search(rf"\b{re.escape(g)}\b", body):
                    fn_to_g[fname].add(g)
        OUT.mkdir(parents=True, exist_ok=True)
        path = OUT / "global_clusters.jsonl"
        with path.open("a", encoding="utf-8") as f:
            f.write(
                json.dumps(
                    {
                        "file": unit.path,
                        "globals": gnames,
                        "fn_globals": {k: sorted(v) for k, v in fn_to_g.items()},
                    }
                )
                + "\n"
            )
        refusals = []
        if gnames:
            refusals.append(_ref(unit, self.name, "GLOBAL_CLUSTER", 0, f"{len(gnames)} globals"))
        return PassResult(text=unit.text, refusals=refusals, edits=[])


class LockDisciplinePass(Pass):
    name = "lock_discipline_propose"
    tier = 3

    def apply(self, unit: TranslationUnit) -> PassResult:
        refusals = []
        for m in re.finditer(
            r"\b(mtx_lock|mtx_unlock|rw_rlock|rw_wlock|sx_xlock|sx_slock|sx_xunlock|sx_sunlock)\s*\(\s*&?(\w+)",
            unit.text,
        ):
            _propose_lock(
                unit,
                "LOCK_DISCIPLINE",
                {"line": unit.line_col(m.start())[0], "snippet": f"{m.group(1)}:{m.group(2)}"},
            )
        return PassResult(text=unit.text, refusals=refusals, edits=[])


TIER3_PASSES: list[Pass] = [
    LockDisciplinePass(),
    # error_code_expected_log → promote_null_bitcast_goto.ErrorCodeExpectedAnnotatePass
]
