#!/bin/bash
# summarize2.sh <rundir> <title>   Regenerate the tables of a run2.sh/bv run (stdout, markdown) and write <rundir>/labels.tsv, <rundir>/repair_list.tsv.
#   label per function:
#     PROVED            WP proved contract + run-time-error guards + not vacuous, and no per-unit assumption below
#     PROVED-ASSUMING   as above WITH at least one of: callees-without-spec, pointer-or-memory-precondition, x86_64-config-used-for-non-x86-arch-file,
#                       header-function-in-probe-unit, probe-adds-..., stub-headers:..  (the list is in labels.tsv column 7)
#                       or, for units WP could not prove: bit-vector proof (CBMC, BV-PROVED), always with the assumption bv-sat-uncertified
#     CONTRACT-ONLY     contract proved, but the contract admits undefined behaviour (WP or CBMC)  -> repair list
#     FALSE             CBMC counterexample under C semantics                                      -> repair list
#     FAILED / UNBUILT / SKIPPED  not proved (cause in column 8)
D=$1; T=${2:-run}
cd "$D" || exit 2
: >labels.tsv; : >repair_list.tsv
for r in u*/result.tsv; do
  d=$(dirname "$r"); bv=$d/result.bv.tsv
  awk -F'\t' -v bvf="$bv" 'BEGIN{OFS="\t"; bs=""; bd=""; if ((getline l < bvf) > 0) { split(l, b, "\t"); bs=b[4]; bd=b[6] } }
    { n=$1; name=$2; kind=$3; cls=$4; st=$5; tree=$6; as=$14; cause=$13;
      down = (as ~ /callees-without-spec|pointer-or-memory-precondition|x86_64-config-used-for-non-x86-arch-file|header-function-in-probe-unit|probe-adds|stub-headers|config:|termination-not-proved/);
      lab=st; how="WP";
      if (st=="PROVED") lab = down ? "PROVED-ASSUMING" : "PROVED";
      else if (st=="CONTRACT-ONLY") { lab="CONTRACT-ONLY" }
      if ((st=="FAILED"||st=="CONTRACT-ONLY") && bs!="") {
        if (bs=="BV-PROVED") { lab="PROVED-ASSUMING"; how="CBMC-bitvector"; as=as bd; cause="" }
        else if (bs=="BV-BOUNDED") { lab="BOUNDED"; how="CBMC-bitvector"; as=as bd; cause="" }
        else if (bs=="BV-FALSE") { lab="FALSE"; how="CBMC"; cause=bd }
        else if (bs=="BV-CONTRACT-ONLY") { lab="CONTRACT-ONLY"; how="CBMC"; cause=bd }
        else if (bs=="BV-VACUOUS") { lab="FAILED"; cause="vacuous requires (CBMC)" }
        else { cause=cause " [bv:" bs " " substr(bd,1,60) "]" }
      }
      print n, name, kind, cls, tree, lab, how, as, cause }' "$r" >>labels.tsv
done
awk -F'\t' '$6=="FALSE"||$6=="CONTRACT-ONLY"{print}' labels.tsv >repair_list.tsv
echo "## $T"; echo
echo "Functions: $(wc -l <labels.tsv)"; echo
echo "### Label by kind"; echo
awk -F'\t' '{k=$3; l=$6; n[k,l]++; kinds[k]=1; labs[l]=1; tot[k]++}
  END{ split("PROVED PROVED-ASSUMING CONTRACT-ONLY FALSE FAILED UNBUILT BOUNDED", L, " "); printf "| kind | n |"; for(i=1;i<=7;i++) printf " %s |", L[i]; printf " proved (P+PA) |\n|---|---|"; for(i=1;i<=7;i++) printf "---|"; printf "---|\n";
    for(k in kinds){ printf "| %s | %d |", k, tot[k]; for(i=1;i<=7;i++) printf " %d |", n[k,L[i]]; printf " %d (%.0f%%) |\n", n[k,"PROVED"]+n[k,"PROVED-ASSUMING"], 100*(n[k,"PROVED"]+n[k,"PROVED-ASSUMING"])/tot[k] }
    printf "| **all** | %d |", NR; for(i=1;i<=7;i++){ s=0; for(k in kinds) s+=n[k,L[i]]; printf " %d |", s } p=0; for(k in kinds) p+=n[k,"PROVED"]+n[k,"PROVED-ASSUMING"]; printf " **%d (%.1f%%)** |\n", p, 100*p/NR }' labels.tsv | sort -t'|' -k2,2 | awk 'NR==1||1'
echo; echo "### Label by tree"; echo
awk -F'\t' '{n[$5,$6]++; t[$5]++} END{for(k in t){ printf "- %s: %d functions, PROVED %d, PROVED-ASSUMING %d, CONTRACT-ONLY %d, FALSE %d, FAILED %d, UNBUILT %d\n", k, t[k], n[k,"PROVED"], n[k,"PROVED-ASSUMING"], n[k,"CONTRACT-ONLY"], n[k,"FALSE"], n[k,"FAILED"], n[k,"UNBUILT"]}}' labels.tsv
echo; echo "### Why not built (UNBUILT causes, normalised)"; echo
awk -F'\t' '$6=="UNBUILT"{c=$9; gsub(/Location: "[^"]*", /,"",c); gsub(/lines? [0-9-]+,? ?/,"",c); gsub(/unit.c:[0-9]+/,"unit.c",c); gsub(/`[^`'"'"']*'"'"'/,"`X'"'"'",c); print substr(c,1,90)}' labels.tsv | sort | uniq -c | sort -rn | head -14 | awk '{printf "- %s\n", $0}'
echo; echo "### Per-unit assumptions on proved functions (count)"; echo
awk -F'\t' '$6=="PROVED"||$6=="PROVED-ASSUMING"{n=split($8,a,";"); for(i=1;i<=n;i++){ if(a[i]=="") continue; s=a[i]; sub(/:.*/,"",s); c[s]++ }} END{for(k in c) printf "- %s: %d\n", k, c[k]}' labels.tsv | sort
