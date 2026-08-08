#!/bin/sh
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}; CXX=${CXX:-c++}; CFLAGS=${CFLAGS:--O2}; CXXFLAGS=${CXXFLAGS:--O2}
MODNAME=pbsd.bin.cp.b0191s2
WRAP_FLAGS="-Wl,--wrap=exit -Wl,--wrap=sysconf -Wl,--wrap=malloc -Wl,--wrap=read -Wl,--wrap=write -Wl,--wrap=copy_file_range -Wl,--wrap=close -Wl,--wrap=open -Wl,--wrap=openat -Wl,--wrap=fstat -Wl,--wrap=fstatat -Wl,--wrap=stat -Wl,--wrap=lstat -Wl,--wrap=readlink -Wl,--wrap=unlinkat -Wl,--wrap=linkat -Wl,--wrap=symlinkat -Wl,--wrap=mkfifoat -Wl,--wrap=mknodat -Wl,--wrap=mkdir -Wl,--wrap=mkdirat -Wl,--wrap=rmdir -Wl,--wrap=futimens -Wl,--wrap=utimensat -Wl,--wrap=fchown -Wl,--wrap=fchownat -Wl,--wrap=fchmod -Wl,--wrap=fchmodat -Wl,--wrap=fchflags -Wl,--wrap=chflagsat -Wl,--wrap=fpathconf -Wl,--wrap=acl_get_fd_np -Wl,--wrap=acl_is_trivial_np -Wl,--wrap=acl_set_fd_np -Wl,--wrap=acl_free -Wl,--wrap=getchar -Wl,--wrap=signal -Wl,--wrap=fts_open -Wl,--wrap=fts_read -Wl,--wrap=fts_set -Wl,--wrap=fts_close -Wl,--wrap=asprintf"
rm -rf gcm.cache; rm -f oracle.o port.o harness.o port.pcm harness
$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o
if $CXX --version 2>&1 | grep -qi clang; then
  $CXX -std=c++23 $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
  $CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
  $CXX -std=c++23 $CXXFLAGS -fmodule-file=$MODNAME=port.pcm -c harness.cpp -o harness.o
else
  $CXX -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ port.cppm -o port.o
  $CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi
$CXX -std=c++23 $CXXFLAGS $WRAP_FLAGS -o harness harness.o port.o oracle.o -lbsd
exec ./harness
