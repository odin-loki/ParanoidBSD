# frama_gen.awk   One small C stub per mined contract, for Frama-C. Each contract becomes an independent block, so one bad contract (a parser crash, a prose word that is a
# C keyword) cannot hide the verdicts of the others.
# Input: the sorted partition TSV used by verify12.awk (key file start end idx name kind contract prose). Only rows with a contract are used.
# Output, per contract:
#   //==== <N>\t<key>\t<idx>\t<name>
#   <extern/typedef/forward declarations the stub needs>
#   /*@ <contract> */ long f<N>(<real parameters>);
#   //---- fallback
#   /*@ <contract> */ long f<N>(long p1);       (used when the real-signature stub does not compile as C)
# The prelude (common kernel typedefs) is in frama_prelude.h and is #included by the runner.
# Bare `result` becomes `\result` (Frama-C spelling; our pipeline uses `result`). Quote characters and non-ASCII bytes are neutralised (they crash or abort Frama-C).
BEGIN {
  FS = "\t"
  split("auto break case char const continue default do double else enum extern float for goto if inline int long register restrict return short signed sizeof static struct switch typedef union unsigned void volatile while _Bool", cl, " ")
  for (k in cl) CKW[cl[k]] = 1
  split("requires ensures assigns result true false NULL old valid iff forall exists behavior assumes complete disjoint nothing at null loop invariant variant decreases", al, " ")
  for (k in al) AKW[al[k]] = 1
  split("u8 u16 u32 u64 s8 s16 s32 s64 size_t ssize_t uint ulong uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t off_t loff_t uintptr_t intptr_t ptrdiff_t gfp_t dma_addr_t phys_addr_t __u8 __u16 __u32 __u64 __s8 __s16 __s32 __s64 __le16 __le32 __le64 __be16 __be32 __be64 bool", pre, " ")
  for (k in pre) PRE[pre[k]] = 1
  curfile = ""; N = 0
}

function load(f,    line) {
  delete src; nsrc = 0
  if ((getline line < f) < 0) { close(f); return 0 }
  src[++nsrc] = line
  while ((getline line < f) > 0) src[++nsrc] = line
  close(f); return 1
}

function params(name, from, to,    i, line, pos, txt, j, depth, c, out, m) {
  for (i = from; i <= to && i <= nsrc; i++) {
    line = src[i]
    if (!match(line, "(^|[^A-Za-z0-9_])" name "[ \t]*\\(")) continue
    pos = RSTART; if (substr(line, pos, 1) !~ /[A-Za-z0-9_]/) pos++
    if (substr(line, 1, pos - 1) ~ /[=();.>]|return/) continue
    txt = substr(line, pos)
    for (j = i + 1; j <= i + 12 && j <= nsrc && txt !~ /\)[ \t]*(\{|;|__|$)/; j++) txt = txt " " src[j]
    sub("^" name "[ \t]*\\(", "", txt)
    depth = 1; out = ""
    for (m = 1; m <= length(txt); m++) { c = substr(txt, m, 1); if (c == "(") depth++; else if (c == ")") { depth--; if (depth == 0) break }; out = out c }
    if (depth == 0) return out
  }
  return ""
}

# declarations for the type names used by a parameter list (typedef long / struct forward / enum), appended to `decls`
function decl_types(p,    n, parts, k, w, ws, nw, t, prev, i) {
  n = split(p, parts, ",")
  for (k = 1; k <= n; k++) {
    w = parts[k]; gsub(/[*\[\]]/, " ", w); nw = split(w, ws, " "); prev = ""
    for (i = 1; i < nw; i++) {
      t = ws[i]
      if (t ~ /^__/ || t ~ /^[A-Z_0-9]+$/ || (t in CKW)) { prev = t; continue }
      if (t !~ /^[A-Za-z_][A-Za-z0-9_]*$/) { prev = t; continue }
      if (prev == "struct" || prev == "union") { if (!((prev " " t) in DEF)) { DEF[prev " " t] = 1; decls = decls prev " " t ";\n" } }
      else if (prev == "enum") { if (!(("enum " t) in DEF)) { DEF["enum " t] = 1; decls = decls "enum " t " { " t "_dummy_ };\n" } }
      else if (!(t in PRE) && !(t in DEF)) { DEF[t] = 1; decls = decls "typedef long " t ";\n" }
      prev = t
    }
  }
}

# the same parameter NAMES, all typed long: keeps the contract's variables bound when the real signature does not compile as C
function fb_params(cp,    n, parts, k, w, out, nm) {
  n = split(cp, parts, ","); out = ""
  for (k = 1; k <= n; k++) {
    w = parts[k]; nm = ""
    if (match(w, /[A-Za-z_][A-Za-z0-9_]*(\[[^]]*\])*[ \t]*$/)) { nm = substr(w, RSTART); sub(/[^A-Za-z0-9_].*/, "", nm) }
    if (nm == "" || (nm in CKW)) nm = "p" k
    out = out (out == "" ? "" : ", ") "long " nm
  }
  return out
}

function clean_params(p,    n, parts, k, q, out) {
  n = split(p, parts, ","); out = ""
  for (k = 1; k <= n; k++) {
    q = parts[k]; gsub(/^[ \t]+|[ \t]+$/, "", q)
    gsub(/(^|[ \t])__[A-Za-z0-9_]+(\([^)]*\))?/, " ", q)
    if (q == "" || q == "void") continue
    if (q ~ /\(|\.\.\./) { out = out (out == "" ? "" : ", ") "long p" k; continue }
    if (q !~ /[A-Za-z_][A-Za-z0-9_]*[ \t*]+\**[ \t]*[A-Za-z_][A-Za-z0-9_]*(\[[^]]*\])*$/ && q !~ /^[A-Za-z_]+[ \t]+[A-Za-z_]+$/) { out = out (out == "" ? "" : ", ") "long p" k; continue }
    out = out (out == "" ? "" : ", ") q
  }
  return out
}

{
  key = $1; file = $2; start = $3 + 0; endl = $4 + 0; idx = $5; name = $6; contract = $8
  if (contract == "" || name !~ /^[A-Za-z_][A-Za-z0-9_]*$/) next
  if (file != curfile) { ok = load(file); curfile = file }
  p = ok ? params(name, start, endl) : ""
  cp = clean_params(p)
  body = contract; sub(/^[ \t]*\/\*@/, "", body); sub(/\*\/[ \t]*$/, "", body)
  gsub(/\*\//, " ", body); gsub(/\/\*/, " ", body)
  gsub(/'/, "_Q_", body); gsub(/"/, "_D_", body)
  gsub(/[^\t\x20-\x7e]/, "_U_", body)
  gsub(/\\\\/, "\\", body)
  body = gensub(/(^|[^\\A-Za-z0-9_])result([^A-Za-z0-9_]|$)/, "\\1\\\\result\\2", "g", body)
  body = gensub(/(^|[^\\A-Za-z0-9_])result([^A-Za-z0-9_]|$)/, "\\1\\\\result\\2", "g", body)     # second pass: adjacent occurrences share a delimiter
  delete DEF; decls = ""
  # parameter names (so that they are not redeclared as extern)
  delete isp; n = split(cp, pp, ",")
  for (k = 1; k <= n; k++) { w = pp[k]; if (match(w, /[A-Za-z_][A-Za-z0-9_]*(\[[^]]*\])*[ \t]*$/)) { w = substr(w, RSTART); sub(/[^A-Za-z0-9_].*/, "", w); isp[w] = 1 } }
  if (cp != "") decl_types(cp)
  # every other identifier of the contract is declared as `extern long` so that binding errors do not decide the syntax verdict; C keywords are not declared (they stay errors)
  tmp = body; gsub(/\\[A-Za-z_]+/, " ", tmp); gsub(/(->|\.)[ \t]*[A-Za-z_][A-Za-z0-9_]*/, " ", tmp); gsub(/[^A-Za-z0-9_]+/, " ", tmp)
  nid = split(tmp, ids, " "); delete seenx
  for (k = 1; k <= nid; k++) { w = ids[k]; if (w ~ /^[0-9]/ || (w in AKW) || (w in CKW) || (w in isp) || (w in PRE) || (w in DEF) || (w in seenx) || w == "result") continue; seenx[w] = 1; decls = decls "extern long " w ";\n" }
  gsub(/(->|\.)[ \t]*[A-Za-z_][A-Za-z0-9_]*/, "", body)
  N++
  printf "//==== %d\t%s\t%s\t%s\n%s/*@%s*/ long f%d(%s);\n//---- fallback\n/*@%s*/ long f%d(%s);\n", N, key, idx, name, decls, body, N, cp, body, N, fb_params(cp)
}
