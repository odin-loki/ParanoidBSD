#!/bin/bash
# goedel_think.sh <problem.lean> <seed>   One Goedel-Prover-V2-8B sample WITH thinking, then three checks. Output goes straight into the repository
# folder (not /tmp: WSL wipes /tmp). Env: GDL_CTX (default 49152), GDL_N (default 40000), GDL_RP repeat penalty (default 1.05).
. ~/.pbsd/env; export FORGE_DEVICE=cuda
REPO=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79
DATA=$REPO/docs/plan/rehearsal-data/goedel-thinking-2026-11-07
P=$1; S=$2; B=$(basename "$P" .lean); O=$DATA/samples/${B}_s$S; mkdir -p "$O"
CTX=${GDL_CTX:-49152}; N=${GDL_N:-40000}; RP=${GDL_RP:-1.05}
FORMAL=$(cat "$P")
PROMPT=$(printf '<|im_start|>user\nComplete the following Lean 4 code:\n\n```lean4\n%s\n```\n\nBefore producing the Lean 4 code to formally prove the given theorem, provide a detailed proof plan outlining the main proof steps and strategies.\nThe plan should highlight key ideas, intermediate lemmas, and proof structures that will guide the construction of the final formal proof.<|im_end|>\n<|im_start|>assistant\n' "$FORMAL")
echo "cmd: forge run goedel-8b-q4_k.fpk -n $N --ctx $CTX --temp 0.6 --repeat-penalty $RP --top-p 0.95 --seed $S" > "$O/cmd.txt"
T0=$(date +%s.%N)
~/forge-rel/forge run /mnt/d/forgebox/models/goedel-8b-q4_k.fpk --prompt "$PROMPT" -n $N --ctx $CTX --temp 0.6 --repeat-penalty $RP --top-p 0.95 --seed $S > "$O/gen.txt" 2> "$O/gen.err"
T1=$(date +%s.%N)
NTOK=$(grep -o '[0-9]* new tokens' "$O/gen.err" | head -1 | cut -d' ' -f1)
awk '/^```lean4/{c=1;buf="";next} /^```/{if(c){last=buf};c=0;next} c{buf=buf $0 "\n"} END{printf "%s", last}' "$O/gen.txt" > "$O/cand.lean"
THM=$(grep -m1 -o '^theorem [A-Za-z_0-9]*' "$P" | cut -d' ' -f2)
STMT=$(grep -m1 '^theorem' "$P" | sed 's/ := by$//')
C1=no; grep -qF "$STMT" "$O/cand.lean" && C1=yes      # check 1: the locked statement is present unchanged
C2=no; C3=no
if [ -s "$O/cand.lean" ]; then
  cp "$O/cand.lean" ~/lean49/GdlCand_${B}_$S.lean; printf '\n#print axioms %s\n' "$THM" >> ~/lean49/GdlCand_${B}_$S.lean
  (cd ~/lean49 && timeout 300 lake env lean GdlCand_${B}_$S.lean > "$O/lean.out" 2>&1) && C2=yes   # check 2: Lean + Mathlib compile it
  rm -f ~/lean49/GdlCand_${B}_$S.lean
  # check 3: no sorry/admit and only the standard axioms
  if [ $C2 = yes ] && ! grep -qE 'sorry|admit' "$O/cand.lean" && ! grep -q sorryAx "$O/lean.out"; then
    if grep -q "does not depend on any axioms" "$O/lean.out" || grep -qE "depends on axioms: \[(propext|Classical.choice|Quot.sound)" "$O/lean.out"; then C3=yes; fi
  fi
fi
printf '%s seed=%s gen_s=%.0f new_tokens=%s statement=%s compiles=%s clean_axioms=%s\n' "$B" "$S" "$(echo "$T1 - $T0" | bc)" "${NTOK:-?}" $C1 $C2 $C3 | tee "$O/result.txt" | tee -a "$DATA/results.txt"
