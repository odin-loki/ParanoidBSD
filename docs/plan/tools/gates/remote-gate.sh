#!/usr/bin/env bash
# Runs on a cloud worker as /opt/pbsd/bin/pbsd-gate <role> <task-id> <commit>.
# Fetches the commit from the mirror into a tmpfs worktree, runs the same tools/gates/<role>.sh
# the PC would run, prints the log and removes the worktree. Exit 3 = infrastructure error.
set -uo pipefail
role="$1"; id="$2"; sha="$3"
. /etc/pbsd/worker.env 2>/dev/null || true
cache="${PBSD_CACHE_REPO:-/run/pbsd/cache.git}"; wt="/run/pbsd/wt/$id-$$"
mkdir -p /run/pbsd/wt
trap 'rm -rf "$wt"' EXIT
# One shared object cache per worker; each gate gets a cheap worktree from it.
[ -d "$cache" ] || git clone -q --bare "$PBSD_MIRROR" "$cache" || exit 3
for i in 1 2 3; do
  flock "$cache.lock" git -C "$cache" fetch -q "$PBSD_MIRROR" "+refs/gates/$id:refs/gates/$id" && break
  sleep $((i*5)); [ $i -eq 3 ] && { echo "fetch of $id failed"; exit 3; }
done
[ "$(git -C "$cache" rev-parse "refs/gates/$id")" = "$sha" ] || { echo "mirror has a different commit for $id"; exit 3; }
flock "$cache.lock" git -C "$cache" worktree add -q --detach "$wt" "$sha" || exit 3
export CCACHE_DIR=/run/pbsd/ccache PBSD_PROOF_CACHE=/run/pbsd/proof-cache TMPDIR=/run/pbsd/tmp
mkdir -p "$TMPDIR"
gate="$wt/tools/gates/$role.sh"; [ -f "$gate" ] || gate="/opt/pbsd/gates/$role.sh"
bash "$gate" "$id" "$wt"; rc=$?
flock "$cache.lock" git -C "$cache" worktree remove --force "$wt" 2>/dev/null
exit $rc
