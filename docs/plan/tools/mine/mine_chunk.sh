#!/bin/bash
# mine_chunk.sh <corpus> <file> <start> <end> <out.jsonl>   One GLM-5.3-Flash call on lines start..end of a C/C++ file; appends one JSON line to out.jsonl.
# Output is a HYPOTHESIS (like PRISM's LLM verdicts): contracts and invariants are leads, never evidence, until PRISM or Lean confirms them.
. ~/.pbsd/env
CORPUS=$1; FILE=$2; S=$3; E=$4; OUT=$5
KEY="$CORPUS:$FILE:$S-$E"
grep -qF "\"key\":\"$KEY\"" "$OUT" 2>/dev/null && exit 0
CODE=$(sed -n "${S},${E}p" "$FILE")
[ -z "$CODE" ] && exit 0
SYS='You mine C and C++ kernel code for reusable verification patterns. Reply with ONE JSON object and nothing else, in this shape:
{"functions":[{"name":str,"kind":one of ["list-op","tree-op","refcount","lock-or-sync","ring-buffer","bitmap","allocator","state-machine","parser","table-lookup","string-op","arith-helper","init-teardown","error-path","driver-glue","other"],"summary":str(<=20 words),"contract":str(ONE line /*@ requires ...; ensures ...; */ in C expressions over the parameters and the word result (not \result); ONLY for a function whose parameters and return value are integers, booleans or enums, otherwise ""),"prose_contract":str(the precondition and postcondition in one plain sentence each),"loops":[{"shape":str,"invariant":str}],"sync":str(locks or RCU or atomics used, "" if none),"ownership":str(who frees or holds a reference, "" if n/a),"error_convention":str(e.g. returns -errno, NULL on failure, "" if n/a),"model_hint":str(an abstract model a Lean proof could use, e.g. "counter with generation", "bitmask rights", "bounded ring with head and tail", "" if none)}],"idioms":[str,...]}
List only functions that are defined in the code you are given. Be short and concrete. Never invent a function.'
REQ=$(jq -n --arg sys "$SYS" --arg code "$CODE" --arg model "morph-glm53flash" \
  '{model:$model, messages:[{role:"system",content:$sys},{role:"user",content:("File excerpt:\n```c\n"+$code+"\n```")}], max_tokens:3000, reasoning_effort:"low", temperature:0.2}')
for try in 1 2 3; do
  RESP=$(curl -sS --max-time 180 https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' -d "$REQ" 2>/dev/null) && echo "$RESP" | jq -e '.choices[0].message.content' >/dev/null 2>&1 && break
  sleep $((try*5)); RESP=""
done
[ -z "$RESP" ] && { echo "FAIL $KEY" >&2; exit 0; }
TXT=$(echo "$RESP" | jq -r '.choices[0].message.content' | sed -e 's/^```json//' -e 's/^```//' -e 's/```$//')
echo "$TXT" | jq -e '.functions' >/dev/null 2>&1 || { echo "BADJSON $KEY" >&2; echo "$TXT" > "${OUT%.jsonl}.bad.$(echo "$KEY" | md5sum | cut -c1-8).txt"; exit 0; }
LINE=$(jq -c -n --arg key "$KEY" --arg corpus "$CORPUS" --arg file "$FILE" --argjson s "$S" --argjson e "$E" --argjson r "$TXT" --argjson u "$(echo "$RESP" | jq -c '.usage')"   '{key:$key,corpus:$corpus,file:$file,start:$s,end:$e,usage:$u,result:$r}')
# Lines are about 4.5 KB: appends above 4 KB are not atomic, so parallel writers corrupted phase 1 (7 Nov 2026). One writer at a time.
( flock 9; printf '%s\n' "$LINE" >> "$OUT" ) 9> "$OUT.lock"
