#!/usr/bin/env bash
# Runs on a gate worker as /opt/pbsd/bin/pbsd-gate <role> <task-id> <commit> [<proof-tier>].
# Fetches the commit from the control machine (git daemon on the private network) into a RAM
# worktree with the same sparse pattern as the agents' worktrees, runs the PINNED gate script from
# /opt/pbsd/gates (never the copy inside the commit, which an agent could have changed), prints the
# log and removes the worktree. Exit 3 = infrastructure error (the runner retries elsewhere).
set -uo pipefail
role="$1"; id="$2"; sha="$3"; tier="${4:-}"; [ "$tier" = - ] && tier=""; export PBSD_PROOF_TIER="$tier"
set -a; . /etc/pbsd/worker.env 2>/dev/null || true; set +a
cache="${PBSD_CACHE_REPO:-/run/pbsd/cache.git}"; wt="/run/pbsd/wt/$id-$$"
sparse="${PBSD_SPARSE:-/* !/hbsd/ !/kde/ !/linux/ !/docs/migration/ !/docs/verify/ !/docs/PORT_PLAN.md}"
mkdir -p /run/pbsd/wt
trap 'flock "$cache.lock" git -C "$cache" worktree remove --force "$wt" 2>/dev/null; rm -rf "$wt"' EXIT
# One shared object cache per worker; each gate gets a cheap sparse worktree from it.
[ -d "$cache" ] || flock "$cache.lock" git clone -q --bare "$PBSD_MIRROR" "$cache" || exit 3
for i in 1 2 3; do
  flock "$cache.lock" git -C "$cache" fetch -q "$PBSD_MIRROR" "+refs/gates/$id:refs/gates/$id" && break
  sleep $((i*5)); [ $i -eq 3 ] && { echo "fetch of $id failed"; exit 3; }
done
[ "$(git -C "$cache" rev-parse "refs/gates/$id")" = "$sha" ] || { echo "mirror has a different commit for $id"; exit 3; }
flock "$cache.lock" git -C "$cache" worktree add -q --no-checkout --detach "$wt" "$sha" || exit 3
# shellcheck disable=SC2086
git -C "$wt" sparse-checkout set --no-cone $sparse && git -C "$wt" checkout -q || exit 3
export CCACHE_DIR=/run/pbsd/ccache PBSD_PROOF_CACHE=/run/pbsd/proof-cache TMPDIR=/run/pbsd/tmp
export PBSD_REF="${PBSD_REF:-/srv/ref}" PBSD_BUILD_DIR="/run/pbsd/build/$id-$$"
mkdir -p "$TMPDIR" "$PBSD_BUILD_DIR"
gate="/opt/pbsd/gates/$role.sh"; [ -f "$gate" ] || { echo "no pinned gate for $role"; exit 3; }
# No network for agent-written code: a private network namespace when unshare is allowed.
if unshare --net --map-root-user true 2>/dev/null; then
  unshare --net --map-root-user bash "$gate" "$id" "$wt"; rc=$?
else
  bash "$gate" "$id" "$wt"; rc=$?
fi
rm -rf "$PBSD_BUILD_DIR"
exit $rc
