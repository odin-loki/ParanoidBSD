#!/bin/bash
# dsv_chunk.sh <line-number> <out.jsonl>   Fix the functions of ONE packet record (line N of $D/work.jsonl) with morph-dsv41flash-1017 ($0.12 in / $0.80 out per 1M).
# Output lines are in the manual result format {"key","i","name","action","prose","contract","note"}. Side files next to out.jsonl (under one flock): ledger.txt, failed.txt, errors.log, done.keys.
. ~/.pbsd/env
N=$1; OUT=$2; D=$(dirname "$OUT"); HERE=$(cd "$(dirname "$0")" && pwd)
[ -f "$D/STOP" ] && exit 0
REC=$(sed -n "${N}{p;q}" "$D/work.jsonl"); [ -z "$REC" ] && exit 0
KEY=$(printf '%s' "$REC" | jq -r .key)
REQ=$(printf '%s' "$REC" | python3 "$HERE/dsv_prompt.py") || { ( flock 9; echo "$KEY prompt build failed" >> "$D/errors.log"; echo "$N" >> "$D/failed.txt" ) 9> "$OUT.lock"; exit 0; }
for attempt in 1 2 3; do
  RESP=$(printf '%s' "$REQ" | curl -sS --max-time 600 https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' -d @- 2>&1)
  if echo "$RESP" | jq -e '.usage.prompt_tokens' > /dev/null 2>&1; then
    ( flock 9; echo "$(echo "$RESP" | jq -r '.usage.prompt_tokens // 0') $(echo "$RESP" | jq -r '.usage.completion_tokens // 0')" >> "$D/ledger.txt" ) 9> "$OUT.lock"
  fi
  TXT=$(echo "$RESP" | jq -r '.choices[0].message.content // empty' 2>/dev/null | sed -e 's/^```json//' -e 's/^```//' -e 's/```$//')
  if [ -n "$TXT" ] && echo "$TXT" | jq -e '.functions|type=="array"' > /dev/null 2>&1; then
    # keep only idx values that are in the record; convert to the manual result format; a function the model skipped is simply absent (stays in the queue)
    JL=$(printf '%s' "$REC" | jq -c --argjson r "$TXT" '. as $p | ($p.functions | map(.i)) as $ok | $r.functions[] | select(.idx as $x | $ok | index($x)) | select(.action == "fix" or .action == "drop")
          | {key:$p.key, i:.idx, name:(.name // ""), action:.action, prose:(.prose_contract // ""), contract:(.contract // ""), note:((.note // "") + " [dsv41flash]")}' 2>/dev/null)
    if [ -n "$JL" ]; then ( flock 9; printf '%s\n' "$JL" >> "$OUT"; printf '%s\n' "$KEY" >> "$D/done.keys" ) 9> "$OUT.lock"; exit 0; fi
  fi
  ( flock 9; echo "$(date +%F_%T) $KEY attempt $attempt: $(echo "$RESP" | head -c 250 | tr '\n' ' ')" >> "$D/errors.log" ) 9> "$OUT.lock"
  sleep $((attempt*8))
done
( flock 9; echo "$N" >> "$D/failed.txt" ) 9> "$OUT.lock"
exit 0
