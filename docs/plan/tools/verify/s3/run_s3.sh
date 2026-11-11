#!/bin/bash
# run_s3.sh   Step 3 of VERIFICATION_PLAN.md: random testing of pure helpers against their mined contracts.
# Candidates: kind arith-helper, bitmap, string-op or table-lookup, with a formal contract, step 1 pass and step 2 pass. Each function is extracted from its
# source, its contract is translated to C by contract2c (exact 128-bit integer semantics; unsupported constructs are skipped and labelled), and a harness draws
# 200000 random and edge-value integer inputs: every input that meets `requires` must meet `ensures`. A counterexample means the contract is wrong.
# This replaces the plan's libFuzzer/AFL + E-ACSL design: E-ACSL's runtime could not be built without e-acsl-gcc.sh (not in the Ubuntu package), and for scalar
# functions coverage guidance adds little over edge values.
# Scratch: ~/s3-scratch (regenerable). Results: /mnt/c/ParanoidBSD/mined-data/verify/s3/s3.jsonl and summary.json.
HERE=$(cd "$(dirname "$0")" && pwd); V=${VERIFY_DIR:-/mnt/c/ParanoidBSD/mined-data/verify}; O=$V/s3; S=${SCRATCH_DIR:-$HOME/s3-scratch}; mkdir -p "$O" "$S"
log() { echo "$(date '+%F %T') $*" | tee -a "$O/s3.log"; }
g++ -O2 -std=c++20 "$HERE/contract2c.cpp" -o "$S/contract2c" || { log "contract2c does not build"; exit 1; }
log "selecting candidates"
jq -r 'select(.has_contract and .s1=="pass" and .s2=="pass" and (.kind=="arith-helper" or .kind=="bitmap" or .kind=="string-op" or .kind=="table-lookup")) | "\(.key)\t\(.i)"' "$V/step12.jsonl" > "$S/cand.tsv"
awk -F'\t' 'FILENAME == ARGV[1] { c[$1 SUBSEP $2] = 1; next } (($1 SUBSEP $5) in c)' "$S/cand.tsv" "$V/rows.tsv" | sort -t "$(printf '\t')" -k2,2 -k3,3n -s > "$S/rows.tsv"
log "candidates: $(wc -l < "$S/rows.tsv")"
gawk -f "$HERE/s3_gen.awk" "$S/rows.tsv" > "$S/blocks.txt"
# meta (id key idx name), signatures, contracts, function sources; SKIPs straight to the results
rm -rf "$S/t"; mkdir -p "$S/t"
awk -v dir="$S/t" -v meta="$S/meta.tsv" -v sig="$S/sig.tsv" -v skips="$S/skips.tsv" '
  function flush() { if (id != "" && src != "") { f = dir "/f_" id ".c"; printf "%s", src > f; close(f) } }
  /^\/\/==== / { flush(); split(substr($0, 8), a, "\t"); id = a[1]; print a[1] "\t" a[2] "\t" a[3] "\t" a[4] > meta; src = ""; next }
  /^\/\/SKIP / { print id "\tskip\t" substr($0, 8) > skips; next }
  /^\/\/SIG /  { print id "\t" substr($0, 7) > sig; next }
  /^\/\/CONTRACT / { contract[id] = substr($0, 12); print id "\t" substr($0, 12) > (dir "/../contracts.tsv"); next }
  { src = src $0 "\n" }
  END { flush() }' "$S/blocks.txt"
# contract2c input: id, parameter names, contract
awk -F'\t' 'FILENAME == ARGV[1] { names = ""; for (k = 4; k <= NF; k += 2) names = names (names == "" ? "" : ",") $k; pn[$1] = names; next }
            ($1 in pn) { print $1 "\t" pn[$1] "\t" $2 }' "$S/sig.tsv" "$S/contracts.tsv" | "$S/contract2c" > "$S/trans.tsv"
log "signatures ok: $(wc -l < "$S/sig.tsv"), skipped before translation: $(wc -l < "$S/skips.tsv" 2>/dev/null || echo 0), translated: $(grep -c $'\tOK\t' "$S/trans.tsv"), not translatable: $(grep -c $'\tSKIP\t' "$S/trans.tsv")"
# harness per translated contract
rm -f "$S/todo.txt"
awk -F'\t' -v dir="$S/t" 'FILENAME == ARGV[1] { nm[$1] = $4; next }
  FILENAME == ARGV[2] { ret[$1] = $2; np[$1] = 0; for (k = 3; k <= NF; k += 2) { np[$1]++; pt[$1, np[$1]] = $k; pn[$1, np[$1]] = $(k + 1) } next }
  $2 == "OK" && ($1 in ret) {
    id = $1; f = dir "/t_" id ".c"
    print "#include \"s3_shim.h\"" > f
    print "#include \"f_" id ".c\"" > f
    print "#include <stdio.h>\nstatic int s3_undef;" > f
    print "static __int128 s3_shl(__int128 a, __int128 b) { if (b < 0 || b > 120) { s3_undef = 1; return 0; } return a * ((__int128)1 << b); }" > f
    print "static __int128 s3_shr(__int128 a, __int128 b) { if (b < 0 || b > 120) { s3_undef = 1; return 0; } return a >= 0 ? a >> b : -((-a - 1) >> b) - 1; }" > f
    print "static __int128 s3_div(__int128 a, __int128 b) { if (b == 0) { s3_undef = 1; return 0; } return a / b; }" > f
    print "static __int128 s3_mod(__int128 a, __int128 b) { if (b == 0) { s3_undef = 1; return 0; } return a % b; }" > f
    print "static unsigned long long s3_st = 88172645463325252ULL;\nstatic unsigned long long s3_rand(void) { s3_st ^= s3_st << 13; s3_st ^= s3_st >> 7; s3_st ^= s3_st << 17; return s3_st; }" > f
    print "static long long s3_val(void) { static const long long e[] = {0,1,-1,2,-2,3,4,7,8,15,16,31,32,63,64,127,128,255,256,511,512,1023,1024,4095,4096,0x7fff,0x8000,0xffff,0x10000,0x7fffffff,-0x7fffffffLL-1,0xffffffffLL,0x100000000LL,0x7fffffffffffffffLL,-0x7fffffffffffffffLL-1};" > f
    print "  unsigned long long r = s3_rand(); switch (r & 3) { case 0: return e[(r >> 2) % (sizeof e / sizeof e[0])]; case 1: return (long long)(s3_rand() % 300) - 20; default: return (long long)s3_rand(); } }" > f
    print "int main(void) {\n  long ok = 0, rej = 0, und = 0;\n  for (long it = 0; it < 200000; it++) {" > f
    args = ""; show = ""; fmt = ""
    for (k = 1; k <= np[id]; k++) { printf "    %s %s = (%s)s3_val();\n", pt[id, k], pn[id, k], pt[id, k] > f; args = args (k > 1 ? ", " : "") pn[id, k]; fmt = fmt " " pn[id, k] "=%lld"; show = show ", (long long)" pn[id, k] }
    printf "    s3_undef = 0; if (!(%s) || s3_undef) { rej++; continue; }\n", $3 > f
    printf "    fprintf(stderr, \"S3-LAST%s\\n\"%s);\n", fmt, show > f
    printf "    %s s3_r = %s(%s);\n", ret[id], nm[id], args > f
    printf "    s3_undef = 0; int s3_ok = (%s) != 0; if (s3_undef) { und++; continue; }\n", $4 > f
    printf "    if (!s3_ok) { printf(\"S3-COUNTEREXAMPLE%s result=%%lld\\n\"%s, (long long)s3_r); return 42; }\n", fmt, show > f
    print "    ok++;\n  }\n  if (ok == 0) { printf(\"S3-NOINPUT rejected=%ld undefined=%ld\\n\", rej, und); return 0; }\n  printf(\"S3-DONE ok=%ld rejected=%ld undefined=%ld\\n\", ok, rej, und); return 0;\n}" > f
    close(f); print id > (dir "/../todo.txt")
  }' "$S/meta.tsv" "$S/sig.tsv" "$S/trans.tsv"
log "harnesses: $(wc -l < "$S/todo.txt" 2>/dev/null || echo 0); compiling and running them, 32 at a time"
xargs -P 32 -I{} bash "$HERE/s3_one.sh" "$S/t" {} < "$S/todo.txt" > "$S/results.tsv" 2> "$S/run.err"
# one JSON line per candidate: tested results, translation skips, extraction skips
{ cat "$S/results.tsv"; awk -F'\t' '$2 == "SKIP" { print $1 "\tuntranslatable\t" $3 }' "$S/trans.tsv"; awk -F'\t' '{ print $1 "\tnotscalar\t" $3 }' "$S/skips.tsv" 2>/dev/null; } |
  awk -F'\t' 'FILENAME == ARGV[1] { meta[$1] = $2 "\t" $3 "\t" $4; next }
       !($1 in seen) { seen[$1] = 1; split(meta[$1], m, "\t"); d = $3; gsub(/\\/, "\\\\", d); gsub(/"/, "\\\"", d)
         printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"s3\":\"%s\",\"detail\":\"%s\"}\n", m[1], m[2], m[3], $2, d }' "$S/meta.tsv" - > "$O/s3.jsonl"
jq -s '{candidates: length, s3: (group_by(.s3)|map({(.[0].s3): length})|add)}' "$O/s3.jsonl" > "$O/summary.json"
tee -a "$O/s3.log" < "$O/summary.json"
touch "$O/DONE"
