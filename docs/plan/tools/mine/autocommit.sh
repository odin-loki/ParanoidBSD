#!/bin/bash
# autocommit.sh <path-in-repo> <done-file> [interval-s]   Commit and push <path> every interval (default 600 s) until <done-file> exists, then once more. (Rule zero: save work as you go.)
cd /c/ParanoidBSD/dreamy-mahavira-7e0e79 || exit 1
P=$1; DONE=$2; I=${3:-600}
while true; do
  FINAL=""; [ -f "$DONE" ] && FINAL=1
  git add "$P" >/dev/null 2>&1
  if ! git diff --cached --quiet; then git commit -qm "data: $P (in progress, $(date +%H:%M))

Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>" && git push -q origin HEAD 2>&1 | tail -1; fi
  [ -n "$FINAL" ] && break
  sleep "$I"
done
