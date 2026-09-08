#!/usr/bin/env python3
"""A lock this function releases on some return paths and not on others.

The default clang checkers have no model for `mtx_lock', so a leaked
mutex is invisible to the sweep: `sys/netipsec/ipsec.c' compiles clean
and reports nothing, and `ipsec_chkreplay()' returns while holding
`replay->lock' on one of its eleven paths. That one was found by reading
a finding in a *different* translation unit and following the callee.
Reading is not a gate, so this is.

The rule is deliberately narrow, because the interesting property is not
"balanced" - plenty of functions in this tree take a lock and hand it to
their caller on purpose, and `..._locked()' names a whole family of
them. The property is INCONSISTENCY:

    within one function, a lock that is released before some returns
    and not before others.

A function that never unlocks is not reported at all; it has a contract.
A function that unlocks on ten paths and not on the eleventh has a bug
or an undocumented contract, and either is worth a line of output.

"Before" means dominates, not merely precedes. Each statement carries
the stack of blocks it sits in, and an unlock counts for a return only
when its block is an ancestor of (or the same as) the return's - so an
unlock inside one arm of an `if' does not excuse a return in the other.
That direction of error is the safe one: it can only over-report.
"""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from param_premise import definitions  # noqa: E402

def stripped(lines: list[str]) -> list[str]:
    """The file with comments and literal contents blanked, line for line.

    Brace counting is the whole mechanism here, so a `{' inside a string
    or a comment is not a detail. Done once for the file rather than per
    line, because a block comment spans lines and a line cannot tell on
    its own whether it is inside one.
    """
    out, in_block = [], False
    for ln in lines:
        buf, i, n = [], 0, len(ln)
        while i < n:
            c = ln[i]
            if in_block:
                if ln.startswith("*/", i):
                    in_block = False
                    i += 2
                else:
                    i += 1
                continue
            if ln.startswith("/*", i):
                in_block = True
                i += 2
                continue
            if ln.startswith("//", i):
                break
            if c in "\"'":
                quote, i = c, i + 1
                while i < n and ln[i] != quote:
                    i += 2 if ln[i] == "\\" else 1
                i += 1
                buf.append(quote + quote)
                continue
            buf.append(c)
            i += 1
        out.append("".join(buf))
    return out


def joined(lines: list[str]) -> list[str]:
    """One logical statement per line, the rest blanked.

    kern_osd.c wraps `rm_runlock(&osdm[type].osd_object_lock, &tracker);'
    across two lines, and a line-at-a-time reader does not see the
    release at all - it reported the return below it as a leak. Statements
    are accumulated until their parentheses balance, and the whole thing
    is put on the line the statement STARTED on so every index still
    means what it meant.
    """
    out = [""] * len(lines)
    i = 0
    while i < len(lines):
        s = lines[i].strip()
        if not s:
            i += 1
            continue
        depth = s.count("(") - s.count(")")
        j = i
        while depth > 0 and j + 1 < len(lines):
            j += 1
            nxt = lines[j].strip()
            s = s + " " + nxt
            depth += nxt.count("(") - nxt.count(")")
        out[i] = s
        i = j + 1
    return out


# A call statement on a line of its own: NAME(args);
CALL = re.compile(r"^\s*([A-Za-z_]\w*)\s*\((.*)\)\s*;\s*$")
RETURN = re.compile(r"^\s*return\b")
_LOCK_WORD = re.compile(r"(?i)lock")

# Any call at all, anywhere on a line - including inside an `if' condition.
ANYCALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
_NOT_A_CALL = {"if", "while", "for", "switch", "return", "sizeof", "defined",
               "case", "do", "else", "typeof", "__typeof", "offsetof"}
# Calls that cannot release a lock, so they do not hide a hand-off. Kept
# to shapes with no plausible reading as "and this one unlocks": an
# assertion, a trace point, a counter, a print.
_HARMLESS = re.compile(
    r"^(KASSERT|MPASS|VNASSERT|VNPASS|panic|printf|device_printf|uprintf|"
    r"log|ipseclog|bootverbose|CTR\d|SDT_PROBE\d|.*_ASSERT|.*ASSERT_.*|"
    r"counter_u64_add|.*STAT_INC|.*STAT_ADD|.*STAT_SUB|nitems|howmany|"
    r"atomic_(load|store|add|subtract|set|clear|fetch|thread_fence)\w*)$")


def hands_off(line: str, skip: tuple = ()) -> bool:
    """Does this line call something that might have released the lock?

    `skip' is the pair being tracked. Taking the lock is not a hand-off,
    and the first version counted it as one - which silently excused
    every leak whose acquire was the statement above the return.
    """
    for m in ANYCALL.finditer(line):
        name = m.group(1)
        if name in _NOT_A_CALL or name in skip or _HARMLESS.match(name):
            continue
        return True
    return False


def unlock_of(name: str) -> str | None:
    """The name that releases what `name' takes, or None if it is not a lock.

    Rewriting the LAST `lock' in the identifier covers every family the
    tree uses: SECREPLAY_LOCK/UNLOCK, mtx_lock/unlock,
    mtx_lock_spin/unlock_spin, rw_wlock/wunlock, sx_xlock/xunlock,
    lockmgr_xlock/xunlock. It also rewrites names that merely contain
    the word, which is why a candidate only becomes a pair when the
    partner is actually called in the same function.
    """
    if "ASSERT" in name.upper():
        return None                     # SOCKBUF_LOCK_ASSERT takes nothing
    hits = list(_LOCK_WORD.finditer(name))
    if not hits:
        return None
    m = hits[-1]
    if name[max(0, m.start() - 2):m.start()].lower() == "un":
        return None                     # this IS an unlock
    got = m.group(0)
    rep = "UNLOCK" if got.isupper() else "unlock"
    return name[:m.start()] + rep + name[m.end():]


OPENS = re.compile(r"^\s*(\}\s*)?(if|else\s+if|else|for|while|do|switch)\b")
BARE_ELSE = re.compile(r"^\s*(\}\s*)?else\b")
GOTO = re.compile(r"^\s*goto\b")
CASE = re.compile(r"^\s*(case\b|default\s*:)")
BARE_MACRO = re.compile(r"^[A-Za-z_]\w*\s*;$")
PANIC = re.compile(r"^\s*(panic|vpanic|kassert_panic)\s*\(")


class _Frame:
    """One `{ ... }', and what the construct that opened it does on exit."""

    __slots__ = ("kind", "entry", "other")

    def __init__(self, kind: str, entry: bool, other=None):
        self.kind = kind            # if / else / loop / switch / plain
        self.entry = entry
        self.other = other          # the `if' arm's (exit, terminated)


def _construct(line: str) -> str:
    m = OPENS.match(line)
    if not m:
        return "plain"
    word = m.group(2).split()[0]
    if word == "else":
        return "else"
    if word == "if":
        return "if"
    if word == "switch":
        return "switch"
    return "loop"                   # for, while, do


def _walk(lines, head, end, lock, unlock, arg):
    """([lines whose `return' must still hold it], final state, term), or None.

    A textual must-analysis over the brace structure. `state' is "the
    lock is definitely held here", so the join for two arms is AND, and
    a construct that can be skipped joins with its own entry state.

    The two things that make it more than a dominance test:

      * an arm that cannot fall through does not join. `if (x) { UNLOCK;
        return; }' leaves the lock held afterwards, and the earlier
        version of this read it as released.
      * an if/else where BOTH arms release does release.
        intr_event_bind_ithread_cpuset() is the first shape and
        udp_ctloutput() is the second, written as two #ifdef arms.

    None means the walk lost track - a `goto' joins paths it does not
    model, and unbalanced braces mean an #ifdef or a macro body.
    """
    state = term = False
    stack: list[_Frame] = []
    pending = None                  # an `if' arm closed; `else' may follow
    cond = False                    # previous line was a braceless head
    # The `{' is often not on the line that says what it opens:
    # ipsec_chkreplay()'s widest `if' spans two lines and the brace is on
    # the second. Reading the construct off the brace's own line called
    # that a plain block, which carries its arm's state out instead of
    # joining it, and the whole point of the function was lost.
    start, ended = "", True
    out = []

    for i in range(head, end + 1):
        s = lines[i].strip()
        if not s or s[0] == "#":
            continue
        if ended:
            start = s
        ended = s.endswith((";", "{", "}", ":"))

        if s[0] == "}":
            if not stack:
                return None
            f = stack.pop()
            exit_, exit_term = state, term
            if f.kind == "if":
                pending, state, term = (exit_, exit_term), f.entry, False
            elif f.kind == "else":
                if_exit, if_term = f.other or (f.entry, False)
                if if_term and exit_term:
                    state, term = False, True
                elif if_term:
                    state, term = exit_, False
                elif exit_term:
                    state, term = if_exit, False
                else:
                    state, term = if_exit and exit_, False
            elif f.kind in ("loop", "switch"):
                state, term = f.entry and exit_, False
            else:
                state, term = exit_, exit_term
            s = s[1:].strip()
            if not s:
                continue
        elif pending is not None and not BARE_ELSE.match(s):
            arm_exit, arm_term = pending
            state = state if arm_term else (state and arm_exit)
            pending = None

        opens = "{" in s
        stmt = s.split("{")[0].strip() if opens else s

        m = CALL.match(stmt) if stmt else None
        name = m.group(1) if m else None
        a = " ".join(m.group(2).split()) if m else None
        if name == lock and a == arg:
            if not cond:
                state = True
            term = False
        elif name == unlock and a == arg:
            state, term = False, False
        elif PANIC.match(stmt):
            # siba.c writes `panic(...); return (ENXIO);' in two switch
            # arms. The return is unreachable, so it leaks nothing.
            state, term = False, True
        elif RETURN.match(stmt):
            if state:
                out.append(i)
            term = True
        elif GOTO.match(stmt):
            return None
        elif stmt.startswith(("break", "continue")):
            term = True
        elif CASE.match(stmt):
            # every arm of a switch starts from the switch's entry state:
            # carp_modevent() takes carp_mtx under `case MOD_UNLOAD' and
            # a line-at-a-time reader carried that into `default:'.
            for f in reversed(stack):
                if f.kind == "switch":
                    state, term = f.entry, False
                    break
        elif stmt:
            term = False

        if opens:
            kind = _construct(start)
            if kind == "else":
                stack.append(_Frame("else", state, pending))
                pending = None
            else:
                stack.append(_Frame(kind, state))
            state = state if kind != "else" else state
            term = False
            cond = False
            continue

        cond = bool(OPENS.match(start)) and not s.endswith(";")
    if stack:
        return None
    # The last way out says what the function's contract is. If control
    # leaves the bottom still holding, or the final `return' does, then
    # returning locked is the whole point and the earlier ones are not
    # leaks: udp_input()'s tunnel helper re-takes INP_RLOCK for its
    # caller, and scsi_pass.c's passdoioctl() re-takes the periph lock
    # after cam_periph_mapmem(), which cannot run holding it.
    return (out, state, term)


def _blocks(lines, head, end):
    """{index: block path} for every line of the body, `definitions'-indexed.

    The path is the tuple of `{' ids the line sits inside. Block A is an
    ancestor of block B exactly when A's path is a prefix of B's.

    `definitions()' is 0-based and `end' is the closing brace, so the
    stack empties exactly at the last line. A `}' with nothing open is a
    body an #ifdef or a macro made unreadable: give up on that function
    rather than compute a wrong path for it.
    """
    nxt, stack, out = 0, [], {}
    for i in range(head, end + 1):
        out[i] = tuple(stack)
        for ch in lines[i]:
            if ch == "{":
                stack.append(nxt)
                nxt += 1
            elif ch == "}":
                if not stack:
                    return None
                stack.pop()
    return out if not stack else None


def _events(lines, head, end, path):
    ev = []
    for i in range(head, end + 1):
        src = lines[i]
        if RETURN.match(src):
            ev.append(("return", None, None, i, path[i]))
            continue
        m = CALL.match(src)
        if not m:
            continue
        name, arg = m.group(1), " ".join(m.group(2).split())
        un = unlock_of(name)
        if un is not None:
            ev.append(("lock", name, arg, i, path[i]))
        elif "UNLOCK" in name.upper():
            ev.append(("unlock", name, arg, i, path[i]))
    return ev


def _dominates(a, b) -> bool:
    return b[:len(a)] == a


def _guard(lines, path, rln, rpath) -> str:
    """The line that opened the block this return sits in.

    `if (in_pcbrele_rlocked(inp)) { ... return (true); }' - udp_append(),
    with the comment above it explaining that the caller must not unlock
    in this case - hands the lock off in the CONDITION, where neither the
    return's own line nor the statement above it can see it.
    """
    for k in range(rln - 1, -1, -1):
        if k not in path:
            break
        if len(path[k]) < len(rpath):
            return lines[k]
    return ""


def _previous(lines, path, rln, rpath, skip=()) -> str:
    """The statement whose call could have handed the lock off.

    Not literally the previous line. A braceless `if' head sits between
    the return and the call that matters - xenstore.c does `error =
    msleep(..., PDROP, ...); if (error && ...) return (error);', and
    PDROP means msleep returned WITHOUT the lock. And an assertion sits
    there in if_ovpn.c, which calls ovpn_finish_rx() to hand the lock on
    and then writes OVPN_UNLOCK_ASSERT(sc) to say so. So walk back past
    heads and past calls that cannot release, and answer with the first
    statement that could.
    """
    for k in range(rln - 1, -1, -1):
        if k not in path or path[k] != rpath:
            break
        s = lines[k].strip()
        if not s:
            continue
        if OPENS.match(s) and not s.endswith(";"):
            continue                # a braceless if/for/while head
        # A bare macro statement takes no argument, so it releases
        # nothing: intel_ctx.c writes TD_PINNED_ASSERT; between the
        # hand-off to dmar_free_ctx_locked() and the return. An
        # assignment is NOT this - `ta = tc->ta;' has no call in it
        # either, and skipping past it walked back to an unrelated one.
        if BARE_MACRO.match(s):
            continue
        if hands_off(s, skip):
            return s
        m = CALL.match(s)
        if m is None:
            break                   # an assignment or a plain expression
    return ""


def check(src: Path, loose: bool = False):
    """[(line, name, arg, head)] - returns that still hold it, 1-based."""
    lines = src.read_text(errors="replace").splitlines()
    bare = joined(stripped(lines))
    out = []
    for _brace, head, end in definitions(lines):
        path = _blocks(bare, head, end)
        if path is None:
            continue
        ev = _events(bare, head, end, path)
        locks = [e for e in ev if e[0] == "lock"]
        unlocks = [e for e in ev if e[0] == "unlock"]
        if not locks or not unlocks or not any(e[0] == "return" for e in ev):
            continue
        seen = set()
        for _k, name, arg, ln, _lpath in locks:
            un = unlock_of(name)
            if (name, arg) in seen:
                continue
            seen.add((name, arg))
            mine = [u for u in unlocks if u[1] == un and u[2] == arg]
            if not mine:
                continue            # no partner call: not a pair here
            # A release ABOVE the first acquire means the function was
            # entered holding this lock, dropped it for work it could not
            # do holding it, and took it back - so returning holding it
            # is the contract, not a leak. udp_append() says so in a
            # comment ("indicating that you must unlock the inpcb"),
            # scsi_pass.c's passdoioctl() drops it across
            # cam_periph_mapmem(), and ext2_nodealloccg() across bread().
            if any(u[3] < ln for u in mine):
                continue
            walked = _walk(bare, head, end, name, un, arg)
            if walked is None:
                continue            # goto, or a body the braces do not bound
            held, final, term = walked
            if held and (final or held[-1] == max(
                    e[3] for e in ev if e[0] == "return")):
                continue            # the contract is to return holding it
            for rln in held:
                # A callee that took the lock off our hands. Common in
                # this tree: vm_object_deallocate() calls
                # vm_object_deallocate_vnode(object) and returns, and the
                # callee owns the lock from there. Nothing textual tells
                # that apart from a leak, so a call as the statement
                # immediately before the return - or as the return's own
                # expression, which is how tcp_ctloutput() ends -
                # disqualifies it.
                pair = (name, un)
                if not loose and (
                        hands_off(bare[rln], pair)
                        or hands_off(_previous(bare, path, rln, path[rln],
                                               pair), pair)
                        or hands_off(_guard(bare, path, rln, path[rln]),
                                     pair)):
                    continue
                # `definitions' counts from 0 and a reader counts from 1
                out.append((rln + 1, name, arg, head + 1))
    return out


def main(argv):
    args = [a for a in argv[1:] if a != "--all"]
    loose = "--all" in argv
    roots = [Path(a) for a in args] or [Path("hbsd/src/sys")]
    files = []
    for r in roots:
        files.extend(sorted(r.rglob("*.c")) if r.is_dir() else [r])
    total = 0
    for f in files:
        try:
            hits = check(f, loose)
        except (OSError, RecursionError):
            continue
        for ln, name, arg, head in hits:
            print(f"{f}:{ln}: return holds {name}({arg}) taken in the "
                  f"function at :{head}")
            total += 1
    kind = "candidate" if loose else "gated"
    print(f"{total} {kind} return(s) in {len(files)} file(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
