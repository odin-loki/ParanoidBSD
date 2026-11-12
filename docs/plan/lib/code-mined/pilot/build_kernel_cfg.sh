#!/bin/bash
# build_kernel_cfg.sh <name> <ARCH> <config-target> [make args...]
#   Extra kernel configurations next to ~/kb/x86 (allyesconfig minus sanitizers, see build_kernel_x86.sh), each in its own out-of-tree dir ~/kb/<name>:
#     x86d   : ARCH=x86_64 defconfig                  (the functions that exist only in the "option off" arm of an #ifdef: stubs such as zswap_swapon)
#     arm64  : ARCH=arm64 allyesconfig, LLVM=1 (clang preprocessor; no cross binutils needed for -E)
#     arm, riscv, powerpc, mips, s390 : same pattern
#   The sanitizer/coverage/werror switches are turned off as for x86.  Everything the label needs is recorded in ~/kb/<name>/CONFIG.txt (config sha256, compiler, make line).
name=$1; arch=$2; target=$3; shift 3
L=${LINUX_SRC:-/home/odin/pbsd-ref/linux}; KO=$HOME/kb/$name
export CPATH=$HOME/kb/inc LIBRARY_PATH=$HOME/kb/lib
mkdir -p "$KO"; cd "$L" || exit 2
M="make O=$KO ARCH=$arch $*"
$M $target >"$KO/config.log" 2>&1 || { echo "config failed"; tail -3 "$KO/config.log"; exit 1; }
C=$KO/.config
for o in KASAN KCSAN KMSAN UBSAN GCOV_KERNEL KCOV WERROR UNWINDER_ORC DEBUG_INFO_BTF MODVERSIONS GCC_PLUGINS KFENCE X86_NATIVE_CPU; do "$L/scripts/config" --file "$C" -d $o; done
$M olddefconfig >>"$KO/config.log" 2>&1
$M -j16 prepare >"$KO/prepare.log" 2>&1; rc=$?
{ echo "name=$name arch=$arch target=$target args=$*"; echo "config_sha256=$(sha256sum "$C" | cut -c1-16)"; echo "prepare_rc=$rc"; echo "kernel=$(make -s kernelversion 2>/dev/null)"; } >"$KO/CONFIG.txt"
cat "$KO/CONFIG.txt"; [ $rc -eq 0 ] || tail -5 "$KO/prepare.log"
