#!/usr/bin/env bash
# On the PC (WSL2 Ubuntu 24.04), after pc-tools.sh and build-prism.sh: create the directories the runner uses, install the pinned
# gate scripts and task list, and write ~/.pbsd/env and ~/.pbsd/config.json sized for THIS machine. Idempotent. Never writes an API key.
#   tools/cloud/pc-setup.sh                      (uses sudo for /opt/pbsd and /run/pbsd; or run it as root with PBSD_USER=<you>)
# /run is a tmpfs, so /run/pbsd is empty after every WSL restart: rerun this (or tools/cloud/preflight.sh --fix) before `pbsd_run init`.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; plan="$(cd "$here/../.." && pwd)"
u="${PBSD_USER:-${SUDO_USER:-$USER}}"; home="$(getent passwd "$u" | cut -d: -f6)"
priv() { if [ "$(id -u)" -eq 0 ]; then "$@"; else sudo "$@"; fi; }


# 1. directories and the pinned copies the gates run (agents can never change these: the gate never uses the copy in its own commit)
priv install -d -o "$u" -g "$u" /opt/pbsd /opt/pbsd/gates /run/pbsd /run/pbsd/state /run/pbsd/wt
priv install -m 755 -o "$u" -g "$u" "$plan"/tools/gates/*.sh /opt/pbsd/gates/
priv install -m 644 -o "$u" -g "$u" "$plan/tasks.json" /opt/pbsd/tasks.json
priv install -d -o "$u" -g "$u" /opt/pbsd/templates /opt/pbsd/templates/model-lake
priv cp -r "$plan/templates/model-lake/." /opt/pbsd/templates/model-lake/ && priv chown -R "$u":"$u" /opt/pbsd/templates   # pinned: the Lean spec gate builds from this copy
mkdir -p /mnt/d/pbsd-ckpt "$home/.pbsd" || echo "WARNING: could not create /mnt/d/pbsd-ckpt (is D: mounted?)"

# 2. the runner config, derived from the shipped one (Morph, one provider) and sized for this machine
base="$plan/tools/runner/config.glm-api.json"
ram_gb="$(awk '/MemTotal/ {printf "%d", $2/1048576}' /proc/meminfo)"
# WSL here is capped at 48 GB (.wslconfig). The shipped slots assume ~140 GB, so: compile 4 x 2 GB, qemu 1 x 4 GB, proof 3 x 6 GB.
# prover_slots stays 1 while a 32B escalation is configured: it needs ~22 GB of the 3090's 24 GB, so nothing else may hold the GPU.
filter="$(mktemp)"
cat > "$filter" <<'JQ'
  .slots.agent.count = 16
| .slots.compile = {count:4, cores:1, ram_gb:2, node:0} | .slots.qemu = {count:1, cores:2, ram_gb:4, node:0}
| .slots.proof = {count:3, cores:2, ram_gb:6, node:-1}
| .providers[0].concurrency = 16
| .gates_dir = "/opt/pbsd/gates" | .checkpoint_dir = "/mnt/d/pbsd-ckpt" | .checkpoint_repo = "/mnt/d/pbsd-ckpt/pbsd.git"
| .ref_roots = {linux: ($home + "/pbsd-ref/linux"), wine: ($home + "/pbsd-ref/wine"), hbsd: "/run/pbsd/repo/hbsd", kde: "/run/pbsd/repo/kde"}
| .prism_bin = ($home + "/PRISM/build/prism") | .prover_slots = 1 | .reasoning_effort = "low"
| .lean_packages = ($home + "/lean49/.lake/packages") | .prover_fpk_escalate = ($home + "/models/goedel32.fpk")
JQ
jq --arg home "$home" -f "$filter" "$base" > "$home/.pbsd/config.json"; rm -f "$filter"

# 3. the environment file (secrets are NOT written: add MORPH_API_KEY yourself)
if [ ! -f "$home/.pbsd/env" ]; then
  cat > "$home/.pbsd/env" <<ENV
# PBSD runner environment. Source it from ~/.bashrc: . ~/.pbsd/env   (gates never see keys: the runner scrubs *KEY* and *TOKEN*)
export MORPH_API_KEY=                         # <- paste your Morph key here; never commit this file
export PBSD_STATE=/run/pbsd/state PBSD_WT=/run/pbsd/wt PBSD_REPO=/run/pbsd/repo PBSD_CHECKPOINT=/mnt/d/pbsd-ckpt
export PBSD_CONFIG=$home/.pbsd/config.json PBSD_TASKS=/opt/pbsd/tasks.json
export PBSD_JOBS=2
# Prover: PRISM runs the Goedel model in process on the GPU. 8B first; the 32B is the automatic retry when no proof is found.
export PRISM_BIN=$home/PRISM/build/prism PRISM_TOOLS_DIR=$home/.prism/tools PRISM_DEVICE=cuda
export PRISM_PROVER_FPK=/mnt/d/forgebox/models/goedel-8b-q4_k.fpk PBSD_PROVER_FPK_ESCALATE=$home/models/goedel32.fpk
export PBSD_LEAN_PACKAGES=$home/lean49/.lake/packages
# Interim per-unit build and test (the whole pbsd/ tree does not build yet; P0-07 replaces these)
export PBSD_BUILD_CMD=/opt/pbsd/gates/unit_build.sh PBSD_TEST_CMD=/opt/pbsd/gates/unit_test.sh
export PATH=\$HOME/.elan/bin:\$PATH
ENV
  chmod 600 "$home/.pbsd/env"; echo "wrote $home/.pbsd/env (add MORPH_API_KEY)"
else echo "kept existing $home/.pbsd/env"; fi
[ "$(id -u)" -eq 0 ] && chown -R "$u":"$u" "$home/.pbsd" /mnt/d/pbsd-ckpt 2>/dev/null || true

# 4. the repository the runner works on, in tmpfs. It needs a checkout of docs/plan (prompts, plan) and hbsd/ + kde/ (reference sources);
#    task worktrees are cut from it and stay sparse. Rerun this after every WSL restart (/run is wiped).
repo_url="${PBSD_REPO_URL:-https://github.com/odin-loki/ParanoidBSD}"; base_branch="${PBSD_BASE_BRANCH:-claude/dreamy-mahavira-7e0e79}"
if [ ! -d /run/pbsd/repo/.git ]; then
  echo "cloning $repo_url ($base_branch) into /run/pbsd/repo (about 700 MB of history) ..."
  git clone -q --no-checkout --branch "$base_branch" "$repo_url" /run/pbsd/repo
  git -C /run/pbsd/repo sparse-checkout set docs/plan hbsd kde
  git -C /run/pbsd/repo checkout -q -B pbsd-cutover "origin/$base_branch"   # pbsd-cutover is the integration branch (merge_target)
  [ "$(id -u)" -eq 0 ] && chown -R "$u":"$u" /run/pbsd/repo || true
else echo "kept /run/pbsd/repo ($(git -C /run/pbsd/repo rev-parse --short HEAD))"; fi
echo "RAM visible to WSL: ${ram_gb} GB. Config: $home/.pbsd/config.json"
echo "next: . ~/.pbsd/env   then   tools/cloud/preflight.sh"
