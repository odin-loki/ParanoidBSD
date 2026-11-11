#!/bin/bash
# tb_build.sh <count> [seed]   Tiebreak packets for step 4: a random sample of functions DeepSeek judged DISAGREE, one text file per function with everything a
# reviewer needs (the source slice, the GLM annotation, DeepSeek's reason). Written to mined-data/verify/tiebreak/packets/NNN.txt plus index.tsv.
V=/mnt/c/ParanoidBSD/mined-data/verify; T=$V/tiebreak; mkdir -p "$T/packets"; COUNT=${1:-60}; SEED=${2:-5}
jq -r '.key as $k | .verdicts[] | select(.verdict=="disagree") | [$k, .name, .reason] | @tsv' "$V/xcheck/xcheck.jsonl" | shuf -n "$COUNT" --random-source=<(yes "$SEED") > "$T/sample.tsv"
awk -F'\t' 'FILENAME == ARGV[1] { want[$1] = 1; next } ($1 in want) { print }' "$T/sample.tsv" "$V/s4todo.tsv" > "$T/records.tsv"
: > "$T/index.tsv"; n=0
while IFS=$'\t' read -r key fname reason; do
  n=$((n+1)); P=$(printf '%s/packets/%03d.txt' "$T" "$n")
  line=$(awk -F'\t' -v k="$key" '$1 == k' "$T/records.tsv" | head -1)
  file=$(printf '%s' "$line" | cut -f2); s=$(printf '%s' "$line" | cut -f3); e=$(printf '%s' "$line" | cut -f4); funcs=$(printf '%s' "$line" | cut -f5-)
  ann=$(printf '%s' "$funcs" | awk -v f="$fname" 'BEGIN{RS=" ¶ "; FS=" ¦ "} $1 == f {printf "prose contract: %s\nformal contract: %s\n", $2, ($3 == "" ? "(none)" : $3)}')
  {
    echo "PACKET $n"
    echo "record: $key"
    echo "function to judge: $fname"
    echo
    echo "=== GLM annotation of $fname"
    echo "$ann"
    echo
    echo "=== DeepSeek's objection (it answered DISAGREE)"
    echo "$reason"
    echo
    echo "=== source, lines $s-$e of ${file#/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/}"
    sed -n "${s},${e}p" "$file" | awk -v s="$s" '{ printf "%5d  %s\n", s + NR - 1, $0 }'
  } > "$P"
  printf '%03d\t%s\t%s\n' "$n" "$key" "$fname" >> "$T/index.tsv"
done < "$T/sample.tsv"
echo "packets: $n in $T/packets"
