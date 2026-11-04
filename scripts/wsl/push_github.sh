#!/bin/bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Commit WSL work. Windows pulls via wslclone (scripts/sync-wsl-github.ps1).
set -euo pipefail
cd ~/pbsd
git add -A
git diff --cached --quiet && git diff --quiet || \
  git commit -q -m "pbsd: push sync $(date -u +%FT%TZ)" || true
echo "$(date -u +%FT%TZ) committed $(git rev-parse --short HEAD) — Windows sync via wslclone" >> ~/pbsd_sync.log
