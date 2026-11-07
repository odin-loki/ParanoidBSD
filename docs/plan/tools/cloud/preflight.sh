#!/usr/bin/env bash
# Everything that must be true before `pbsd_run init && pbsd_run run`. Prints OK / WARN / FAIL per check and exits 1 on any FAIL.
# Run it after `. ~/.pbsd/env`. With a MORPH_API_KEY it also makes one small real API call (a few hundredths of a cent).
#   tools/cloud/preflight.sh
set -uo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; plan="$(cd "$here/../.." && pwd)"
[ -f "$HOME/.pbsd/env" ] && . "$HOME/.pbsd/env"
fails=0; warns=0
ok()   { printf '  OK    %s\n' "$*"; }
warn() { printf '  WARN  %s\n' "$*"; warns=$((warns+1)); }
bad()  { printf '  FAIL  %s\n' "$*"; fails=$((fails+1)); }
chk()  { local what="$1"; shift; if "$@" >/dev/null 2>&1; then ok "$what"; else bad "$what"; fi; }

echo "== machine"
ram=$(awk '/MemTotal/ {printf "%d", $2/1048576}' /proc/meminfo); [ "$ram" -ge 40 ] && ok "RAM visible to WSL: ${ram} GB" || warn "RAM visible to WSL only ${ram} GB (.wslconfig memory=)"
if command -v nvidia-smi >/dev/null; then
  read -r used total < <(nvidia-smi --query-gpu=memory.used,memory.total --format=csv,noheader,nounits | head -1 | tr -d ',')
  [ "${used:-9999}" -lt 2000 ] && ok "GPU free: ${used} of ${total} MiB in use" || warn "GPU busy: ${used} of ${total} MiB in use (training or another model running?)"
else bad "nvidia-smi not found (GPU not visible in WSL)"; fi
df -P /run /mnt/d 2>/dev/null | awk 'NR>1 && $4<20000000 {print "  WARN  low disk on " $6 ": " int($4/1048576) " GB free"}'

echo "== tools"
for t in jq git tmux clang++ cmake ninja bwrap numactl curl; do chk "$t" command -v "$t"; done
chk "pbsd_run built"      test -x "${PBSD_RUN:-$HOME/pbsd-run-build/pbsd_run}"
chk "PRISM runs"          "${PRISM_BIN:-prism}" --version
for t in esbmc cbmc cppcheck; do ls "${PRISM_TOOLS_DIR:-$HOME/.prism/tools}"/$t/*/bin/$t >/dev/null 2>&1 && ok "$t (pinned, via PRISM)" || bad "$t missing: tools/cloud/build-prism.sh"; done
chk "lake"                command -v lake
chk "apalache-mc"         command -v apalache-mc
[ -d "${PBSD_LEAN_PACKAGES:-/nonexistent}/mathlib" ] && ok "Mathlib packages at $PBSD_LEAN_PACKAGES" || bad "Mathlib packages missing: tools/cloud/lean-mathlib.sh"

echo "== models"
if [ -z "${PRISM_PROVER_FPK:-}" ]; then
  ok "Goedel prover is off (decided 7 Nov 2026): proofs are written by GLM and checked by lean_check and the gate"
else
  [ -f "$PRISM_PROVER_FPK" ] && ok "prover (first pass): $PRISM_PROVER_FPK" || bad "PRISM_PROVER_FPK is not a file"
  [ -z "${PBSD_PROVER_FPK_ESCALATE:-}" ] && warn "no escalation model set" || { [ -f "$PBSD_PROVER_FPK_ESCALATE" ] && ok "prover (escalation): $PBSD_PROVER_FPK_ESCALATE" || bad "PBSD_PROVER_FPK_ESCALATE is not a file"; }
  [ "${PRISM_DEVICE:-}" = cuda ] && ok "PRISM_DEVICE=cuda" || warn "PRISM_DEVICE is '${PRISM_DEVICE:-unset}': the prover will run on the CPU"
fi

echo "== layout"
for d in /run/pbsd/state /run/pbsd/wt /mnt/d/pbsd-ckpt /opt/pbsd/gates; do [ -d "$d" ] && [ -w "$d" ] && ok "$d writable" || bad "$d missing or not writable: tools/cloud/pc-setup.sh"; done
if [ -d /opt/pbsd/gates ]; then
  if diff -rq "$plan/tools/gates" /opt/pbsd/gates >/dev/null 2>&1; then ok "pinned gates match the repository"; else bad "/opt/pbsd/gates differs from the repository: tools/cloud/pc-setup.sh"; fi
fi
cmp -s "$plan/tasks.json" /opt/pbsd/tasks.json && ok "pinned tasks.json matches" || bad "/opt/pbsd/tasks.json differs or is missing"
[ -d "$HOME/pbsd-ref/linux/.git" ] && ok "Linux reference tree: $(cat "$HOME/pbsd-ref/TAGS" 2>/dev/null)" || bad "Linux reference tree missing (280 pilot tasks read it)"
[ -d /run/pbsd/repo/.git ] && ok "PBSD repository at /run/pbsd/repo" || bad "no repository at /run/pbsd/repo (the runner works on a clone; see CLOUD_SETUP.md step 10)"
[ -f "${PBSD_CONFIG:-}" ] && jq -e . "$PBSD_CONFIG" >/dev/null 2>&1 && ok "config parses: $PBSD_CONFIG" || bad "config missing or not JSON"

echo "== Morph"
if [ -z "${MORPH_API_KEY:-}" ]; then bad "MORPH_API_KEY is empty (edit ~/.pbsd/env)"; else
  r="$(curl -s -m 60 https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' \
       -d '{"model":"morph-glm53flash","max_tokens":200,"messages":[{"role":"user","content":"Say ok"}]}')"
  if jq -e '.choices[0]' <<<"$r" >/dev/null 2>&1; then ok "Morph answers: $(jq -r '.choices[0].message.content // "(empty content)"' <<<"$r" | head -c 40)"
    jq -e '.usage.prompt_tokens_details.cached_tokens != null' <<<"$r" >/dev/null 2>&1 && ok "usage reports cached_tokens (cache hits can be billed correctly)" || warn "no prompt_tokens_details.cached_tokens in usage: cost shown will be too high"
  else bad "Morph call failed: $(jq -r '.error.message // .message // empty' <<<"$r" 2>/dev/null | head -c 120) $(head -c 120 <<<"$r")"; fi
fi

echo "== reminders (cannot be checked from here)"
echo "  Windows must not sleep or update during the run: Settings > Power, sleep = Never, and pause updates."
echo "  Run the runner inside tmux:  tmux new -s pbsd   then   pbsd_run init && pbsd_run run"
echo; echo "preflight: $fails FAIL, $warns WARN"; [ "$fails" -eq 0 ]
