#!/bin/bash
# guard_annot.sh <orig_fn.c> <new_fn.c>   Accept a model-annotated function only if it is the SAME C code plus ACSL annotations that cannot change what is proved.
#   1. Removing every comment and all whitespace from both files gives identical text  (the code is untouched).
#   2. Every ACSL annotation (/*@ ... */ or //@ ...) in the new file is one of: loop invariant / loop assigns / loop variant / assert / check?? no: only these four.
#      No lemma, axiom, axiomatic, assume, admit, behavior, requires, ensures, assigns outside loop, ghost, predicate, logic, terminates, exits, \at is allowed (it is a term, not a claim).
#   Prints OK or REJECT:<reason>; exit 0 / 1.
strip() { # remove comments (/* */ and //) and whitespace
  awk 'BEGIN{RS="\001"} { s=$0; out=""; n=length(s); i=1
    while (i<=n) { c=substr(s,i,2)
      if (c=="/*") { j=index(substr(s,i+2),"*/"); if (j==0) break; i+=j+3; continue }
      if (c=="//") { j=index(substr(s,i),"\n"); if (j==0) break; i+=j; continue }
      if (substr(s,i,1)=="\"") { k=i+1; while (k<=n && substr(s,k,1)!="\"") { if (substr(s,k,1)=="\\") k++; k++ } out=out substr(s,i,k-i+1); i=k+1; continue }
      out=out substr(s,i,1); i++ }
    gsub(/[ \t\r\n]+/,"",out); printf "%s", out }' "$1"
}
acsl() { # print all ACSL annotations of a file, one per line
  awk 'BEGIN{RS="\001"} { s=$0; n=length(s); i=1
    while (i<=n) {
      if (substr(s,i,3)=="/*@") { j=index(substr(s,i+3),"*/"); if (j==0) break; a=substr(s,i+3,j-1); gsub(/\n/," ",a); print a; i+=j+4; continue }
      if (substr(s,i,3)=="//@") { j=index(substr(s,i),"\n"); if (j==0) { print substr(s,i+3); break } print substr(s,i+3,j-4); i+=j; continue }
      if (substr(s,i,2)=="/*") { j=index(substr(s,i+2),"*/"); if (j==0) break; i+=j+3; continue }
      if (substr(s,i,2)=="//") { j=index(substr(s,i),"\n"); if (j==0) break; i+=j; continue }
      i++ } }' "$1"
}
[ "$(strip "$1")" = "$(strip "$2")" ] || { echo "REJECT:code changed"; exit 1; }
while IFS= read -r a; do
  [ -z "${a// /}" ] && continue
  t=$(printf '%s' "$a" | sed -E 's/^[ \t]+//')
  case "$t" in
    "loop invariant"*|"loop assigns"*|"loop variant"*|"assert "*|"assert	"*) ;;
    *) echo "REJECT:annotation not allowed: ${t:0:60}"; exit 1 ;;
  esac
  if printf '%s' "$t" | grep -q -w -E 'lemma|axiom|axiomatic|assume|assumes|admit|behavior|requires|ensures|ghost|predicate|logic|terminates|exits|check'; then echo "REJECT:forbidden word in: ${t:0:60}"; exit 1; fi
done < <(acsl "$2")
echo OK; exit 0
