#!/bin/bash
# mkunit.sh <N> <outdir>   Build the verification unit for sample row N (sample.tsv): <outdir>/uNNN/{unit.c,unit.min.c,meta.tsv,...}.
#   unit.c      = prelude + all #defines of the file + top-level types of the file + [contract] + function     (level L2)
#   unit.min.c  = prelude + all #defines of the file + [contract] + function                                (level L1, fallback)
# The contract is the mined text with the bare word `result` -> `\result`; nothing else is changed (no strengthening, no weakening).
# The function text is the file's text, unmodified (mechanical extraction, no model).
# meta.tsv: N name kind class file lines sha256(contract) sha256(function text) sha256(prelude) status   (status: ok | nofunc)
set -u
HERE=$(cd "$(dirname "$0")" && pwd); S=${SAMPLE:-/mnt/c/ParanoidBSD/mined-data/pilot/sample.tsv}; N=$1; O=$2
row=$(sed -n "${N}p" "$S"); [ -n "$row" ] || exit 2
IFS=$'\t' read -r key file start end idx name kind contract cls <<<"$row"
U=$O/u$(printf %03d "$N"); mkdir -p "$U"
PRE=${PRELUDE:-$HERE/prelude.h}
if ! awk -v name="$name" -v start="$start" -v end="$end" -f "$HERE/extract_fn.awk" "$file" >"$U/fn.c" 2>"$U/fn.lines"; then
  printf '%s\t%s\t%s\t%s\t%s\t%s\t-\t-\t-\tnofunc\n' "$N" "$name" "$kind" "$cls" "$file" "$start-$end" >"$U/meta.tsv"; exit 0
fi
# result -> \result, only as a whole word not already preceded by a backslash (done twice so adjacent matches are all caught)
c=$(printf '%s' "$contract" | tr -d '\r' | sed -E 's/(^|[^\\A-Za-z0-9_])result([^A-Za-z0-9_]|$)/\1\\result\2/g' | sed -E 's/(^|[^\\A-Za-z0-9_])result([^A-Za-z0-9_]|$)/\1\\result\2/g')
printf '%s\n' "$c" >"$U/contract.acsl"
awk -v skip="$name" -f "$HERE/extract_defs.awk" "$file" >"$U/defs.h"
awk -f "$HERE/extract_types.awk" "$file" >"$U/types.h"
cp "$PRE" "$U/prelude.h"
{ echo '#include "prelude.h"'; cat "$U/defs.h"; cat "$U/types.h"; cat "$U/contract.acsl"; cat "$U/fn.c"; } >"$U/unit.c"
{ echo '#include "prelude.h"'; cat "$U/defs.h"; cat "$U/contract.acsl"; cat "$U/fn.c"; } >"$U/unit.min.c"
h() { sha256sum | cut -c1-16; }
printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\tok\n' "$N" "$name" "$kind" "$cls" "$file" "$(cat "$U/fn.lines")" "$(h <"$U/contract.acsl")" "$(h <"$U/fn.c")" "$(h <"$U/prelude.h")" >"$U/meta.tsv"
