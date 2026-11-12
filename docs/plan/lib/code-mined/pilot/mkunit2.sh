#!/bin/bash
# mkunit2.sh <N> <outdir>   Build the verification unit for sample row N (SAMPLE=sample.tsv) from the REAL translation unit of its source file.
#   1. tu_<tree>.sh preprocesses the source file (or a probe that includes the header) with the flags of its own build; cached in $TUCACHE (not committed, 2-5 MB each)
#   2. tuslice cuts the function out and keeps only the declarations it (transitively) needs; every other function becomes a prototype (no body, no spec)
#   3. the mined contract: `result` -> `\result`; then every macro in it is expanded with the macro definitions that were live at the function (gcc -E -dD stream);
#      an identifier that is a parameter of the function or an ACSL word is never expanded.  Nothing else is changed.
#   4. gcc spellings the Frama-C parser cannot read are rewritten by compat.sed (listed in tu.how)
# Output: <outdir>/uNNN/{meta.tsv, contract.acsl, contract.exp, unit.c, unit.c.callees, tu.how, build.err}.   meta.tsv: N name kind class file tree status(ok|notu|nofunc) detail
set -u
HERE=$(cd "$(dirname "$0")" && pwd); S=${SAMPLE:-/mnt/c/ParanoidBSD/mined-data/pilot/sample.tsv}; N=$1; O=$2
TUCACHE=${TUCACHE:-$HOME/kb/tu}; SLICER=${SLICER:-$HOME/pbsd-tools/tuslice}; mkdir -p "$TUCACHE"
row=$(sed -n "${N}p" "$S"); [ -n "$row" ] || exit 2
IFS=$'\t' read -r key file start end idx name kind contract cls <<<"$row"
U=$O/u$(printf %03d "$N"); mkdir -p "$U"
case "$file" in
  /home/odin/pbsd-ref/linux/tools/*) tree=linux-tools ;;
  /home/odin/pbsd-ref/linux/*) tree=linux ;;
  */hbsd/src/sys/*|*/pbsd/sys/*) tree=bsd-kernel ;;
  */hbsd/*|*/pbsd/*) tree=bsd-user ;;
  *) tree=other ;;
esac
meta() { printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$kind" "$cls" "$file" "$tree" "$1" "$2" >"$U/meta.tsv"; exit 0; }
case "$file" in *.cc|*.cpp|*.cxx|*.hpp|*.hh|*.C) meta notu "C++ source" ;; esac
case "$name" in *[!A-Za-z0-9_]*|"") meta nofunc "function name is not an identifier" ;; esac
# contract
c=$(printf '%s' "$contract" | tr -d '\r' | sed -E 's/(^|[^\\A-Za-z0-9_])result([^A-Za-z0-9_]|$)/\1\\result\2/g' | sed -E 's/(^|[^\\A-Za-z0-9_])result([^A-Za-z0-9_]|$)/\1\\result\2/g')
printf '%s\n' "$c" >"$U/contract.acsl"
# translation unit
key2=$(printf '%s' "$file" | sha256sum | cut -c1-16)-$(basename "$file")
TU=$TUCACHE/$key2.i
if [ ! -s "$TU" ] || [ -f "$TU.fail" ]; then
  rm -f "$TU.fail"
  case "$tree" in
    linux) bash "$HERE/tu_linux.sh" "$file" "$TU" ;;
    bsd-kernel|bsd-user|linux-tools) bash "$HERE/tu_bsd.sh" "$tree" "$file" "$TU" ;;
    *) echo "no flags for tree $tree" >"$TU.err"; false ;;
  esac
  rc=$?
  if [ $rc -ne 0 ]; then touch "$TU.fail"; cp "$TU.err" "$U/build.err" 2>/dev/null; meta notu "preprocess failed: $(grep -m1 -E 'error|No such' "$TU.err" 2>/dev/null | cut -c1-140)"; fi
fi
cp "$TU.how" "$U/tu.how" 2>/dev/null; cp "$TU.stubs" "$U/bsd.stubs" 2>/dev/null
# pass 1: locate the function, get macro definitions in force at it and the identifiers of its head
"$SLICER" "$TU" "$file" "$name" "$start" "$end" - "$U/unit.pass1.c" "$U/defs.txt" 2>"$U/slice.err" || meta nofunc "$(head -1 "$U/slice.err" | cut -c1-140)"
head -c 20000 "$U/unit.pass1.c" >/dev/null
# identifiers never to expand: ACSL words + everything in the function head (parameters, types)
awk 'BEGIN{RS="\\{"} NR==1{print; exit}' "$U/unit.pass1.c" >/dev/null 2>&1
fnhead=$(awk -v n="$name" '{ if (!f && match($0, "(^|[^A-Za-z0-9_])" n "[ \t]*\\(")) f=1; if (f) { print; if (index($0, "{")) exit } }' "$U/unit.pass1.c" | tail -n +1 | tr -c 'A-Za-z0-9_\n' ' ' | tr ' ' '\n' | grep -E '^[A-Za-z_][A-Za-z0-9_]*$' | sort -u)
# expand
inner=$(sed -E 's#^[[:space:]]*/\*@##; s#\*/[[:space:]]*$##' "$U/contract.acsl")
{
  cat "$U/defs.txt"
  for id in $fnhead requires ensures assigns terminates decreases behavior behaviors complete disjoint loop invariant variant valid read_valid separated old at result exit_behavior nothing forall exists integer real ghost lemma logic predicate axiomatic type inductive; do echo "#undef $id"; done
  echo "__PBSD_CONTRACT_BEGIN__"
  printf '%s\n' "$inner" | sed -E 's/\\([A-Za-z_]+)/__BSL_\1/g'
} >"$U/contract.cpp.c"
if ! gcc -E -P -w -x c "$U/contract.cpp.c" -o "$U/contract.cpp.out" 2>"$U/contract.cpp.err"; then meta notu "contract macro expansion failed: $(head -1 "$U/contract.cpp.err" | cut -c1-100)"; fi
exp=$(sed -n '/__PBSD_CONTRACT_BEGIN__/,$p' "$U/contract.cpp.out" | tail -n +2 | tr '\n' ' ' | sed -E 's/__BSL_([A-Za-z_]+)/\\\1/g; s/[[:space:]]+/ /g; s/^ //; s/ $//')
printf '/*@ %s */\n' "$exp" >"$U/contract.exp"
rm -f "$U/contract.cpp.c" "$U/contract.cpp.out"
# pass 2: with the expanded contract
"$SLICER" "$TU" "$file" "$name" "$start" "$end" "$U/contract.exp" "$U/unit.raw.c" "$U/defs.txt" 2>"$U/slice.err" || meta nofunc "$(head -1 "$U/slice.err" | cut -c1-140)"
sed -E -f "$HERE/compat.sed" "$U/unit.raw.c" >"$U/unit.c"
rm -f "$U/unit.pass1.c" "$U/unit.pass1.c.callees" "$U/unit.raw.c"; mv "$U/unit.raw.c.callees" "$U/unit.c.callees" 2>/dev/null
sort -u "$U/unit.c.callees" -o "$U/unit.c.callees" 2>/dev/null; mv "$U/unit.raw.c.sig" "$U/unit.c.sig" 2>/dev/null; rm -f "$U/unit.pass1.c.sig"
gzip -f "$U/defs.txt"
meta ok "$(cat "$U/slice.err")"
