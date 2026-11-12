#!/bin/bash
# cl_review_todo.sh <result.jsonl> <todo.tsv>   Build the step-4 reviewer todo list (format of verify/s4todo.tsv) from manual-format results: one line per record,
# "key<TAB>file<TAB>start<TAB>end<TAB>name ¦ prose ¦ contract ¶ name ¦ ...", only action "fix" entries, (key,i) deduplicated (last wins). file/start/end from verify/rows.tsv.
RES=${1:?result.jsonl}; OUT=${2:?todo.tsv}; D=/mnt/c/ParanoidBSD/mined-data
T=$OUT.work; mkdir -p "$T"
jq -r -s 'group_by([.key, .i]) | map(.[-1]) | .[] | select(.action == "fix") | [.key, (.i|tostring), (.name // "" | gsub("[\t\n\r]";" ")), (.contract // "" | gsub("[\t\n\r]";" ")), (.prose // "" | gsub("[\t\n\r]";" "))] | join("\t")' "$RES" > "$T/fixes.tsv"
cut -f1 "$T/fixes.tsv" | sort -u > "$T/keys.txt"
awk -F'\t' 'FILENAME == ARGV[1] { k[$1] = 1; next } ($1 in k) && !($1 in seen) { seen[$1] = 1; print $1 "\t" $2 "\t" $3 "\t" $4 }' "$T/keys.txt" "$D/verify/rows.tsv" > "$T/bounds.tsv"
awk -F'\t' 'FILENAME == ARGV[1] { b[$1] = $2 "\t" $3 "\t" $4; next }
     ($1 in b) { if ($1 != cur) { if (cur != "") print line; cur = $1; line = $1 "\t" b[$1] "\t"; first = 1 }
                 line = line (first ? "" : " ¶ ") $3 " ¦ " $5 " ¦ " $4; first = 0 }
     END { if (cur != "") print line }' "$T/bounds.tsv" "$T/fixes.tsv" > "$OUT"
rm -f "$T"/fixes.tsv "$T"/keys.txt "$T"/bounds.tsv; rmdir "$T"
echo "review todo: $(wc -l < "$OUT") records"
