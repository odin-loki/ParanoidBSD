#!/bin/bash
# cl_test_run.sh <packet.jsonl> <model> <outdir> <budget-usd> [parallel=6] [max-records]   Quality test, stage 1: run the packet through one model (synchronous API), spend-capped.
HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/cl_lib.sh"
KIND=${KIND:-repair}; IN=$1; MODEL=$2; OD=$3; B=$4; P=${5:-6}; MAXN=${6:-100000}; mkdir -p "$OD"; rm -f "$OD/STOP" "$OD/FIN"
touch "$OD/ledger.txt"
( while [ ! -f "$OD/FIN" ]; do sleep 5; c=$(ledger_cost "$OD/ledger.txt"); awk -v c="$c" -v b="$B" 'BEGIN{exit !(c>=b)}' && touch "$OD/STOP"
    tail -n 20 "$OD/errors.log" 2>/dev/null | grep -qiE "$fatal_re" && touch "$OD/STOP"; done ) &
WD=$!
seq 1 "$(wc -l < "$IN")" | head -n "$MAXN" | xargs -P "$P" -I{} bash "$HERE/cl_call.sh" "$KIND" "$IN" "{}" "$MODEL" "$OD"
touch "$OD/FIN"; kill $WD 2>/dev/null
echo "done: records $(sort -u "$OD/done.keys" 2>/dev/null | wc -l), results $(wc -l < "$OD/res.jsonl" 2>/dev/null), failed $(sort -u "$OD/failed.txt" 2>/dev/null | wc -l), spent \$$(ledger_cost "$OD/ledger.txt")"
