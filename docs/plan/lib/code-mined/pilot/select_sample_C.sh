#!/bin/bash
# select_sample_C.sh  Fresh stratified sample C (200 functions, same quotas and 75% scalar rule as sample A, other seed), disjoint from samples A and B.
# Output: /mnt/c/ParanoidBSD/mined-data/pilot-C/sample.tsv  (copied to data2/sampleC.tsv by export)
HERE=$(cd "$(dirname "$0")" && pwd)
P=/mnt/c/ParanoidBSD/mined-data; D=$P/pilot-C; mkdir -p "$D"
cut -f1 $P/pilot/sample.tsv $P/pilot-B/sample.tsv 2>/dev/null | sort -u > "$D/exclude.keys"
awk -F'\t' 'FILENAME==ARGV[1]{ex[$1]=1; next} !($1 in ex)' "$D/exclude.keys" $P/pilot/cands.tsv > "$D/cands.tsv"
D=$D SEED=pbsd-s4-pilot-C bash "$HERE/select_sample.sh"
