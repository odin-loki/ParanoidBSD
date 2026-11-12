#!/bin/bash
# audit_sample.sh <rundir> <outdir> [N=400] [seed]   S5 statement-audit sample, ready for a reviewer pass.
#   Picks N proved units (label PROVED, PROVED-ASSUMING or BOUNDED in <rundir>/labels.tsv), stratified by kind (quota proportional to the proved count of the kind, at least
#   min(8, available) per kind, so rare kinds are not lost), seeded shuffle inside a kind. If fewer than N exist, all are taken (and the shortfall is printed).
#   For every unit it writes <outdir>/<kind>/<name>-<N>.md with exactly what a reviewer needs and nothing else:
#     1. identity and PROOF LABEL + ASSUMPTIONS + engine + goals + termination + mutation result + contract strength
#     2. the C SOURCE of the function (as proved: preprocessed, macros expanded; the original line range is given)
#     3. the MINED PROSE (the step-4 description of what the function must do; rows.tsv column 9)
#     4. the MINED CONTRACT (as mined) and the FORMAL CONTRACT that was proved (macro-expanded, \result)
#     5. the QUESTIONS (the same definition of a defect for every reviewer: weaker than the prose = defect)
#   and <outdir>/index.tsv (one line per unit) plus <outdir>/REVIEW_PROTOCOL.md.
R=$1; O=$2; N=${3:-400}; SEED=${4:-pbsd-audit-1}
ROWS=${ROWS:-/mnt/c/ParanoidBSD/mined-data/verify/rows.tsv}; SAMPLE=${SAMPLE:-$R/sample.tsv}
mkdir -p "$O"; : >"$O/index.tsv"
cd "$R" || exit 2
awk -F'\t' '$6 ~ /^PROVED|^BOUNDED/' labels.tsv >"$O/.proved.tsv"
tot=$(wc -l <"$O/.proved.tsv"); [ "$tot" -lt "$N" ] && echo "only $tot proved units available, taking all (shortfall $((N-tot)))"
# quotas
awk -F'\t' -v N="$N" -v tot="$tot" '{c[$3]++} END{ nk=0; for(k in c) nk++; for(k in c){ q=int(N*c[k]/tot+0.5); m=(c[k]<8?c[k]:8); if(q<m) q=m; if(q>c[k]) q=c[k]; print k "\t" q } }' "$O/.proved.tsv" >"$O/.quota.tsv"
: >"$O/.chosen.tsv"
while IFS=$'\t' read -r k q; do
  awk -F'\t' -v k="$k" '$3==k' "$O/.proved.tsv" | shuf --random-source=<(yes "$SEED$k") | head -n "$q" >>"$O/.chosen.tsv"
done <"$O/.quota.tsv"
# prose lookup in ONE pass over rows.tsv (it is large): wanted keys first
while IFS=$'\t' read -r n _rest; do sed -n "${n}p" "$SAMPLE" | awk -F'\t' '{print $1 "\t" $5}'; done <"$O/.chosen.tsv" | sort -u >"$O/.want.tsv"
LC_ALL=C awk -F'\t' 'FILENAME==ARGV[1]{w[$1 SUBSEP $2]=1; next} ($1 SUBSEP $5) in w {print $1 "\t" $5 "\t" $9}' "$O/.want.tsv" "$ROWS" >"$O/.prose.tsv"
# function text extractor (brace matching from the line that defines the function)
fn_text() { awk -v n="$2" '
  { L[NR]=$0 } END {
    s=0; for (i=1;i<=NR;i++) if (match(L[i], "(^|[^A-Za-z0-9_])" n "[ \t]*\\(") && L[i] !~ /;[ \t]*$/) { b=0; for (j=i;j<=NR && j<i+6;j++) if (index(L[j],"{")) { b=1; break } if (b) { s=i; break } }
    if (!s) { print "(function text not found)"; exit }
    d=0; started=0
    for (i=s;i<=NR;i++) { print L[i]; for (c=1;c<=length(L[i]);c++) { ch=substr(L[i],c,1); if (ch=="{") { d++; started=1 } else if (ch=="}") d-- } if (started && d==0) break }
  }' "$1"; }
n_done=0
while IFS=$'\t' read -r n name kind cls tree label how asum cause; do
  d=u$(printf %03d "$n"); [ -d "$d" ] || d=u$n; [ -d "$d" ] || continue
  src=$d/unit.final.c; [ -f "$src" ] || src=$d/unit.c
  row=$(sed -n "${n}p" "$SAMPLE"); IFS=$'\t' read -r key file start end idx rname rkind contract rcls <<<"$row"
  prose=$(awk -F'\t' -v k="$key" -v i="$idx" '$1==k && $2==i {print $3; exit}' "$O/.prose.tsv")
  mkdir -p "$O/$kind"; f=$O/$kind/${name}-$n.md
  goals=$(cut -f7-10,11 "$d/result.tsv" 2>/dev/null | tr '\t' ' ')
  mut=$(cut -f3 "$d/mutation.tsv" 2>/dev/null | sort | uniq -c | tr '\n' ' ')
  strn=$(awk -F'\t' -v n="$n" '$1==n{print $5}' strength.tsv 2>/dev/null)
  {
    echo "# Audit unit: $name  ($kind)"
    echo
    echo "- unit: $n   tree: $tree   source: \`${file#/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/}\` lines $start-$end (chunk), key \`$key\` idx $idx"
    echo "- **LABEL: $label**   engine: $how"
    echo "- **ASSUMPTIONS:** ${asum:-none}"
    echo "- goals (contract A, RTE B, vacuity): $goals ;  mutation test (killed/survived...): ${mut:-not run} ;  contract strength: ${strn:-not computed}"
    echo "- replay (clean dir, cache off): $(cut -f9 "$d/replay.tsv" 2>/dev/null)"
    echo
    echo "## 1. C source of the function (as proved)"; echo '```c'; fn_text "$src" "$name" | head -250; echo '```'
    echo; echo "## 2. Mined prose (what the function must do)"; echo; echo "${prose:-(no prose found in rows.tsv)}"
    echo; echo "## 3. Mined contract (as mined)"; echo '```'; cat "$d/contract.acsl"; echo '```'
    echo; echo "## 4. Formal contract that was PROVED (macros expanded)"; echo '```'; cat "$d/contract.exp"; echo '```'
    echo; echo "## 5. Questions (answer each with YES / NO / UNSURE and one line)"
    echo "Q1. Does the formal contract (4) say everything the prose (2) requires of the result and of the effects? (weaker than the prose = defect, even if true)"
    echo "Q2. Is the formal contract true of the C source (1), i.e. does it describe what the code really does and not what the prose wishes?"
    echo "Q3. Does the precondition exclude inputs the prose allows, or admit inputs under which the code has undefined behaviour?"
    echo "Q4. Are the listed ASSUMPTIONS acceptable for this function (callee behaviour, pointer model, configuration, termination)? Which one matters?"
    echo "Q5. Verdict: SOUND-CLAIM / WEAK-CLAIM (true but weaker than the prose) / WRONG-CLAIM / ASSUMPTION-PROBLEM."
  } >"$f"
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$n" "$name" "$kind" "$label" "$how" "${f#$O/}" "$strn" >>"$O/index.tsv"
  n_done=$((n_done+1))
done <"$O/.chosen.tsv"
cat >"$O/REVIEW_PROTOCOL.md" <<'EOF'
# Statement-audit protocol (S5)

One reviewer (a second model, then a human on a 10% re-sample) reads each unit file and answers Q1-Q5. Same definition of a defect for everybody: a contract that is true but weaker than the
prose is a DEFECT (WEAK-CLAIM). A claim is sellable as PROVED/PROVED-ASSUMING only with verdict SOUND-CLAIM and acceptable assumptions. Record the verdicts in verdicts.tsv
(unit TAB verdict TAB one-line reason). Report: verdict counts per kind, and the rate of WEAK-CLAIM and WRONG-CLAIM with a 95% interval. The units are stratified by kind (see index.tsv),
so overall rates must be post-stratified with the proved counts per kind of the full run.
EOF
rm -f "$O/.want.tsv" "$O/.prose.tsv" "$O/.proved.tsv" "$O/.quota.tsv" "$O/.chosen.tsv"
echo "audit sample: $n_done units in $O"; cut -f3 "$O/index.tsv" | sort | uniq -c
