# extract_defs.awk  Print every #define of the file (line continuations joined), each preceded by #undef, skipping the macro called `skip`.
# Usage: awk -v skip=foo -f extract_defs.awk file.c
{ sub(/\r$/, "") }
/^[ \t]*#[ \t]*define[ \t]/ {
  line = $0
  while (line ~ /\\[ \t]*$/ && (getline nxt) > 0) { sub(/\r$/, "", nxt); sub(/\\[ \t]*$/, "", line); line = line " " nxt }
  s = line; sub(/^[ \t]*#[ \t]*define[ \t]+/, "", s)
  match(s, /^[A-Za-z_][A-Za-z0-9_]*/); nm = substr(s, RSTART, RLENGTH)
  if (nm == "" || nm == skip) next
  gsub(/\/\*[^*]*\*\//, "", line)
  sub(/\/\/.*$/, "", line)
  print "#undef " nm; print line
}
