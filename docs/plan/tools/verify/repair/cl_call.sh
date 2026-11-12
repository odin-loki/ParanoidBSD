#!/bin/bash
# cl_call.sh <kind: repair|review> <input> <N> <model> <outdir>   One request (line N of the input, see cl_lib.sh) through the normal synchronous Messages API.
# Appends result lines to <outdir>/res.jsonl; side files (under one flock): ledger.txt ("model rt in out"), errors.log, failed.txt, done.keys. STOP file honoured.
KIND=$1; IN=$2; N=$3; MODEL=$4; OD=$5; HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/cl_lib.sh"
[ -f "$OD/STOP" ] && exit 0
OUT=$OD/res.jsonl; LK=$OD/.lock
T=$(cl_tmp); trap 'cl_tmp_clean "$T"' EXIT
KEY=$(cl_key "$KIND" "$IN" "$N"); [ -n "$KEY" ] || exit 0
cl_build "$KIND" "$IN" "$N" "$MODEL" > "$T/req.json" || { ( flock 9; echo "$KEY request build failed" >> "$OD/errors.log"; echo "$KEY" >> "$OD/failed.txt" ) 9> "$LK"; exit 0; }
for attempt in 1 2 3; do
  [ -f "$OD/STOP" ] && exit 0
  curl -sS --max-time 1200 "$API/messages" -H "x-api-key: $ANTHROPIC_API_KEY" -H 'anthropic-version: 2023-06-01' -H 'content-type: application/json' -d @"$T/req.json" > "$T/resp.json" 2> "$T/curl.err"
  if jq -e '.usage.input_tokens' "$T/resp.json" > /dev/null 2>&1; then
    TI=$(jq -r '(.usage.input_tokens // 0) + (.usage.cache_creation_input_tokens // 0) + (.usage.cache_read_input_tokens // 0)' "$T/resp.json"); TO=$(jq -r '.usage.output_tokens // 0' "$T/resp.json")
    ( flock 9; echo "$MODEL rt $TI $TO" >> "$OD/ledger.txt" ) 9> "$LK"
    jq -r '[.content[]? | select(.type == "text") | .text] | join("")' "$T/resp.json" > "$T/txt"
    JL=$(cl_handle "$KIND" "$IN" "$N" "$T/txt" "${MODEL#claude-}" "$TI" "$TO")
    if [ -n "$JL" ]; then ( flock 9; printf '%s\n' "$JL" >> "$OUT"; printf '%s\n' "$KEY" >> "$OD/done.keys" ) 9> "$LK"; exit 0; fi
  fi
  ( flock 9; echo "$(date +%F_%T) $KEY attempt $attempt: $(head -c 300 "$T/resp.json" "$T/curl.err" | tr '\n' ' ')" >> "$OD/errors.log" ) 9> "$LK"
  grep -qiE "$fatal_re" "$T/resp.json" 2>/dev/null && exit 0     # the watchdog sees it in errors.log and stops the stream
  sleep $((attempt*10))
done
( flock 9; echo "$KEY" >> "$OD/failed.txt" ) 9> "$LK"
exit 0
