#!/bin/bash
# build_kernel_x86.sh   Configure + prepare the Linux reference tree (out of tree, ~/kb/x86) so that any .c file can be preprocessed with its real flags.
# Config: x86_64 allyesconfig MINUS sanitizers/coverage/werror/ORC (they change flags, not the C that was mined), -march=x86-64 (no host CPU macros).
# Assumption list for labels: see kconfig_x86.md.  Needs libelf headers (objtool): ~/kb/inc/{gelf,libelf,nlist}.h and ~/kb/lib/libelf.so (no root on this PC).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
L=${LINUX_SRC:-/home/odin/pbsd-ref/linux}; KO=${KO:-$HOME/kb/x86}
mkdir -p "$KO" "$HOME/kb/inc" "$HOME/kb/lib"
[ -f "$HOME/kb/inc/gelf.h" ] || { d=$(mktemp -d); (cd "$d" && apt-get download libelf-dev && dpkg -x libelf-dev*.deb x && cp x/usr/include/{gelf,libelf,nlist}.h "$HOME/kb/inc/"); rm -rf "$d"; }
ln -sf /usr/lib/x86_64-linux-gnu/libelf.so.1 "$HOME/kb/lib/libelf.so"
bash "$HERE/kmake.sh" allyesconfig
C=$KO/.config
for o in KASAN KCSAN KMSAN UBSAN GCOV_KERNEL KCOV WERROR UNWINDER_ORC DEBUG_INFO_BTF MODVERSIONS GCC_PLUGINS KFENCE X86_NATIVE_CPU; do "$L/scripts/config" --file "$C" -d $o; done
"$L/scripts/config" --file "$C" -e UNWINDER_FRAME_POINTER -e GENERIC_CPU
bash "$HERE/kmake.sh" olddefconfig
bash "$HERE/kmake.sh" -j16 prepare
grep -E '^CONFIG_(KASAN|UBSAN|KCOV|WERROR|X86_NATIVE_CPU|GENERIC_CPU)=' "$C" || true
sha256sum "$C" | cut -c1-16
