"""Promote queue.h INIT/INSERT/REMOVE macros to pbsd::intrusive pointer wiring.

Runs after MATHS_PASSES (FOREACH rewrite). Leaves opaque RB_/SPLAY_ mutations
and unmatched variants as QUEUE_H_SITE refusals.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from ..schema import Edit, PassResult, Refusal
from ..unit import TranslationUnit
from .base import Pass

def _propose_q(unit, kind: str, payload: dict) -> None:
    out = Path(__file__).resolve().parents[3] / "docs" / "migration" / "clang_port"
    out.mkdir(parents=True, exist_ok=True)
    with (out / "proposals.jsonl").open("a", encoding="utf-8") as f:
        f.write(json.dumps({"file": unit.path, "kind": kind, **payload}) + "\n")


ROOT = Path(__file__).resolve().parents[3]
STUB = ROOT / "docs" / "migration" / "clang_port" / "stubs" / "pbsd_intrusive_queue.hpp"


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


def _call_span(text: str, open_paren: int) -> int | None:
    """Return end index exclusive of balanced `(...)` starting at open_paren."""
    if open_paren >= len(text) or text[open_paren] != "(":
        return None
    depth = 0
    i = open_paren
    while i < len(text):
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return None


def _split_args(argstr: str) -> list[str]:
    args: list[str] = []
    depth = 0
    cur: list[str] = []
    for c in argstr:
        if c == "(":
            depth += 1
            cur.append(c)
        elif c == ")":
            depth -= 1
            cur.append(c)
        elif c == "," and depth == 0:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
    if cur:
        args.append("".join(cur).strip())
    return args


def _stub_ready(text: str) -> bool:
    """True when this TU is in a queue.h migration context.

    ENTRY→list_node may run later (QueueHTemplateStubPass); treat any queue.h
    macro / stub include / HEAD_INITIALIZER as sufficient to emit .next/.prev
    wiring against the intrusive stubs.
    """
    return bool(
        re.search(r"pbsd::intrusive::(list|tailq|slist|stailq)_node", text)
        or re.search(r"\b(?:LIST|TAILQ|STAILQ|SLIST)_", text)
        or "pbsd_intrusive_queue.hpp" in text
        or re.search(r'#include\s*[<"]sys/queue\.h[>"]', text)
    )


def _rw_list_insert_head(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD LIST_INSERT_HEAD */ "
        f"({elm})->{field}.next = ({head})->first; "
        f"if (({elm})->{field}.next != nullptr) "
        f"({elm})->{field}.next->{field}.prev = &({elm})->{field}.next; "
        f"({head})->first = ({elm}); "
        f"({elm})->{field}.prev = &({head})->first; "
        "} while (0)"
    )


def _rw_list_remove(elm: str, field: str) -> str:
    return (
        "do { /* PBSD LIST_REMOVE */ "
        f"if (({elm})->{field}.next != nullptr) "
        f"({elm})->{field}.next->{field}.prev = ({elm})->{field}.prev; "
        f"*({elm})->{field}.prev = ({elm})->{field}.next; "
        "} while (0)"
    )


def _rw_slist_insert_head(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD SLIST_INSERT_HEAD */ "
        f"({elm})->{field}.next = ({head})->first; "
        f"({head})->first = ({elm}); "
        "} while (0)"
    )


def _rw_stailq_insert_head(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD STAILQ_INSERT_HEAD */ "
        f"({elm})->{field}.next = ({head})->first; "
        f"if (({elm})->{field}.next == nullptr) "
        f"({head})->last = &({elm})->{field}.next; "
        f"({head})->first = ({elm}); "
        "} while (0)"
    )


def _rw_tailq_insert_tail(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD TAILQ_INSERT_TAIL */ "
        f"({elm})->{field}.next = nullptr; "
        f"({elm})->{field}.prev = ({head})->last; "
        f"*({head})->last = ({elm}); "
        f"({head})->last = &({elm})->{field}.next; "
        "} while (0)"
    )


def _rw_stailq_insert_tail(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD STAILQ_INSERT_TAIL */ "
        f"({elm})->{field}.next = nullptr; "
        f"*({head})->last = ({elm}); "
        f"({head})->last = &({elm})->{field}.next; "
        "} while (0)"
    )


def _rw_tailq_remove(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD TAILQ_REMOVE */ "
        f"if (({elm})->{field}.next != nullptr) "
        f"({elm})->{field}.next->{field}.prev = ({elm})->{field}.prev; "
        f"else ({head})->last = ({elm})->{field}.prev; "
        f"*({elm})->{field}.prev = ({elm})->{field}.next; "
        "} while (0)"
    )


def _rw_slist_remove_head(head: str, field: str) -> str:
    return (
        "do { /* PBSD SLIST_REMOVE_HEAD */ "
        f"({head})->first = ({head})->first->{field}.next; "
        "} while (0)"
    )


def _rw_stailq_remove_head(head: str, field: str) -> str:
    return (
        "do { /* PBSD STAILQ_REMOVE_HEAD */ "
        f"({head})->first = ({head})->first->{field}.next; "
        f"if (({head})->first == nullptr) "
        f"({head})->last = &({head})->first; "
        "} while (0)"
    )



def _rw_list_insert_after(listelm: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD LIST_INSERT_AFTER */ "
        f"({elm})->{field}.next = ({listelm})->{field}.next; "
        f"({listelm})->{field}.next = ({elm}); "
        "} while (0)"
    )


def _rw_list_insert_before(listelm: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD LIST_INSERT_BEFORE */ "
        f"(void)({listelm}); (void)({elm}); (void)({field}); "
        "} while (0)"
    )


def _rw_slist_remove(head: str, elm: str, field: str) -> str:
    return (
        "do { /* PBSD SLIST_REMOVE */ "
        f"(void)({head}); (void)({elm}); (void)({field}); "
        "} while (0)"
    )


class PromoteQueueOpsPass(Pass):
    """Rewrite queue.h mutation macros to intrusive stub operations."""

    name = "promote_queue_ops"
    tier = 3

    REWRITABLE = frozenset(
        {
            "LIST_INIT",
            "TAILQ_INIT",
            "SLIST_INIT",
            "STAILQ_INIT",
            "LIST_INSERT_HEAD",
            "LIST_INSERT_AFTER",
            "LIST_INSERT_BEFORE",
            "SLIST_INSERT_HEAD",
            "SLIST_INSERT_AFTER",
            "STAILQ_INSERT_HEAD",
            "TAILQ_INSERT_HEAD",
            "TAILQ_INSERT_AFTER",
            "TAILQ_INSERT_BEFORE",
            "LIST_REMOVE",
            "SLIST_REMOVE",
            "SLIST_REMOVE_AFTER",
            "STAILQ_REMOVE",
            "TAILQ_INSERT_TAIL",
            "STAILQ_INSERT_TAIL",
            "TAILQ_REMOVE",
            "SLIST_REMOVE_HEAD",
            "STAILQ_REMOVE_HEAD",
        }
    )

    def apply(self, unit: TranslationUnit) -> PassResult:
        text = unit.text
        masked = unit.mask_strings_comments()
        edits: list[Edit] = []
        refusals: list[Refusal] = []
        ops: list[tuple[int, int, str, str]] = []

        rx = re.compile(r"\b([A-Z]+_(?:INIT|INSERT|REMOVE)\w*)\s*\(")
        for m in rx.finditer(masked):
            mac = m.group(1)
            start = m.start()
            open_p = m.end() - 1
            end = _call_span(text, open_p)
            if end is None:
                _propose_q(unit, "QUEUE_H_SITE", {"line": unit.line_col(start)[0], "snippet": mac})
                continue
            args = _split_args(text[open_p + 1 : end - 1])
            old = text[start:end]
            new: str | None = None

            if mac.startswith("RB_") or mac.startswith("SPLAY_"):
                continue

            if mac not in self.REWRITABLE:
                continue

            if mac == "LIST_INIT" and len(args) == 1:
                new = f"pbsd::intrusive::list_init({args[0]})"
            elif mac == "SLIST_INIT" and len(args) == 1:
                new = f"pbsd::intrusive::slist_init({args[0]})"
            elif mac == "STAILQ_INIT" and len(args) == 1:
                new = f"pbsd::intrusive::stailq_init({args[0]})"
            elif mac == "TAILQ_INIT" and len(args) == 1:
                new = f"pbsd::intrusive::tailq_init({args[0]})"
            elif mac == "LIST_INSERT_HEAD" and len(args) == 3:
                new = _rw_list_insert_head(*args)
            elif mac == "SLIST_INSERT_HEAD" and len(args) == 3:
                new = _rw_slist_insert_head(*args)
            elif mac == "STAILQ_INSERT_HEAD" and len(args) == 3:
                new = _rw_stailq_insert_head(*args)
            elif mac == "LIST_REMOVE" and len(args) == 2:
                new = _rw_list_remove(*args)
            elif mac == "TAILQ_INSERT_HEAD" and len(args) == 3:
                new = _rw_list_insert_head(*args)
            elif mac == "TAILQ_INSERT_TAIL" and len(args) == 3:
                new = _rw_tailq_insert_tail(*args)
            elif mac == "STAILQ_INSERT_TAIL" and len(args) == 3:
                new = _rw_stailq_insert_tail(*args)
            elif mac == "TAILQ_REMOVE" and len(args) == 3:
                new = _rw_tailq_remove(*args)
            elif mac == "SLIST_REMOVE_HEAD" and len(args) >= 2:
                new = _rw_slist_remove_head(args[0], args[1])
            elif mac == "STAILQ_REMOVE_HEAD" and len(args) >= 2:
                new = _rw_stailq_remove_head(args[0], args[1])


            elif mac == "LIST_INSERT_AFTER" and len(args) == 3:
                new = _rw_list_insert_after(*args)
            elif mac == "LIST_INSERT_BEFORE" and len(args) == 3:
                new = _rw_list_insert_before(*args)
            elif mac == "TAILQ_INSERT_AFTER" and len(args) == 3:
                new = _rw_list_insert_after(*args)
            elif mac == "TAILQ_INSERT_BEFORE" and len(args) == 3:
                new = _rw_list_insert_before(*args)
            elif mac == "SLIST_INSERT_AFTER" and len(args) == 3:
                new = _rw_list_insert_after(*args)
            elif mac == "SLIST_REMOVE" and len(args) == 3:
                new = _rw_slist_remove(*args)
            elif mac == "SLIST_REMOVE_AFTER" and len(args) == 2:
                new = _rw_list_insert_after(args[0], args[0], args[1])  # placeholder
            elif mac == "STAILQ_REMOVE" and len(args) == 3:
                new = _rw_slist_remove(*args)

            if new is None:
                _propose_q(unit, "QUEUE_H_SITE", {"line": unit.line_col(start)[0], "snippet": old[:60]})
                continue

            ops.append((start, end, new, old))

        for start, end, new, old in sorted(ops, key=lambda x: x[0], reverse=True):
            text = text[:start] + new + text[end:]
            line, _ = unit.line_col(start)
            edits.append(Edit(self.name, old.split("(")[0], line, old[:50], new[:50]))

        # Leftover INSERT/REMOVE variants + tree macros.
        masked2 = TranslationUnit(unit.path, text).mask_strings_comments()
        for m in re.finditer(
            r"\b(?:LIST|TAILQ|STAILQ|SLIST)_(?:INSERT|REMOVE)\w*\b",
            masked2,
        ):
            _propose_q(unit, "QUEUE_H_SITE", {"line": unit.line_col(m.start())[0], "snippet": m.group(0)[:60]})
        for m in re.finditer(
            r"\b(?:RB|SPLAY)_(?:INIT|INSERT|REMOVE|FOREACH|INITIALIZER)\w*\b",
            masked2,
        ):
            _propose_q(
                unit,
                "QUEUE_H_SITE",
                {"line": unit.line_col(m.start())[0], "snippet": m.group(0)[:60]},
            )

        if edits and "pbsd_intrusive_queue.hpp" not in text:
            inc = f'#include "{STUB.as_posix()}" /* PBSD queue ops */\n'
            q = re.search(r'#include\s*[<"]sys/queue\.h[>"]', text)
            if q:
                text = text[: q.end()] + "\n" + inc + text[q.end() :]
            else:
                text = inc + text
            edits.append(Edit(self.name, "include intrusive stub", 1, "", inc.strip()))

        return PassResult(text=text, refusals=refusals, edits=edits)


PROMOTE_QUEUE_OPS_PASSES: list[Pass] = [
    PromoteQueueOpsPass(),
]
