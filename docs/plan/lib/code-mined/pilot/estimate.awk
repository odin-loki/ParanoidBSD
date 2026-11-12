BEGIN { FS="\t" }
NR==FNR { if ($3 != "") { t[$3]++; if ($5 ~ /^PROVED/) p[$3]++ } next }
{ N[$1]=$2 }
END {
  tot=0; est=0; lo=0; hi=0
  for (k in N) { n=t[k]; x=p[k]+0; pr=(n>0)? x/n : 0
    # Wilson 95%
    z=1.96; if (n>0) { c=(pr+z*z/(2*n))/(1+z*z/n); h=z*sqrt(pr*(1-pr)/n+z*z/(4*n*n))/(1+z*z/n); l=c-h; u=c+h } else { l=0; u=1 }
    if (l<0) l=0; if (u>1) u=1
    printf "%-14s N=%6d sample %2d/%2d  rate %.2f  (%.2f-%.2f)  est %.0f\n", k, N[k], x, n, pr, l, u, N[k]*pr
    est+=N[k]*pr; lo+=N[k]*l; hi+=N[k]*u; tot+=N[k] }
  printf "TOTAL candidates %d  estimate %.0f (%.1f%%)  sum-of-kind Wilson bounds %.0f - %.0f\n", tot, est, 100*est/tot, lo, hi
}
