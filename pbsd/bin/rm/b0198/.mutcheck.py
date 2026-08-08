#!/usr/bin/env python3
"""Local mutation check for the b0198 harness (scratch tool, not part of the batch)."""
import os, shutil, subprocess, sys, tempfile

SRC = "/home/odin/pbsd/pbsd/bin/rm/b0198"

MUTANTS = [
    ("slash_cmp0",   54, "(a)[0] == '/'", "(a)[0] != '/'"),
    ("slash_cmp1",   54, "(a)[1] == '\\0'", "(a)[1] != '\\0'"),
    ("slash_andor",  54, "'/' && (a)[1]", "'/' || (a)[1]"),
    ("slash_compl0", 61, "complained = 0", "complained = 1"),
    ("slash_incdec", 64, "!complained++", "!complained--"),
    ("slash_eval",   66, "eval = 1", "eval = 0"),
    ("slash_unull",  67, "u[0] != NULL", "u[0] == NULL"),
    ("slash_uinc",   67, "++u", "--u"),
    ("slash_shift",  68, "u[0] = u[1]", "u[1] = u[0]"),
    ("slash_tinc",   70, "++t", "--t"),

    ("c2_iinc",      86, "++i", "--i"),
    ("c2_lstat",     87, "== 0", "!= 0"),
    ("c2_dinc",      89, "++dcount", "--dcount"),
    ("c2_finc",      92, "++fcount", "--fcount"),
    ("c2_fcinit",    81, "fcount = 0", "fcount = 1"),
    ("c2_dcinit",    82, "dcount = 0", "dcount = 1"),
    ("c2_first0",    96, "first = 0", "first = 1"),
    ("c2_whileor",   97, "'n' && first", "'n' || first"),
    ("c2_whileor2",  97, "'y' && first", "'y' || first"),
    ("c2_dandr",     98, "dcount && rflag", "dcount || rflag"),
    ("c2_deq1",     100, "dcount == 1", "dcount != 1"),
    ("c2_feq1",     104, "fcount == 1", "fcount != 1"),
    ("c2_fgt1",     106, "fcount > 1", "fcount < 1"),
    ("c2_plus",     108, "dcount + fcount > 3", "dcount - fcount > 3"),
    ("c2_gt3",      108, "> 3", "< 3"),
    ("c2_three",    108, "> 3", "> 2"),
    ("c2_plus2",    109, "dcount + fcount)", "dcount - fcount)"),
    ("c2_ret1",     111, "return(1)", "return(0)"),
    ("c2_innor",    117, "'\\n' && ch", "'\\n' || ch"),
    ("c2_eofcmp",   119, "ch == EOF", "ch != EOF"),
    ("c2_retand",   122, "'y' || first", "'y' && first"),
    ("c2_rety",     122, "== 'y'", "== 'Y'"),

    ("dot_cmp0",    125, "(a)[0] == '.'", "(a)[0] != '.'"),
    ("dot_andor",   125, "'.' && (!(a)[1]", "'.' || (!(a)[1]"),
    ("dot_oror",    125, "!(a)[1] ||", "!(a)[1] &&"),
    ("dot_cmp1",    125, "(a)[1] == '.'", "(a)[1] != '.'"),
    ("dot_not2",    125, "&& !(a)[2]", "&& (a)[2]"),
    ("dot_andor2",  125, "'.' && !(a)[2]", "'.' || !(a)[2]"),
    ("dot_compl0",  132, "complained = 0", "complained = 1"),
    ("dot_strrchr", 134, "!= NULL", "== NULL"),
    ("dot_pinc",    135, "++p", "--p"),
    ("dot_incdec",  139, "!complained++", "!complained--"),
    ("dot_eval",    141, "eval = 1", "eval = 0"),
    ("dot_shiftne", 142, "!= NULL", "== NULL"),
    ("dot_shift",   142, "t[0] = t[1]", "t[1] = t[0]"),
    ("dot_tinc",    142, "++t", "--t"),
    ("dot_tinc2",   146, "++t", "--t"),
    ("dot_save",    144, "t = save", "save = t"),

    ("sig_one",     154, "info = 1", "info = 0"),
]


def main():
    work = tempfile.mkdtemp(prefix="b0198mut_")
    for f in ("oracle.c", "port.cppm", "harness.cpp", "build.sh"):
        shutil.copy(os.path.join(SRC, f), os.path.join(work, f))
    baseline = open(os.path.join(SRC, "port.cppm")).read().split("\n")

    def run():
        try:
            r = subprocess.run(["sh", "build.sh"], cwd=work,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, timeout=600)
            return r.returncode, r.stdout.decode("utf-8", "replace")
        except subprocess.TimeoutExpired:
            return "timeout", ""

    rc, out = run()
    print("baseline rc=%s" % rc)
    if rc != 0:
        print(out[-4000:])
        return 1

    killed, survived, nocompile = [], [], []
    for (tag, ln, old, new) in MUTANTS:
        lines = list(baseline)
        idx = ln - 1
        if old not in lines[idx]:
            print("%-14s SETUP-ERROR line %d: %r" % (tag, ln, lines[idx]))
            survived.append(tag + "(setup)")
            continue
        lines[idx] = lines[idx].replace(old, new, 1)
        open(os.path.join(work, "port.cppm"), "w").write("\n".join(lines))
        rc, out = run()
        if rc == "timeout":
            print("%-14s KILLED (hang)" % tag)
            killed.append(tag)
        elif "differential test vs oracle" not in out:
            print("%-14s did-not-compile" % tag)
            nocompile.append(tag)
        elif rc != 0:
            print("%-14s KILLED rc=%s" % (tag, rc))
            killed.append(tag)
        else:
            print("%-14s *** SURVIVED ***" % tag)
            survived.append(tag)

    print("\nkilled=%d survived=%d did-not-compile=%d (workdir %s)" %
          (len(killed), len(survived), len(nocompile), work))
    if nocompile:
        print("NOCOMPILE: " + ", ".join(nocompile))
    if survived:
        print("SURVIVORS: " + ", ".join(survived))
    shutil.rmtree(work, ignore_errors=True)
    return 0 if not survived else 1


if __name__ == "__main__":
    sys.exit(main())
