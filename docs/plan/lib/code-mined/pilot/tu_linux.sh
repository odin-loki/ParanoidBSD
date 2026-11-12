#!/bin/bash
# tu_linux.sh <srcfile> <out.i>   Preprocess a Linux source file (.c or .h) of /home/odin/pbsd-ref/linux with the flags kbuild itself uses (x86_64, ~/kb/x86 config),
# keeping the macro definitions (-dD).
#   .c : its own kbuild command line.
#   .h : the preprocessed unit of a .c file that really includes the header (same directory first, then parents, then subdirectories), so the header is seen in the
#        context of a real user (its includes, its -D flags);  if no .c includes it (include/linux/*.h and the like), a probe unit that includes the usual kernel
#        headers (module.h kernel.h slab.h device.h) and then the header, with the flags of lib/bitmap.c (or of a .c file of the directory).
# Writes <out.i> and <out.i>.how (flags file, mode).  Exit 0 ok, 1 no flags, 2 preprocessor error (stderr in <out.i>.err).
HERE=$(cd "$(dirname "$0")" && pwd)
L=/home/odin/pbsd-ref/linux; KO=${KO:-$HOME/kb/x86}
f=$1; o=$2; rel=${f#$L/}
mkdir -p "$(dirname "$o")"
mode=c; base=$f; probe=0
if [ "${f##*.}" = h ]; then
  bn=$(basename "$f"); base=""
  d=$(dirname "$rel")
  # a .c file that includes this header: same dir, parent dirs (up to 3 levels), then one level of subdirectories
  cand=""
  dd=$d
  for lvl in 0 1 2 3; do
    [ "$dd" = "." ] && break
    cand=$(grep -l -E "^[[:space:]]*#[[:space:]]*include[[:space:]]*[\"<][^\">]*$bn[\">]" "$L/$dd"/*.c 2>/dev/null | head -1)
    [ -n "$cand" ] && break
    dd=$(dirname "$dd")
  done
  if [ -z "$cand" ]; then cand=$(grep -l -E "^[[:space:]]*#[[:space:]]*include[[:space:]]*[\"<][^\">]*$bn[\">]" "$L/$d"/*/*.c 2>/dev/null | head -1); fi
  if [ -n "$cand" ]; then base=$cand; mode="includer:${cand#$L/}"
  else
    probe=1; mode=probe
    # flags: a .c file of the directory (or ancestor), else lib/bitmap.c
    dd=$d
    while [ "$dd" != "." ] && [ -z "$base" ]; do base=$(ls "$L/$dd"/*.c 2>/dev/null | head -1); [ -z "$base" ] && dd=$(dirname "$dd"); done
    [ -n "$base" ] || base=$L/lib/bitmap.c
  fi
fi
[ -n "$base" ] || exit 1
brel=${base#$L/}; stem=${brel%.c}
cmdf=$KO/$(dirname "$brel")/.$(basename "$stem").i.cmd
if [ ! -f "$cmdf" ]; then flock "$HOME/kb/kmake.lock" bash "$HERE/kmake.sh" "$stem.i" >"$o.make.log" 2>&1; fi
[ -f "$cmdf" ] || { echo "kbuild produced no flags for $brel (see $o.make.log)" >"$o.err"; exit 1; }
cmd=$(sed -n '1s/^savedcmd_[^ ]* := //p' "$cmdf")
cmd=$(printf '%s' "$cmd" | sed -E 's/ -Wp,-MMD,[^ ]*//; s/ -o [^ ]+ [^ ]+\.c *$//')
src=$base
if [ $probe = 1 ]; then
  src=$o.probe.c
  printf '#include <linux/module.h>\n#include <linux/kernel.h>\n#include <linux/slab.h>\n#include <linux/device.h>\n#include "%s"\n' "$f" >"$src"
fi
(cd "$KO" && eval "$cmd -dD -w -o '$o' '$src'") 2>"$o.err"
rc=$?
[ $rc -ne 0 ] && exit 2
echo "flags_from=$brel mode=$mode probe=$probe" >"$o.how"
[ $probe = 1 ] && echo "extra=+" >>"$o.how"
exit 0
