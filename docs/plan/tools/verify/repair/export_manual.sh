#!/bin/bash
# export_manual.sh [funcs-per-packet=40]   Manual queue: every function that is still failing after the automated rounds, in packets of about 40 functions for a human or a Claude agent.
# Run state.sh first (final/state.tsv). Included statuses: still-rejected, fixed-disagree, dropped (check whether it is really not a function), fixed-unreviewed (review call failed),
# rejected-unrepaired of kinds other than driver-glue (repair call failed). rejected-unrepaired driver-glue (never attempted: out of budget/time) goes to packets-lowprio/.
# A packet is a jsonl file; one line per RECORD (a source slice) with the failing functions in it:
#   {"key","file","start","end","source":"<lines start-end>","functions":[{"i","name","kind","status","prose","contract","reasons":"<every recorded failure, by round>","history":"<status per round>"}]}
# Output: mined-data/manual/packets/packet-NNNN.jsonl, packets-lowprio/..., manual/index.tsv (packet, records, functions) and manual/functions.tsv (key i name kind status packet).
# Repo copy: docs/plan/lib/code-mined/manual/{index.tsv,functions.tsv.gz,README.md}. Result format and ingest: ingest_manual.sh.
PER=${1:-40}; D=/mnt/c/ParanoidBSD/mined-data; F=$D/final; M=$D/manual; R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/manual
mkdir -p "$M/work" "$R"; T=$(printf '\t')
# current annotation of each function: the latest rows file that has it
ROWS="$D/verify/rows.tsv"; k=1; while true; do if [ $k -eq 1 ]; then W=$D/verify-repair; else W=$D/verify-repair$k; fi; [ -s "$W/rows.tsv" ] || break; ROWS="$ROWS $W/rows.tsv"; k=$((k+1)); done
awk -F'\t' '$5 == "still-rejected" || $5 == "fixed-disagree" || $5 == "dropped" || $5 == "fixed-unreviewed" || $5 == "rejected-unrepaired" { print $1 "\t" $2 "\t" $3 "\t" $4 "\t" $5 "\t" (($5 == "rejected-unrepaired" && $4 == "driver-glue") ? "low" : "main") }' "$F/state.tsv" > "$M/work/sel.tsv"
[ -n "$LIMIT" ] && { head -n "$LIMIT" "$M/work/sel.tsv" > "$M/work/sel.lim" && mv "$M/work/sel.lim" "$M/work/sel.tsv"; }   # LIMIT=N: test run on the first N
# history: original reason + every round's status and reason
awk -F'\t' 'FILENAME == ARGV[1] { h[$1 SUBSEP $2] = "original rejection: " $3; next }
            { q = $1 SUBSEP $2; h[q] = h[q] " || round " $3 " " $4 (($5 != "") ? ": " $5 : "") }
            END { for (q in h) { split(q, a, SUBSEP); print a[1] "\t" a[2] "\t" h[q] } }' "$F/origrepair/problems.tsv" "$F/ev/events.tsv" > "$M/work/hist.tsv"
awk -F'\t' -v OFS='\t' 'FILENAME == ARGV[1] { s[$1 SUBSEP $2] = $0; next } FILENAME == ARGV[2] { h[$1 SUBSEP $2] = $3; next }
     { q = $1 SUBSEP $5; if (q in s) row[q] = $2 "\t" $3 "\t" $4 "\t" $9 "\t" $8 }
     END { for (q in s) { split(s[q], x, "\t"); split(q, a, SUBSEP); split(row[q], r, "\t"); gsub(/[\t\n]/, " ", h[q])
             print a[1], a[2], x[3], x[4], x[5], x[6], r[1], r[2], r[3], r[4], r[5], h[q] } }' "$M/work/sel.tsv" "$M/work/hist.tsv" $ROWS 2>/dev/null |
  sort -s -t "$T" -k6,6 -k1,1 -k2,2n > "$M/work/funcs.tsv"
# columns: key i name kind status prio file start end prose contract history   (history = reasons by round; the latest status is in column 5)
echo "selected functions: $(wc -l < "$M/work/funcs.tsv")"
rm -rf "$M/packets" "$M/packets-lowprio"; mkdir -p "$M/packets" "$M/packets-lowprio"; : > "$M/index.tsv"; : > "$M/functions.tsv"
for prio in main low; do
  dir=$M/packets; [ $prio = low ] && dir=$M/packets-lowprio
  awk -F'\t' -v p=$prio '$6 == p' "$M/work/funcs.tsv" > "$M/work/f_$prio.tsv"
  # group consecutive records into packets of about PER functions (never split a record)
  awk -F'\t' -v per=$PER -v dir="$dir" -v prio=$prio 'function flush() { if (nrec) { printf "%s\t%d\t%d\n", pk, nrec, nf > (dir "/../index.tsv.part"); } }
     { if ($1 != cur) { if (nf >= per || pk == "") { flush(); n++; pk = sprintf("%s%04d", (prio == "low" ? "L" : "P"), n); nrec = 0; nf = 0 } cur = $1; nrec++ }
       nf++; print $0 "\t" pk > (dir "/../work/assign_" prio ".tsv") } END { flush() }' "$M/work/f_$prio.tsv"
  cat "$M/index.tsv.part" >> "$M/index.tsv" 2>/dev/null; rm -f "$M/index.tsv.part"
  # write the packets
  cut -f1,7,8,9,13 "$M/work/assign_$prio.tsv" | sort -u -s -t "$T" -k5,5 -k1,1 | while IFS="$T" read -r key file s e pk; do
    src=$(sed -n "${s},${e}p" "$file" 2>/dev/null | jq -Rs .)
    awk -F'\t' -v k="$key" '$1 == k' "$M/work/assign_$prio.tsv" |
      jq -R -s -c --arg key "$key" --arg file "$file" --argjson s "$s" --argjson e "$e" --argjson src "$src" '{key:$key, file:$file, start:$s, end:$e, source:$src, functions: (split("\n") | map(select(. != "") | split("\t") | {i:(.[1]|tonumber), name:.[2], kind:.[3], status:.[4], prose:.[9], contract:.[10], reasons:.[11], history:.[4]}))}' >> "$dir/packet-$pk.jsonl"
  done
done
cut -f1,2,3,4,5,13 "$M/work/assign_main.tsv" "$M/work/assign_low.tsv" > "$M/functions.tsv" 2>/dev/null
sort -u "$M/index.tsv" -o "$M/index.tsv"
cp "$M/index.tsv" "$R/index.tsv"; gzip -9 -c "$M/functions.tsv" > "$R/functions.tsv.gz"
echo "packets (main): $(ls "$M/packets" | wc -l), low priority: $(ls "$M/packets-lowprio" | wc -l); functions main $(wc -l < "$M/work/f_main.tsv"), low $(wc -l < "$M/work/f_low.tsv")"
