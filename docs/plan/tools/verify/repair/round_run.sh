#!/bin/bash
# round_run.sh <round> <repair-budget-usd> <review-budget-usd> [repair-parallel] [review-parallel]
# One repair round, resumable (markers in mined-data/repair<N>/ and verify-repair<N>/): state -> todo -> GLM repair (watched) -> re-verify steps 1-3 + Frama-C -> DeepSeek review
# of the fixed functions (watched) -> state. Spend caps are per stage; job spend is reported by spend.sh. Stop cleanly: touch mined-data/repair<N>/STOP or verify-repair<N>/xcheck/STOP.
# Env: REPAIR_EFFORT (default medium), REVIEW_RETRY=1 re-runs round 1's failed reviews first (round 2 only).
N=${1:?round}; RB=${2:?repair budget}; VB=${3:?review budget}; RPAR=${4:-100}; VPAR=${5:-120}
HERE=$(cd "$(dirname "$0")" && pwd); VH=$(cd "$HERE/.." && pwd); D=/mnt/c/ParanoidBSD/mined-data
RP=$D/repair$N; W=$D/verify-repair$N; X=$W/xcheck; mkdir -p "$RP" "$W"
say() { echo "$(date '+%F %T') [round $N] $*" | tee -a "$RP/round.log"; }
. ~/.pbsd/env
alarm() { grep -q "ALERT credit or auth" "$1" 2>/dev/null; }
say "start: repair budget \$$RB, review budget \$$VB"
# 0. round 2: retry the reviews that failed in round 1 (cheap), so every round-1 fix has a verdict
if [ "$N" = 2 ] && [ ! -f "$RP/retry-review.done" ]; then
  W1=$D/verify-repair; [ -s "$W1/xcheck/failed.txt" ] && mv "$W1/xcheck/failed.txt" "$W1/xcheck/failed.round1.txt"
  rm -f "$W1/xcheck/STOP" "$W1/xcheck/DONE-batch"
  B0=$(bash "$HERE/spend.sh" review "$W1/xcheck/ledger.txt")   # step4_all's budget counts the whole ledger, so the cap is what is already spent plus $3
  say "re-review of round 1's failed reviews (cap \$3 above the \$$B0 already spent)"
  S4_V=$W1 S4_X=$W1/xcheck bash "$VH/step4_watch.sh" "$(awk -v b="$B0" 'BEGIN{printf "%.2f", b + 3}')" "$VPAR"
  touch "$RP/retry-review.done"
fi
# 1. state and todo
if [ ! -f "$RP/todo.done" ]; then
  bash "$HERE/state.sh" > "$RP/state-before.json" && bash "$HERE/build_todo.sh" "$N" | tee -a "$RP/round.log" && cp "$D/final/state-summary.json" "$RP/state-before-summary.json" && touch "$RP/todo.done" || { say "state/todo failed"; exit 1; }
fi
# 2. repair
if [ ! -f "$RP/repair.stage.done" ]; then
  rm -f "$RP/STOP"
  export RP TODO=$RP/todo.tsv REPAIR_EFFORT=${REPAIR_EFFORT:-medium} REPAIR_SYS_EXTRA="$(cat "$HERE/repair_rule.txt")"
  for pass in 1 2 3; do
    bash "$HERE/repair_watch.sh" "$RB" "$RPAR"
    if alarm "$RP/watch.log" || alarm "$RP/repair.log"; then say "credit/auth alert: round halted"; exit 2; fi
    [ -f "$RP/DONE" ] || break          # stopped by the budget or by hand: no retry pass
    nfail=$(sort -u "$RP/failed.txt" 2>/dev/null | wc -l); [ "$nfail" -eq 0 ] && break
    say "pass $pass: $nfail records failed (API overload or empty reply); retrying them"
    mv "$RP/failed.txt" "$RP/failed.pass$pass.txt"; rm -f "$RP/DONE"
  done
  touch "$RP/repair.stage.done"
  unset RP TODO
fi
RP=$D/repair$N
say "repair stage over: $(wc -l < "$RP/repair.jsonl") records repaired, spent \$$(bash "$HERE/spend.sh" repair "$RP/ledger.txt")"
# 3. re-verify the repaired annotations (steps 1-3 + Frama-C), own directory
if [ ! -f "$W/summary.json" ] || [ "$W/summary.json" -ot "$RP/repair.stage.done" ]; then
  say "re-verify"; W=$W RJ=$RP/repair.jsonl SFX=r$N bash "$HERE/repair_verify.sh" > "$RP/verify.out" 2>&1
fi
say "re-verified: $(tr -d '\n ' < "$W/summary.json")"
# 4. DeepSeek review of the fixed ones
if [ ! -f "$RP/review.stage.done" ]; then
  W=$W bash "$HERE/s4_repaired_build.sh" | tee -a "$RP/round.log"
  rm -f "$X/STOP" "$X/DONE-batch"
  S4_V=$W S4_X=$X bash "$VH/step4_watch.sh" "$VB" "$VPAR"
  if alarm "$X/watch.log" || alarm "$X/step4.log"; then say "credit/auth alert: round halted"; exit 2; fi
  touch "$RP/review.stage.done"
fi
say "review stage over: $(wc -l < "$X/xcheck.jsonl") records, spent \$$(bash "$HERE/spend.sh" review "$X/ledger.txt"), failed keys $(sort -u "$X/failed.txt" 2>/dev/null | wc -l)"
# 5. state after the round
bash "$HERE/state.sh" > "$RP/state-after.json" && cp "$D/final/state-summary.json" "$RP/state-after-summary.json"
touch "$RP/ROUND-DONE"; say "round done"
