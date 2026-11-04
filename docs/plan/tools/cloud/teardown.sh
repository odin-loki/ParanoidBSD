#!/usr/bin/env bash
# On the PC: drain every worker and print what is still billing. Deleting the machines is done in
# each provider's console or CLI (commands are in docs/plan/CLOUD_SETUP.md step 14).
set -euo pipefail
f="${PBSD_WORKERS:-$HOME/.pbsd/workers.txt}"
[ -f "$f" ] && { cp "$f" "$f.bak.$(date +%s)"; grep '^#' "$f" > "$f.tmp" || true; mv "$f.tmp" "$f"; echo "drained all workers (backup kept)"; }
echo "Waiting 60 s for gates in flight to finish..."; sleep 60
command -v gcloud >/dev/null && gcloud compute instances list --filter='labels.pbsd=1' 2>/dev/null || true
command -v aws >/dev/null && aws ec2 describe-instances --filters Name=tag:pbsd,Values=1 Name=instance-state-name,Values=running --query 'Reservations[].Instances[].InstanceId' --output text 2>/dev/null || true
command -v hcloud >/dev/null && hcloud server list -l pbsd=1 2>/dev/null || true
echo "Anything listed above is still billing. Delete it (not stop) and also delete its disks."
