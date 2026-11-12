#!/bin/bash
# export_manual.sh [funcs-per-packet=40]   Manual queue: every function that is still failing after the automated rounds, in packets of about 40 functions for a human or a Claude agent.
# Run state.sh first (final/state.tsv). Included statuses: still-rejected, fixed-disagree, dropped (check whether it is really not a function), fixed-unreviewed (review call failed),
# rejected-unrepaired of kinds other than driver-glue (repair call failed). rejected-unrepaired driver-glue (never attempted: out of budget/time) goes to packets-lowprio/.
# A packet is a jsonl file; one line per RECORD (a source slice) with the failing functions in it:
#   {"key","file","start","end","source":"<lines start-end>","functions":[{"i","name","kind","status","prose","contract","reasons":"<every recorded failure, by round>","history":"<status per round>"}]}
# Output: mined-data/manual/packets/packet-NNNN.jsonl, packets-lowprio/..., manual/index.tsv (packet, records, functions) and manual/functions.tsv (key i name kind status packet).
# Repo copy: docs/plan/lib/code-mined/manual/{index.tsv,functions.tsv.gz,README.md}. Result format and ingest: ingest_manual.sh.
. "$(dirname "$0")/lc.sh"; PER=${1:-40}; D=/mnt/c/ParanoidBSD/mined-data; F=$D/final; M=${M:-$D/manual}; R=${R:-/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/manual}
WK=${EXPORT_WORK:-$HOME/kb/scratch/export-work}; mkdir -p "$WK" "$M/work" "$R"; T=$(printf '\t')
# current annotation of each function: the latest rows file that has it
ROWS="$(lc "$D/verify/rows.tsv")"; k=1; while true; do if [ $k -eq 1 ]; then W=$D/verify-repair; else W=$D/verify-repair$k; fi; [ -s "$W/rows.tsv" ] || break; ROWS="$ROWS $(lc "$W/rows.tsv")"; k=$((k+1)); done
awk -F'\t' '$5 == "still-rejected" || $5 == "fixed-disagree" || $5 == "dropped" || $5 == "fixed-unreviewed" || $5 == "rejected-unrepaired" { print $1 "\t" $2 "\t" $3 "\t" $4 "\t" $5 "\t" (($5 == "rejected-unrepaired" && $4 == "driver-glue") ? "low" : "main") }' "$(state_tsv)" > "$WK/sel.tsv"
[ -n "$LIMIT" ] && { head -n "$LIMIT" "$WK/sel.tsv" > "$WK/sel.lim" && mv "$WK/sel.lim" "$WK/sel.tsv"; }   # LIMIT=N: test run on the first N
# history: original reason + every round's status and reason
awk -F'\t' 'FILENAME == ARGV[1] { h[$1 SUBSEP $2] = "original rejection: " $3; next }
            { q = $1 SUBSEP $2; h[q] = h[q] " || round " $3 " " $4 (($5 != "") ? ": " $5 : "") }
            END { for (q in h) { split(q, a, SUBSEP); print a[1] "\t" a[2] "\t" h[q] } }' "$(lc "$F/origrepair/problems.tsv")" "$F/ev/events.tsv" > "$WK/hist.tsv"
awk -F'\t' -v OFS='\t' 'FILENAME == ARGV[1] { s[$1 SUBSEP $2] = $0; next } FILENAME == ARGV[2] { h[$1 SUBSEP $2] = $3; next }
     { q = $1 SUBSEP $5; if (q in s) row[q] = $2 "\t" $3 "\t" $4 "\t" $9 "\t" $8 }
     END { for (q in s) { split(s[q], x, "\t"); split(q, a, SUBSEP); split(row[q], r, "\t"); gsub(/[\t\n]/, " ", h[q])
             print a[1], a[2], x[3], x[4], x[5], x[6], r[1], r[2], r[3], r[4], r[5], h[q] } }' "$WK/sel.tsv" "$WK/hist.tsv" $ROWS 2>/dev/null |
  sort -s -t "$T" -k6,6 -k1,1 -k2,2n > "$WK/funcs.tsv"
# columns: key i name kind status prio file start end prose contract history   (history = reasons by round; the latest status is in column 5)
echo "selected functions: $(wc -l < "$WK/funcs.tsv")"
# everything below is built on WSL-local ext4 ($WK); only packets whose content changed are copied to /mnt/c (sync_dir), so 763 unchanged packets are not rewritten
rm -rf "$WK/packets" "$WK/packets-lowprio"; mkdir -p "$WK/packets" "$WK/packets-lowprio"; : > "$WK/index.tsv"
for prio in main low; do
  dir=$WK/packets; [ $prio = low ] && dir=$WK/packets-lowprio
  awk -F'\t' -v p=$prio '$6 == p' "$WK/funcs.tsv" > "$WK/f_$prio.tsv"
  # group consecutive records into packets of about PER functions (never split a record)
  awk -F'\t' -v per=$PER -v prio=$prio -v idx="$WK/index.tsv.part" -v asg="$WK/assign_$prio.tsv" 'function flush() { if (nrec) { printf "%s\t%d\t%d\n", pk, nrec, nf > idx; } }
     { if ($1 != cur) { if (nf >= per || pk == "") { flush(); n++; pk = sprintf("%s%04d", (prio == "low" ? "L" : "P"), n); nrec = 0; nf = 0 } cur = $1; nrec++ }
       nf++; print $0 "\t" pk > asg } END { flush() }' "$WK/f_$prio.tsv"
  [ -f "$WK/assign_$prio.tsv" ] || : > "$WK/assign_$prio.tsv"
  cat "$WK/index.tsv.part" >> "$WK/index.tsv" 2>/dev/null; rm -f "$WK/index.tsv.part"
  [ $prio = low ] && [ -z "$LOW" ] && { echo "low priority (never attempted, driver-glue): assignments listed in functions.tsv, packets not written (LOW=1 writes them)"; continue; }
  # write the packets: one awk (source lines + function lines of every record, one record after the other) and ONE jq per packet; the old script started sed, jq, awk, jq for every record (about 100,000 processes)
  rm -rf "$WK/pk_$prio"; mkdir -p "$WK/pk_$prio"
  awk -F"	" -v d="$WK/pk_$prio" '{ f = d "/" $13 ".tsv"; if (f != prev) { if (prev != "") close(prev); prev = f } print >> f }' "$WK/assign_$prio.tsv"
  # per packet (16 in parallel): stream per record = the record's function lines, a line with \002, the source lines start..end (exactly what sed -n 's,ep' prints, incl. a missing final newline), then \001 and a newline.
  # awk runs with LC_ALL=C (bytes, no multibyte warnings); jq does the JSON escaping exactly as the old per-record `jq -Rs` did.
  cat > "$WK/pk.awk" <<'AWK'
function load(f,   c, n) { if (f == cf) return; cf = f; N = 0; endnl = 1; delete L; RS = "\001"; c = ""; if ((getline c < f) > 0) { n = split(c, L, "\n"); if (substr(c, length(c)) == "\n") N = n - 1; else { N = n; endnl = 0 } } close(f); RS = "\n" }
function flush(   i, e2) { if (cur == "") return; printf "%s", fl; printf "\002\n"; load(cf0); e2 = (e0 < N) ? e0 : N; for (i = s0; i <= e2; i++) printf "%s%s", L[i], ((i == N && !endnl) ? "" : "\n"); printf "\001\n" }
{ if ($1 != cur) { flush(); cur = $1; cf0 = $7; s0 = $8 + 0; e0 = $9 + 0; fl = "" } fl = fl $0 "\n" } END { flush() }
AWK
  cat > "$WK/pk.jq" <<'JQ'
split("\u0001\n") | map(select(. != "")) | .[] | split("\u0002\n") as $sp | $sp[0] as $h | ($sp[1:] | join("\u0002\n")) as $src
  | ($h | split("\n") | map(select(. != "") | split("\t"))) as $F
  | {key: $F[0][0], file: $F[0][6], start: ($F[0][7] | tonumber), end: ($F[0][8] | tonumber), source: $src,
     functions: ($F | map({i: (.[1] | tonumber), name: .[2], kind: .[3], status: .[4], prose: .[9], contract: .[10], reasons: .[11]}))}
JQ
  ls "$WK/pk_$prio" | sed 's/\.tsv$//' | xargs -P "${EXPORT_PAR:-16}" -I{} bash -c 'LC_ALL=C awk -F"\t" -f "$1/pk.awk" "$1/pk_$2/$3.tsv" | jq -R -s -c -f "$1/pk.jq" > "$4/packet-$3.jsonl"' _ "$WK" "$prio" {} "$dir"
done
cut -f1,2,3,4,5,13 "$WK/assign_main.tsv" "$WK/assign_low.tsv" > "$WK/functions.tsv" 2>/dev/null
sort -u "$WK/index.tsv" -o "$WK/index.tsv"
# publish: copy only what differs from the previously published local copy ($WK/pub) and was not touched on /mnt/c since; remove packets that no longer exist
sync_dir() {   # sync_dir <built-dir> <target-dir> <published-copy-dir>
  local b=$1 t=$2 p=$3 f n=0 x
  mkdir -p "$t" "$p"
  for f in "$b"/*; do [ -e "$f" ] || continue; x=$(basename "$f")
    if [ -f "$t/$x" ] && [ -f "$p/$x" ] && [ "$(stat -c %s "$t/$x")" = "$(stat -c %s "$f")" ] && cmp -s "$f" "$p/$x"; then continue; fi
    cp "$f" "$t/$x.part" && mv "$t/$x.part" "$t/$x" && cp "$f" "$p/$x"; n=$((n+1))
  done
  for f in "$t"/*; do [ -e "$f" ] || continue; x=$(basename "$f"); [ -e "$b/$x" ] || { rm -f "$f" "$p/$x"; n=$((n+1)); }; done
  echo "  $t: $n files written/removed"
}
sync_dir "$WK/packets" "$M/packets" "$WK/pub/packets"; sync_dir "$WK/packets-lowprio" "$M/packets-lowprio" "$WK/pub/packets-lowprio"
for f in index.tsv functions.tsv; do cp "$WK/$f" "$M/$f.part" && mv "$M/$f.part" "$M/$f"; done
cp "$WK/funcs.tsv" "$M/work/funcs.tsv.part" && mv "$M/work/funcs.tsv.part" "$M/work/funcs.tsv"
cp "$WK/index.tsv" "$R/index.tsv"; gzip -9 -c "$WK/functions.tsv" > "$R/functions.tsv.gz"
echo "packets (main): $(ls "$M/packets" | wc -l), low priority: $(ls "$M/packets-lowprio" | wc -l); functions main $(wc -l < "$WK/f_main.tsv"), low $(wc -l < "$WK/f_low.tsv")"
