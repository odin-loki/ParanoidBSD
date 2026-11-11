#!/bin/bash
# step4_chunk.sh "<todo line>" <out.jsonl>   Step 4 of VERIFICATION_PLAN.md for ONE record: a second model family (DeepSeek V4.1 Flash 1017 on Morph) reads the source
# slice and the GLM annotations of the listed functions and answers one plain-text verdict per function: "<name>: AGREE" or "<name>: DISAGREE: <reason>".
# The todo line is tab separated: key, file, start, end, functions (each "name ¦ prose ¦ contract", joined by " ¶ ").
# Side files next to out.jsonl, all written under one flock: ledger.txt (tokens per call), failed.txt (keys), errors.log.
. ~/.pbsd/env
LINE=$1; OUT=$2; D=$(dirname "$OUT")
[ -f "$D/STOP" ] && exit 0   # the watchdog in step4_all.sh stops the stream (budget, credit error) by creating this file
KEY=$(printf '%s' "$LINE" | cut -f1); FILE=$(printf '%s' "$LINE" | cut -f2); S=$(printf '%s' "$LINE" | cut -f3); E=$(printf '%s' "$LINE" | cut -f4); FUNCS=$(printf '%s' "$LINE" | cut -f5-)
CODE=$(sed -n "${S},${E}p" "$FILE"); [ -z "$CODE" ] && exit 0
# fixed instruction prefix first (prompt caching), then the variable part
SYS='You are a strict reviewer of annotations written by another AI for C kernel code. For each listed function, decide whether its annotation (the prose contract and the formal contract, if any) is CONSISTENT with what the code actually does: the return values and error codes, what the caller must guarantee, side effects and ownership. A formal contract that is weaker than the prose, or that omits a return path the code has, or that says something the code does not do, is a DISAGREE. If you cannot tell from the code you are shown, answer AGREE only if nothing contradicts the annotation. Reply with exactly one line per listed function and nothing else, in the form
<name>: AGREE
or
<name>: DISAGREE: <the specific problem in at most 20 words>'
NL=$'\n'
FLIST=$(printf '%s' "$FUNCS" | awk 'BEGIN{RS=" ¶ "; FS=" ¦ "} {printf "FUNCTION %s%sprose contract: %s%sformal contract: %s%s%s", $1, "\n", $2, "\n", ($3==""?"(none)":$3), "\n", "\n"}')
USER="Source (lines ${S}-${E} of ${FILE##*/}):${NL}${CODE}${NL}${NL}Functions to review:${NL}${FLIST}"
# Reasoning effort. Measured 11 Nov 2026 on 40 records against the provider default: "minimal" $0.00165/call but lenient (19 of 158 DISAGREEs became AGREE);
# "low" $0.00162/call (29% cheaper than the default $0.00229) with 91% the same verdicts and no bias (7 vs 5 flips). Odin: not max, the right amount -> low.
EFFORT=${S4_EFFORT:-low}
REQ=$(jq -n --arg sys "$SYS" --arg user "$USER" --arg eff "$EFFORT" '{model:"morph-dsv41flash-1017", messages:[{role:"system",content:$sys},{role:"user",content:$user}], max_tokens:10000, temperature:0} + (if $eff == "" then {} else {reasoning_effort:$eff} end)')
# (pilot, 11 Nov 2026: at max_tokens 4000 about 9% of replies were empty because the model's reasoning used the whole budget; reasoning_effort made no difference)
for attempt in 1 2 3; do
  RESP=$(curl -sS --max-time 300 https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' -d "$REQ" 2>&1)
  # every call that returned usage is billed, so it goes in the ledger even when the reply is unusable
  if ! echo "$RESP" | jq -e '.choices[0].message.content' > /dev/null 2>&1 && echo "$RESP" | jq -e '.usage.prompt_tokens' > /dev/null 2>&1; then
    ( flock 9; echo "$(echo "$RESP" | jq -r '.usage.prompt_tokens // 0') $(echo "$RESP" | jq -r '.usage.completion_tokens // 0') $(echo "$RESP" | jq -r '.usage.prompt_tokens_details.cached_tokens // 0')" >> "$D/ledger.txt" ) 9> "$OUT.lock"
  fi
  if echo "$RESP" | jq -e '.choices[0].message.content' > /dev/null 2>&1; then
    TXT=$(echo "$RESP" | jq -r '.choices[0].message.content')
    TOK="$(echo "$RESP" | jq -r '.usage.prompt_tokens // 0') $(echo "$RESP" | jq -r '.usage.completion_tokens // 0') $(echo "$RESP" | jq -r '.usage.prompt_tokens_details.cached_tokens // 0')"
    V=$(printf '%s\n' "$TXT" | awk -F': ' '/^[A-Za-z_][A-Za-z0-9_]*: (AGREE|DISAGREE)/ {n=$1; v=$2; r=""; if (v ~ /^DISAGREE/) { r=$0; sub(/^[^:]*: DISAGREE:? ?/, "", r) } sub(/ .*/, "", v); gsub(/"/, "\\\"", r); gsub(/\\/, "\\\\", r); printf "%s{\"name\":\"%s\",\"verdict\":\"%s\",\"reason\":\"%s\"}", (c++ ? "," : ""), n, tolower(v), r}')
    JL=$(jq -c -n --arg key "$KEY" --arg raw "$TXT" --argjson v "[$V]" --arg tok "$TOK" '{key:$key, verdicts:$v, tokens:$tok, raw:$raw}' 2>/dev/null)
    if [ -n "$JL" ]; then
      ( flock 9; printf '%s\n' "$JL" >> "$OUT"; echo "$TOK" >> "$D/ledger.txt" ) 9> "$OUT.lock"
      exit 0
    fi
  fi
  ( flock 9; echo "$(date +%F_%T) $KEY attempt $attempt: $(echo "$RESP" | head -c 250 | tr '\n' ' ')" >> "$D/errors.log" ) 9> "$OUT.lock"
  sleep $((attempt*8))
done
( flock 9; echo "$KEY" >> "$D/failed.txt" ) 9> "$OUT.lock"
exit 0
