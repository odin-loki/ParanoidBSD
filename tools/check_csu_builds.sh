#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Compile lib/csu's crt1_c.c for every architecture, on Linux, with no
# cross toolchain and no FreeBSD sysroot.
#
# crt1_c.c is where a process begins. It is also the one file per
# architecture that no PBSD job builds: the architecture matrix builds
# kernels for all six, and the only world anyone builds is amd64. So four
# of the seven had been unable to compile at all - they still had
#
#     #include "ignore_init.c"
#
# and that file is in neither this tree nor the HardenedBSD tree it was
# imported from. The helpers it used to define moved into libc as
# __libc_start1(); amd64, aarch64 and riscv were converted and arm, i386,
# powerpc and powerpc64 were left behind, half-converted in i386's case
# (its crt1_s.S already calls __libc_start1 while its crt1_c.c still
# defined the _start1 that call replaced).
#
# What makes this checkable here is that crt1_c.c needs almost nothing: a
# handful of macros from <sys/cdefs.h>, the __libc_start1 prototypes, and
# the tree's own lib/csu/common/csu_common.h. The shim below supplies the
# first two and the tree supplies the third, so clang -target is enough.
# That is a narrower check than a real build - it does not link, and the
# shim's <sys/cdefs.h> is not the tree's - and it is exactly wide enough
# to catch a missing include, an undeclared callee or a signature that
# does not match its crt1_s.S.
#
# Each file is compiled three ways, because the GCRT and PIC paths are
# #ifdef'd and would otherwise never be looked at.
#
# The shim also carries do-nothing <stdlib.h>, <stdint.h>, <sys/param.h>,
# <sys/elf.h> and <sys/elf_common.h>. Not because a converted crt1_c.c
# needs them - it does not - but so that an UNCONVERTED one gets past its
# standard includes and the error this check reports is the real one. The
# first version of this script had no such stubs and reported
# "'stdlib.h' file not found" for three of the four broken files, which
# would have sent a reader looking for a sysroot instead of at the
# missing ignore_init.c one line further down.

set -e

ROOT="${1:-$(cd "$(dirname "$0")/.." && pwd)}"
SRC="$ROOT/hbsd/src"
CC="${CC:-clang}"

if ! command -v "$CC" >/dev/null 2>&1; then
    echo "$CC not found; skipping the csu compile check."
    exit 0
fi

SHIM="$(mktemp -d)"
trap 'rm -rf "$SHIM"' EXIT
mkdir -p "$SHIM/sys"

cat > "$SHIM/sys/cdefs.h" <<'EOF'
#ifndef _CSU_SHIM_CDEFS_H_
#define _CSU_SHIM_CDEFS_H_
#define __dead2         __attribute__((__noreturn__))
#define __unused        __attribute__((__unused__))
#define __weak_symbol   __attribute__((__weak__))
#define __hidden        __attribute__((__visibility__("hidden")))
#define __BEGIN_DECLS
#define __END_DECLS
#endif
EOF

# The two entry points crt1_c.c is allowed to call, spelled as
# lib/libc/include/libc_private.h spells them.
cat > "$SHIM/libc_private.h" <<'EOF'
#ifndef _CSU_SHIM_LIBC_PRIVATE_H_
#define _CSU_SHIM_LIBC_PRIVATE_H_
#include <sys/cdefs.h>
void __libc_start1(int, char *[], char *[], void (*)(void),
    int (*)(int, char *[], char *[])) __dead2;
void __libc_start1_gcrt(int, char *[], char *[], void (*)(void),
    int (*)(int, char *[], char *[]), int *, int *) __dead2;
#endif
EOF

# Enough of each standard header to get past the #include and no more.
cat > "$SHIM/stdlib.h" <<'EOF'
#ifndef _CSU_SHIM_STDLIB_H_
#define _CSU_SHIM_STDLIB_H_
#include <sys/cdefs.h>
int atexit(void (*)(void));
void exit(int) __dead2;
#endif
EOF
cat > "$SHIM/stdint.h" <<'EOF'
#ifndef _CSU_SHIM_STDINT_H_
#define _CSU_SHIM_STDINT_H_
typedef unsigned int uint32_t;
#endif
EOF
cat > "$SHIM/sys/param.h" <<'EOF'
#include <sys/cdefs.h>
EOF
: > "$SHIM/sys/elf_common.h"
cat > "$SHIM/sys/elf.h" <<'EOF'
#ifndef _CSU_SHIM_ELF_H_
#define _CSU_SHIM_ELF_H_
typedef struct { long a_type; union { long a_val; } a_un; } Elf_Auxinfo;
#define AT_NULL   0
#define AT_HWCAP  25
#define AT_HWCAP2 26
#endif
EOF

TARGETS="amd64:x86_64-unknown-freebsd15.1
aarch64:aarch64-unknown-freebsd15.1
arm:armv7-unknown-freebsd15.1-gnueabihf
i386:i386-unknown-freebsd15.1
powerpc:powerpc-unknown-freebsd15.1
powerpc64:powerpc64-unknown-freebsd15.1
riscv:riscv64-unknown-freebsd15.1"

fail=0
for entry in $TARGETS; do
    arch="${entry%%:*}"
    triple="${entry##*:}"
    f="$SRC/lib/csu/$arch/crt1_c.c"
    if [ ! -f "$f" ]; then
        echo "FAIL $arch: no lib/csu/$arch/crt1_c.c"
        fail=1
        continue
    fi
    for variant in "default:" "gcrt:-DGCRT" "pic:-DPIC -fPIC"; do
        name="${variant%%:*}"
        flags="${variant#*:}"
        # shellcheck disable=SC2086
        if out=$("$CC" -target "$triple" -nostdinc -ffreestanding \
                 -std=gnu17 -Wall -Wextra -Werror \
                 -I "$SHIM" \
                 -I "$SRC/lib/csu/common" \
                 -I "$SRC/lib/csu/$arch" \
                 $flags -S -o /dev/null "$f" 2>&1) && [ -z "$out" ]; then
            printf '  ok   %-10s %s\n' "$arch" "$name"
        else
            printf '  FAIL %-10s %s\n' "$arch" "$name"
            echo "$out" | head -6 | sed 's/^/         /'
            fail=1
        fi
    done
done

if [ "$fail" = 0 ]; then
    echo
    echo "every architecture's crt1_c.c compiles for its own target."
fi
exit "$fail"
