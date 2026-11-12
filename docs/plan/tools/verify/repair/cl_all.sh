#!/bin/bash
# cl_all.sh <jobdir>   One continuous Claude Message-Batches job, resumable. <jobdir>/job.env defines: KIND (repair|review), MODEL, IN (input file, see cl_lib.sh),
# CANDS (command printing "key<TAB>line" candidates in priority order, re-run before every submit), BSZ (records per batch), MAXINFL (batches in flight), CAP (total USD cap
# over ALL Claude spend, default 150), EST_OUT (output tokens per request assumed for the reservation), optional MAXREC (stop after that many records are claimed).
# Loop: collect finished batches (results -> res.jsonl, ledger.txt "model batch in out", done.keys), then submit the next batch when below MAXINFL and the cap allows.
# Cap check: ledger spend of every job and test + reservation of every unfinished batch + this batch's estimate must stay below CAP. Stop files: <jobdir>/STOP or mined-data/claude/STOP.
# Credit/auth errors: STOP + ALERT in job.log. Re-check of the DeepSeek done.keys happens inside CANDS before each submit (nothing is paid twice).
JD=${1:?jobdir}; HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/cl_lib.sh"; . "$JD/job.env"
BSZ=${BSZ:-300}; MAXINFL=${MAXINFL:-2}; CAP=${CAP:-150}; EST_OUT=${EST_OUT:-1500}; MAXREC=${MAXREC:-100000000}; LK=$JD/.lock
log() { echo "$(date '+%F %T') $*" | tee -a "$JD/job.log"; }
all_spent() { ledger_cost "$CLD"/test/*/ledger.txt "$CLD"/jobs/*/ledger.txt; }
reserved() { cat "$CLD"/jobs/*/b*/reserve 2>/dev/null | awk '{s+=$1} END{printf "%.4f", s+0}'; }   # reserve file exists only while the batch is unfinished
stopped() { [ -f "$JD/STOP" ] || [ -f "$CLD/STOP" ]; }
touch "$JD/claimed.keys" "$JD/gaveup.keys" "$JD/done.keys" "$JD/ledger.txt" "$JD/attempts.txt"; rm -f "$JD/DONE"
api() { curl -sS --max-time 600 "$@" -H "x-api-key: $ANTHROPIC_API_KEY" -H 'anthropic-version: 2023-06-01'; }
fatal_check() { # <file>: stop on credit/auth errors
  if grep -qiE "$fatal_re" "$1" 2>/dev/null; then log "ALERT credit or auth error: $(head -c 300 "$1" | tr '\n' ' ' | sed 's/sk-ant-[A-Za-z0-9_-]*/KEY/g')"; touch "$JD/STOP"; return 0; fi; return 1; }

collect() {
  local b id st n
  for b in "$JD"/b*/; do
    [ -f "$b/id" ] && [ ! -f "$b/collected" ] || continue
    id=$(cat "$b/id"); api "$API/messages/batches/$id" > "$b/status.json" 2> /dev/null
    st=$(jq -r '.processing_status // empty' "$b/status.json" 2> /dev/null)
    [ "$st" = ended ] || { [ -z "$st" ] && { echo "$(date +%F_%T) status poll of $id: $(head -c 200 "$b/status.json")" >> "$JD/errors.log"; fatal_check "$b/status.json"; }; continue; }
    api "$(jq -r .results_url "$b/status.json")" > "$b/results.jsonl" 2> "$b/results.err"
    [ -s "$b/results.jsonl" ] || { echo "$(date +%F_%T) empty results for $id" >> "$JD/errors.log"; continue; }
    : > "$b/ok.keys"; : > "$b/bad.keys"
    # one pass: ledger lines, ok/bad per custom_id (r<line>), reply texts to files
    mkdir -p "$b/txt"
    jq -c '{id: .custom_id, t: .result.type, in: ((.result.message.usage.input_tokens // 0) + (.result.message.usage.cache_creation_input_tokens // 0) + (.result.message.usage.cache_read_input_tokens // 0)), out: (.result.message.usage.output_tokens // 0), err: (.result.error.error.message // .result.error.message // ""),
            text: ([.result.message.content[]? | select(.type == "text") | .text] | join(""))}' "$b/results.jsonl" > "$b/flat.jsonl"
    jq -r 'select(.t == "succeeded") | "\(.in) \(.out)"' "$b/flat.jsonl" | awk -v m="$MODEL" '{ print m " batch " $1 " " $2 }' >> "$JD/ledger.txt"
    jq -r 'select(.t != "succeeded") | "\(.id)\t\(.t)\t\(.err)"' "$b/flat.jsonl" >> "$b/errored.tsv"
    [ -s "$b/errored.tsv" ] && { cat "$b/errored.tsv" | cut -c1-300 | head -5 >> "$JD/errors.log"; fatal_check "$b/errored.tsv"; }
    jq -c 'select(.t == "succeeded")' "$b/flat.jsonl" | while IFS= read -r line; do
      cid=$(printf '%s' "$line" | jq -r .id); N=${cid#r}
      printf '%s' "$line" | jq -r .text > "$b/txt/$N"
      JL=$(cl_handle "$KIND" "$IN" "$N" "$b/txt/$N" "${MODEL#claude-}" "$(printf '%s' "$line" | jq -r .in)" "$(printf '%s' "$line" | jq -r .out)")
      KEY=$(cl_key "$KIND" "$IN" "$N")
      if [ -n "$JL" ]; then printf '%s\n' "$JL" >> "$JD/res.jsonl"; printf '%s\n' "$KEY" >> "$b/ok.keys"; else printf '%s\n' "$KEY" >> "$b/bad.keys"; fi
    done
    # keys that got no usable reply (errored, expired, unusable text): attempt count; second failure = gave up
    awk -F'\t' 'FILENAME == ARGV[1] { ok[$1] = 1; next } ($1 in ok) { next } { print $1 }' "$b/ok.keys" "$b/keys.tsv" > "$b/fail.keys"
    cat "$b/ok.keys" >> "$JD/done.keys"
    while IFS= read -r k; do
      echo "$k" >> "$JD/attempts.txt"
      if [ "$(grep -cxF -- "$k" "$JD/attempts.txt")" -ge 2 ]; then echo "$k" >> "$JD/gaveup.keys"; fi
    done < "$b/fail.keys"
    # failed keys (not gave up) become candidates again: drop them from claimed.keys
    awk -F'\t' 'FILENAME == ARGV[1] { f[$1] = 1; next } !($1 in f)' "$b/fail.keys" "$JD/claimed.keys" > "$JD/claimed.keys.new"; mv "$JD/claimed.keys.new" "$JD/claimed.keys"
    cat "$JD/gaveup.keys" >> "$JD/claimed.keys"
    rm -f "$b/reserve"; touch "$b/collected"
    log "batch $id collected: ok $(wc -l < "$b/ok.keys"), failed $(wc -l < "$b/fail.keys"), spent(all) \$$(all_spent)"
  done
}

submit() {
  stopped && return
  local infl; infl=$(ls -d "$JD"/b*/ 2>/dev/null | while read -r b; do [ -f "$b/collected" ] || echo x; done | wc -l)
  [ "$infl" -ge "$MAXINFL" ] && return
  [ "$(wc -l < "$JD/claimed.keys")" -ge "$MAXREC" ] && return
  local seq b; seq=$(ls -d "$JD"/b*/ 2>/dev/null | wc -l); seq=$((seq+1)); b=$JD/b$(printf '%04d' $seq)
  # candidates: CANDS re-reads the DeepSeek done.keys now; our own claimed/gave-up keys are removed as well
  bash -c "$CANDS" | awk -F'\t' 'FILENAME == ARGV[1] { t[$1] = 1; next } !($1 in t)' <(cat "$JD/claimed.keys" "$JD/gaveup.keys" "$JD/done.keys") - | head -n "$BSZ" > "$JD/next.tsv"
  # producer still running (FLUSHIF false): wait for MINB candidates before submitting, and never end the job (used by the review job, whose todo grows while the DeepSeek stage runs)
  local flush=0; bash -c "${FLUSHIF:-true}" > /dev/null 2>&1 && flush=1
  if [ "$flush" -eq 0 ] && [ "$(wc -l < "$JD/next.tsv")" -lt "${MINB:-1}" ]; then return; fi
  [ -s "$JD/next.tsv" ] || { [ "$infl" -eq 0 ] && [ "$flush" -eq 1 ] && { touch "$JD/DONE"; log "no candidates left"; }; return; }
  mkdir -p "$b/req"; cp "$JD/next.tsv" "$b/keys.tsv"
  cut -f2 "$b/keys.tsv" | xargs -P 8 -I{} bash -c '. "$0/cl_lib.sh"; cl_build "$1" "$2" "$3" "$4" > "$5/req/$3.json" || rm -f "$5/req/$3.json"' "$HERE" "$KIND" "$IN" "{}" "$MODEL" "$b" 2> "$b/build.err"
  : > "$b/req.jsonl"
  # last look at the DeepSeek stage's done.keys (it advanced while the requests were built): those records are not submitted
  sort -u /mnt/c/ParanoidBSD/mined-data/dsv/done.keys > "$b/dsvdone"
  while IFS=$'\t' read -r k n; do
    [ "$KIND" = repair ] && grep -qxF -- "$k" "$b/dsvdone" && { echo "$k" >> "$JD/claimed.keys"; echo "$k" >> "$b/skipped.keys"; continue; }
    [ -s "$b/req/$n.json" ] && { jq -c --arg id "r$n" '{custom_id:$id, params:.}' "$b/req/$n.json" >> "$b/req.jsonl"; printf '%s\t%s\n' "$k" "$n" >> "$b/keys.kept"; }
  done < "$b/keys.tsv"
  touch "$b/keys.kept"; mv "$b/keys.kept" "$b/keys.tsv"
  local nreq est; nreq=$(wc -l < "$b/req.jsonl")
  [ "$nreq" -gt 0 ] || { if [ -s "$b/skipped.keys" ]; then log "batch $seq: all $(wc -l < "$b/skipped.keys") records were done by the DeepSeek stage meanwhile"; touch "$b/collected"; else log "no request could be built for batch $seq (see $b/build.err)"; touch "$JD/STOP"; fi; return; }
  est=$(wc -c < "$b/req.jsonl" | awk -v m="$MODEL" -v n="$nreq" -v eo="$EST_OUT" 'BEGIN{pi = (m ~ /opus/) ? 4 : ((m ~ /sonnet/) ? 2 : 0.10); po = (m ~ /opus/) ? 20 : ((m ~ /sonnet/) ? 10 : 0.50)} {printf "%.4f", 0.5 * ($1 / 3 * pi + n * eo * po) / 1000000}')
  if awk -v s="$(all_spent)" -v r="$(reserved)" -v e="$est" -v c="$CAP" 'BEGIN{exit !(s + r + e > c)}'; then log "cap \$$CAP would be exceeded (spent \$$(all_spent), reserved \$$(reserved), this batch \$$est): not submitting"; touch "$JD/STOP"; return; fi
  jq -c -s '{requests: .}' "$b/req.jsonl" > "$b/body.json"
  # last look at the DeepSeek done.keys (they may have advanced while the requests were built): drop those requests
  api "$API/messages/batches" -H 'content-type: application/json' -d @"$b/body.json" > "$b/create.json" 2> "$b/create.err"
  if jq -e '.id' "$b/create.json" > /dev/null 2>&1; then
    jq -r .id "$b/create.json" > "$b/id"; echo "$est" > "$b/reserve"; cut -f1 "$b/keys.tsv" >> "$JD/claimed.keys"
    log "submitted batch $(cat "$b/id"): $nreq requests, estimate \$$est, spent(all) \$$(all_spent), reserved \$$(reserved)"
  else
    echo "$(date +%F_%T) create failed: $(head -c 400 "$b/create.json" "$b/create.err" | tr '\n' ' ')" >> "$JD/errors.log"; log "batch create failed: $(head -c 300 "$b/create.json" | sed 's/sk-ant-[A-Za-z0-9_-]*/KEY/g')"
    fatal_check "$b/create.json" || fatal_check "$b/create.err" || sleep 60
  fi
}

log "job $(basename "$JD"): kind $KIND model $MODEL, batch size $BSZ, in flight $MAXINFL, cap \$$CAP, spent(all) \$$(all_spent)"
while true; do
  collect
  submit
  stopped && { [ "$(ls -d "$JD"/b*/ 2>/dev/null | while read -r b; do [ -f "$b/id" ] && [ ! -f "$b/collected" ] && echo x; done | wc -l)" -eq 0 ] && break; }   # stopped: still collect what is in flight
  [ -f "$JD/DONE" ] && break
  sleep 60
done
log "job ended: done $(sort -u "$JD/done.keys" | wc -l) records, gave up $(sort -u "$JD/gaveup.keys" | wc -l), spent(all) \$$(all_spent)"
