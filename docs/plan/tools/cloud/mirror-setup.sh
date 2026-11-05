#!/usr/bin/env bash
# On the PC (WSL2), once per mirror box: seed the bare mirror with the full PBSD history so workers
# only fetch the small per-task gate commits afterwards, then build the worker's read-only hbsd/ and
# kde/ reference trees from it.
#   tools/cloud/mirror-setup.sh <pbsd-repo-path> [mirror-alias] [base-branch]
set -euo pipefail
repo="${1:?path to the PBSD repo}"; m="${2:-pbsd-mirror}"; base="${3:-pbsd-base}"
ssh -o BatchMode=yes "$m" 'test -d /srv/pbsd/pbsd.git' || { echo "$m has no /srv/pbsd/pbsd.git (bootstrap with PBSD_IS_MIRROR=1)"; exit 1; }
git -C "$repo" push --mirror "$m:/srv/pbsd/pbsd.git"
ssh -o BatchMode=yes "$m" "/opt/pbsd/bin/pbsd-ref-setup $base"
# PRISM is the proof checker: copy the PC's build (same Ubuntu 24.04) and its ESBMC/CBMC support to the worker.
if [ -n "${PRISM_BIN:-}" ] && [ -x "$PRISM_BIN" ]; then
  scp -q "$PRISM_BIN" "$m:/opt/pbsd/prism/prism" && ssh "$m" '/opt/pbsd/prism/prism --version' || echo "WARNING: PRISM copy failed or does not run on the worker"
else echo "NOTE: set PRISM_BIN=/path/to/prism to copy PRISM to the worker (proof gates exit 3 without it)"; fi
echo "mirror seeded; runner pushes refs/gates/<task> to $m:/srv/pbsd/pbsd.git"
