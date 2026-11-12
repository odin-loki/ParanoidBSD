# extract_types.awk  Print the top-level struct/union/enum/typedef declarations of a C file (brace depth 0 statements that start with one of those words),
# comments removed. Function bodies and preprocessor lines are ignored.
{ sub(/\r$/, ""); L[NR] = $0 }
END {
  incom = 0; depth = 0; stmt = ""; ppcont = 0
  for (i = 1; i <= NR; i++) {
    t = L[i]
    if (ppcont) { ppcont = (t ~ /\\[ \t]*$/); continue }
    if (depth == 0 && stmt == "" && t ~ /^[ \t]*#/) { ppcont = (t ~ /\\[ \t]*$/); continue }
    line = ""
    for (k = 1; k <= length(t); k++) {
      c = substr(t, k, 1); c2 = substr(t, k, 2)
      if (incom) { if (c2 == "*/") { incom = 0; k++ } ; continue }
      if (c2 == "/*") { incom = 1; k++; continue }
      if (c2 == "//") break
      line = line c
    }
    for (k = 1; k <= length(line); k++) {
      c = substr(line, k, 1)
      if (stmt == "" && c ~ /[ \t]/) continue
      stmt = stmt c
      if (c == "{") depth++
      else if (c == "}") {
        depth--
        if (depth == 0 && (stmt !~ /^(typedef|struct|union|enum)[^A-Za-z0-9_]/ || stmt ~ /\)[ \t]*\{.*\}$/)) stmt = ""
      } else if (c == ";" && depth == 0) {
        if (stmt ~ /^(typedef|struct|union|enum)[^A-Za-z0-9_]/) print stmt
        stmt = ""
      }
    }
    if (stmt != "") stmt = stmt " "
  }
}
