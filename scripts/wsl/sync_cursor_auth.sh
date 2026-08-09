#!/bin/bash
# Sync JWT tokens from Cursor IDE (Windows) into WSL auth.json.
set -e
unset CURSOR_API_KEY
REPO="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System"
EXTRACT="$REPO/scripts/wsl/extract_tokens.py"
OUT="/mnt/c/Users/odinl/AppData/Local/Temp/wsl_auth.json"
PY="/mnt/c/Windows/System32/py.exe"
if [ ! -f "$PY" ]; then
  PY="/mnt/c/Windows/py.exe"
fi
if ! "$PY" -3 "$EXTRACT" >/dev/null 2>&1; then
  echo "warn: could not refresh tokens from IDE (continuing with cached auth)"
  exit 0
fi
mkdir -p "$HOME/.config/cursor"
cp "$OUT" "$HOME/.config/cursor/auth.json"
chmod 600 "$HOME/.config/cursor/auth.json"
