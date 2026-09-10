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

Two ways of predicting which is which were tried and both were dropped,
because both over-predicted against the one batch whose answer was
known.  Ranking by findings within 40 lines after a call site scored
that batch 18; the true answer was 0.  Ranking by the guard shape above
-- the call alone in an `if' body, a name from the condition used after
the block -- scored it 34 sites across 11 functions; the true answer was
still 0, because `if (argc < 2) usage();' is that shape and an int
argc produces no finding.

So this reports and does not rank.  The oracle for whether marking a
function pays is a before/after sweep at the same scope on the same
tree, which is cheap, and no heuristic here beat it.
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

# A definition's opening: a name at column 0 followed by a parameter list.
# KNF puts the return type on the line above, so the name really is at
# column 0 and that is what makes this findable without a parser.
DEF_RE = re.compile(r"^([A-Za-z_]\w*)\s*\(")

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
    last = None
    for k in range(body_start + 1, end):
        s = lines[k]
        if not s.strip() and not cur:
            continue
        cur.append(s)
        depth += s.count("{") - s.count("}")
        joined = " ".join(x.strip() for x in cur).strip()
        if depth <= 0 and (joined.endswith(";") or joined.endswith("}")):
            last = joined
            cur = []
            depth = 0
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
    return _HDR_CACHE[d]


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

    noreturn = set(SEED)
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
                    rows.append((rel, line, name, callee))

    total = len(rows)
    rows.sort()
    for rel, line, name, callee in rows:
        print(f"{rel}:{line}: {name}() ends in {callee}(), "
              f"not declared noreturn")
    print(f"\n{total} function(s) across {files} file(s)", file=sys.stderr)
    return 1 if (args.gate and total) else 0


if __name__ == "__main__":
    sys.exit(main())
