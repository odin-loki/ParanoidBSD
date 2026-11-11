#!/bin/bash
# finish_repair.sh   Git Bash on Windows. Every hour: gzip snapshot of the repairs (repair/repair.jsonl) into the repository, commit, push. When the repair run and its
# re-verification are finished (verify-repair/summary.json newer than repair/DONE): commits the re-verified tags and exits. Rule zero: save work as you go.
R=/c/ParanoidBSD/dreamy-mahavira-7e0e79; RP=/c/ParanoidBSD/mined-data/verify/repair; W=/c/ParanoidBSD/mined-data/verify-repair; O=$R/docs/plan/lib/code-mined/repair
mkdir -p "$O"; cd "$R" || exit 1
snap() {
  [ -s "$RP/repair.jsonl" ] && gzip -9 -c "$RP/repair.jsonl" > "$O/repair.jsonl.gz"; cp "$RP/repair.log" "$O/repair.log" 2>/dev/null
  if [ "$2" = final ]; then gzip -9 -c "$W/repair-tags.jsonl" > "$O/repair-tags.jsonl.gz"; cp "$W/summary.json" "$O/summary.json"; fi
  git add docs/plan/lib/code-mined/repair >/dev/null 2>&1
  git diff --cached --quiet || { git commit -qm "data: annotation repairs ($(wc -l < "$RP/repair.jsonl") records, $1)

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" && git push -q origin HEAD 2>&1 | tail -1; }
}
while true; do
  if [ -f "$RP/DONE" ] && [ -f "$W/summary.json" ] && [ "$W/summary.json" -nt "$RP/DONE" ]; then snap "final, re-verified" final; echo "$(date '+%F %T') done"; break; fi
  [ -f "$RP/STOP" ] && { snap "stopped"; echo "STOP found"; break; }
  snap "$(date '+%H:%M')"; echo "$(date '+%F %T') snapshot committed; next in 1 hour"
  sleep 3600
done
