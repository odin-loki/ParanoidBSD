# has_loop.awk  Exit 0 if the C text (file argument) contains a for/while/do loop outside comments and string/char literals, else exit 1.
# Usage: awk -f has_loop.awk fn.c   (a loop is `for (`, `while (` or `do {` / `do` at statement level)
{ s = s $0 "\n" }
END {
  n = length(s); i = 1; out = ""
  while (i <= n) {
    c2 = substr(s, i, 2); c = substr(s, i, 1)
    if (c2 == "/*") { j = index(substr(s, i + 2), "*/"); if (j == 0) break; i += j + 3; out = out " "; continue }
    if (c2 == "//") { j = index(substr(s, i), "\n"); if (j == 0) break; i += j; out = out "\n"; continue }
    if (c == "\"") { k = i + 1; while (k <= n && substr(s, k, 1) != "\"") { if (substr(s, k, 1) == "\\") k++; k++ } i = k + 1; out = out "\"\""; continue }
    if (c == "'") { k = i + 1; while (k <= n && substr(s, k, 1) != "'") { if (substr(s, k, 1) == "\\") k++; k++ } i = k + 1; out = out "0"; continue }
    out = out c; i++
  }
  if (out ~ /(^|[^A-Za-z0-9_])(for|while)[ \t\n]*\(/ || out ~ /(^|[^A-Za-z0-9_])do[ \t\n]*[{]/ || out ~ /(^|[^A-Za-z0-9_])do[ \t\n]+[A-Za-z_(]/) exit 0
  exit 1
}
