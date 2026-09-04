#!/bin/bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Source this file, then call pbsd_load_secrets.
# Reads KEY=value from secrets/api-keys (never prints values).
PBSD_WINDOWS_ROOT="${PBSD_WINDOWS_ROOT:-/mnt/c/Users/odinl/OneDrive/Desktop/Operating System}"

_pbsd_apply_secrets_file() {
  local file="$1" line key val
  [ -f "$file" ] || return 1
  while IFS= read -r line || [ -n "$line" ]; do
    line="${line%$'\r'}"
    case "$line" in
      ''|\#*|export\ \#*) continue ;;
    esac
    line="${line#export }"
    case "$line" in
      *=*) ;;
      *) continue ;;
    esac
    key="${line%%=*}"
    val="${line#*=}"
    key="${key%"${key##*[![:space:]]}"}"
    key="${key#"${key%%[![:space:]]*}"}"
    val="${val%"${val##*[![:space:]]}"}"
    val="${val#"${val%%[![:space:]]*}"}"
    if [ "${#val}" -ge 2 ]; then
      case "$val" in
        \"*\") val="${val#\"}"; val="${val%\"}" ;;
        \'*\') val="${val#\'}"; val="${val%\'}" ;;
      esac
    fi
    [ -n "$key" ] && [ -n "$val" ] || continue
    case "$key" in
      *[!A-Za-z0-9_]*) continue ;;
    esac
    # Do not override a non-empty value already in the environment.
    eval "cur=\${$key-}"
    if [ -z "$cur" ]; then
      export "$key=$val"
    fi
  done < "$file"
  return 0
}

pbsd_load_secrets() {
  local f
  for f in \
    "${PBSD_SECRETS:-}" \
    "$PBSD_WINDOWS_ROOT/secrets/api-keys" \
    "$PBSD_WINDOWS_ROOT/secrets/api-keys.env" \
    "${HOME}/pbsd/secrets/api-keys"
  do
    [ -n "$f" ] || continue
    if _pbsd_apply_secrets_file "$f"; then
      return 0
    fi
  done
  return 0
}
