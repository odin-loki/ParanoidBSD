#!/bin/bash
# finish_step4.sh   Runs in Git Bash on Windows (it has the GitHub credentials; WSL does not). Every hour: copies a gzip snapshot of the step 4 results into the
# repository and commits and pushes it. When step 4 has finished (xcheck/DONE-batch and no step4_all.sh running): re-runs the tagging (tag_all.sh, in WSL),
# commits the final tags and results, and exits. Rule zero: save work as you go.
R=/c/ParanoidBSD/dreamy-mahavira-7e0e79; X=/c/ParanoidBSD/mined-data/verify/xcheck; O=$R/docs/plan/lib/code-mined/verify-results
cd "$R" || exit 1
snap() {
  gzip -9 -c "$X/xcheck.jsonl" > "$O/xcheck.jsonl.gz"; cp "$X/watch.log" "$O/step4-watch.log" 2>/dev/null
  git add docs/plan/lib/code-mined >/dev/null 2>&1
  git diff --cached --quiet || { git commit -qm "data: step 4 cross-check snapshot ($(wc -l < "$X/xcheck.jsonl") records, $1)

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" && git push -q origin HEAD 2>&1 | tail -1; }
}
while true; do
  # "[s]tep4" so that this command's own shell (whose command line contains the pattern) does not count as a running step 4
  running=$(MSYS_NO_PATHCONV=1 wsl -d Ubuntu -- bash -c 'pgrep -fc "[s]tep4_all.sh"' 2>/dev/null | tr -d '\r')
  if [ -f "$X/DONE-batch" ] && [ "${running:-0}" = "0" ]; then
    echo "$(date '+%F %T') step 4 finished: re-tagging"
    MSYS_NO_PATHCONV=1 wsl -d Ubuntu -- bash /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/tag_all.sh > /dev/null 2>&1
    MSYS_NO_PATHCONV=1 wsl -d Ubuntu -- bash /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/copy_results.sh > /dev/null 2>&1
    snap "final, tags re-generated"
    echo "$(date '+%F %T') done"; break
  fi
  [ -f "$X/STOP" ] && { snap "stopped"; echo "STOP found"; break; }
  snap "$(date '+%H:%M')"
  echo "$(date '+%F %T') snapshot committed; next in 1 hour"
  sleep 3600
done
