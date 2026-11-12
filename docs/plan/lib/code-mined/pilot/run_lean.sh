#!/bin/bash
# run_lean.sh [P]   Route (c) experiment on the built-but-not-proved scalar units that have no loop, no asm and no pointer parameter (what an integer-model WP cannot do: bit-level reasoning).
# Resumable (units with lean.tsv are skipped), stops on $D/STOP (credit error or USD cap from morph_call.sh). Log: $D/lean.log
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; U=$D/units; P=${1:-4}; LOG=$D/lean.log
say() { echo "$(date '+%F %T') $*" >>"$LOG"; }
for d in "$U"/u*; do
  [ -f "$d/lean.tsv" ] && continue
  st=$(cut -f5 "$d/result.tsv" 2>/dev/null); [ "$st" = FAILED ] || [ "$st" = CONTRACT-ONLY ] || continue
  grep -qE '__asm__|\basm\b' "$d/fn.c" && continue
  awk -f "$HERE/has_loop.awk" "$d/fn.c" && continue
  head -3 "$d/fn.c" | tr '\n' ' ' | sed -E 's/\{.*//' | grep -q '\*' && continue
  echo "$d"
done >"$D/todo.lean"
say "lean experiment: $(wc -l <"$D/todo.lean") units"
xargs -a "$D/todo.lean" -P "$P" -I{} bash -c '[ -f "'"$D"'/STOP" ] || bash "'"$HERE"'/lean_unit.sh" "$0"' {}
say "lean experiment done: $(cat "$U"/u*/lean.tsv 2>/dev/null | cut -f3 | sort | uniq -c | tr '\n' ' ')"
touch "$D/DONE-lean"
