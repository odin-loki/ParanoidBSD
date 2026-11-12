#!/bin/bash
# morph_call.sh <req.json> <resp.json> <tag>   One GLM-5.3-Flash call through Morph with a hard spend cap and a token ledger.
#   exit 0  response saved to <resp.json> (may still hold a model error: the caller checks .choices)
#   exit 7  credit/quota/auth error from the API (an API error RESPONSE, matched on its error object only) -> STOP file written, caller must stop cleanly
#   exit 9  the cap (CAP_USD, default 40) is reached -> STOP file written
#   exit 8  transport failure or non-JSON/HTTP>=500 after retries (no tokens billed that we can see)
# Ledger (append-only, flock): $LEDGER = pilot/morph_ledger.tsv  columns: ts tag http prompt cached completion reasoning usd_conservative usd_listed
# Prices per 1M tokens, conservative (higher of the two Morph price lists in CLOUD_SETUP.md): in 0.15, cache hit 0.02, out 0.42; "listed" = 0.047 / 0.02 / 0.268.
# The key is read from ~/.pbsd/env and is never printed or written anywhere.
D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; LEDGER=${LEDGER:-$D/morph_ledger.tsv}; CAP=${CAP_USD:-40}
REQ=$1; RESP=$2; TAG=$3
[ -f "$D/STOP" ] && exit 9
. ~/.pbsd/env
touch "$LEDGER"
spent() { awk -F'\t' '{s+=$8} END{printf "%.6f", s+0}' "$LEDGER"; }
if awk -v s="$(spent)" -v c="$CAP" 'BEGIN{exit !(s>=c)}'; then echo "cap reached" >"$D/STOP"; exit 9; fi
for try in 1 2 3; do
  code=$(curl -sS --max-time 400 -o "$RESP.tmp" -w '%{http_code}' https://api.morphllm.com/v1/chat/completions \
        -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' -d @"$REQ" 2>/dev/null); rc=$?
  if [ $rc -ne 0 ]; then sleep $((try*5)); continue; fi
  if jq -e '.error' "$RESP.tmp" >/dev/null 2>&1; then
    msg=$(jq -r '[.error.message?, .error.code?, .error.type?] | map(select(.!=null) | tostring) | join(" ")' "$RESP.tmp" | tr 'A-Z' 'a-z')
    if [ "$code" = 401 ] || [ "$code" = 402 ] || [ "$code" = 403 ] || echo "$msg" | grep -q -E 'insufficient|credit|balance|quota|billing|payment|exceeded your|invalid api key|unauthorized'; then
      echo "$(date -u +%FT%TZ) credit/auth error http=$code" >"$D/STOP"; mv "$RESP.tmp" "$RESP"; exit 7
    fi
    sleep $((try*8)); continue          # other API error (rate limit, overload): retry
  fi
  if [ "${code:-0}" -ge 500 ] 2>/dev/null; then sleep $((try*8)); continue; fi
  if jq -e '.usage.prompt_tokens' "$RESP.tmp" >/dev/null 2>&1; then
    mv "$RESP.tmp" "$RESP"
    line=$(jq -r --arg tag "$TAG" --arg code "$code" '
      (.usage.prompt_tokens // 0) as $p | (.usage.prompt_tokens_details.cached_tokens // 0) as $h | (.usage.completion_tokens // 0) as $c |
      (.usage.completion_tokens_details.reasoning_tokens // 0) as $r |
      [ (now|todate), $tag, $code, $p, $h, $c, $r,
        ((($p-$h)*0.15 + $h*0.02 + $c*0.42)/1000000 | . * 1000000 | round / 1000000),
        ((($p-$h)*0.047 + $h*0.02 + $c*0.268)/1000000 | . * 1000000 | round / 1000000) ] | @tsv' "$RESP")
    ( flock 9; printf '%s\n' "$line" >>"$LEDGER" ) 9>"$LEDGER.lock"
    exit 0
  fi
  sleep $((try*5))
done
rm -f "$RESP.tmp"; exit 8
