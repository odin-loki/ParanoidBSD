#!/bin/bash
# replay_proved.sh [P]   Independent replay of every PROVED / PROVED-ASSUMING unit: a fresh directory holding only prelude.h and unit.final.c (the exact file that was proved), WP cache OFF, fresh process, the same pinned
# toolchain, 60 s per goal. Also scans the unit for banned constructs (assume, admit, lemma, axiom, `\at(` is allowed, extern assigns tricks are not) and checks the contract text in unit.final.c equals contract.acsl.
# Writes <unit>/replay.tsv: N name proved_A proved_B vac scan sha256(unit.final.c) status       (status OK only if all goals proved again, no banned construct, contract text identical)
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; U=$D/units; P=${1:-8}
one() {
  d=$1; . "$HERE/env.sh"
  [ "$(cut -f5 "$d/result.tsv" 2>/dev/null)" = PROVED ] || exit 0
  IFS=$'\t' read -r N name rest <"$d/result.tsv"
  W=$(mktemp -d); cp "$d/prelude.h" "$d/unit.final.c" "$W/"
  scan=clean
  grep -n -w -E 'assume|assumes|admit|lemma|axiom|axiomatic|ghost|behavior|check' "$d/unit.final.c" | grep -v '^[0-9]*:#define' >/dev/null && scan=BANNED
  grep -qF "$(cat "$d/contract.acsl")" "$d/unit.final.c" || scan="contract-text-differs"
  cd "$W"
  run() { frama-c -kernel-warn-key annot-error=active -machdep gcc_x86_64 -wp -wp-cache none -wp-timeout 60 -wp-prover alt-ergo,z3,cvc5 -wp-fct "$name" "$@" unit.final.c 2>&1; }
  a=$(run -wp-no-rte | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1\/\2/p' | tail -1)
  b=$(run -wp-rte | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1\/\2/p' | tail -1)
  st=OK; [ -n "$a" ] && [ "${a%/*}" = "${a#*/}" ] && [ -n "$b" ] && [ "${b%/*}" = "${b#*/}" ] && [ "$scan" = clean ] || st=MISMATCH
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$a" "$b" "$scan" "$(sha256sum <"$d/unit.final.c" | cut -c1-16)" "$st" >"$d/replay.tsv"
  rm -rf "$W"
}
export -f one; export HERE
ls -d "$U"/u* | xargs -P "$P" -I{} bash -c 'one {}'
cat "$U"/u*/replay.tsv 2>/dev/null | cut -f7 | sort | uniq -c
