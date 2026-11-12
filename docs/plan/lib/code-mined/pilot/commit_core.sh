#!/bin/bash
# commit_core.sh   Commit and push the snapshot of the core run (labels, summary, repair list, milestones; not the per-unit directories, which live in mined-data/prove-core).
# Uses only `git add` of the snapshot files so it never touches other work; a lock keeps two committers apart.
HERE=$(cd "$(dirname "$0")" && pwd); OUT=/mnt/c/ParanoidBSD/mined-data/prove-core; D=$HERE/data2/core
mkdir -p "$D"
cp "$OUT/summary.md" "$OUT/labels.tsv" "$OUT/repair_list.tsv" "$OUT/MILESTONES.md" "$D/" 2>/dev/null
gzip -f "$D/labels.tsv" "$D/repair_list.tsv"
cd "$HERE" || exit 0
( flock -w 120 9 || exit 0
  git add data2/core 2>/dev/null
  git -c user.name="Odin Loch" commit -q -m "pilot: core-tier proving run snapshot ($(wc -l <"$OUT/labels.tsv" 2>/dev/null) functions)

Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>" -- data2/core 2>&1 | tail -1
  git push -q origin HEAD 2>&1 | tail -1 ) 9>"$HOME/kb/git.lock"
