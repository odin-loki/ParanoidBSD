#!/usr/bin/env bash
# On the PC (WSL2): register a worker with the runner. pbsd_run re-reads the list every 30 s.
#   tools/cloud/register.sh <ssh-alias> [slots]      add or update
#   tools/cloud/register.sh --remove <ssh-alias>     drain: no new gates go there
set -euo pipefail
f="${PBSD_WORKERS:-$HOME/.pbsd/workers.txt}"; mkdir -p "$(dirname "$f")"; touch "$f"
if [ "${1:-}" = --remove ]; then sed -i "/^$2 /d" "$f"; echo "removed $2"; cat "$f"; exit 0; fi
h="${1:?usage: register.sh <ssh-alias> [slots]}"
ssh -o BatchMode=yes -o ConnectTimeout=10 "$h" true || { echo "cannot ssh to $h"; exit 1; }
slots="${2:-$(ssh "$h" "echo \$(( (\$(lscpu -p=CORE,SOCKET | grep -v '^#' | sort -u | wc -l) - 4) / 2 ))")}"
ssh "$h" "test -x /opt/pbsd/bin/pbsd-gate" || { echo "$h has no /opt/pbsd/bin/pbsd-gate: run bootstrap.sh"; exit 1; }
sed -i "/^$h /d" "$f"; echo "$h $slots" >> "$f"; echo "registered $h with $slots slots"; cat "$f"
