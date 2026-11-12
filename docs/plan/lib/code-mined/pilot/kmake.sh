#!/bin/bash
# kmake.sh <make args>   make in the Linux reference tree with out-of-tree build dir ~/kb/x86 (allyesconfig minus sanitizers, see kconfig_x86.md).
# libelf headers/lib are taken from ~/kb/inc and ~/kb/lib (objtool needs them; no root available): see build_kernel_x86.sh.
export CPATH=$HOME/kb/inc LIBRARY_PATH=$HOME/kb/lib
cd "${LINUX_SRC:-/home/odin/pbsd-ref/linux}" || exit 2
exec make O="${KO:-$HOME/kb/x86}" ARCH=x86_64 "$@"
