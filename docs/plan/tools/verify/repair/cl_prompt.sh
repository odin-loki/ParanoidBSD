#!/bin/bash
# cl_prompt.sh   stdin: one manual-packet record (jsonl line). stdout: the user message for the Claude repair call (same content as dsv_prompt.py: slice, failure history,
# full function source when the slice does not hold the whole function, macro definition for macro-generated functions). Shell/awk/jq only.
HERE=$(cd "$(dirname "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cat > "$T/rec.json"
jq -r '.functions[].name' "$T/rec.json" > "$T/names"
F=$(jq -r .file "$T/rec.json"); S=$(jq -r .start "$T/rec.json"); E=$(jq -r .end "$T/rec.json")
if [ -f "$F" ]; then awk -f "$HERE/cl_extract.awk" -v FILE="$F" -v S="$S" -v E="$E" -v NAMES="$T/names" > "$T/full" 2>/dev/null; else : > "$T/full"; fi
jq -r --rawfile full "$T/full" '
  "Source slice (lines \(.start)-\(.end) of \(.file | split("/") | last)):\n\(.source)\n\n"
  + (if ($full | length) > 0 then "Full source from the real file (the slice does not contain these functions completely):\n\($full)\n" else "" end)
  + "Functions to fix:\n"
  + ([.functions[] | "[#\(.i)] \(.name) (kind \(.kind // ""), status now: \(.status // ""))\nfailure history: \(((.reasons // "") | if . == "" then "(none)" else .[0:1500] end))\ncurrent prose contract: \(if (.prose // "") == "" then "(none)" else .prose end)\ncurrent formal contract: \(if (.contract // "") == "" then "(none)" else .contract end)\n"] | join("\n"))' "$T/rec.json"
