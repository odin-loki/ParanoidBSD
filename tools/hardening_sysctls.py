#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""What do the hardening knobs default to, and does a running system agree?

tools/ci/show_hardening.sh reports build options - whether PAX_ASLR is
compiled in. That is not the same question as whether
hardening.pax.aslr.status defaults to on, and this project has already been
caught once by exactly that distinction: WITHOUT_MACHDEP_OPTIMIZATIONS was
set, reported as set, and inert.

There are 59 distinct hardening.* sysctls in the kernel and nothing checked
one of them. This extracts each knob and its compiled-in default from the
source, and compares a dump from a booted system against it.

HardenedBSD declares these with its own macros:

  SYSCTL_HBSD_4STATE(pax_aslr_status, pr_hbsd.aslr.status,
      _hardening_pax_aslr, status, ...)

so the node path comes from the third and fourth arguments and the default
from the C initialiser of the first. The four states are

  0 disabled   1 opt-in   2 opt-out   3 force-enabled

and for a hardened system the interesting distinction is 1 against 2: opt-in
means off unless a binary asks, opt-out means on unless a binary asks not to.
A knob silently moving from 2 to 1 across an upstream merge is a mitigation
turning itself off for everything that does not know to ask.

Half of these defaults are behind an #ifdef, so "the default" is not a
property of the C file alone - it is a property of the C file and the
kernel config. kern_malloc.c declares kmalloc_zero twice, enabled under
PAX_HARDEN_KMALLOC and disabled without it, and sys/conf/std.hardenedbsd
has that option commented out. Reading the first initialiser and calling
it the default reported a FAIL against the only system this project has
ever booted, for a knob the kernel had exactly right.

So --kernconf names the config the guards are resolved against: its
options are parsed the way config(8) reads them, includes followed and
nooptions honoured, and a guard the evaluator cannot decide leaves the
knob reported as "?" and skipped rather than asserted from whichever
branch happens to come first. It is worth passing - 18 knobs checked
with it against run 60, 10 without.

Usage:
  hardening_sysctls.py                 the table, from source
  hardening_sysctls.py --kernconf NAME resolve #ifdef-guarded defaults
                                       against that kernel config
  hardening_sysctls.py --check FILE    FILE is `sysctl hardening` from a
                                       booted system; compare and exit 1 on
                                       any difference
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SYS = ROOT / "hbsd/src/sys"

STATE = {
    "PAX_FEATURE_DISABLED": ("0", "disabled"),
    "PAX_FEATURE_OPTIN": ("1", "opt-in"),
    "PAX_FEATURE_OPTOUT": ("2", "opt-out"),
    "PAX_FEATURE_FORCE_ENABLED": ("3", "forced"),
    "PAX_FEATURE_SIMPLE_DISABLED": ("0", "disabled"),
    "PAX_FEATURE_SIMPLE_ENABLED": ("1", "enabled"),
}

# SYSCTL_HBSD_<n>STATE(var, jail_field, parent_node, leaf, ...)
HBSD = re.compile(
    r"SYSCTL_HBSD_(\d)STATE(?:_GLOBAL)?\s*\(\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,\s*"
    r"[^,]+,\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,",
    re.S)

# plain SYSCTL_INT / SYSCTL_UINT under a hardening node
PLAIN = re.compile(
    r"SYSCTL_(?:U?INT|LONG)\s*\(\s*"
    r"(_hardening[A-Za-z0-9_]*)\s*,\s*[^,]+,\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,[^;]*?&\s*([A-Za-z_][A-Za-z0-9_]*)",
    re.S)

INIT = re.compile(
    r"^\s*(?:static\s+)?(?:int|u_int|long|pax_state_t)\s+"
    r"{var}\s*=\s*([A-Za-z0-9_]+)\s*;", re.M)


# SYSCTL_NODE(parent, OID_AUTO, name, ...) - the C node variable is the
# parent concatenated with the name, so the tree has to be built from the
# declarations. Splitting the variable on underscores looks like it works
# and gets hardening.pax.disallow_map32bit wrong, because the leaf name has
# an underscore in it and nothing in the variable name says which
# underscores are separators.
NODE = re.compile(
    r"SYSCTL_(?:NODE|DECL)\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,"
    r"(?:\s*OID_AUTO\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*,)?", re.S)


def build_nodes() -> dict[str, str]:
    """C node variable -> dotted sysctl path."""
    nodes = {"": ""}
    pending = []
    for p in SYS.rglob("*.c"):
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        if "_hardening" not in text:
            continue
        for m in NODE.finditer(text):
            parent, name = m.group(1), m.group(2)
            if not name:
                continue
            pending.append((parent, name))
    # The root is declared as SYSCTL_NODE(_kern, OID_AUTO, hardening, ...)
    # or as its own top-level node; seed whatever names itself "hardening".
    for parent, name in pending:
        if name == "hardening":
            nodes[f"{parent}_hardening" if parent != "_kern"
                  else "_hardening"] = "hardening"
    nodes.setdefault("_hardening", "hardening")
    # Resolve the rest by repeated passes, since declaration order is not
    # dependency order across files.
    for _ in range(8):
        for parent, name in pending:
            if parent in nodes and f"{parent}_{name}" not in nodes:
                base = nodes[parent]
                nodes[f"{parent}_{name}"] = f"{base}.{name}" if base else name
    return nodes


NODES: dict[str, str] = {}


def node_to_path(node: str, leaf: str) -> str:
    base = NODES.get(node)
    if base is None:
        # Unresolved. Say so rather than guessing, because a guessed path
        # silently becomes "not present in the dump" at --check time.
        return f"?{node}.{leaf}"
    return f"{base}.{leaf}"


# config(8) turns each `options FOO` into a #define in an opt_*.h, so the
# guards around a default are decided by the kernel config, not by the C
# file. Parsed here in config(8)'s own order: a later nooptions removes an
# earlier options, and a commented-out `#options` line sets nothing.
CONF_OPT = re.compile(r"^\s*(no)?options?\s+([A-Za-z_][A-Za-z0-9_]*)")
CONF_INC = re.compile(r'^\s*include\s+"?([^"\s]+)"?')

# The compiler's own defines for the target, for the arch-guarded case.
ARCH_MACROS = {
    "amd64": {"__amd64__", "__x86_64__", "__LP64__"},
    "arm64": {"__aarch64__", "__LP64__"},
    "arm": {"__arm__"},
    "i386": {"__i386__"},
    "powerpc": {"__powerpc__"},
    "riscv": {"__riscv", "__LP64__"},
}


def config_options(kernconf: str, arch: str) -> set[str]:
    """The options a kernel config actually sets, following its includes."""
    dirs = [SYS / arch / "conf", SYS / "conf"]
    opts = set(ARCH_MACROS.get(arch, ()))
    seen: set[str] = set()

    def load(name: str) -> None:
        if name in seen:
            return
        seen.add(name)
        for d in dirs:
            p = d / name
            if p.is_file():
                break
        else:
            print(f"warning: kernel config {name} not found under "
                  f"{', '.join(str(d) for d in dirs)}", file=sys.stderr)
            return
        for line in p.read_text(errors="replace").splitlines():
            line = line.split("#")[0]
            if not line.strip():
                continue
            m = CONF_INC.match(line)
            if m:
                load(m.group(1))
                continue
            m = CONF_OPT.match(line)
            if m:
                if m.group(1):
                    opts.discard(m.group(2))
                else:
                    opts.add(m.group(2))

    load(kernconf)
    return opts


# A .c file that is not listed in sys/conf/files* is not compiled into any
# kernel, and its "defaults" are not defaults of anything.
# hardenedbsd/hbsd_pax_SKEL.c is the template a new PaX feature is copied
# from; reading it produced hardening.pax.SKEL.status, a knob no kernel has
# ever had. Left in, it lands in --check's "not present in the dump" pile,
# where a knob that really did go missing would be indistinguishable from
# it.
FILES_LINE = re.compile(
    r"^(\S+\.c)\s+(standard|mandatory|optional)\s*(.*)$")
FILES_EXTRA = {"amd64": ["files.x86"], "i386": ["files.x86"]}
# Everything from here on in a files* line says how to build the file, not
# whether to.
FILES_HOW = {"compile-with", "no-obj", "no-implicit-rule", "dependency",
             "clean", "before-depend", "warning", "no-ctfconvert", "local",
             "nowerror", "profiling-routine"}


def built_files(arch: str) -> dict[str, str]:
    """source path (relative to sys/) -> its condition in conf/files*."""
    out: dict[str, str] = {}
    names = ["files", f"files.{arch}"] + FILES_EXTRA.get(arch, [])
    for name in names:
        p = SYS / "conf" / name
        if not p.is_file():
            continue
        text = p.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = FILES_LINE.match(line)
            if m:
                out.setdefault(m.group(1),
                               m.group(3) if m.group(2) == "optional" else "")
    return out


def file_is_built(cond: str, opts: set[str] | None) -> bool:
    """Does the `optional` clause hold under these options?

    The clause is an and-list of or-groups: `optional pax pax_aslr` needs
    both, `optional foo | bar` needs either. config(8) lowercases option
    names, so the comparison does too. Without a config nothing is ruled
    out.
    """
    if opts is None or not cond:
        return True
    lower = {o.lower() for o in opts}
    groups: list[list[str]] = []
    after_bar = False
    for tok in cond.split():
        if tok in FILES_HOW:
            break
        if tok == "|":
            after_bar = True
            continue
        if after_bar and groups:
            groups[-1].append(tok)
            after_bar = False
        else:
            groups.append([tok])
    return all(any(t.lower() in lower for t in g) for g in groups)


CPP = re.compile(r"^\s*#\s*(ifdef|ifndef|if|elif|else|endif)\b\s*(.*?)\s*$")
DEFINED = re.compile(r"\bdefined\s*(?:\(\s*([A-Za-z_]\w*)\s*\)|([A-Za-z_]\w*))")
BOOLEAN = re.compile(r"^(?:\s|True|False|and|or|not|[()])+$")


def eval_cond(expr: str, opts: set[str] | None) -> bool | None:
    """Three-valued: True, False, or None for "this evaluator cannot say".

    Only the defined()/&&/||/! subset is understood. Anything else - an
    arithmetic comparison, a macro with a value, a #if on something config
    does not control - returns None, and None keeps the branch reachable
    rather than guessing at it.
    """
    if opts is None:
        return None
    e = DEFINED.sub(
        lambda m: str((m.group(1) or m.group(2)) in opts), expr)
    e = e.replace("&&", " and ").replace("||", " or ").replace("!", " not ")
    e = re.sub(r"\bnot\s+not\b", "", e)
    if not BOOLEAN.match(e) or not e.strip():
        return None
    try:
        return bool(eval(e, {"__builtins__": {}}, {}))  # noqa: S307
    except Exception:
        return None


def initialisers(text: str, var: str) -> list[tuple[str, list[str]]]:
    """Every initialiser of var, with the #if conditions guarding it."""
    rx = re.compile(INIT.pattern.format(var=re.escape(var)))
    # Each nesting level is (condition of the current branch, conditions of
    # the branches above it) - an #else is live only when none of its
    # siblings were.
    stack: list[tuple[str, list[str]]] = []
    out: list[tuple[str, list[str]]] = []
    for line in text.splitlines():
        m = CPP.match(line)
        if m:
            kind, rest = m.group(1), m.group(2)
            if kind == "ifdef":
                stack.append((f"defined({rest.split()[0]})", []))
            elif kind == "ifndef":
                stack.append((f"!defined({rest.split()[0]})", []))
            elif kind == "if":
                stack.append((rest, []))
            elif kind in ("elif", "else") and stack:
                cur, prev = stack[-1]
                stack[-1] = (rest if kind == "elif" else "", prev + [cur])
            elif kind == "endif" and stack:
                stack.pop()
            continue
        m = rx.match(line)
        if m:
            conds = []
            for cur, prev in stack:
                if cur:
                    conds.append(cur)
                conds.extend(f"!({p})" for p in prev)
            out.append((m.group(1), conds))
    return out


def any_reachable(conds: list[str], opts: set[str] | None) -> bool:
    """Can this branch be the one compiled in, under these options?"""
    return all(eval_cond(c, opts) is not False for c in conds)


def default_of(text: str, var: str,
               opts: set[str] | None) -> tuple[str, str]:
    """The initialiser the compiler actually sees for var, in this config.

    A single re.search here reads the first initialiser in the file and
    calls it the default. For a variable declared once that is right; for
    one declared twice, under #ifdef and #else, it is a coin flip that
    always lands on the enabled branch. kern_malloc.c's kmalloc_zero is
    exactly that shape:

      #ifdef PAX_HARDEN_KMALLOC
      static int kmalloc_zero = PAX_FEATURE_SIMPLE_ENABLED;
      #else
      static int kmalloc_zero = PAX_FEATURE_SIMPLE_DISABLED;
      #endif

    and sys/conf/std.hardenedbsd has that option commented out, so the
    kernel reports 0 and is right to. It was reported as a FAIL against the
    only system this project has ever booted - the worst kind of check, a
    red step that is the instrument's own fault, which teaches the reader
    to ignore it.

    So resolve the guards. Where the configuration does not decide - no
    config given, or a condition this evaluator does not understand -
    every reachable branch is kept, and a disagreement among them reports
    "?", excluding the knob from --check instead of asserting one branch.
    """
    cands = initialisers(text, var)
    if not cands:
        return ("?", "not found in this file")
    reachable = [tok for tok, conds in cands if any_reachable(conds, opts)]
    if not reachable:
        return ("?", "not compiled in: every declaration of it is "
                     "behind an option this configuration does not set")
    if len(set(reachable)) > 1:
        return ("?", "conditional: " + " or ".join(
            f"{t} ({STATE[t][1]})" if t in STATE else t
            for t in dict.fromkeys(reachable)))
    tok = reachable[0]
    if tok in STATE:
        return STATE[tok]
    return (tok, "")


def collect(opts: set[str] | None,
            arch: str = "amd64") -> tuple[list[tuple[str, str, str, str, str]],
                                          list[str]]:
    global NODES
    NODES = build_nodes()
    files = built_files(arch)
    rows = []
    skipped = []
    for p in sorted(SYS.rglob("*.c")):
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        if "hardening" not in text:
            continue
        rel = p.relative_to(SYS).as_posix()
        if rel not in files:
            if HBSD.search(text) or PLAIN.search(text):
                skipped.append(f"{rel}: not listed in sys/conf/files*, so no "
                               f"kernel compiles it")
            continue
        if not file_is_built(files[rel], opts):
            if HBSD.search(text) or PLAIN.search(text):
                skipped.append(f"{rel}: optional {files[rel].split()[0]}, "
                               f"not set in this configuration")
            continue
        for m in HBSD.finditer(text):
            nstates, var, node, leaf = m.groups()
            val, meaning = default_of(text, var, opts)
            rows.append((node_to_path(node, leaf), val, meaning,
                         f"{nstates}-state", rel))
        for m in PLAIN.finditer(text):
            node, leaf, var = m.groups()
            val, meaning = default_of(text, var, opts)
            rows.append((node_to_path(node, leaf), val, meaning, "int", rel))
    # deduplicate on the path, keeping the first
    seen = set()
    out = []
    for r in rows:
        if r[0] in seen:
            continue
        seen.add(r[0])
        out.append(r)
    return sorted(out), skipped


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", metavar="FILE",
                    help="output of `sysctl hardening` from a booted system")
    ap.add_argument("--kernconf", metavar="NAME",
                    help="kernel config the dump came from; its options "
                         "decide which side of an #ifdef a default is on. "
                         "Without it, a guarded default is reported as "
                         "conditional and skipped by --check.")
    ap.add_argument("--arch", default="amd64",
                    help="architecture whose conf/ directory to search "
                         "(default amd64)")
    args = ap.parse_args()

    opts = config_options(args.kernconf, args.arch) if args.kernconf else None
    rows, skipped = collect(opts, args.arch)
    unresolved = [r for r in rows if r[0].startswith("?")]
    undecided = [r for r in rows if r[1] == "?" and not r[0].startswith("?")]
    absent = [r for r in undecided if r[2].startswith("not compiled in")]
    conditional = [r for r in undecided if r not in absent]
    if not args.check:
        print(f"{len(rows)} hardening knobs declared with a compiled-in "
              f"default")
        if opts is None:
            print("no --kernconf: an #ifdef-guarded default is reported as "
                  "conditional")
        else:
            print(f"config {args.kernconf} ({args.arch}), "
                  f"{len(opts)} options set")
        print()
        print(f"{'default':>8s} {'kind':>8s}  sysctl")
        for path, val, meaning, kind, src in rows:
            note = f"  ({meaning})" if meaning else ""
            print(f"{val:>8s} {kind:>8s}  {path}{note}")
        print()
        print("A 4-state knob at 2 (opt-out) is on for everything that does")
        print("not ask to be excused; at 1 (opt-in) it is off for everything")
        print("that does not ask for it. That difference is a mitigation")
        print("being on or off for the whole system, and it is one token in")
        print("one C file.")
        print()
        if skipped:
            print(f"{len(skipped)} file(s) declare hardening knobs and are "
                  f"not in this kernel:")
            for line in skipped:
                print(f"  {line}")
            print()
        if absent:
            print(f"{len(absent)} knob(s) are declared but not compiled into")
            print("this kernel; they are skipped rather than counted as")
            print("missing from the dump, which is what a knob that really")
            print("did disappear would look like.")
            print()
        if conditional:
            print(f"{len(conditional)} default(s) are #ifdef-guarded and this")
            print("configuration does not decide them; they are reported as ?")
            print("and skipped by --check rather than asserted from whichever")
            print("branch happens to be written first.")
            print()
        if unresolved:
            print(f"{len(unresolved)} node path(s) could not be resolved from")
            print("the SYSCTL_NODE declarations and are printed with a")
            print("leading ?. They are excluded from --check rather than")
            print("guessed, because a guessed path becomes a silent")
            print("\"not present in the dump\".")
            print()
        print("--check takes `sysctl hardening` from a booted system and")
        print("compares. Pass the same --kernconf the image was built with,")
        print("or a guarded knob is skipped instead of checked.")
        return 0

    live: dict[str, str] = {}
    for line in Path(args.check).read_text(errors="replace").splitlines():
        if ":" not in line:
            continue
        k, _, v = line.partition(":")
        live[k.strip()] = v.strip()

    if opts is None:
        print("warning: no --kernconf, so #ifdef-guarded defaults are not "
              "resolved and are skipped rather than checked.", file=sys.stderr)
    expected = {r[0]: r[1] for r in rows
                if r[1] != "?" and not r[0].startswith("?")}
    bad = 0
    missing = 0
    for path, want in sorted(expected.items()):
        got = live.get(path)
        if got is None:
            missing += 1
            continue
        if got != want:
            print(f"FAIL {path}: source says {want}, kernel says {got}")
            bad += 1
    print(f"\n{len(expected) - bad - missing} knobs agree, {bad} differ, "
          f"{missing} not present in the dump, "
          f"{len(undecided) + len(unresolved)} not decided by the source.")
    if bad:
        print("A knob whose runtime default is not its compiled-in one is")
        print("either a loader tunable in the image or a bug. Both are worth")
        print("knowing; neither should be a surprise.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
