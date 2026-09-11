#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""lxs_args_cnt[] against the handler that receives what it copied.

linux_socketcall() is the i386 / amd64-linux32 entry point for the
multiplexed socket(2) family: userspace passes an opcode and a pointer to
an array of arguments, and the kernel copies the arguments in before
dispatching.  How many it copies is a table:

    static const unsigned char lxs_args_cnt[] = {
            0 /* unused*/,          3 /* socket */,
            3 /* bind */,           3 /* connect */,
            ...
    };
    #define LINUX_ARG_SIZE(x)       (lxs_args_cnt[x] * sizeof(l_ulong))

    error = copyin(PTRIN(args->args), a, LINUX_ARG_SIZE(args->what));

and who reads them is a struct:

    struct linux_connect_args {
            char s_l_[PADL_(l_int)]; l_int s; char s_r_[PADR_(l_int)];
            char name_l_[PADL_(l_uintptr_t)]; ...
            char namelen_l_[PADL_(l_int)]; ...
    };

Nothing in the language connects the two.  The table is a hand-maintained
list of small integers in a different file from most of the structs it
describes, and the buffer it fills is a fixed six-word array on the kernel
stack:

    l_ulong a[6];
    register_t l_args[6];

So the failure mode is quiet and it is one-directional.  A count too LARGE
copies more of the user's array than the handler declares - harmless.  A
count too SMALL leaves the tail of `a' holding whatever the previous frame
left on the kernel stack, and the handler reads those words as its last
arguments.  For linux_sendto that is a userspace address and a length; for
linux_getsockopt it is an out-pointer the kernel then writes through.  A
count above six overruns the buffer outright.

This is exactly the shape of defect a static analyser will not find,
because the analyser is what raised the question: 72 of the findings in
linux_socket.c rest on

    for (int i = 0; i < lxs_args_cnt[args->what]; ++i)
            l_args[i] = a[i];                       (linux_socket.c:2761)

whose bound it cannot fold, so it reports every downstream argument as
possibly-uninitialised.  Those 72 are false, but only because the table is
right, and "the table is right" was, until this file, a claim nobody had
checked since the table was written.  It is checked here by counting, on
every run, rather than by having been read once.

The check refuses to pass on absence.  A handler whose argument struct is
not found is a FAILURE, not a skip: a reader that finds nothing and a tree
with nothing wrong produce the same silence otherwise, and the whole point
of the file is to be the thing that would have noticed.

    python3 tools/verify/socketcall_args.py            # report
    python3 tools/verify/socketcall_args.py --gate     # exit 1 on any fault
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys

SRC = pathlib.Path(__file__).resolve().parents[2] / "hbsd" / "src"

SOCKET_C = "sys/compat/linux/linux_socket.c"
SOCKET_H = "sys/compat/linux/linux_socket.h"

# linux_socketcall() is compiled only for these two ABIs:
#   #if defined(__i386__) || (defined(__amd64__) && defined(COMPAT_LINUX32))
# so these are the proto headers whose structs it actually dispatches into.
# A struct missing from either of them is a fault.
SOCKETCALL_PROTO = (
    "sys/i386/linux/linux_proto.h",
    "sys/amd64/linux32/linux32_proto.h",
)
# The 64-bit ABIs have no socketcall, but they declare the same argument
# structs for the direct system calls.  A field count that disagrees there
# is still worth saying out loud, so they are searched too.
OTHER_PROTO = (
    "sys/amd64/linux/linux_proto.h",
    "sys/arm64/linux/linux_proto.h",
)

# linux_socket.h names hundreds of LINUX_* constants - socket options,
# address families, message flags - and their numbers collide with the
# socketcall opcodes freely (LINUX_TCP_NODELAY is also 1).  Only the block
# under this comment is the opcode enumeration, so only it is read.
_OP_BLOCK = "/* Operations for socketcall */"
_OP_DEFINE = re.compile(r"^#define\s+(LINUX_[A-Z0-9_]+)\s+(\d+)\s*$")
_TABLE = re.compile(r"lxs_args_cnt\[\]\s*=\s*\{(.*?)\}\s*;", re.S)
_ENTRY = re.compile(r"(\d+)\s*/\*\s*([A-Za-z0-9_]+)\s*\*/")
_CASE = re.compile(
    r"case\s+(LINUX_[A-Z0-9_]+)\s*:\s*return\s*\(\s*([A-Za-z0-9_]+)\s*\(", re.S
)
_BUF = re.compile(r"^\s*(?:l_ulong|register_t)\s+(a|l_args)\[(\d+)\]\s*;", re.M)


class Fault(Exception):
    """Something the reader needs and cannot get.  Never a silent skip."""


def _read(rel: str) -> str:
    p = SRC / rel
    if not p.is_file():
        raise Fault(f"{rel}: not in the tree")
    return p.read_text(errors="replace")


def struct_fields(text: str, name: str) -> list[str] | None:
    """The field names of `struct <name> { ... }', or None if not declared.

    Two spellings.  A hand-written struct is one declaration per line:

        struct linux_accept_args {
                register_t s;
                register_t addr;
                register_t namelen;
        };

    A syscalls.master-generated one pads every field for the ABI:

        char s_l_[PADL_(l_int)]; l_int s; char s_r_[PADR_(l_int)];

    where the padding members are not arguments.  Counting PADL_ counts the
    arguments exactly once, so that spelling is recognised and counted on
    its own terms rather than by stripping.
    """
    m = re.search(r"\bstruct\s+" + re.escape(name) + r"\s*\{", text)
    if m is None:
        return None
    i = m.end()
    depth = 1
    while i < len(text) and depth:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    if depth:
        raise Fault(f"struct {name}: unterminated body")
    body = text[m.end() : i - 1]

    pad = re.findall(r"\bPADL_\(", body)
    if pad:
        return [n for n in re.findall(r"\b([A-Za-z0-9_]+)_l_\s*\[\s*PADL_\(", body)]

    fields = []
    for decl in body.split(";"):
        decl = re.sub(r"/\*.*?\*/", " ", decl, flags=re.S).strip()
        if not decl:
            continue
        n = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\])?$", decl)
        if n is None:
            raise Fault(f"struct {name}: cannot read the declaration {decl!r}")
        fields.append(n.group(1))
    return fields


def collect() -> dict:
    """Everything the check compares, or a Fault saying what is missing."""
    csrc = _read(SOCKET_C)
    hsrc = _read(SOCKET_H)

    if hsrc.count(_OP_BLOCK) != 1:
        raise Fault(
            f"{SOCKET_H}: expected exactly one {_OP_BLOCK!r} block, found "
            f"{hsrc.count(_OP_BLOCK)}"
        )
    block = hsrc[hsrc.index(_OP_BLOCK) + len(_OP_BLOCK) :]
    end = block.find("#endif")
    if end < 0:
        raise Fault(f"{SOCKET_H}: the socketcall opcode block has no #endif")
    ops = {}
    for line in block[:end].splitlines():
        m = _OP_DEFINE.match(line)
        if m:
            ops[m.group(1)] = int(m.group(2))
    if not ops:
        raise Fault(f"{SOCKET_H}: no LINUX_* socketcall opcode defines")

    m = _TABLE.search(csrc)
    if m is None:
        raise Fault(f"{SOCKET_C}: lxs_args_cnt[] not found")
    table = [(int(c), nm) for c, nm in _ENTRY.findall(m.group(1))]
    if not table:
        raise Fault(f"{SOCKET_C}: lxs_args_cnt[] parsed empty")

    body = csrc[csrc.index("linux_socketcall(") :]
    cases = dict(_CASE.findall(body))
    if not cases:
        raise Fault(f"{SOCKET_C}: linux_socketcall() has no dispatch cases")

    bufs = {n: int(sz) for n, sz in _BUF.findall(body[: body.index("switch")])}
    if "a" not in bufs:
        raise Fault(f"{SOCKET_C}: linux_socketcall()'s copyin buffer not found")

    protos = {}
    for rel in SOCKETCALL_PROTO + OTHER_PROTO:
        protos[rel] = _read(rel)
    protos[SOCKET_C] = csrc
    protos[SOCKET_H] = hsrc

    return {"ops": ops, "table": table, "cases": cases, "bufs": bufs, "protos": protos}


def check(data: dict) -> tuple[list[str], list[str]]:
    """(faults, lines).  A fault is anything that is not provably fine."""
    ops, table, cases = data["ops"], data["table"], data["cases"]
    bufs, protos = data["bufs"], data["protos"]
    faults: list[str] = []
    lines: list[str] = []

    cap = min(bufs.values())
    lines.append(
        "copyin buffer: " + ", ".join(f"{n}[{s}]" for n, s in sorted(bufs.items()))
    )
    lines.append("")

    by_index = {v: k for k, v in ops.items()}
    if len(by_index) != len(ops):
        faults.append("two socketcall opcodes share a value")

    for idx, (count, label) in enumerate(table):
        if idx == 0:
            if count != 0:
                faults.append(f"lxs_args_cnt[0] is {count}, not the unused 0")
            continue

        op = by_index.get(idx)
        if op is None:
            faults.append(f"lxs_args_cnt[{idx}] ({label}) has no LINUX_* opcode")
            continue
        if op.removeprefix("LINUX_").lower() != label.lower():
            faults.append(f"{op} is {idx}, but lxs_args_cnt[{idx}] is marked {label}")

        if count > cap:
            faults.append(
                f"{op}: copies {count} words into a {cap}-word buffer"
            )

        fn = cases.get(op)
        if fn is None:
            faults.append(f"{op}: copied {count} words, dispatched by nothing")
            continue

        struct = f"struct linux_{fn.removeprefix('linux_')}_args"
        name = struct.split()[1]
        found: dict[str, list[str]] = {}
        for rel, text in protos.items():
            f = struct_fields(text, name)
            if f is not None:
                found[rel] = f

        missing = [r for r in SOCKETCALL_PROTO if r not in found]
        if not found:
            faults.append(f"{op}: {struct} is not declared anywhere searched")
            continue
        if missing and not (set(found) & {SOCKET_C, SOCKET_H}):
            # Declared for the 64-bit ABIs only, but socketcall dispatches
            # into it on 32-bit.  That is a real gap, not a reader gap.
            faults.append(
                f"{op}: {struct} missing from " + ", ".join(missing)
            )

        counts = {len(f) for f in found.values()}
        if len(counts) != 1:
            faults.append(
                f"{op}: {struct} has "
                + ", ".join(f"{len(f)} fields in {r}" for r, f in sorted(found.items()))
            )
            continue
        nfields = counts.pop()

        where = min(found, key=lambda r: (r not in (SOCKET_C, SOCKET_H), r))
        mark = "  " if nfields == count else "!!"
        lines.append(
            f"{mark} {op:<22} copies {count}  {struct} has {nfields}"
            f"  [{', '.join(found[where])}]"
        )
        if nfields != count:
            short = count < nfields
            faults.append(
                f"{op}: lxs_args_cnt says {count}, {struct} declares {nfields}"
                + (
                    f" - the handler reads {nfields - count} word(s) of kernel"
                    " stack the copyin never wrote"
                    if short
                    else " - copies more of the user array than the handler reads"
                )
            )

    for op, fn in sorted(cases.items()):
        idx = ops.get(op)
        if idx is None:
            faults.append(f"{op}: dispatched to {fn}, but has no opcode define")
        elif idx >= len(table):
            faults.append(f"{op} ({idx}): dispatched to {fn}, but not in lxs_args_cnt[]")

    return faults, lines


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--gate", action="store_true", help="exit 1 on any fault")
    args = ap.parse_args(argv)

    try:
        data = collect()
        faults, lines = check(data)
    except Fault as exc:
        print(f"socketcall_args: cannot check: {exc}", file=sys.stderr)
        return 1

    print("\n".join(lines))
    print()
    checked = len(data["table"]) - 1
    if faults:
        print(f"{len(faults)} fault(s) over {checked} socketcall opcodes:")
        for f in faults:
            print(f"  {f}")
        return 1 if args.gate else 0
    print(
        f"{checked} socketcall opcodes: every lxs_args_cnt[] entry matches the "
        "field count of the struct its handler receives."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
