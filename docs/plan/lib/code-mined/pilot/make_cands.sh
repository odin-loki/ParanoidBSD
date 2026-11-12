#!/bin/bash
# make_cands.sh  Candidate list for the S4 pilot: functions tagged verified by step 4, of a core kind, with a formal contract.
# Output cands.tsv: key file start end idx name kind contract   (contract has no tabs/newlines)
D=${D:-/mnt/c/ParanoidBSD/mined-data}
KINDS='arith-helper bit-op bitmask bitmap allocator refcount ring-buffer list-op lock-or-sync parser error-path init-teardown'
jq -r '[.key,.i,.name,.kind,.status]|@tsv' $D/verify/tags.jsonl > $D/pilot/tags.tsv
awk -F'\t' -v kinds="$KINDS" 'BEGIN{n=split(kinds,k," ");for(i=1;i<=n;i++)K[k[i]]=1}
  NR==FNR { if($5=="verified" && ($4 in K)) T[$1 SUBSEP $2]=1; next }
  ($1 SUBSEP $5) in T && $8!="" { print $1"\t"$2"\t"$3"\t"$4"\t"$5"\t"$6"\t"$7"\t"$8 }' $D/pilot/tags.tsv $D/verify/rows.tsv > $D/pilot/cands.tsv
wc -l $D/pilot/cands.tsv
cut -f7 $D/pilot/cands.tsv | sort | uniq -c | sort -rn
