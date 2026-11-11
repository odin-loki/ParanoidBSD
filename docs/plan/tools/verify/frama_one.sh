#!/bin/bash
# frama_one.sh <dir> <N>   Parse stub c_<N>.c (and, if the stub itself is not valid C, c_<N>.fb.c) with Frama-C. Prints one line:  N <TAB> status <TAB> sig <TAB> message
#   status: pass     Frama-C accepted the annotation
#           fail     Frama-C reported an annotation error (the contract is not valid ACSL), or crashed on it
#           unknown  the stub could not be compiled even with the fallback signature (our stub's fault, not a verdict on the contract)
D=$1; N=$2
try() {   # try <file> -> sets status/msg
  out=$(cd "$D" && timeout 30 frama-c -no-autoload-plugins -kernel-warn-key annot-error=active -cpp-extra-args="-I$D/.." "$1" 2>&1); rc=$?
  if echo "$out" | grep -q 'annot-error'; then
    status=fail; msg=$(echo "$out" | grep -m1 -A2 'annot-error' | tr '\t\n' '  ' | sed 's/.*Warning: *//' | tr -s ' ' | cut -c1-90)
  elif echo "$out" | grep -qE 'Unexpected error|crash|Fatal error|Stack overflow'; then
    status=fail; msg="frama-c crashed on this annotation"
  elif [ $rc -eq 124 ]; then status=unknown; msg="timeout"
  elif echo "$out" | grep -qE 'syntax error|Frama-C aborted|User Error|Error:'; then status=stubfail; msg=$(echo "$out" | grep -m1 -A1 'syntax error\|User Error' | tr '\n' ' ' | cut -c1-90)
  else status=pass; msg=""; fi
}
status=""; msg=""; sig=real
try "c_$N.c"
if [ "$status" = stubfail ]; then
  sig=fallback; try "c_$N.fb.c"; [ "$status" = stubfail ] && status=unknown
fi
# Typing errors are reported apart (status "type"): every identifier that is not a parameter is declared `extern long` in the stub, and a function whose
# signature was not found (macro-generated) has no parameters at all, so a type error may be the stub's fault. Syntax errors, calls to undefined
# functions (C calls are not allowed in ACSL) and crashes stay "fail".
if [ "$status" = fail ] && echo "$msg" | grep -qiE 'incompatible types|subscripted value|not a pointer|invalid operand|cannot dereference|pointer to void|not a struct|unexpected [^ ]* and|cannot be|is not an lvalue'; then status=type; fi
printf '%s\t%s\t%s\t%s\n' "$N" "$status" "$sig" "$msg"
