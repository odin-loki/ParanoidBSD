#!/bin/bash
# run_verify12.sh   Steps 1 and 2 of VERIFICATION_PLAN.md over every mined function, 16 gawk workers in parallel.
# Reads  /mnt/c/ParanoidBSD/mined-data/tier{B,C}.jsonl (+ retry/retry.jsonl if present), de-duplicated by record key (the first occurrence wins).
# Writes /mnt/c/ParanoidBSD/mined-data/verify/step12.jsonl (one line per function, see verify12.awk) and summary.json.
# Resumable per partition; all data on the Windows disk (not /tmp).
HERE=$(cd "$(dirname "$0")" && pwd); D=/mnt/c/ParanoidBSD/mined-data; V=${VERIFY_DIR:-$D/verify}; mkdir -p "$V/parts"; N=16
log() { echo "$(date '+%F %T') $*" | tee -a "$V/verify.log"; }
if [ ! -s "$V/rows.tsv" ]; then
  log "flattening records to rows (one jq pass per file)"
  fi=0
  for f in "$D/tierB.jsonl" "$D/tierC.jsonl" "$D/retry/retry.jsonl"; do
    fi=$((fi+1)); [ -s "$f" ] || continue
    # field 10 = "<file index>:<line number>" of the record, used to keep only the first record of a key
    jq -r --arg fi "$fi" '. as $r | input_line_number as $ln | $r.result.functions | to_entries[] | [$r.key, $r.file, ($r.start|tostring), ($r.end|tostring), (.key|tostring),
           (.value.name // "" | gsub("[\t\n\r]";" ")), (.value.kind // "" | gsub("[\t\n\r]";" ")),
           (.value.contract // "" | gsub("[\t\n\r]";" ")), (.value.prose_contract // "" | gsub("[\t\n\r]";" ")), ($fi + ":" + ($ln|tostring))] | join("\t")' "$f"
  done | awk -F'\t' '{ if (!($1 in owner)) owner[$1] = $10; if (owner[$1] == $10) { NF = 9; print } }' OFS='\t' > "$V/rows.tsv.part"
  mv "$V/rows.tsv.part" "$V/rows.tsv"
  log "rows: $(wc -l < "$V/rows.tsv")"
fi
if [ ! -f "$V/parts.done" ]; then
  log "partitioning rows by file into $N parts, sorted by file and start line"
  rm -f "$V"/parts/p*.tsv
  awk -F'\t' -v n=$N -v dir="$V/parts" '{ if (!($2 in id)) id[$2] = c++ % n; print > (dir "/p" id[$2] ".tsv") }' "$V/rows.tsv"
  for i in $(seq 0 $((N-1))); do
    [ -f "$V/parts/p$i.tsv" ] && sort -t "$(printf '\t')" -k2,2 -k3,3n -s "$V/parts/p$i.tsv" -o "$V/parts/p$i.tsv"
  done
  touch "$V/parts.done"
fi
log "running $N workers"
for i in $(seq 0 $((N-1))); do
  [ -f "$V/parts/p$i.tsv" ] || continue
  [ -f "$V/parts/p$i.out.done" ] && continue
  ( gawk -f "$HERE/verify12.awk" "$V/parts/p$i.tsv" > "$V/parts/p$i.out" 2> "$V/parts/p$i.err" && touch "$V/parts/p$i.out.done" ) &
done
wait
cat "$V"/parts/p*.out > "$V/step12.jsonl"
log "steps 1+2 done: $(wc -l < "$V/step12.jsonl") functions tagged; worker errors: $(cat "$V"/parts/p*.err | wc -l)"
jq -s '{functions: length,
  with_contract: ([.[]|select(.has_contract)]|length),
  s1: (group_by(.s1)|map({(.[0].s1): length})|add),
  s2: (group_by(.s2)|map({(.[0].s2): length})|add),
  s1_fail_with_contract: ([.[]|select(.has_contract and .s1=="fail")]|length),
  s2_fail: ([.[]|select(.s2=="fail")]|length)}' "$V/step12.jsonl" > "$V/summary.json"
tee -a "$V/verify.log" < "$V/summary.json"
touch "$V/DONE12"
