#!/usr/bin/env bash
# PRISM proof-check hook (placeholder until the PBSD proof layout is fixed).
# Checks every model under <tree>/proofs: foo.pm (or .prism) with foo.props next to it.
# Every property must come back true; "false" or an error fails the gate.
# Cached by file hash in $PBSD_PROOF_CACHE so a discharged model is not re-checked.
set -uo pipefail
wt="$1"; cache="${PBSD_PROOF_CACHE:-/run/pbsd/proof-cache}"; mkdir -p "$cache"
command -v prism >/dev/null || { echo "prism not installed"; exit 3; }
[ -d "$wt/proofs" ] || { echo "no proofs/ directory: nothing to check"; exit 0; }
fail=0; n=0
while IFS= read -r -d '' m; do
  props="${m%.*}.props"; [ -f "$props" ] || { echo "missing $props"; fail=1; continue; }
  key=$(cat "$m" "$props" | sha256sum | cut -c1-32)
  [ -f "$cache/$key" ] && { echo "cached ok: ${m#$wt/}"; continue; }
  n=$((n+1)); out=$(prism -javamaxmem "${PBSD_PRISM_MEM:-6g}" "$m" "$props" 2>&1); rc=$?
  if [ $rc -ne 0 ] || grep -Eq '^Result: false|^Error' <<<"$out"; then echo "FAIL ${m#$wt/}"; tail -40 <<<"$out"; fail=1
  else echo "ok ${m#$wt/}"; touch "$cache/$key"; fi
done < <(find "$wt/proofs" -type f \( -name '*.pm' -o -name '*.prism' \) -print0)
echo "checked $n model(s)"; exit $fail
