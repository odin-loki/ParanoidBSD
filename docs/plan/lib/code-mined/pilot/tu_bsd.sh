#!/bin/bash
# tu_bsd.sh <tree> <srcfile> <out.i>   Preprocess a source file of HBSD/PBSD (tree bsd-kernel | bsd-user) or of the Linux tools/ directory (tree linux-tools).
# There is no configured build tree for these, so the flags are the ones the build would pass, reconstructed:
#   bsd-kernel: -D_KERNEL -D__FreeBSD__ -nostdinc -I$S/sys (machine/ -> sys/amd64/include, x86/ -> sys/x86/include through a stub directory) plus
#               generated headers that the kernel build makes: opt_*.h (EMPTY: every kernel option off), *_if.h (made with makeobjops.awk from the .m file)
#   bsd-user:   -nostdinc -I<dir of the file and 3 parents> -I$S/include -I$S/sys, same machine/ stub, host gcc builtin headers (stddef/stdarg) are NOT used
#   linux-tools: -I tools/include -I tools/arch/x86/include -I tools/include/uapi (+ the file's directory) and the host glibc headers
# Every stub that had to be invented is listed in <out.i>.stubs (-> assumption "stub-headers:..." in the label).  Exit 0 ok, 2 preprocessor error (stderr in <out.i>.err).
tree=$1; f=$2; o=$3
mkdir -p "$(dirname "$o")"
case "$f" in
  */hbsd/src/*) S=${f%%/hbsd/src/*}/hbsd/src ;;
  */pbsd/*) S=${f%%/pbsd/*}/hbsd/src ;;   # PBSD files are compiled against the HBSD headers
esac
L=/home/odin/pbsd-ref/linux
STUB=${BSDSTUB:-$HOME/kb/bsdstub}; mkdir -p "$STUB"
D=$STUB/$(printf '%s' "$f" | sha256sum | cut -c1-12); rm -rf "$D"; mkdir -p "$D"   # per-file stub dir (opt_*.h, *_if.h)
if [ "$tree" != linux-tools ]; then ln -sfn "$S/sys/amd64/include" "$STUB/machine"; ln -sfn "$S/sys/x86/include" "$STUB/x86"; fi
dirs=""; d=$(dirname "$f"); for i in 0 1 2 3; do dirs="$dirs -I$d"; d=$(dirname "$d"); done
case "$tree" in
  bsd-kernel) flags="-nostdinc -D_KERNEL -D__FreeBSD__=15 -D__FreeBSD_version=1500000 -DAMD64 -D__amd64__ -D__x86_64__ -DNO_SYSCTL_DESCR -ffreestanding -fwrapv -std=gnu99 -I$D -I$STUB -I$S/sys -I$S/sys/contrib/ck/include -I$S/sys/contrib/libfdt -I$S/sys/contrib/ncsw/inc -I$S/sys/contrib/openzfs/include -I$S/sys/contrib/dev -I$S/sys/dev $dirs" ;;
  bsd-user)   flags="-nostdinc -D__FreeBSD__=15 -D__FreeBSD_version=1500000 -D__amd64__ -D__x86_64__ -std=gnu99 -I$D -I$STUB $dirs -I$S/include -I$S/sys -I$S/lib/libc/include" ;;
  linux-tools) flags="-D_GNU_SOURCE -I$D $dirs -I$L/tools/include -I$L/tools/arch/x86/include -I$L/tools/include/uapi -I$L/tools/lib -I$L/tools/perf -I$L/tools/testing/selftests" ;;
esac
: >"$o.stubs"
run() { gcc -E -dD -w -x c $flags -o "$o" "$f" 2>"$o.err"; }
for round in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
  run && break
  miss=$(grep -m1 -oE "fatal error: [^ :]+: No such file" "$o.err" | sed -E 's/fatal error: ([^ :]+): .*/\1/')
  [ -z "$miss" ] && break
  bn=$(basename "$miss")
  case "$bn" in
    opt_*.h|offset.inc|*.inc) : >"$D/$bn"; echo "empty:$bn" >>"$o.stubs" ;;
    *_if.h)  # 13 Nov 2026: one cached index of every .m file (a find over the whole tree on the Windows file share, run per unit, took up to 30 minutes each under load)
             IDX=$HOME/kb/bsd_m_index.txt
             if [ ! -s "$IDX" ]; then ( flock 8; [ -s "$IDX" ] || { find "$S/sys" "$S" -maxdepth 6 -name '*.m' 2>/dev/null >"$IDX.part.$$" && mv "$IDX.part.$$" "$IDX"; } ) 8>"$IDX.lock"; fi
             m=$(grep -m1 "/${bn%.h}\.m\$" "$IDX" 2>/dev/null)
             if [ -n "$m" ]; then (cd "$D" && awk -f "$S/sys/tools/makeobjops.awk" -- -h "$m" >/dev/null 2>&1); echo "generated:$bn from ${m#$S/}" >>"$o.stubs"; [ -f "$D/$bn" ] || { echo "makeobjops failed for $bn" >>"$o.err"; break; }
             else break; fi ;;
    *)  # a header the real build would find in an installed include directory (userland: /usr/include = include/ + sys/sys/ + libc private ...): alias by basename
        a=""; for dd in "$S/include" "$S/sys/sys" "$S/lib/libc/include" "$S/lib/msun/src" "$S/lib/libsys" "$S/lib/libc/stdlib" "$S/lib/libc"; do [ -f "$dd/$bn" ] && { a="$dd/$bn"; break; }; done
        [ -z "$a" ] && break
        mkdir -p "$D/$(dirname "$miss")"; ln -sf "$a" "$D/$miss"; echo "alias:$miss->${a#$S/}" >>"$o.stubs" ;;
  esac
done
[ -s "$o" ] && ! grep -q "fatal error" "$o.err" || exit 2
echo "flags_from=$tree mode=reconstructed" >"$o.how"
exit 0
