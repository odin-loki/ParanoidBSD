#!/bin/bash
# s3_one.sh <dir> <id>   Compile and run the step 3 harness t_<id>.c. Prints:  id TAB status TAB detail
#   pass            200000 random/edge inputs, those meeting `requires` all satisfied `ensures`
#   counterexample  an input meeting `requires` where `ensures` is false (detail: the input and result)  -> the contract is WRONG
#   crash           the function trapped or faulted on an input meeting `requires` (BUG_ON, division by zero ...)
#   noinput         no generated input met `requires` (nothing was tested)
#   nocompile       the function does not compile on its own (calls or macros from elsewhere): not tested
#   timeout         the run took longer than 20 s
D=$1; N=$2
if ! gcc -O0 -fwrapv -w -I"$(dirname "$0")" "$D/t_$N.c" -o "$D/b_$N" 2> "$D/cc_$N.err"; then
  printf '%s\tnocompile\t%s\n' "$N" "$(grep -m1 'error' "$D/cc_$N.err" | sed 's/.*error: //' | tr '\t' ' ' | cut -c1-100)"; rm -f "$D/cc_$N.err"; exit 0
fi
rm -f "$D/cc_$N.err"
out=$(timeout 20 "$D/b_$N" 2>&1); rc=$?
rm -f "$D/b_$N"
case $rc in
  0)   if echo "$out" | grep -q 'S3-NOINPUT'; then printf '%s\tnoinput\t%s\n' "$N" "$(echo "$out" | tail -1 | cut -c1-100)"; else printf '%s\tpass\t%s\n' "$N" "$(echo "$out" | tail -1 | cut -c1-100)"; fi ;;
  42)  printf '%s\tcounterexample\t%s\n' "$N" "$(echo "$out" | grep -m1 'S3-COUNTEREXAMPLE' | cut -c1-200)" ;;
  124) printf '%s\ttimeout\t\n' "$N" ;;
  *)   printf '%s\tcrash\texit %s %s\n' "$N" "$rc" "$(echo "$out" | grep -m1 'S3-LAST' | cut -c1-150)" ;;
esac
