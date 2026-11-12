#!/bin/bash
# repair_chunk.sh "<todo line>" <out.jsonl>   Repair the rejected annotations of ONE record with GLM-5.3-Flash: the model sees the source slice, the current annotation
# of each rejected function and the exact reasons it was rejected, and writes a corrected annotation (or drops a record that is not a real function).
# The repaired annotations are re-verified by the same checks (steps 1-3, Frama-C) before anything trusts them; this script only proposes.
# todo line (tab separated): key, file, start, end, functions ("idx ¦ name ¦ prose ¦ contract ¦ problems" joined by " ¶ ")
# Side files next to out.jsonl, under one flock: ledger.txt, failed.txt, errors.log.
. ~/.pbsd/env
LINE=$1; OUT=$2; D=$(dirname "$OUT")
[ -f "$D/STOP" ] && exit 0   # the watchdog in repair_all.sh stops the stream (budget, credit error) by creating this file
KEY=$(printf '%s' "$LINE" | cut -f1); FILE=$(printf '%s' "$LINE" | cut -f2); S=$(printf '%s' "$LINE" | cut -f3); E=$(printf '%s' "$LINE" | cut -f4); FUNCS=$(printf '%s' "$LINE" | cut -f5-)
CODE=$(sed -n "${S},${E}p" "$FILE"); [ -z "$CODE" ] && exit 0
SYS='You repair annotations of C and C++ kernel functions. Another tool wrote them and independent checkers rejected them for the reasons given. For each listed function, read the code and write a CORRECT annotation. Reply with ONE JSON object and nothing else:
{"functions":[{"idx":int (copy it),"name":str (the real name of the function as defined in the code),"action":"fix" or "drop","prose_contract":str,"contract":str}]}
Rules:
- action "drop" only if the listed item is not a function defined in this code (a macro, a declaration, a fragment); then the other fields may be "".
- prose_contract: the precondition and the postcondition, one plain sentence each, covering EVERY return value and error code the code can produce and any lock, reference or memory the caller gets or loses.
- contract: ONLY for a function whose parameters and return value are all integers, booleans or enums; otherwise "". Exactly one line: /*@ requires <C expression>; ensures <C expression>; */
  * use only the parameter names and the word result (not \result) for the return value; no calls to functions or macros, no struct fields, no pointers, no words;
  * logic: && || ! ==> (implies) <==> (if and only if) ?: and the C comparison and arithmetic operators; numbers in decimal or hex; true is \true;
  * PARENTHESISE bit operations: write result == (x & 3), never result == x & 3 (== binds tighter than & | ^ in C and ACSL);
  * integers are mathematical: if the code can overflow or wrap (a + 1, x << n, a * b), the requires must exclude those inputs (for example a < 2147483647, n < 32);
  * the ensures must be TRUE for every input that meets the requires, and must cover every return path (all error codes);
  * the requires must state only what the code needs; never write ensures \true.
- If a problem says the contract is false for an input, check that input against the code and fix the contract so it is true.'
NL=$'\n'
SYS="$SYS${REPAIR_SYS_EXTRA:+$NL$REPAIR_SYS_EXTRA}"   # later rounds add the reviewer rule (REPAIR_SYS_EXTRA, see start_round.sh)
FLIST=$(printf '%s' "$FUNCS" | awk 'BEGIN{RS=" ¶ "; FS=" ¦ "} { gsub(/\n$/, "", $5); printf "[#%s] %s\ncurrent prose contract: %s\ncurrent formal contract: %s\nrejected because: %s\n\n", $1, $2, $3, ($4 == "" ? "(none)" : $4), $5 }')
USER="Source (lines ${S}-${E} of ${FILE##*/}):${NL}${CODE}${NL}${NL}Functions to repair:${NL}${FLIST}"
MAXTOK=14000   # the remaining records are large slices: at 6000 the reasoning used the whole budget and the reply was empty (money spent for nothing)
for attempt in 1 2 3; do
  REQ=$(jq -n --arg sys "$SYS" --arg user "$USER" --argjson mt $MAXTOK --arg eff "${REPAIR_EFFORT:-low}" '{model:"morph-glm53flash", messages:[{role:"system",content:$sys},{role:"user",content:$user}], max_tokens:$mt, reasoning_effort:$eff, temperature:0.2}')
  RESP=$(curl -sS --max-time 300 https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' -d "$REQ" 2>&1)
  if echo "$RESP" | jq -e '.usage.prompt_tokens' > /dev/null 2>&1; then
    ( flock 9; echo "$(echo "$RESP" | jq -r '.usage.prompt_tokens // 0') $(echo "$RESP" | jq -r '.usage.completion_tokens // 0')" >> "$D/ledger.txt" ) 9> "$OUT.lock"
  fi
  TXT=$(echo "$RESP" | jq -r '.choices[0].message.content // empty' 2>/dev/null | sed -e 's/^```json//' -e 's/^```//' -e 's/```$//')
  if [ -n "$TXT" ] && echo "$TXT" | jq -e '.functions' > /dev/null 2>&1; then
    JL=$(jq -c -n --arg key "$KEY" --arg file "$FILE" --argjson s "$S" --argjson e "$E" --argjson r "$TXT" '{key:$key, file:$file, start:$s, end:$e, result:$r}')
    ( flock 9; printf '%s\n' "$JL" >> "$OUT" ) 9> "$OUT.lock"
    exit 0
  fi
  ( flock 9; echo "$(date +%F_%T) $KEY attempt $attempt: $(echo "$RESP" | head -c 250 | tr '\n' ' ')" >> "$D/errors.log" ) 9> "$OUT.lock"
  MAXTOK=20000; sleep $((attempt*6))
done
( flock 9; echo "$KEY" >> "$D/failed.txt" ) 9> "$OUT.lock"
exit 0
