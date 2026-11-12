#!/bin/bash
# estimate2.sh   Extrapolate to the core-kind pool, post-stratified by kind (same method as estimate.awk of the pilot), from labels.tsv of samples A and C.
#   engines: "wp" = only WP proofs counted (step 1);  "wp+bv" = WP + bit-vector (step 2).  Pools: A, C, A+C.
HERE=$(cd "$(dirname "$0")" && pwd); D=$HERE/data2
est() { # est <label> <engine> <files...>
  lab=$1; eng=$2; shift 2
  cat "$@" | awk -F'\t' -v eng="$eng" 'BEGIN{OFS="\t"} { l=$6; if (eng=="wp" && $7!="WP") l=($6=="PROVED-ASSUMING" ? "X" : $6); print $1,$2,$3,$4,l }' >"$HERE/.est.tmp"
  echo "== $lab"; awk -f "$HERE/estimate.awk" "$HERE/.est.tmp" "$HERE/core_kind_counts.tsv" 2>/dev/null || awk -f "$HERE/estimate.awk" "$HERE/core_kind_counts.tsv" "$HERE/.est.tmp"
}
# estimate.awk reads the sample first (NR==FNR) then the counts
estf() { lab=$1; eng=$2; shift 2
  cat "$@" | awk -F'\t' -v eng="$eng" 'BEGIN{OFS="\t"} { l=$6; if (eng=="wp" && $7!="WP") l="X"; print $1,$2,$3,$4,l }' >"$HERE/.est.tmp"
  echo "== $lab"; awk -f "$HERE/estimate.awk" "$HERE/.est.tmp" "$HERE/core_kind_counts.tsv" | tail -13; }
estf "A, WP only (step 1)" wp "$D/A/labels.tsv"
estf "A, WP + bit-vector (step 1+2)" bv "$D/A/labels.tsv"
estf "C, WP only (step 1)" wp "$D/C/labels.tsv"
estf "C, WP + bit-vector (step 1+2)" bv "$D/C/labels.tsv"
estf "A+C pooled, WP only (step 1)" wp "$D/A/labels.tsv" "$D/C/labels.tsv"
estf "A+C pooled, WP + bit-vector (step 1+2)" bv "$D/A/labels.tsv" "$D/C/labels.tsv"
rm -f "$HERE/.est.tmp"
