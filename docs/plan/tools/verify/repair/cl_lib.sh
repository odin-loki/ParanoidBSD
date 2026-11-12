# cl_lib.sh   shared helpers of the Claude API runner (source it). Shell/jq/awk only. Never prints the API key.
CLD=${CLD:-/mnt/c/ParanoidBSD/mined-data/claude}
CLHERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. ~/.pbsd/env
API=https://api.anthropic.com/v1
# list prices USD per 1M tokens (input / output): opus-5-5 4/20, sonnet-5-5 2/10, haiku-5-5 0.10/0.50; batch = 50%
# ledger line: "<model> <rt|batch> <input_tokens> <output_tokens>"  (cache tokens are folded into input: upper bound; thinking tokens are inside output_tokens)
ledger_cost() {   # ledger_cost <ledger...>  -> USD, upper bound at list prices (batch at 50%)
  awk '{ pi = ($1 ~ /opus/) ? 4 : (($1 ~ /sonnet/) ? 2 : 0.10); po = ($1 ~ /opus/) ? 20 : (($1 ~ /sonnet/) ? 10 : 0.50); f = ($2 == "batch") ? 0.5 : 1; c += f * ($3 * pi + $4 * po) } END { printf "%.4f", c / 1000000 }' "$@" 2>/dev/null || echo 0
}
effort_for() { case "$1" in *opus*) echo high;; *sonnet*) echo medium;; *) echo "";; esac; }
# cl_params <model> <system-file> <user-file> <max_tokens> -> the "params" JSON object of one Messages request
cl_params() {
  local eff; eff=$(effort_for "$1")
  jq -c -n --arg m "$1" --rawfile sys "$2" --rawfile user "$3" --argjson mt "$4" --arg eff "$eff" \
    '{model:$m, max_tokens:$mt, system:$sys, messages:[{role:"user", content:$user}]} + (if $eff == "" then {} else {output_config:{effort:$eff}} end)'
}
# cl_parse <record-json-file> <reply-text-file> <tag>  -> manual-result jsonl lines on stdout (nothing when the reply is not usable)
cl_parse() {
  local txt; txt=$(sed -z -e 's/^[^{]*//' -e 's/[^}]*$//' "$2")   # first { to last }: tolerates a code fence or a sentence around the JSON
  printf '%s' "$txt" | jq -e '.functions|type=="array"' > /dev/null 2>&1 || return 1
  jq -n -c --slurpfile p "$1" --argjson r "$txt" --arg tag "$3" '$p[0] as $p | ($p.functions | map(.i)) as $ok | $r.functions[] | select(.idx as $x | $ok | index($x)) | select(.action == "fix" or .action == "drop")
     | {key:$p.key, i:.idx, name:(.name // ""), action:.action, prose:(.prose_contract // ""), contract:(.contract // ""), note:((.note // "") + " [" + $tag + "]")}' 2>/dev/null
}
# error classes worth stopping on
fatal_re='authentication_error|permission_error|billing|credit balance|insufficient|invalid x-api-key|payment'
# scratch dir without rm -r: cl_tmp prints a fresh dir, cl_tmp_clean removes it
cl_tmp() { mktemp -d; }
cl_tmp_clean() { [ -n "$1" ] && [ -d "$1" ] && { find "$1" -maxdepth 1 -type f -delete; rmdir "$1" 2>/dev/null; }; return 0; }

# ---- request builders / reply handlers, shared by the synchronous runner (cl_call.sh) and the batch runner (cl_batch.sh) ----
# kind "repair": input = manual-packet style jsonl (one record per line); kind "review": input = step-4 todo tsv (key, file, start, end, "name ¦ prose ¦ contract" joined by " ¶ ")
cl_key() { if [ "$1" = repair ]; then sed -n "${3}{p;q}" "$2" | jq -r .key; else sed -n "${3}{p;q}" "$2" | cut -f1; fi; }
cl_review_user() {   # <todo-line> -> user message of the step-4 reviewer (same text as step4_chunk.sh)
  local LINE=$1 S E F FUNCS CODE FLIST NL=$'\n'
  F=$(printf '%s' "$LINE" | cut -f2); S=$(printf '%s' "$LINE" | cut -f3); E=$(printf '%s' "$LINE" | cut -f4); FUNCS=$(printf '%s' "$LINE" | cut -f5-)
  CODE=$(sed -n "${S},${E}p" "$F"); [ -z "$CODE" ] && return 1
  FLIST=$(printf '%s' "$FUNCS" | awk 'BEGIN{RS=" ¶ "; FS=" ¦ "} {printf "FUNCTION %s%sprose contract: %s%sformal contract: %s%s%s", $1, "\n", $2, "\n", ($3==""?"(none)":$3), "\n", "\n"}')
  printf 'Source (lines %s-%s of %s):%s%s%s%sFunctions to review:%s%s' "$S" "$E" "${F##*/}" "$NL" "$CODE" "$NL" "$NL" "$NL" "$FLIST"
}
cl_build() {   # cl_build <kind> <input> <N> <model> -> params JSON on stdout; returns 1 when the request cannot be built
  local T rc=0; T=$(cl_tmp)
  if [ "$1" = repair ]; then
    sed -n "${3}{p;q}" "$2" > "$T/rec.json"; [ -s "$T/rec.json" ] || { cl_tmp_clean "$T"; return 1; }
    bash "$CLHERE/cl_prompt.sh" < "$T/rec.json" > "$T/user.txt" || rc=1
    [ $rc = 0 ] && cl_params "$4" "$CLHERE/cl_system.txt" "$T/user.txt" "${CL_MAXTOK:-32000}" || rc=1
  else
    cl_review_user "$(sed -n "${3}{p;q}" "$2")" > "$T/user.txt" || rc=1
    [ $rc = 0 ] && cl_params "$4" "$CLHERE/cl_review_system.txt" "$T/user.txt" "${CL_REVMAXTOK:-8000}" || rc=1
  fi
  cl_tmp_clean "$T"; return $rc
}
cl_handle() {   # cl_handle <kind> <input> <N> <reply-text-file> <tag> <in-tokens> <out-tokens>  -> result line(s) on stdout; return 1 when the reply is unusable
  if [ "$1" = repair ]; then
    local T rc; T=$(cl_tmp); sed -n "${3}{p;q}" "$2" > "$T/rec.json"; cl_parse "$T/rec.json" "$4" "$5" > "$T/out"; [ -s "$T/out" ] && { cat "$T/out"; rc=0; } || rc=1; cl_tmp_clean "$T"; return $rc
  else
    local KEY TXT V
    KEY=$(sed -n "${3}{p;q}" "$2" | cut -f1); TXT=$(cat "$4"); [ -n "$TXT" ] || return 1
    V=$(printf '%s\n' "$TXT" | awk -f "$CLHERE/cl_verdicts.awk")
    [ -n "$V" ] || return 1
    jq -c -n --arg key "$KEY" --arg raw "$TXT" --argjson v "[$V]" --arg tok "$6 $7 0" --arg tag "$5" '{key:$key, verdicts:$v, tokens:$tok, raw:$raw, model:$tag}'
  fi
}
