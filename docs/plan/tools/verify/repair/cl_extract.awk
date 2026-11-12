# cl_extract.awk -v FILE=<source file> -v S=<slice start> -v E=<slice end> -v NAMES=<file with one function name per line>
# Port of dsv_prompt.py's extract()/macro_def() (shell/awk only). Prints, per function that is not wholly inside the slice, the full function source from the real file
# (and the #define of a macro-generated function). Limits: 14000 chars per function, 60000 per record.
function isident(n) { return n ~ /^[A-Za-z_][A-Za-z0-9_]*$/ }
function find_end(i0,   i, s, j, n, c, q, depth, started, inblk) {
  depth = 0; started = 0; inblk = 0
  for (i = i0; i <= NL; i++) {
    s = L[i]; n = length(s); j = 1
    while (j <= n) {
      c = substr(s, j, 1)
      if (inblk) { if (substr(s, j, 2) == "*/") { inblk = 0; j++ } }
      else if (substr(s, j, 2) == "/*") { inblk = 1; j++ }
      else if (substr(s, j, 2) == "//") break
      else if (c == "\"" || c == "'") { q = c; j++; while (j <= n && substr(s, j, 1) != q) { if (substr(s, j, 1) == "\\") j++; j++ } }
      else if (c == "{") { depth++; started = 1 }
      else if (c == "}") { depth--; if (started && depth <= 0) return i }
      j++
    }
    if (!started && i - i0 > 6) return 0
  }
  return 0
}
function rx(s) { gsub(/[][\134.^$*+?(){}|]/, "\134\134&", s); return s }
function best_def(name, plain,   pat, i, t, c0, e0, d, bd, bi, be, last, ch, mm, w, a, cand_n, first) {
  bd = -1; first = 0; cand_n = 0
  if (plain) {
    last = name; sub(/^.*::/, "", last); sub(/^~/, "", last)
    if (name ~ /^operator/) pat = rx(last); else pat = "(^|[^A-Za-z0-9_.>])" rx(name ~ /::/ ? name : last) "[ \t]*[(]"
  } else {
    if (!match(name, /^[ \t]*[A-Za-z0-9_]+[ \t]*\(/)) return ""
    w = substr(name, RSTART, RLENGTH); sub(/^[ \t]*/, "", w); sub(/[ \t]*\($/, "", w)
    a = substr(name, RSTART + RLENGTH); sub(/^[ \t]*/, "", a); if (match(a, /^[A-Za-z0-9_]*/)) a = substr(a, 1, RLENGTH); else a = ""
    pat = "(^|[^A-Za-z0-9_])" w "[ \t]*[(][ \t]*" a
  }
  for (i = 1; i <= NL; i++) {
    t = L[i]
    if (plain) { ch = substr(t, 1, 1); if (t == "" || ch == " " || ch == "\t" || ch == "#" || ch == "}" || ch == "*" || ch == "/" || t ~ /;[ \t\r]*$/ || substr(t, 1, 6) == "return") continue }
    else if (t ~ /^[ \t]*#[ \t]*define/) continue
    if (t !~ pat) continue
    cand_n++; if (!first) first = i
    e0 = find_end(i); if (!e0) continue
    d = (i >= S && i <= E) ? 0 : (i < S ? S - i : i - E)
    if (bd < 0 || d < bd) { bd = d; bi = i; be = e0 }
  }
  if (bd < 0) { if (cand_n && !plain) return first " " ((first + 12 > NL) ? NL : first + 12); return "" }
  f = bi
  while (f > 1 && bi - f < 4 && L[f-1] ~ /[^ \t\r]/ && L[f-1] !~ /(;|\}|\{|\*\/)[ \t\r]*$/ && L[f-1] !~ /^[ \t]*#/) f--
  return f " " be
}
function body(a, b,   i, r) { r = ""; for (i = a; i <= b; i++) r = r L[i] "\n"; return r }
function enclosing(   i, t, e0, f) {   # the function whose body contains line S: nearest column-0 line with "(" above S whose brace-balanced end reaches S
  for (i = S; i >= 1 && i > S - 4000; i--) {
    t = L[i]
    if (t !~ /^[A-Za-z_]/ || t !~ /[(]/ || t ~ /;[ \t\r]*$/) continue
    if (t ~ /^(typedef|if|for|while|switch|else|do|return|case|goto)[ \t(]/) continue
    e0 = find_end(i); if (!e0 || e0 < S) continue
    f = i
    while (f > 1 && i - f < 4 && L[f-1] ~ /[^ \t\r]/ && L[f-1] !~ /(;|\}|\{|\*\/)[ \t\r]*$/ && L[f-1] !~ /^[ \t]*#/) f--
    return f " " e0
  }
  return ""
}
function emit(label, a, b,   bd) {
  if ((a "-" b) in printed) return
  printed[a "-" b] = 1
  if (a >= S && b <= E) return
  bd = body(a, b); if (length(bd) > MAXF) bd = substr(bd, 1, MAXF) "\n... (truncated)\n"
  if (total + length(bd) <= MAXT) { printf "[%s: lines %d-%d of %s]\n%s\n", label, a, b, base, bd; total += length(bd) }
}
BEGIN {
  MAXF = 60000; MAXT = 160000     # chars per function / per record (Claude takes far more than the 14000 / 60000 of the DeepSeek prompt)
  while ((getline ln < FILE) > 0) L[++NL] = ln
  close(FILE); base = FILE; sub(/^.*\//, "", base); total = 0
  while ((getline nm < NAMES) > 0) {
    if (nm == "" || (nm in seen)) continue; seen[nm] = 1
    plain = isident(nm) || nm ~ /::/ || nm ~ /^operator/
    r = best_def(nm, plain); hit = 0
    if (r != "") {
      split(r, ab, " "); a = ab[1]; b = ab[2]
      if (a <= E && b >= S) hit = 1
      emit("full source of " nm, a, b)
    }
    if (!hit && (r == "" || plain)) {   # the listed name is not defined near the slice (placeholder such as "unnamed-...-body"): give the function the slice sits in
      er = enclosing()
      if (er != "") { split(er, ab, " "); emit("function that contains the first line of the slice (shown because the listed name '" nm "' is not defined there; if it is a placeholder, use this function's real name)", ab[1], ab[2]) }
    }
    if (!plain && match(nm, /^[ \t]*[A-Za-z0-9_]+[ \t]*[(]/)) {
      w = substr(nm, RSTART, RLENGTH); sub(/^[ \t]*/, "", w); sub(/[ \t]*[(]$/, "", w)
      for (i = 1; i <= NL; i++) if (L[i] ~ ("^[ \t]*#[ \t]*define[ \t]+" w "([^A-Za-z0-9_]|$)")) {
        j = i; while (j < NL && L[j] ~ /\134[ \t\r]*$/ && j - i < 60) j++
        bd = substr(body(i, j), 1, 4000)
        if (total + length(bd) <= MAXT) { printf "[macro definition used by %s: lines %d-%d of %s]\n%s\n", nm, i, j, base, bd; total += length(bd) }
        break
      }
    }
  }
}
