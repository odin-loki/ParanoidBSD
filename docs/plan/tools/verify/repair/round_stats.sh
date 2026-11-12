#!/bin/bash
# round_stats.sh <round>   What round N did, from final/state.tsv (run state.sh first) and repair<N>/: functions attempted (their record was repaired), and their outcome.
# "fixed" = fixed-agree (passes every check AND the DeepSeek review agrees). The stopping rule: a round that fixes under 2% of the functions it attempted
# (and under 2% of the functions that were still bad when it started) ends the loop. Output: repair<N>/stats.json
N=${1:?round}; D=/mnt/c/ParanoidBSD/mined-data; F=$D/final; RP=$D/repair$N
# keys repaired in this round
jq -r '.key' "$RP/repair.jsonl" | sort -u > "$RP/stats.keys"
awk -F'\t' -v n="$N" 'FILENAME == ARGV[1] { rk[$1] = 1; next } FILENAME == ARGV[2] { if ($1 in rk) att[$1 SUBSEP $5] = 1; todo++; next }
     { q = $1 SUBSEP $2; if (q in att) { a++; if ($6 == n) c[$5]++; else c["no result (the repair reply omitted it, or an earlier round still decides)"]++ } }
     END { printf "{\"round\": %d, \"bad_at_start\": %d, \"attempted\": %d", n, todo, a; for (s in c) printf ", \"%s\": %d", s, c[s]; print "}" }' "$RP/stats.keys" "$RP/funcs.tsv" "$F/state.tsv" |
  jq --argjson bad "$(wc -l < "$RP/funcs.tsv")" '. + {fixed: (.["fixed-agree"] // 0), fixed_pct_of_attempted: (100 * (.["fixed-agree"] // 0) / .attempted), fixed_pct_of_bad_at_start: (100 * (.["fixed-agree"] // 0) / .bad_at_start)}' > "$RP/stats.json"
cat "$RP/stats.json"
