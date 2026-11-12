#!/bin/bash
# annotate_unit.sh <unitdir>   For a unit whose contract WP could not prove (status FAILED), ask GLM-5.3-Flash (Morph) for loop invariants / loop assigns / loop variant / assert annotations ONLY.
# The model may not touch the C code or the contract (guard_annot.sh rejects anything else); WP then re-checks the annotated function from scratch, so a wrong annotation costs money, never soundness.
# Up to 3 attempts; reasoning_effort low on attempts 1-2, high on attempt 3. Each attempt is logged: <unit>/att<k>.{req,resp}.json, att<k>.result.tsv. Final result of the unit: result.tsv (best attempt) + annot.tsv.
# annot.tsv: N attempts final_status tokens_in tokens_out usd_conservative usd_listed seconds
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}
U=$1; cd "$U" || exit 2
IFS=$'\t' read -r N name kind cls file lines hc hf hp mstat <meta.tsv
[ -f "$D/STOP" ] && exit 0
cp result.tsv result.base.tsv; cp wp.A.log wp.A.base.log 2>/dev/null; cp unit.final.c unit.final.base.c 2>/dev/null
SYS='You add ACSL annotations so that Frama-C WP can PROVE the given function contract. You may add ONLY: loop invariant, loop assigns, loop variant, and assert (inside the function body, before the loop or statement they refer to). You must return the COMPLETE function exactly as given (same C code, same order, same names) with those annotations inserted, and nothing else: no explanation, no markdown fence, no contract (the contract above the function is added by the tool). Do NOT write requires, ensures, assigns outside a loop, lemma, axiom, assume, behavior, ghost code or predicates. ACSL integers are mathematical; C unsigned types wrap, so invariants must say what is true of the C values (for example (unsigned)(x + 1) when wrap-around matters). Give invariants that are inductive: true before the loop, preserved by one iteration, and strong enough to imply the contract at exit. If the contract is false for some input, still do your best; the tool will report failure.'
t0=$(date +%s); tin=0; tout=0; usd=0; usdl=0; att=0; final=FAILED
prev=$(tail -25 wp.A.log | cut -c1-300)
for k in 1 2 3; do
  [ -f "$D/STOP" ] && break
  att=$k; eff=low; mt=14000; [ $k -ge 3 ] && { eff=high; mt=20000; }
  USER="Contract (do not change, do not repeat it):
$(cat contract.acsl)

Function to annotate:
$(cat fn.c)

Frama-C WP output of the last attempt (tail):
$prev"
  jq -n --arg sys "$SYS" --arg user "$USER" --argjson mt $mt --arg eff $eff '{model:"morph-glm53flash", messages:[{role:"system",content:$sys},{role:"user",content:$user}], max_tokens:$mt, reasoning_effort:$eff, temperature:0.2}' >att$k.req.json
  bash "$HERE/morph_call.sh" att$k.req.json att$k.resp.json "$N:att$k:$name"; rc=$?
  [ $rc -ne 0 ] && { final="STOPPED(rc=$rc)"; break; }
  p=$(jq -r '.usage.prompt_tokens // 0' att$k.resp.json); c=$(jq -r '.usage.completion_tokens // 0' att$k.resp.json); h=$(jq -r '.usage.prompt_tokens_details.cached_tokens // 0' att$k.resp.json)
  tin=$((tin+p)); tout=$((tout+c))
  usd=$(awk -v u=$usd -v p=$p -v h=$h -v c=$c 'BEGIN{printf "%.6f", u+((p-h)*0.15+h*0.02+c*0.42)/1e6}'); usdl=$(awk -v u=$usdl -v p=$p -v h=$h -v c=$c 'BEGIN{printf "%.6f", u+((p-h)*0.047+h*0.02+c*0.268)/1e6}')
  jq -r '.choices[0].message.content // empty' att$k.resp.json | sed -e '/^```/d' >att$k.fn.c
  if [ ! -s att$k.fn.c ]; then prev="(your last reply was empty or cut off; reply with the full function only)"; continue; fi
  g=$(bash "$HERE/guard_annot.sh" fn.c att$k.fn.c)
  if [ "$g" != OK ]; then prev="the tool rejected your reply: $g. Return the same C code with only the allowed annotations."; echo "$g" >att$k.guard.txt; continue; fi
  FNFILE=att$k.fn.c RESULT=att$k.result.tsv bash "$HERE/prove_unit.sh" "$U"
  st=$(cut -f5 att$k.result.tsv)
  if [ "$st" = PROVED ]; then final=PROVED; cp att$k.result.tsv result.tsv; cp att$k.fn.c fn.ann.c; break; fi
  prev="your annotated function was checked and the result is $st ($(cut -f14 att$k.result.tsv)). WP output tail:
$(tail -25 wp.A.log | cut -c1-300)"
  # keep the A log of this attempt
  cp wp.A.log att$k.wp.A.log
done
secs=$(( $(date +%s) - t0 ))
[ "$final" != PROVED ] && { cp result.base.tsv result.tsv; cp unit.final.base.c unit.final.c 2>/dev/null; }
printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$att" "$final" "$tin" "$tout" "$usd" "$usdl" "$secs" >annot.tsv
