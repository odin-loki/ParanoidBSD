#!/bin/bash
# lean_unit.sh <unitdir>   Route (c) experiment: GLM-5.3-Flash writes a Lean 4 model of a scalar loop-free C function over BitVec plus a theorem for its ACSL contract, Lean checks it.
# Only for units that Frama-C WP built but could not prove (bit-level reasoning is what the integer-model WP cannot do and bv_decide can).
# Result label of a passing proof: LEAN-MODEL-PROVED = the Lean kernel accepted a theorem about the MODEL the LLM wrote. Whether the model equals the C function and the theorem equals the ACSL contract is NOT
# machine-checked here (needs an audit or a differential test), so this is REVIEWED-grade, not PROVED (SELLABLE_PLAN section 1). bv_decide adds the axiom Lean.ofReduceBool (trusted compiler), reported.
# Up to 3 attempts (reasoning low, low, high), compile errors fed back. Writes lean.tsv: N attempts status axioms tokens_in tokens_out usd_cons usd_listed seconds
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}
LEAN=${LEAN_BIN:-$HOME/.elan/toolchains/leanprover--lean4---v4.34.0/bin/lean}
U=$1; cd "$U" || exit 2
IFS=$'\t' read -r N name kind cls file lines hc hf hp mstat <meta.tsv
[ -f "$D/STOP" ] && exit 0
SYS='You translate a small C function and its ACSL contract into Lean 4 (core Lean only, no Mathlib) and prove the contract. Reply with ONE Lean file and nothing else (no markdown fence, no explanation). Rules:
- First line: import Std.Tactic.BVDecide
- Model every C integer type as BitVec n (unsigned char 8, short 16, int/unsigned int 32, long/long long/size_t/uint64_t 64). Use the C semantics of the machine: unsigned arithmetic wraps; for signed types use the signed operations (BitVec.slt, sle, sshiftRight, BitVec.sdiv, srem); C shifts need a count below the width: put that in the precondition if the contract does.
- Write `def model` taking the function parameters (in order, same names) and returning the C return value as a BitVec (for bool or a comparison, return a BitVec 32 equal to 1 or 0, as C does). Translate the body faithfully, statement by statement; do not simplify it into the contract.
- ACSL integers are mathematical: translate the requires into hypotheses and the ensures into the conclusion about (model ...), using .toNat for unsigned values and .toInt for signed ones, `\result` = model applied to the parameters. Do not weaken or strengthen: the theorem must say exactly what the contract says (requires -> ensures, all parameters universally quantified).
- Name the theorem `contract`. Prove it with `bv_decide`, `bv_omega`, `omega`, `simp` or a short combination; unfold model first (`simp only [model]` or `unfold model`). No sorry, admit, axiom, native_decide, opaque, unsafe, implemented_by.
- If something in the function is not an integer computation (calls, pointers, asm), still model what you can and say nothing else; the tool will report failure.'
t0=$(date +%s); tin=0; tout=0; usd=0; usdl=0; att=0; final=FAILED; axs=-
prev=""
for k in 1 2 3; do
  [ -f "$D/STOP" ] && break
  att=$k; eff=low; mt=14000; [ $k -ge 3 ] && { eff=high; mt=20000; }
  USER="ACSL contract (the return value is \\result):
$(cat contract.acsl)

C function (file ${file##*/}, definitions of macros it uses are listed after it):
$(cat fn.c)

Macros and resolved names it uses:
$(cat resolved.h 2>/dev/null)
$(grep -E '^#define' defs.h 2>/dev/null | head -60)
$prev"
  jq -n --arg sys "$SYS" --arg user "$USER" --argjson mt $mt --arg eff $eff '{model:"morph-glm53flash", messages:[{role:"system",content:$sys},{role:"user",content:$user}], max_tokens:$mt, reasoning_effort:$eff, temperature:0.2}' >lean$k.req.json
  bash "$HERE/morph_call.sh" lean$k.req.json lean$k.resp.json "$N:lean$k:$name"; rc=$?
  [ $rc -ne 0 ] && { final="STOPPED(rc=$rc)"; break; }
  p=$(jq -r '.usage.prompt_tokens // 0' lean$k.resp.json); c=$(jq -r '.usage.completion_tokens // 0' lean$k.resp.json); h=$(jq -r '.usage.prompt_tokens_details.cached_tokens // 0' lean$k.resp.json)
  tin=$((tin+p)); tout=$((tout+c))
  usd=$(awk -v u=$usd -v p=$p -v h=$h -v c=$c 'BEGIN{printf "%.6f", u+((p-h)*0.15+h*0.02+c*0.42)/1e6}'); usdl=$(awk -v u=$usdl -v p=$p -v h=$h -v c=$c 'BEGIN{printf "%.6f", u+((p-h)*0.047+h*0.02+c*0.268)/1e6}')
  jq -r '.choices[0].message.content // empty' lean$k.resp.json | sed -e '/^```/d' >lean$k.lean
  [ -s lean$k.lean ] || { prev="(your last reply was empty or cut off)"; continue; }
  if grep -n -w -E 'sorry|admit|axiom|native_decide|opaque|unsafe|implemented_by|partial|noncomputable' lean$k.lean >lean$k.scan.txt; then prev="REJECTED by the tool: forbidden word: $(head -1 lean$k.scan.txt | cut -c1-100)"; continue; fi
  grep -q 'theorem contract' lean$k.lean || { prev="REJECTED by the tool: no theorem named contract"; continue; }
  { cat lean$k.lean; echo; echo '#print axioms contract'; } >lean$k.chk.lean
  out=$(timeout 300 "$LEAN" lean$k.chk.lean 2>&1); rc=$?
  echo "$out" >lean$k.out.txt
  if [ $rc -eq 0 ] && ! echo "$out" | grep -qE 'error|sorry'; then
    axs=$(echo "$out" | sed -n "s/.*depends on axioms: *\[\(.*\)\].*/\1/p" | tr -d ' ')
    [ -z "$axs" ] && echo "$out" | grep -q 'does not depend on any axioms' && axs=none
    bad=$(echo "$axs" | tr ',' '\n' | grep -v -x -E 'propext|Classical.choice|Quot.sound|Lean.ofReduceBool|none|' | head -1)
    if [ -n "$bad" ]; then prev="REJECTED by the tool: unexpected axiom $bad"; continue; fi
    final=LEAN-MODEL-PROVED; break
  fi
  prev="Lean 4.34 rejected your file with:
$(echo "$out" | head -25 | cut -c1-220)
Fix it and return the complete file."
done
secs=$(( $(date +%s) - t0 ))
printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$att" "$final" "$axs" "$tin" "$tout" "$usd" "$usdl" "$secs" >lean.tsv
