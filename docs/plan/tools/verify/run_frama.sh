#!/bin/bash
# run_frama.sh [max-per-part]   Step 2, second opinion: parse every mined contract with Frama-C's ACSL parser.
# Needs verify/parts/p*.tsv (from run_verify12.sh). Each contract is its own tiny C file (frama_gen.awk), so one crash or one bad stub can never hide other verdicts.
# Scratch files (about 150k tiny files) are in ~/verify-scratch in WSL (regenerable); results are in /mnt/c/ParanoidBSD/mined-data/verify/frama/p<N>.jsonl, merged into
# frama12.jsonl, summary in frama-summary.json. Resumable per part. Optional arg: process only the first N contracts of each part (for a test).
HERE=$(cd "$(dirname "$0")" && pwd); V=${VERIFY_DIR:-/mnt/c/ParanoidBSD/mined-data/verify}; F=$V/frama; S=${SCRATCH_DIR:-$HOME/verify-scratch}; MAXP=${1:-0}; mkdir -p "$F" "$S"
log() { echo "$(date '+%F %T') $*" | tee -a "$F/frama.log"; }
cp "$HERE/frama_prelude.h" "$S/prelude.h"
one_part() {
  n=$1; T=$S/p$n
  [ -f "$F/p$n.done" ] && return
  rm -rf "$T"; mkdir -p "$T"
  awk -F'\t' '$8!=""' "$V/parts/p$n.tsv" | { [ "$MAXP" -gt 0 ] && head -n "$MAXP" || cat; } > "$T/in.tsv"
  gawk -f "$HERE/frama_gen.awk" "$T/in.tsv" > "$T/blocks.txt" 2> "$T/gen.err"
  # split the blocks into c_N.c / c_N.fb.c and write meta.tsv (N key idx name)
  awk -v dir="$T" 'function flush() { if (n == "") return; print "#include \"../prelude.h\"\n" real > (dir "/c_" n ".c"); close(dir "/c_" n ".c")
                                                           print "#include \"../prelude.h\"\n" fb > (dir "/c_" n ".fb.c"); close(dir "/c_" n ".fb.c") }
       /^\/\/==== / { flush(); split(substr($0, 8), a, "\t"); n = a[1]; print a[1] "\t" a[2] "\t" a[3] "\t" a[4] > (dir "/meta.tsv"); real = ""; fb = ""; mode = 1; next }
       /^\/\/---- fallback/ { mode = 2; fbdecl = ""; next }
       { if (mode == 1) { real = real $0 "\n"; decl[n] = decl[n] ((($0 ~ /^\/\*@/) ? "" : $0 "\n")) } else { fb = decl[n] $0 "\n" } }
       END { flush() }' "$T/blocks.txt"
  total=$(wc -l < "$T/meta.tsv")
  seq 1 "$total" | xargs -P 4 -I{} bash "$HERE/frama_one.sh" "$T" {} > "$T/results.tsv" 2> "$T/run.err"
  # join results with meta -> jsonl
  awk -F'\t' 'NR == FNR { meta[$1] = $2 "\t" $3 "\t" $4; next }
              { split(meta[$1], m, "\t"); gsub(/\\/, "\\\\", $4); gsub(/"/, "\\\"", $4); gsub(/\\/, "\\\\", m[3])
                printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"frama\":\"%s\",\"sig\":\"%s\",\"msg\":\"%s\"}\n", m[1], m[2], m[3], $2, $3, $4 }' "$T/meta.tsv" "$T/results.tsv" > "$F/p$n.jsonl"
  echo "part $n: $(wc -l < "$F/p$n.jsonl")/$total contracts" >> "$F/frama.log"
  touch "$F/p$n.done"
}
log "frama-c: $(frama-c -version | head -1); max per part: $MAXP"
for n in $(seq 0 15); do one_part "$n" & done
wait
cat "$F"/p*.jsonl > "$V/frama12.jsonl"
log "merged: $(wc -l < "$V/frama12.jsonl") contracts parsed by Frama-C"
jq -s '{contracts: length, frama: (group_by(.frama)|map({(.[0].frama): length})|add), signature: (group_by(.sig)|map({(.[0].sig): length})|add),
        top_fail_messages: ([.[]|select(.frama=="fail")|.msg|gsub("[0-9]+";"N")|.[0:50]]|group_by(.)|map({m:.[0],n:length})|sort_by(-.n)|.[0:10])}' "$V/frama12.jsonl" > "$V/frama-summary.json"
tee -a "$F/frama.log" < "$V/frama-summary.json"
touch "$V/DONE-frama"
