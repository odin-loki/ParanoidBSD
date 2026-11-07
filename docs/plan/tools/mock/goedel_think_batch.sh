#!/bin/bash
# goedel_think_batch.sh [workers]   Runs every problem in the data folder for seeds 1..3, one at a time by default (two workers do not fit the 3090 at long context).
DATA=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/rehearsal-data/goedel-thinking-2026-11-07
HERE=$(dirname "$(readlink -f "$0")")
rm -f $DATA/DONE
for s in 1 2 3; do for p in $DATA/problems/*.lean; do echo "$p $s"; done; done | xargs -P ${1:-1} -L 1 $HERE/goedel_think.sh
touch $DATA/DONE; echo BATCH-DONE
