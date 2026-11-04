#!/usr/bin/env bash
# On the PC (WSL2), once per mirror box: seed the bare mirror with the full PBSD history so
# workers only fetch the small per-task gate commits afterwards.
#   tools/cloud/mirror-setup.sh <pbsd-repo-path> [mirror-alias]
set -euo pipefail
repo="${1:?path to the PBSD repo}"; m="${2:-pbsd-mirror}"
ssh -o BatchMode=yes "$m" 'test -d /srv/pbsd/pbsd.git' || { echo "$m has no /srv/pbsd/pbsd.git (bootstrap with PBSD_IS_MIRROR=1)"; exit 1; }
git -C "$repo" push --mirror "$m:/srv/pbsd/pbsd.git"
echo "mirror seeded; runner pushes refs/gates/<task> to $m:/srv/pbsd/pbsd.git"
