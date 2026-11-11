# verify12.awk   Steps 1 and 2 of docs/plan/lib/code-mined/VERIFICATION_PLAN.md over mined function records. gawk, no other dependencies.
# Input (tab separated, sorted by file then start line): key file start end idx name kind contract prose
# Output (one JSON line per function): {"key","i","name","kind","has_contract","s1","s1_why","s2","s2_why"}
#   s1 = pass | fail | skip     step 1, mechanical checks against the source slice ("skip": source file not found)
#   s2 = pass | fail | skip     step 2, stub-grammar syntax check of the single-line ACSL contract ("skip": no contract, or quantifier not parsed)
# Everything here is a HEURISTIC that flags records for review: a fail is a reason to distrust the record, a pass is not a proof of anything.
BEGIN {
  FS = "\t"
  split("requires ensures assigns result true false NULL old valid iff forall exists sizeof unsigned signed int char long short void const " \
        "struct union enum bool size_t ssize_t u8 u16 u32 u64 s8 s16 s32 s64 uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t " \
        "off_t loff_t ptrdiff_t uintptr_t intptr_t double float static volatile inline extern behavior assumes complete disjoint nothing at null " \
        "and or not", kwl, " ")
  for (k in kwl) KW[kwl[k]] = 1
  ERRNO_RE = "E(PERM|NOENT|SRCH|INTR|IO|NXIO|2BIG|NOEXEC|BADF|CHILD|AGAIN|NOMEM|ACCES|FAULT|BUSY|EXIST|XDEV|NODEV|NOTDIR|ISDIR|INVAL|NFILE|MFILE|NOTTY|FBIG|NOSPC|SPIPE|ROFS|MLINK|PIPE|DOM|RANGE|DEADLK|NAMETOOLONG|NOLCK|NOSYS|NOTEMPTY|LOOP|NOTSUP|OPNOTSUPP|TIMEDOUT|OVERFLOW|PROTO|NOBUFS|CONNRESET|NOTCONN|ALREADY|INPROGRESS|HOSTUNREACH|NETUNREACH|ADDRINUSE|ADDRNOTAVAIL|AFNOSUPPORT|MSGSIZE|NOTSOCK|DESTADDRREQ|WOULDBLOCK|RESTARTSYS|NODATA|BADMSG|ILSEQ|IDRM|NOMSG|STALE|NOTBLK|NOPROTOOPT|CANCELED|OWNERDEAD|NOTRECOVERABLE|UCLEAN|NOKEY|NOMEDIUM|NOLINK|REMOTEIO|DQUOT|SHUTDOWN|TOOMANYREFS|HOSTDOWN|CONNREFUSED|CONNABORTED|NETDOWN|NETRESET|ISCONN|NOTUNIQ|PROTONOSUPPORT|PFNOSUPPORT|SOCKTNOSUPPORT|NOTCAPABLE|INTEGRITY)"
  curfile = ""; curkey = ""
}

function load(f,    line, n, w, i, ws) {
  delete src; delete fwords; nsrc = 0
  if ((getline line < f) < 0) { close(f); return 0 }
  src[++nsrc] = line
  while ((getline line < f) > 0) src[++nsrc] = line
  close(f)
  for (i = 1; i <= nsrc; i++) { w = src[i]; gsub(/[^A-Za-z0-9_]+/, " ", w); n = split(w, ws, " "); for (k = 1; k <= n; k++) fwords[ws[k]] = 1 }
  return 1
}

function rettype(name, from, to,    i, line, pos, pre, prev, j) {
  for (i = from; i <= to && i <= nsrc; i++) {
    line = src[i]
    if (!match(line, "(^|[^A-Za-z0-9_])" name "[ \t]*\\(")) continue
    pos = RSTART; if (substr(line, pos, 1) !~ /[A-Za-z0-9_]/) pos++
    pre = substr(line, 1, pos - 1)
    if (pre ~ /[=();.>]|return|^[ \t]*(else|goto|if|while|for|case|do)([ \t]|$)/) continue
    if (pre ~ /^[ \t]*[A-Za-z_][A-Za-z0-9_ \t*]*$/) return pre
    if (pre ~ /^[ \t]*$/ && line !~ /^[ \t]/) {          # FreeBSD style: return type on the line before the name
      for (j = i - 1; j >= 1 && src[j] ~ /^[ \t]*$/; j--) ;
      prev = src[j]
      if (prev ~ /^[A-Za-z_][A-Za-z0-9_ \t*]*$/) return prev
    }
  }
  return ""
}

function is_void(t,   x) {
  x = " " t
  if (x !~ /[ \t]void([ \t*]|$)/) return 0
  sub(/.*[ \t]void/, "", x)
  return (x !~ /\*/)
}

function add(s, r) { return (s == "" ? r : s "; " r) }

function esc(s) { gsub(/\\/, "\\\\", s); gsub(/"/, "\\\"", s); return s }

# step 1 ---------------------------------------------------------------------------------------------------------------------------------
function step1(    why, rt, c, t, w, n, ws, k, unk, nun, codes, p, omitted, generic, e, hasreq, hasens) {
  why = ""
  if (name !~ /^[A-Za-z_][A-Za-z0-9_]*$/) {
    # not a plain C identifier (this also protects the regexes below from names like BPF_PROG(sub). Accepted when it is the exact text of the source
    # (SYSCALL_DEFINE0(getpid)) or a C++ name whose last component is in the slice (File::flush, ~iscsi_connection, Ns::Class::operator==); rejected otherwise
    # ("<unnamed VRAM size probe>", "sys_mac_syscall (stub)", "write_gic_<name>_intr_reg")
    if (index(stext, name)) return ""
    last = name; sub(/.*::/, "", last); sub(/^~/, "", last)
    if (name ~ /::|^~/ && (last ~ /^operator/ || ((last ~ /^[A-Za-z_][A-Za-z0-9_]*$/) && (last in swords)))) return ""
    return "function name is not a C or C++ name found in the slice"
  }
  if (!(name in swords)) why = add(why, "function name not in the source slice")
  if (contract != "") {
    hasreq = (contract ~ /requires/); hasens = (contract ~ /ensures/)
    if (!hasreq) why = add(why, "contract has no requires")
    if (!hasens) why = add(why, "contract has no ensures")
    if (hasens) {                                        # ensures that only says true
      e = contract; sub(/.*ensures/, "", e); sub(/\*\/.*/, "", e); gsub(/[ \t;\\]/, "", e)
      if (e == "true" || e == "") why = add(why, "ensures asserts nothing (true)")
    }
    if (contract ~ /(^|[^A-Za-z0-9_])\\?result([^A-Za-z0-9_]|$)/ && src_ok) {
      rt = rettype(name, start, endl)
      if (rt != "" && is_void(rt)) why = add(why, "contract uses result but the function returns void")
    }
    # identifiers in the contract that exist nowhere in the file (prose, invented parameter names)
    c = contract
    gsub(/\\[A-Za-z_]+/, " ", c); gsub(/(->|\.)[ \t]*[A-Za-z_][A-Za-z0-9_]*/, " ", c); gsub(/[^A-Za-z0-9_]+/, " ", c)
    n = split(c, ws, " "); unk = ""; nun = 0
    for (k = 1; k <= n; k++) {
      w = ws[k]
      if (w ~ /^[0-9]/ || (w in KW) || w ~ /^[A-Z0-9_]+$/) continue
      if (src_ok && !(w in fwords)) { if (nun < 3) unk = unk (unk == "" ? "" : ",") w; nun++ }
    }
    if (nun > 0) why = add(why, "contract names " nun " identifier(s) not in the file (" unk ")")
    # error codes the prose lists but the formal ensures omits
    p = prose; omitted = ""
    while (match(p, "(^|[^A-Za-z0-9_])-?" ERRNO_RE "([^A-Za-z0-9_]|$)")) {
      t = substr(p, RSTART, RLENGTH); gsub(/[^A-Z0-9]/, "", t); sub(/^E/, "E", t)
      if (index(contract, t) == 0 && index(omitted, t) == 0) omitted = omitted (omitted == "" ? "" : ",") t
      p = substr(p, RSTART + RLENGTH - 1)
    }
    generic = (contract ~ /result[ \t]*(<|<=|!=)[ \t]*0/)
    if (omitted != "" && !generic) why = add(why, "prose lists " omitted " but the formal contract omits it")
  }
  return why
}

# step 2 ---------------------------------------------------------------------------------------------------------------------------------
function step2(    c, depth, i, ch, clauses, nc, body, t, k) {
  if (contract == "") { s2 = "skip"; s2why = "no formal contract"; return }
  if (contract ~ /\\forall|\\exists/) { s2 = "skip"; s2why = "quantifier: not parsed by the stub grammar"; return }
  if (contract !~ /^[ \t]*\/\*@/) { s2 = "fail"; s2why = "does not start with /*@"; return }
  if (contract !~ /\*\/[ \t]*$/) { s2 = "fail"; s2why = "does not end with */"; return }
  c = contract; sub(/^[ \t]*\/\*@/, "", c); sub(/\*\/[ \t]*$/, "", c)
  depth = 0
  for (i = 1; i <= length(c); i++) { ch = substr(c, i, 1); if (ch ~ /[(\[{]/) depth++; else if (ch ~ /[)\]}]/) depth--; if (depth < 0) break }
  if (depth != 0) { s2 = "fail"; s2why = "unbalanced brackets"; return }
  nc = split(c, clauses, ";")
  for (k = 1; k <= nc; k++) {
    body = clauses[k]; gsub(/^[ \t]+|[ \t]+$/, "", body)
    if (body == "") continue
    if (body !~ /^(requires|ensures|assigns|decreases)([ \t]|\\|\(|$)/) { s2 = "fail"; s2why = "clause does not start with requires/ensures/assigns: " substr(body, 1, 40); return }
    sub(/^(requires|ensures|assigns|decreases)/, "", body)
    t = body
    gsub(/\\[A-Za-z_]+/, "§", t)                                      # ACSL builtins
    gsub(/(^|[^A-Za-z0-9_])(unsigned|signed|long|short|int|char|const|struct|union|enum|void|double|float|sizeof)([^A-Za-z0-9_]|$)/, " ", t)
    gsub(/0[xX][0-9a-fA-F]+[uUlL]*|[0-9]+[uUlL]*/, "N", t)
    gsub(/[A-Za-z_][A-Za-z0-9_]*/, "I", t)
    if (t ~ /[IN][ \t]+[IN]/) { s2 = "fail"; s2why = "two operands side by side: reads like prose, not an expression"; return }
    if (body ~ /[^ \t\x20-\x7e]/) { s2 = "fail"; s2why = "non-ASCII character"; return }
  }
  s2 = "pass"; s2why = ""
}

# main -------------------------------------------------------------------------------------------------------------------------------------
{
  key = $1; file = $2; start = $3 + 0; endl = $4 + 0; idx = $5; name = $6; kind = $7; contract = $8; prose = $9
  if (file != curfile) { src_ok = load(file); curfile = file; curkey = "" }
  if (key != curkey) {
    delete swords; curkey = key; stext = ""
    if (src_ok) { for (i = start; i <= endl && i <= nsrc; i++) { stext = stext src[i] "\n"; w = src[i]; gsub(/[^A-Za-z0-9_]+/, " ", w); n = split(w, ws, " "); for (k = 1; k <= n; k++) swords[ws[k]] = 1 } }
  }
  if (src_ok) { why1 = step1(); s1 = (why1 == "" ? "pass" : "fail") } else { s1 = "skip"; why1 = "source file not found" }
  step2()
  printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"kind\":\"%s\",\"has_contract\":%s,\"s1\":\"%s\",\"s1_why\":\"%s\",\"s2\":\"%s\",\"s2_why\":\"%s\"}\n", \
         esc(key), idx, esc(name), esc(kind), (contract != "" ? "true" : "false"), s1, esc(why1), s2, esc(s2why)
}
