#!/usr/bin/env python3
"""A pointer dereferenced inside the branch that tested it for NULL.

This is not an analysis. It is a grep with a brace matcher, and it looks
for exactly one shape:

	if (p == NULL)		/* or !p, or a && chain containing either */
		... p->field ...

Such a line is either dead code or a guaranteed fault. There is no third
reading, which is what makes the shape worth a dedicated pass: unlike a
path-sensitive finding it needs no premise about reachability, so it can
be run over the whole tree in a second rather than over the translation
units that happen to compile.

The clang sweep does find these -- it is where the shape was noticed --
but only in files it can build, and only when its budget reaches them.
sys/dev/ntb/test/ntb_tool.c's second copy came from a grep, not a
finding.

False positives to expect, and what is done about them:

  - the block reassigns the pointer before using it.  Skipped: a bare
    `p = ' at statement start inside the block disqualifies it.
  - the test is one disjunct of an `||', which proves nothing.  Skipped:
    only a top-level `&&' chain establishes the null.
  - `p' is a macro parameter, so `p->x' is text, not a dereference.
    Not skipped; read the hit.

Calibration. Over hbsd/src at the commit that fixed them, this reports
five sites in sys/dev/{ntb,mpi3mr,firewire} -- ntb_hw_amd.c:697,
ntb_tool.c:532 and :899, mpi3mr_cam.c:1845, sbp.c:2331 -- and after the
fixes it reports nothing, there or anywhere else in sys, lib, bin, sbin,
usr.bin, usr.sbin, libexec, stand and share: 0 sites across 20,324
files. That is what makes it usable as a gate rather than a report.

What it does NOT claim: the three other sbp_action() sites fixed in the
same commit are not of this shape. There `sbp' is merely reachable as
NULL, not proven NULL by the branch, and proving that took reading the
function. This pass is deliberately the narrow half.
"""

import argparse
import os
import re
import sys

IF_RE = re.compile(r'^(\s*)(?:\}\s*else\s+)?if\s*\((.*)$')
# p == NULL / NULL == p / !p, as a whole conjunct
NULLTEST = [
    re.compile(r'^\s*([A-Za-z_]\w*)\s*==\s*NULL\s*$'),
    re.compile(r'^\s*NULL\s*==\s*([A-Za-z_]\w*)\s*$'),
    re.compile(r'^\s*!\s*([A-Za-z_]\w*)\s*$'),
    # tws_intr() writes `if (!(sc))'
    re.compile(r'^\s*!\s*\(\s*([A-Za-z_]\w*)\s*\)\s*$'),
    re.compile(r'^\s*\(\s*([A-Za-z_]\w*)\s*\)\s*==\s*NULL\s*$'),
]


def split_cond(cond):
    """Top-level && / || operands of cond.

    Both connectives are accepted, for the same reason.  With `&&' the body
    is reached only when every operand held, so `p == NULL' there proves p
    is NULL.  With `||' the body is reached when ANY operand held, so there
    is a path into it on which `p == NULL' held -- and a dereference of p
    in the body faults on that path.  `if (!rdma_cxt || !out_params)
    { DP_ERR(p_hwfn->p_dev, ...); }' is wrong exactly as surely as the
    conjunctive form; three of the four ecore sites are written this way.

    A test in the CONDITION rather than the body is a different matter --
    `if (p == NULL || p->x > y)' is safe by short circuit -- and is not
    reached here, because only the body is scanned.
    """
    parts, depth, cur, i = [], 0, [], 0
    while i < len(cond):
        c = cond[i]
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        if depth == 0 and (cond.startswith('&&', i) or
                           cond.startswith('||', i)):
            parts.append(''.join(cur))
            cur = []
            i += 2
            continue
        cur.append(c)
        i += 1
    parts.append(''.join(cur))
    return parts


def condition_of(lines, n):
    """Read the full parenthesised condition starting at lines[n]. Returns
    (cond, index of line after the closing paren, column after it)."""
    m = IF_RE.match(lines[n])
    if not m:
        return None
    depth, buf, i = 0, [], n
    col = lines[n].index('if') + lines[n][lines[n].index('if'):].index('(')
    while i < len(lines):
        line = lines[i]
        start = col if i == n else 0
        for j in range(start, len(line)):
            ch = line[j]
            if ch == '(':
                depth += 1
                if depth == 1:
                    continue
            elif ch == ')':
                depth -= 1
                if depth == 0:
                    return ''.join(buf), i, j + 1
            if depth >= 1:
                buf.append(ch)
        buf.append(' ')
        i += 1
        if i - n > 12:
            return None
    return None


def block_after(lines, i, col):
    """Lines of the guarded block, given the `if' closed at lines[i][col-1]."""
    rest = lines[i][col:]
    out = []
    if rest.strip().startswith('{') or (rest.strip() == '' and
                                        i + 1 < len(lines) and
                                        lines[i + 1].strip().startswith('{')):
        depth = 0
        j = i
        started = False
        while j < len(lines):
            base = col if j == i else 0
            seg = lines[j][base:]
            stop_at = None
            for k, ch in enumerate(seg):
                if ch == '{':
                    depth += 1
                    started = True
                elif ch == '}':
                    depth -= 1
                    # the brace that closes the block may be followed on
                    # the same line by `else', which is NOT in the block
                    if started and depth <= 0:
                        stop_at = base + k
                        break
            if stop_at is not None:
                out.append((j, lines[j][:stop_at]))
                return out
            out.append((j, lines[j]))
            j += 1
            if j - i > 400:
                break
        return out
    # single statement, possibly wrapped
    j = i
    while j < len(lines) and j - i < 12:
        out.append((j, lines[j]))
        seg = lines[j][col:] if j == i else lines[j]
        if ';' in seg:
            return out
        j += 1
    return out


def strip_if0(lines):
    """Blank the body of `#if 0' blocks.  Code the preprocessor throws away
    can hold a fault that would only matter if someone re-enabled it, which
    is worth knowing but is not a defect in the kernel that ships."""
    out = []
    depth = 0          # nesting inside a disabled region
    inner = 0          # nested #if inside a disabled region
    for line in lines:
        st = line.strip()
        if depth == 0:
            if re.match(r'^#\s*if\s+0\s*$', st):
                depth = 1
                out.append('')
                continue
            out.append(line)
            continue
        if re.match(r'^#\s*if', st):
            inner += 1
        elif re.match(r'^#\s*endif', st):
            if inner:
                inner -= 1
            else:
                depth = 0
        out.append('')
    return out


def strip_noise(text):
    """Blank out comments and string literals, keeping line structure, so a
    PBSD comment that quotes the bug is not itself a hit."""
    out = []
    i, n = 0, len(text)
    state = None  # None, 'line', 'block', 'str', 'chr'
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ''
        if state is None:
            if c == '/' and nxt == '*':
                state = 'block'; out.append('  '); i += 2; continue
            if c == '/' and nxt == '/':
                state = 'line'; out.append('  '); i += 2; continue
            if c == '"':
                state = 'str'; out.append(' '); i += 1; continue
            if c == "'":
                state = 'chr'; out.append(' '); i += 1; continue
            out.append(c); i += 1; continue
        if state == 'block':
            if c == '*' and nxt == '/':
                state = None; out.append('  '); i += 2; continue
            out.append(c if c == '\n' else ' '); i += 1; continue
        if state == 'line':
            if c == '\n':
                state = None; out.append('\n'); i += 1; continue
            out.append(' '); i += 1; continue
        # inside a string or char literal
        if c == '\\':
            out.append('  '); i += 2; continue
        if (state == 'str' and c == '"') or (state == 'chr' and c == "'"):
            state = None
        out.append(c if c == '\n' else ' ')
        i += 1
    return ''.join(out)


ALIAS_RE = re.compile(
    r'^\s*(?:const\s+)?(?:struct|union|enum)?\s*[A-Za-z_]\w*\s*\*+\s*'
    r'([A-Za-z_]\w*)\s*=\s*(?:\(\s*(?:const\s+)?(?:struct|union|enum)?'
    r'\s*[A-Za-z_]\w*\s*\*+\s*\)\s*)?([A-Za-z_]\w*)\s*;\s*$')


def alias_map(lines):
    """alias -> base, for `T *alias = (T *)base;' and `T *alias = base;'.

    ecore_roce_create_ud_qp() opens with

        struct ecore_hwfn *p_hwfn = (struct ecore_hwfn *)rdma_cxt;
        ...
        if (!rdma_cxt || !out_params) {
                DP_ERR(p_hwfn->p_dev, ...);

    -- the same shape, one name removed.  Without this the pass reads the
    test and the dereference as being about different pointers, which is
    how four of these went unreported until they were read by hand.

    Reset at every column-0 `}' so an alias does not escape its function.
    """
    out = {}
    per_line = []
    for line in lines:
        if line.startswith('}'):
            out = {}
        # an alias holds only until either name is assigned again:
        # ecore_ooo_add_buffer_to_isle() declares p_prev_isle from p_isle
        # and then reassigns p_isle from the free list
        if out:
            for m in re.finditer(r'(^|[^\w.>=!<+\-*/%&|^])([A-Za-z_]\w*)'
                                 r'\s*=(?!=)', line):
                nm = m.group(2)
                if nm in out or nm in out.values():
                    out = {a: b for a, b in out.items()
                           if a != nm and b != nm}
        m = ALIAS_RE.match(line)
        # `T *p = NULL;' is an initialiser, not an alias
        if m and m.group(1) != m.group(2) and m.group(2) != 'NULL':
            out = dict(out)
            out[m.group(1)] = m.group(2)
        per_line.append(out)
    return per_line


def scan(path, text, raw_lines):
    lines = text.split('\n')
    aliases = alias_map(lines)
    hits = []
    for n, line in enumerate(lines):
        if not IF_RE.match(line):
            continue
        got = condition_of(lines, n)
        if not got:
            continue
        cond, endline, endcol = got
        conj = split_cond(cond)
        if conj is None:
            continue
        names = set()
        for part in conj:
            for r in NULLTEST:
                m = r.match(part)
                if m:
                    names.add(m.group(1))
        if not names:
            continue
        # a pointer aliased to a tested name is the same pointer
        amap = aliases[n]
        for alias, base in amap.items():
            if base in names:
                names.add(alias)
            if alias in names:
                names.add(base)
        block = block_after(lines, endline, endcol)
        if len(block) < 1:
            continue
        for name in names:
            # any assignment to the name, including the very common
            # `if ((p = malloc(n)) == NULL)' nested inside a condition
            reassign = re.compile(r'(^|[^\w.>=!<+\-*/%&|^])' +
                                  re.escape(name) + r'\s*=(?!=)')
            # C macros assign through a bare argument as a matter of course
            # -- TAILQ_FOREACH(p, ...), ELM_MALLOC(p, ...),
            # sctp_alloc_a_chunk(stcb, p) -- and the assignment is not in
            # this file's text at all.  A bare `p' passed to anything
            # disqualifies the block; `p->x' passed to strcmp() does not.
            bare_arg = re.compile(r'[\w\]]\s*\([^()]*?[(,]?\s*' +
                                  re.escape(name) + r'\s*[,)]')
            bare_arg1 = re.compile(r'[\w\]]\s*\(\s*' +
                                   re.escape(name) + r'\s*[,)]')
            # &p passed to anything is the out-parameter idiom
            out_param = re.compile(r'&\s*' + re.escape(name) + r'\s*[,)]')
            # a re-test of p inside the block re-establishes the guard --
            # `if (p == NULL || p->x > y)' is safe by short circuit, and an
            # author who re-tests knows what they are doing
            # any re-test of p in the block re-establishes the guard:
            # an explicit == / != NULL, a `!p |', a short-circuit `p &&',
            # or the ternary `p ? p->x : NULL' that sys/dev/ocs_fc writes
            # at two dozen sites
            # ... and it may be written on any name in the alias group:
            # sli_res_sli_config() tests `buf ?' to guard a read of
            # sli_config, which is buf.
            group = {name}
            for a, b in amap.items():
                if a == name or b == name:
                    group.add(a)
                    group.add(b)
            retest = re.compile('|'.join(
                r'(?<![\w.>])' + re.escape(g) + r'\s*(==|!=)\s*NULL'
                r'|!\s*' + re.escape(g) + r'\s*(\||\))'
                r'|(?<![\w.>])' + re.escape(g) + r'\s*\?'
                r'|(?<![\w.>])' + re.escape(g) + r'\s*&&'
                for g in sorted(group)))
            # not preceded by -> or . : `fc->it[i]' is not a use of `it'
            deref = re.compile(r'(?<![\w.>])(?<!->)\b' + re.escape(name) +
                               r'\s*(->|\[)')
            for (ln, btext) in block:
                seg = btext
                if ln == endline:
                    seg = btext[endcol:]
                if reassign.search(seg) or bare_arg.search(seg) or \
                        bare_arg1.search(seg) or out_param.search(seg) \
                        or retest.search(seg):
                    break
                # sizeof(p[0]) names a type, it does not read memory
                seg = re.sub(r'sizeof\s*\([^()]*\)', ' ', seg)
                if deref.search(seg):
                    hits.append((ln + 1, name,
                                 raw_lines[ln].rstrip()))
                    break
    return hits


def main():
    ap = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default="hbsd/src")
    ap.add_argument("--scope", action="append")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero if any site is found")
    args = ap.parse_args()

    roots = [os.path.join(args.root, s) for s in (args.scope or [""])]
    total = 0
    files = 0
    for root in roots:
        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = [d for d in dirnames if d not in (".git",)]
            for fn in sorted(filenames):
                if not fn.endswith((".c", ".h")):
                    continue
                p = os.path.join(dirpath, fn)
                try:
                    text = open(p, errors="replace").read()
                except OSError:
                    continue
                files += 1
                raw = text.split('\n')
                clean = '\n'.join(strip_if0(strip_noise(text).split('\n')))
                for (ln, name, btext) in scan(p, clean, raw):
                    rel = os.path.relpath(p, args.root)
                    print("%s:%d  %s is NULL here\n      %s" %
                          (rel, ln, name, btext.strip()))
                    total += 1
    print("\n%d site(s) across %d file(s)" % (total, files), file=sys.stderr)
    if args.gate and total:
        print("\nA pointer read inside the branch that tested it for NULL is\n"
              "either dead code or a guaranteed fault. Neither is worth\n"
              "shipping. If the site is genuinely unreachable, say so by\n"
              "deleting it, not by leaving it.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
