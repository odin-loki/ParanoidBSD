#!/usr/bin/env python3
"""Find functions that end in exit() but are not declared noreturn.

A helper that prints a message and exits is written all over the base
system, and every caller knows it:

	if ((p = malloc(n)) == NULL) {
		fatal("malloc");
		/*NOTREACHED*/
	}
	memcpy(p, &x, n);

The NOTREACHED comment is the author telling the reader.  Nothing tells
the compiler.  So the compiler cannot warn about code that really is
unreachable after such a call, it does warn about variables it thinks
may be used uninitialised past it, and every static analysis run over
the program reports the line after every call site.

route6d's fatal() cost seven findings, ppp's AbortProgram() fourteen,
and patch(1)'s fatal() and pfatal() had already cost their own.  This
looks for the rest.

What it reports is a function whose body's LAST statement is a call to
something that does not return -- exit(), _exit(), abort(), err() and
friends, or another function in the same file this pass has already
concluded does not return -- and whose declaration and definition carry
no noreturn attribute.  "Last statement" is the conservative part: a
function that exits down every arm of a switch is not reported, because
proving that needs a control-flow graph and this does not have one.

It is a lint, not a proof, and it does NOT gate CI.  673 functions in
the tree are reported, and marking one is only worth a vendor-tree diff
where it buys something.  The first attempt to pick those by file --
"this file has an undeclared-noreturn function and this file has
findings" -- put __dead2 on 24 functions across 19 files and moved the
sweep by exactly nothing: 116 findings before, 116 after.  That batch
was reverted.

What separates route6d and ppp, where two declarations closed
twenty-one findings, is not the file.  It is that the call sits on the
path the finding walks:

	if ((p = malloc(n)) == NULL) {
		fatal("malloc");
	}
	memcpy(p, &x, n);        <-- the finding, reachable only because
				     the analyser thinks fatal() returns

A usage() called from getopt and never followed by anything costs
nothing, however undeclared it is.

Two ways of predicting which is which were tried and dropped before a
third worked.  Ranking by findings within 40 lines of a call site scored
a known-zero batch 18.  Ranking by the guard shape alone -- the call
alone in an `if' body, a name from the condition used after the block --
scored it 34, because `if (argc < 2) usage();' is that shape and an int
argc produces no finding.

--guards is the third, and the missing word was POINTER.  Every case
that paid tests a pointer for NULL and dereferences it after the block:

    route6d  if ((iffp = malloc(...)) == NULL) fatal(...);   memcpy(iffp, ...)
    ppp      if ((iov[n].iov_base = malloc(sz)) == NULL) ... AbortProgram();
                                                             memcpy(iov[n].iov_base, ...)
    lpc      if ((bp = el_gets(...)) == NULL || num == 0) quit(0, NULL);
                                                             memcpy(cmdline, bp, len)
    dump     if (tmpbuf == NULL && (tmpbuf = malloc(...)) == NULL) quit(...);
                                                             memcpy(buf, &tmpbuf[base], ...)
    pfctl    if (command == NULL) usage();                    strcmp(command, "-F")

and every case that did not pay tests something that is not a pointer,
or does not use it afterwards.  So --guards requires a NULL test and a
dereference of the same name, and scores 0 on all fifteen directories
whose measured answer was 0.

It is still a prior, not the answer.  The oracle is a before/after sweep
at the same scope on the same tree, which is cheap; --guards only says
where to spend one.
"""

import argparse
import os
import re
import sys

# The names the C library and this tree use for "does not come back".
SEED = {
    "exit", "_exit", "_Exit", "abort", "quick_exit",
    "err", "errx", "verr", "verrx", "errc", "verrc",
    "panic", "longjmp", "siglongjmp", "_longjmp", "_siglongjmp",
    "execl", "execle", "execlp", "execv", "execvp", "execvP", "execve",
}

# Anything that marks a declaration as noreturn.  Not `\bnoreturn\b':
# the spelling in the tree is __noreturn__ as often as noreturn, and an
# underscore is a word character, so the boundary never occurs.  Comments
# and strings are already blanked before this runs, so a bare substring
# match inside a declaration is safe.
ATTR = re.compile(r"__dead2|__dead\b|_Noreturn|noreturn")

# A definition's opening at column 0.  KNF puts the return type on the
# line above, so the name is at column 0 -- but plenty of the tree does
# not: chat(1) writes `void terminate(int status)' on one line and every
# one of its helpers was invisible until this matched the return type
# too.  The repetition is greedy so the capture lands on the LAST
# identifier before the parenthesis, which is the name.
DEF_RE = re.compile(
    r"^(?:[A-Za-z_]\w*[ \t]+|[ \t]*\*[ \t]*)*([A-Za-z_]\w*)\s*\(")

CALL_RE = re.compile(r"^\s*(?:\(\s*void\s*\)\s*)?([A-Za-z_]\w*)\s*\(")


def strip_noise(text):
    """Blank out comments and string/char literals, keeping line count."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif c in "\"'":
            q, j = c, i + 1
            while j < n and text[j] != q:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def definitions(lines):
    """Yield (name, name_line, brace_line, end_line) per column-0 definition.

    brace_line is the `{' that opens the body -- last_call() counts
    braces from there, and starting it at the name line instead put the
    opening brace inside the count, so depth never came back to zero and
    every function looked like it ended in nothing.
    """
    i, n = 0, len(lines)
    while i < n:
        m = DEF_RE.match(lines[i])
        if not m:
            i += 1
            continue
        # Find the '{' that opens the body: it must be at column 0 on a
        # line of its own within a few lines, with no ';' before it (a
        # prototype ends in ';', a definition does not).
        j, body = i, None
        while j < min(i + 12, n):
            s = lines[j]
            if ";" in s:
                break
            if s.startswith("{"):
                body = j
                break
            j += 1
        if body is None:
            i += 1
            continue
        k = body + 1
        while k < n and not lines[k].startswith("}"):
            k += 1
        if k < n:
            yield m.group(1), i, body, k
            i = k + 1
        else:
            i += 1


RETURN_RE = re.compile(r"\breturn\b")
VA_END_RE = re.compile(r"va_end\s*\(")

CTRL_RE = re.compile(r"^(?:if|for|while|switch|do|else|return|goto|case|default)\b")


def last_call(lines, body_start, end):
    """The callee of the body's last TOP-LEVEL statement, or None.

    Top-level matters and taking the last line does not.  ifconfig's
    set80211() ends with

    	if (ioctl(...) < 0)
    		err(1, "SIOCS80211");

    which is a function that returns on every path but one; reading only
    the last line called it noreturn and, through the propagation below,
    called eighty of its callers noreturn too.  So walk the body
    accumulating whole constructs at brace depth zero, and look at the
    last one -- a control statement, however it ends, is not a bare call.
    """
    depth = 0
    cur = []
    stmts = []
    for k in range(body_start + 1, end):
        s = lines[k]
        # A preprocessor line is not part of any statement.  chat(1)'s
        # terminate() ends `#endif' then `exit(status);', and folding the
        # directive into the accumulator made the last statement read as
        # "#endif exit(status);", which matches no call -- so terminate()
        # looked like it returned and fatal(), which ends in terminate(),
        # was never reached by the propagation either.
        if s.lstrip().startswith("#"):
            continue
        if not s.strip() and not cur:
            continue
        cur.append(s)
        depth += s.count("{") - s.count("}")
        joined = " ".join(x.strip() for x in cur).strip()
        if depth <= 0 and (joined.endswith(";") or joined.endswith("}")):
            stmts.append(joined)
            cur = []
            depth = 0
    # A varargs wrapper cleans up after the call that does not return.
    # efivar(8)'s rep_err() is
    #
    #     if (quiet)
    #             exit(eval);
    #     va_start(ap, fmt);
    #     verr(eval, fmt, ap);
    #     va_end(ap);
    #
    # and reading only the last statement saw va_end() -- a call that
    # does return -- so rep_err() and rep_errx() were never reported,
    # and breakdown_name() walked out of `rep_errx(1, ...)' and back to
    # `*cp = NUL' with cp still NULL.  va_end() after a call that does
    # not return is unreachable; drop it and look at what precedes it.
    while stmts and VA_END_RE.match(stmts[-1]):
        stmts.pop()
    last = stmts[-1] if stmts else None
    if last is None or not last.endswith(";") or CTRL_RE.match(last):
        return None
    # A `return' anywhere in the body means the function has a path back
    # to its caller, whatever its last statement is.  ifconfig is full of
    # this shape -- mapfreq() scans a table, returns on a hit, and errx()s
    # at the bottom when there is none -- and calling those noreturn
    # would be wrong, not merely noisy.  No return, and a last statement
    # that does not come back, is the whole of the claim.
    for k in range(body_start + 1, end):
        if RETURN_RE.search(lines[k]):
            return None
    m = CALL_RE.match(last)
    return m.group(1) if m else None


def declared_noreturn(text, name):
    """Does any declaration or the definition of `name` say noreturn?

    `text' is the definition's own file with every header in its
    directory appended -- ppp declares AbortProgram() in main.h and
    patch declares fatal() in util.h, so a same-file search would report
    both however they are marked.
    """
    for m in re.finditer(r"\b" + re.escape(name) + r"\s*\(", text):
        # Look at the statement this name sits in: back to the previous
        # ';' or '}' or start, forward to the next ';' or '{'.
        a = max(text.rfind(";", 0, m.start()), text.rfind("}", 0, m.start()),
                text.rfind("{", 0, m.start()))
        b = m.end()
        depth = 1
        while b < len(text) and depth:
            if text[b] == "(":
                depth += 1
            elif text[b] == ")":
                depth -= 1
            b += 1
        c = b
        while c < len(text) and text[c] not in ";{":
            c += 1
        if ATTR.search(text[a + 1:m.start()]) or ATTR.search(text[b:c]):
            return True
    return False


_HDR_CACHE = {}
_HDR_NORETURN = {}

# A declaration in a header that already carries the attribute, e.g.
# `void dumpabort(int signo) __dead2;'.  Without this the propagation
# stops at the file boundary: dump's quit() ends in dumpabort(), which
# is declared __dead2 in dump.h and defined in another file, so quit()
# looked like a function that returns and its own missing attribute went
# unreported.
DECL_NORETURN_RE = re.compile(
    r"\b([A-Za-z_]\w*)\s*\([^;{]*\)[^;{]*(?:__dead2|__dead\b|_Noreturn|noreturn)"
    r"[^;{]*;")


def headers_for(path):
    """Every .h in the file's own directory, stripped, concatenated.

    A program's prototypes live beside its sources; this is where a
    noreturn attribute is written when it is written at all.
    """
    d = os.path.dirname(path)
    if d not in _HDR_CACHE:
        parts = []
        try:
            names = sorted(os.listdir(d))
        except OSError:
            names = []
        for fn in names:
            if not fn.endswith(".h"):
                continue
            try:
                with open(os.path.join(d, fn), encoding="utf-8",
                          errors="surrogateescape") as fh:
                    parts.append(strip_noise(fh.read()))
            except OSError:
                pass
        _HDR_CACHE[d] = "\n".join(parts)
        _HDR_NORETURN[d] = set(DECL_NORETURN_RE.findall(_HDR_CACHE[d]))
    return _HDR_CACHE[d]


def header_noreturn(path):
    """Names the directory's headers already declare noreturn."""
    d = os.path.dirname(path)
    if d not in _HDR_NORETURN:
        headers_for(path)
    return _HDR_NORETURN.get(d, set())


# A guard whose condition tests a pointer against NULL.  `!p' is
# deliberately not here: `if (!interactive) usage();' is that shape and
# an int is not a pointer, which is the mistake the second heuristic
# made.
_LV = r"[A-Za-z_]\w*(?:\s*(?:->|\.)\s*\w+|\s*\[[^\]]*\])*"
NULLTEST_RE = re.compile(
    # p == NULL, p->x == NULL, p[i] == NULL
    r"(" + _LV + r")\s*==\s*NULL"
    # NULL == p
    r"|NULL\s*==\s*(" + _LV + r")"
    # (p = malloc(n)) == NULL -- the idiom ppp, lpc and dump all use,
    # where the name being tested is the assignment's target and sits
    # arbitrarily far left of the ==.
    r"|\(\s*(" + _LV + r")\s*=(?!=)[^;]*?\)\s*==\s*NULL")

GUARD_RE = re.compile(r"^\s*(?:\}\s*else\s+)?if\s*\((.*)\)\s*\{?\s*$")

# Using it as a pointer afterwards: a dereference, an index, or an
# argument to something that will read through it.
def _use_re(name):
    n = re.escape(name)
    return re.compile(
        n + r"\s*(?:->|\[)"                       # p->x, p[i]
        r"|\*\s*" + n + r"\b"                     # *p
        r"|\b(?:str|mem|b)\w*\s*\([^;]*\b" + n + r"\b"   # strcmp(p, ..), memcpy(.., p, ..)
        r"|\b" + n + r"\s*\+\s*\w")             # p + i


_SRC_CACHE = {}


def sources_for(path):
    """Every .c in the file's own directory, stripped, as line lists.

    The guard is usually not in the file that defines the helper:
    ppp declares AbortProgram() in main.h, defines it in main.c and
    guards with it in physical.c, udp.c and four more; lpc defines
    quit() in cmds.c and guards with it in lpc.c; pfctl defines usage()
    in pfctl.c and guards with it in pfctl_table.c.  Only route6d and
    dump keep both in one file, which is why a same-file search found
    exactly those two and none of the other three.
    """
    d = os.path.dirname(path)
    if d not in _SRC_CACHE:
        out = []
        try:
            names = sorted(os.listdir(d))
        except OSError:
            names = []
        for fn in names:
            if not fn.endswith(".c"):
                continue
            try:
                with open(os.path.join(d, fn), encoding="utf-8",
                          errors="surrogateescape") as fh:
                    out.append(strip_noise(fh.read()).split("\n"))
            except OSError:
                pass
        _SRC_CACHE[d] = out
    return _SRC_CACHE[d]


def guarded_sites(path, name, window=25):
    """Call sites of `name' that are a NULL guard the code walks past.

    The call must be the whole body of an `if' whose condition tests
    some pointer against NULL, and that pointer must be used as a
    pointer within `window' lines after the block closes.  Searched
    across the whole directory, not just the defining file.
    """
    total = 0
    for lines in sources_for(path):
        total += _guards_in(lines, name, window)
    return total


def _guards_in(lines, name, window):
    call = re.compile(r"^\s*(?:\(\s*void\s*\)\s*)?" + re.escape(name)
                      + r"\s*\(")
    hits = 0
    for i, line in enumerate(lines):
        m = GUARD_RE.match(line)
        if not m:
            continue
        names = {a or b or c for a, b, c in NULLTEST_RE.findall(m.group(1))}
        names = {re.sub(r"\s+", "", n) for n in names if n}
        if not names:
            continue
        braced = line.rstrip().endswith("{")
        j, body = i + 1, []
        if braced:
            depth = 1
            while j < len(lines) and depth:
                depth += lines[j].count("{") - lines[j].count("}")
                if depth:
                    body.append(lines[j])
                j += 1
        else:
            while j < len(lines) and not lines[j].strip().endswith(";"):
                body.append(lines[j])
                j += 1
            if j < len(lines):
                body.append(lines[j])
                j += 1
        # The call may sit alone, or after a log line -- what matters is
        # that nothing in the body returns or breaks out.
        joined = " ".join(b.strip() for b in body).strip()
        if not any(call.match(b.strip()) for b in body):
            continue
        if re.search(r"\b(?:return|goto|break|continue)\b", joined):
            continue
        after = "\n".join(lines[j:j + window])
        if any(_use_re(v).search(after) for v in names):
            hits += 1
    return hits


def scan(path):
    try:
        with open(path, encoding="utf-8", errors="surrogateescape") as fh:
            raw = fh.read()
    except OSError:
        return []
    if "exit" not in raw and "abort" not in raw and "err" not in raw:
        return []
    text = strip_noise(raw)
    lines = text.split("\n")
    defs = list(definitions(lines))
    if not defs:
        return []
    decls = text + "\n" + headers_for(path)

    noreturn = set(SEED) | header_noreturn(path)
    ends_with = {}
    for name, start, body, end in defs:
        callee = last_call(lines, body, end)
        if callee:
            ends_with[name] = (callee, start)

    # Propagate: fatal() ends in rtdexit() ends in exit().  Two rounds
    # is enough for every shape seen so far; loop to a fixed point
    # anyway, it is cheap.
    changed = True
    while changed:
        changed = False
        for name, (callee, _) in ends_with.items():
            if name not in noreturn and callee in noreturn:
                noreturn.add(name)
                changed = True

    hits = []
    for name, (callee, start) in sorted(ends_with.items(), key=lambda x: x[1][1]):
        if name not in noreturn or name in SEED:
            continue
        # main() is called by the runtime and returns an int to it.
        # __attribute__ at column 0 is a declaration's attribute that
        # DEF_RE cannot tell from a definition's name.
        if name == "main" or name.startswith("__attribute"):
            continue
        if declared_noreturn(decls, name):
            continue
        hits.append((start + 1, name, callee))
    return hits


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append", default=[],
                    help="directory prefix under the vendor tree; repeatable")
    ap.add_argument("--root", default=None,
                    help="vendor tree root (default: hbsd/src beside this repo)")
    ap.add_argument("--guards", action="store_true",
                    help="report only functions called from a NULL guard "
                         "whose pointer is used after the block, with the "
                         "count of such sites; a prior, not the answer")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 if anything is reported. Not wired into CI: "
                         "see the module docstring for why")
    args = ap.parse_args()

    here = os.path.dirname(os.path.abspath(__file__))
    root = args.root or os.path.join(here, "..", "..", "hbsd", "src")
    root = os.path.abspath(root)
    scopes = args.scope or [""]

    rows = []
    files = 0
    for sc in scopes:
        base = os.path.join(root, sc)
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = [d for d in dirnames if d not in (".git", "obj")]
            for fn in filenames:
                if not fn.endswith(".c"):
                    continue
                p = os.path.join(dirpath, fn)
                files += 1
                for line, name, callee in scan(p):
                    rel = os.path.relpath(p, root)
                    if args.guards:
                        n = guarded_sites(p, name)
                        if not n:
                            continue
                        rows.append((rel, line, name, callee, n))
                    else:
                        rows.append((rel, line, name, callee))

    total = len(rows)
    rows.sort()
    for row in rows:
        if args.guards:
            rel, line, name, callee, n = row
            print(f"{n:>3}  {rel}:{line}: {name}() ends in {callee}(), "
                  f"not declared noreturn")
        else:
            rel, line, name, callee = row
            print(f"{rel}:{line}: {name}() ends in {callee}(), "
                  f"not declared noreturn")
    print(f"\n{total} function(s) across {files} file(s)", file=sys.stderr)
    return 1 if (args.gate and total) else 0


if __name__ == "__main__":
    sys.exit(main())
