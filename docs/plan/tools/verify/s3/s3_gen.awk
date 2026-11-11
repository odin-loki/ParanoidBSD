# s3_gen.awk   Step 3 candidates -> function source + scalar signature, one block per function.
# Input: rows TSV (key file start end idx name kind contract prose), already filtered to the step 3 candidates, sorted by file.
# Output per function:
#   //==== <id> TAB <key> TAB <idx> TAB <name>
#   //SIG <ret type> TAB <p1 type> TAB <p1 name> TAB <p2 type> TAB <p2 name> ...     (or //SKIP <reason>)
#   //CONTRACT <contract>
#   <the function definition, exactly as in the source>
# Only functions whose return type and every parameter are plain integer types are kept (the harness draws random integers); everything else is //SKIP.
BEGIN {
  FS = "\t"
  split("char short int long unsigned signed bool _Bool u8 u16 u32 u64 s8 s16 s32 s64 uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t size_t ssize_t uint ulong uintptr_t intptr_t __u8 __u16 __u32 __u64 __s8 __s16 __s32 __s64", it, " ")
  for (k in it) INT[it[k]] = 1
  split("const volatile register static inline extern __inline __inline__ __always_inline noinline __init __exit __pure __attribute_const__ __maybe_unused __unused __must_check __read_mostly notrace __cold __hot", ql, " ")
  for (k in ql) QUAL[ql[k]] = 1
  id = 0; curfile = ""
}
function load(f,    line) { delete src; nsrc = 0; if ((getline line < f) < 0) { close(f); return 0 } src[++nsrc] = line; while ((getline line < f) > 0) src[++nsrc] = line; close(f); return 1 }
# integer type text (qualifiers removed) or "" if not a plain integer type
function inttype(t,    n, w, i, out) {
  gsub(/\*/, " * ", t); n = split(t, w, " "); out = ""
  for (i = 1; i <= n; i++) { if (w[i] in QUAL) continue; if (w[i] == "*" || !(w[i] in INT)) return ""; out = out (out == "" ? "" : " ") w[i] }
  return out
}
{
  key = $1; file = $2; start = $3 + 0; endl = $4 + 0; idx = $5; name = $6; contract = $8
  if (file != curfile) { ok = load(file); curfile = file }
  id++
  printf "//==== %d\t%s\t%s\t%s\n", id, key, idx, name
  if (!ok) { print "//SKIP source file not found"; next }
  # find the definition: "<type> name(" on one line, or "name(" at column 0 with the type on the previous line
  di = 0; rett = ""
  for (i = start; i <= endl && i <= nsrc; i++) {
    line = src[i]
    if (!match(line, "(^|[^A-Za-z0-9_])" name "[ \t]*\\(")) continue
    pos = RSTART; if (substr(line, pos, 1) !~ /[A-Za-z0-9_]/) pos++
    pre = substr(line, 1, pos - 1)
    if (pre ~ /[=();.>]|return/ || pre ~ /^[ \t]+/ && pre !~ /^[ \t]*[A-Za-z_]/) continue
    if (pre ~ /^[A-Za-z_][A-Za-z0-9_ \t*]*$/) { di = i; rett = pre; first = i; break }
    if (pre == "" && i > 1 && src[i - 1] ~ /^[A-Za-z_][A-Za-z0-9_ \t*]*$/) { di = i; rett = src[i - 1]; first = i - 1; break }
  }
  if (!di) { print "//SKIP definition not found in the slice"; next }
  # parameter list up to the matching ')' and the body up to the matching '}'
  txt = ""; depth = 0; started = 0; bodystart = 0; plist = ""; inparams = 0; doneparams = 0
  for (j = di; j <= nsrc && j <= di + 400; j++) {
    l = src[j]
    for (m = 1; m <= length(l); m++) {
      c = substr(l, m, 1)
      if (!doneparams) {
        if (!inparams) { if (c == "(" && substr(l, 1, m) ~ (name "[ \t]*\\($")) { inparams = 1; pd = 1 } continue }
        if (c == "(") pd++; else if (c == ")") { pd--; if (pd == 0) { doneparams = 1; continue } }
        plist = plist c; continue
      }
      if (!started) { if (c == "{") { started = 1; depth = 1; bodystart = j } else if (c == ";") { j = nsrc + 1; break } continue }
      if (c == "{") depth++; else if (c == "}") { depth--; if (depth == 0) { last = j; j = nsrc + 2; break } }
    }
    if (inparams && !doneparams) plist = plist " "
  }
  if (!started || depth != 0) { print "//SKIP no body found (a declaration or a macro)"; next }
  rt = inttype(rett); if (rt == "") { print "//SKIP return type is not a plain integer: " rett; next }
  sig = rt; np = split(plist, ps, ","); bad = ""
  for (k = 1; k <= np; k++) {
    p = ps[k]; gsub(/^[ \t]+|[ \t]+$/, "", p)
    if (p == "void" && np == 1) { np = 0; break }
    if (!match(p, /[A-Za-z_][A-Za-z0-9_]*$/)) { bad = p; break }
    pn = substr(p, RSTART); pt = inttype(substr(p, 1, RSTART - 1))
    if (pt == "") { bad = p; break }
    sig = sig "\t" pt "\t" pn
  }
  if (bad != "") { print "//SKIP parameter is not a plain integer: " bad; next }
  print "//SIG " sig
  print "//CONTRACT " contract
  for (j = first; j <= last; j++) print src[j]
}
