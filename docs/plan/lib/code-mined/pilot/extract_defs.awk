# extract_defs.awk  Print the #defines of the file (line continuations joined, comments removed), each preceded by #undef, skipping the macro called `skip`.
# A macro that is defined MORE THAN ONCE in the file (typically in different #if/#else arms) is dropped: taking the last definition would silently pick a configuration variant.
# Such a name then stays undefined and the unit fails to build with "unresolved symbol" (an honest failure, not a guessed proof).
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
  cnt[nm]++; order[++n] = nm; text[n] = line
}
END {
  for (i = 1; i <= n; i++) if (cnt[order[i]] == 1) { print "#undef " order[i]; print text[i] }
}
