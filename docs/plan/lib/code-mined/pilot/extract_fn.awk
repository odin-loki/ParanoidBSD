# extract_fn.awk  Print the definition of function `name` (signature + body) found in lines start..end of the input C file.
# Usage: awk -v name=foo -v start=10 -v end=200 -f extract_fn.awk file.c
# Exit code 0 and text on stdout; prints nothing (exit 3 = no definition, 4 = unbalanced braces). Purely mechanical (no model).
# The line range "b-e" of the extracted text goes to stderr.
function isid(c) { return c ~ /[A-Za-z0-9_]/ }
{ sub(/\r$/, ""); L[NR] = $0 }
END {
  n = NR; if (end == 0 || end > n) end = n; if (start < 1) start = 1
  def = 0
  for (i = start; i <= end && !def; i++) {
    s = L[i]
    if (s ~ /^[ \t#}]/ || s ~ /^(return|if|for|while|switch|else|case)[^A-Za-z0-9_]/) continue
    p = index(s, name); found = 0
    while (p > 0) {
      pre = (p > 1) ? substr(s, p - 1, 1) : " "
      rest = substr(s, p + length(name))
      if (!isid(pre) && pre != "." && pre != ">" && rest ~ /^[ \t]*\(/) { found = p; break }
      q = index(substr(s, p + 1), name); p = (q > 0) ? p + q : 0
    }
    if (!found) continue
    if (substr(s, 1, found - 1) ~ /[=;]/) continue
    depth = 0; seen = 0; kind = ""
    for (j = i; j <= end + 40 && j <= n && kind == ""; j++) {
      t = L[j]; if (j == i) t = substr(t, found)
      for (k = 1; k <= length(t); k++) {
        c = substr(t, k, 1)
        if (c == "(") { depth++; seen = 1 }
        else if (c == ")") depth--
        else if (depth == 0 && seen && c == "{") { kind = "def"; break }
        else if (depth == 0 && seen && c == ";") { kind = "proto"; break }
      }
    }
    if (kind == "def") def = i
  }
  if (!def) exit 3
  b = def
  for (c = 0; c < 4 && b > 1; c++) {
    pl = L[b - 1]
    if (pl ~ /^[ \t]*$/ || pl ~ /;[ \t]*$/ || pl ~ /\}[ \t]*$/ || pl ~ /\*\/[ \t]*$/ || pl ~ /^[ \t]*#/ || pl ~ /^[ \t]*\/[\/*]/) break
    # a return-type line is only identifiers, '*' and blanks (or an attribute): anything with parentheses, '=', '{', a trailing backslash or leading blanks is the tail of something else (a macro body, a call)
    if (pl ~ /[()={}\\]/ && pl !~ /^(static|extern|inline|__attribute__)/) break
    if (pl ~ /\\[ \t]*$/ || pl ~ /^[ \t]/) break
    b--
  }
  depth = 0; started = 0; incom = 0; pd = 0; seenp = 0; e = 0; out = ""
  for (j = b; j <= n && !e; j++) {
    t = L[j]; out = out t "\n"
    for (k = 1; k <= length(t); k++) {
      c = substr(t, k, 1); c2 = substr(t, k, 2)
      if (incom) { if (c2 == "*/") { incom = 0; k++ } ; continue }
      if (c2 == "/*") { incom = 1; k++; continue }
      if (c2 == "//") break
      if (c == "\"") { for (k++; k <= length(t) && substr(t, k, 1) != "\""; k++) if (substr(t, k, 1) == "\\") k++; continue }
      if (c == "'") { for (k++; k <= length(t) && substr(t, k, 1) != "'"; k++) if (substr(t, k, 1) == "\\") k++; continue }
      if (!started) { if (c == "(") { pd++; seenp = 1 } else if (c == ")") pd--; else if (c == "{" && seenp && pd == 0) { started = 1; depth = 1 } ; continue }
      if (c == "{") depth++
      else if (c == "}") { depth--; if (depth == 0) { e = j; break } }
    }
  }
  if (!e) exit 4
  printf "%s", out
  print b "-" e > "/dev/stderr"
}
