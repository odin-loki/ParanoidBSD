#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Set up a local PBSD development environment on Linux or WSL, and PROVE
# each piece works rather than reporting that a package installed.
#
# Everything in this repository except the disk-image targets runs here:
# the model checking, the static analyser, every gate, and a cross-build of
# world and kernel. makefs and mkimg want a FreeBSD host, so images stay in
# CI - but nothing else has to.
#
# Usage:  sh tools/setup_local.sh [--install] [--check]
#   --install   apt-get the packages (needs sudo)
#   --check     verify each tool actually does its job (default)
set -eu

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DO_INSTALL=0
DO_CHECK=1
for a in "$@"; do
    case "$a" in
    --install) DO_INSTALL=1 ;;
    --check)   DO_CHECK=1 ;;
    *) echo "usage: $0 [--install] [--check]" >&2; exit 2 ;;
    esac
done

PKGS="cbmc clang clang-tools llvm python3 git bmake flex bison libarchive-dev \
build-essential gcc"

if [ "$DO_INSTALL" = 1 ]; then
    echo "== installing"
    sudo apt-get update -qq
    sudo apt-get install -y $PKGS
fi

fail=0
say() { printf '  %-4s %s\n' "$1" "$2"; [ "$1" = FAIL ] && fail=1; return 0; }

echo "== tools present"
for t in cbmc goto-cc goto-instrument clang clang++ gcc python3 bmake; do
    if command -v "$t" >/dev/null 2>&1; then
        say "ok" "$t  ($("$t" --version 2>&1 | head -1 | cut -c1-52))"
    else
        say FAIL "$t is missing - run with --install"
    fi
done

[ "$DO_CHECK" = 1 ] || exit "$fail"

echo
echo "== each tool does its job, on a planted defect"

# CBMC must find a signed overflow it is pointed at. If this passes when it
# should fail, every later 'PROVED' is worthless - which is the failure mode
# this whole file exists to rule out.
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
cat > "$T/ov.c" <<'C'
int bad(int x){ return x + 1; }
C
if cbmc "$T/ov.c" --function bad --signed-overflow-check 2>&1 |
        grep -q "VERIFICATION FAILED"; then
    say "ok" "cbmc reports the planted signed overflow"
else
    say FAIL "cbmc did NOT report a planted signed overflow"
fi

# The static analyser, same idea.
if [ -f "$ROOT/tools/verify/test_analyze.py" ]; then
    if python3 "$ROOT/tools/verify/test_analyze.py" >/dev/null 2>&1; then
        say "ok" "clang --analyze finds all four planted defects"
    else
        say FAIL "clang --analyze missed a planted defect"
    fi
fi

# UBSan is the second opinion every finding in docs/security/UB_FINDINGS.md
# was confirmed with, so it has to actually abort.
cat > "$T/ub.c" <<'C'
#include <stdio.h>
int main(void){ int s = 1; printf("%d\n", s << 31); return 0; }
C
if gcc -O0 -fsanitize=undefined -fno-sanitize-recover=all "$T/ub.c" \
        -o "$T/ub" 2>/dev/null && ! "$T/ub" >/dev/null 2>&1; then
    say "ok" "UBSan traps 1 << 31 and exits non-zero"
else
    # gcc's ubsan runtime is sometimes absent where clang's is not, and
    # vice versa; say which so it can be fixed rather than guessed at.
    say FAIL "UBSan did not trap 1 << 31 (try: apt install libubsan1)"
fi

echo
echo "== the tree's own headers resolve (no glibc)"
if python3 - "$ROOT" <<'PY'
import subprocess, sys
from pathlib import Path
root = Path(sys.argv[1])
sys.path.insert(0, str(root / "tools" / "verify"))
from includes import include_flags, lang_flags, SRC
ok = 0
tests = ["lib/libc/string/strcat.c", "lib/msun/src/e_exp.c",
         "sys/libkern/strlen.c"]
for t in tests:
    f = SRC / t
    if not f.is_file():
        continue
    p = subprocess.run(["clang", "-fsyntax-only", "-Wno-everything",
                        *lang_flags(f), *include_flags(f), str(f)],
                       capture_output=True, text=True)
    ok += p.returncode == 0
sys.exit(0 if ok == len(tests) else 1)
PY
then
    say "ok" "userland and kernel sources compile against hbsd/src headers"
else
    say FAIL "the include shim is not resolving - see tools/verify/includes.py"
fi

echo
# A Windows drive under WSL goes through drvfs, and these gates read every
# one of the ~24,000 files in hbsd/src. On /mnt/c that is minutes per gate
# with no output, which looks exactly like a hang; on ext4 it is seconds.
# OneDrive on top of it can be worse again, because it may hydrate files on
# access.
case "$ROOT" in
/mnt/*)
    echo "!! $ROOT is a Windows drive mounted through WSL."
    echo "!! File I/O here is 10-50x slower than the WSL filesystem, and"
    echo "!! every check below reads the whole tree. Clone into ~ instead:"
    echo "!!     git clone https://github.com/odin-loki/ParanoidBSD ~/ParanoidBSD"
    echo
    ;;
esac

echo "== the repository's own gates"
for g in "check_exec_bits.py" "check_source_includes.py --gate" \
         "check_libc_srcs.py --gate" "check_port_symbols.py --gate" \
         "check_port_cxx_warnings.py --gate"; do
    # Announce BEFORE running. Each of these can take a while on a slow
    # filesystem and silence is indistinguishable from a hang.
    printf '  ..   tools/%s\r' "$g"
    # shellcheck disable=SC2086
    if (cd "$ROOT" && python3 tools/$g >/dev/null 2>&1); then
        say "ok" "tools/$g            "
    else
        say FAIL "tools/$g            "
    fi
done

echo
if [ "$fail" = 0 ]; then
    cat <<'DONE'
everything works. What you can now do locally:

  model check            python3 tools/verify/classify.py --scope lib/libc \
                             --out /tmp/classes.json
                         python3 tools/verify/cbmc_driver.py --scope lib/libc \
                             --classes /tmp/classes.json --out /tmp/ub.jsonl
  static analyse         python3 tools/verify/analyze.py --scope lib/libc \
                             --out /tmp/an.jsonl
  cross-build the world  see docs/LOCAL_DEV.md - tools/build/make.py, and it
                         is the same tree CI builds

Disk images (makefs, mkimg) still want a FreeBSD host; that is the one
thing that stays in CI.
DONE
else
    echo "FAILURES above. Nothing that reports a number should be trusted"
    echo "until they are fixed - a check that cannot run reports zero."
fi
exit "$fail"
