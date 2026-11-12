#!/bin/bash
# parse_unit.sh <unitdir>   (diagnostic only; prove_unit.sh does its own parsing) Parse unit.c and unit.min.c with the system Frama-C kernel.
U=$1; FC=${FRAMAC:-frama-c}
for lv in unit.c unit.min.c; do
  out=$(cd "$U" && timeout 60 $FC -no-autoload-plugins -kernel-warn-key annot-error=active -machdep x86_64 "$lv" 2>&1); rc=$?
  if [ $rc -eq 0 ] && ! echo "$out" | grep -q 'annot-error'; then [ $lv = unit.c ] && l=L2 || l=L1; printf '%s\t%s\t\n' "$U" "$l"; exit 0; fi
  last=$(echo "$out" | grep -m1 -E 'rror|annot-error' | tr '\t\n' '  ' | cut -c1-160)
done
printf '%s\tnone\t%s\n' "$U" "$last"
