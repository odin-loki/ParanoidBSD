#!/bin/bash
# mine_chunk.sh <corpus> <file> <start> <end> <out.jsonl>   One GLM-5.3-Flash call on lines start..end of a C/C++ file; appends one JSON line to out.jsonl.
# Output is a HYPOTHESIS (like PRISM's LLM verdicts): contracts and invariants are leads, never evidence, until PRISM or Lean confirms them.
# Side files next to out.jsonl (all under one flock, so parallel workers never interleave):
#   ledger.txt    one "<prompt_tokens> <completion_tokens>" line per successful call (the spend counter: never re-read the big data)
#   failed.txt    "<corpus>:<file>:<start>-<end>" of every chunk that failed after retries (retry them in a last pass)
#   errors.log    the start of every error response from the API (credit exhausted, rate limit, bad request) with a timestamp
. ~/.pbsd/env
CORPUS=$1; FILE=$2; S=$3; E=$4; OUT=$5
D=$(dirname "$OUT"); KEY="$CORPUS:$FILE:$S-$E"
CODE=$(sed -n "${S},${E}p" "$FILE")
[ -z "$CODE" ] && exit 0
SYS='You mine C and C++ kernel code for reusable verification patterns. Reply with ONE JSON object and nothing else, in this shape:
{"functions":[{"name":str,"kind":one of ["list-op","tree-op","refcount","lock-or-sync","ring-buffer","bitmap","allocator","state-machine","parser","table-lookup","string-op","arith-helper","init-teardown","error-path","driver-glue","other"],"summary":str(<=20 words),"contract":str(ONE line /*@ requires ...; ensures ...; */ in C expressions over the parameters and the word result (not \result); ONLY for a function whose parameters and return value are integers, booleans or enums, otherwise ""),"prose_contract":str(the precondition and postcondition in one plain sentence each),"loops":[{"shape":str,"invariant":str}],"sync":str(locks or RCU or atomics used, "" if none),"ownership":str(who frees or holds a reference, "" if n/a),"error_convention":str(e.g. returns -errno, NULL on failure, "" if n/a),"model_hint":str(an abstract model a Lean proof could use, e.g. "counter with generation", "bitmask rights", "bounded ring with head and tail", "" if none)}],"idioms":[str,...]}
List only functions that are defined in the code you are given. Be short and concrete. Never invent a function.'
note_fail() { ( flock 9; echo "$KEY" >> "$D/failed.txt"; echo "$(date +%F_%T) $KEY $1" >> "$D/errors.log" ) 9> "$OUT.lock"; }
MAXTOK=3000
for attempt in 1 2 3; do
  REQ=$(jq -n --arg sys "$SYS" --arg code "$CODE" --argjson mt $MAXTOK '{model:"morph-glm53flash", messages:[{role:"system",content:$sys},{role:"user",content:("File excerpt:\n```c\n"+$code+"\n```")}], max_tokens:$mt, reasoning_effort:"low", temperature:0.2}')
  RESP=$(curl -sS --max-time 240 https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' -d "$REQ" 2>&1)
  if ! echo "$RESP" | jq -e '.choices[0].message.content' >/dev/null 2>&1; then
    ( flock 9; echo "$(date +%F_%T) $KEY attempt $attempt: $(echo "$RESP" | head -c 300 | tr '\n' ' ')" >> "$D/errors.log" ) 9> "$OUT.lock"
    sleep $((attempt*8)); continue
  fi
  TXT=$(echo "$RESP" | jq -r '.choices[0].message.content' | sed -e 's/^```json//' -e 's/^```//' -e 's/```$//')
  if echo "$TXT" | jq -e '.functions' >/dev/null 2>&1; then
    LINE=$(jq -c -n --arg key "$KEY" --arg corpus "$CORPUS" --arg file "$FILE" --argjson s "$S" --argjson e "$E" --argjson r "$TXT" --argjson u "$(echo "$RESP" | jq -c '.usage')" \
      '{key:$key,corpus:$corpus,file:$file,start:$s,end:$e,usage:$u,result:$r}')
    # Lines are about 4.5 KB: appends above 4 KB are not atomic, so parallel writers corrupted phase 1 (7 Nov 2026). One writer at a time.
    ( flock 9
      printf '%s\n' "$LINE" >> "$OUT"
      echo "$(echo "$RESP" | jq -r '.usage.prompt_tokens // 0') $(echo "$RESP" | jq -r '.usage.completion_tokens // 0')" >> "$D/ledger.txt"
    ) 9> "$OUT.lock"
    exit 0
  fi
  # not valid JSON: usually truncated by max_tokens. Count the tokens it burned, retry once with more room, then give up on the chunk.
  ( flock 9; echo "$(echo "$RESP" | jq -r '.usage.prompt_tokens // 0') $(echo "$RESP" | jq -r '.usage.completion_tokens // 0')" >> "$D/ledger.txt" ) 9> "$OUT.lock"
  MAXTOK=7000
done
echo "FAIL $KEY" >&2
note_fail "gave up after 3 attempts"
exit 0
