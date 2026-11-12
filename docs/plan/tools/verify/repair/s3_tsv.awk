#!/usr/bin/awk -f
# s3_tsv.awk <s3.jsonl>   key TAB i TAB s3 TAB detail, WITHOUT jq. A function name with a double quote (operator""_u8) makes a line invalid JSON, and jq stops reading at the
# first such line: in round 1 that hid the step 3 verdicts of 1,930 of 11,619 lines. This parses the fixed layout {"key":"..","i":N,"name":"..","s3":"..","detail":".."} by position.
{
  if (substr($0, 1, 8) != "{\"key\":\"") next
  r = substr($0, 9); e = index(r, "\",\"i\":"); if (e == 0) next
  key = substr(r, 1, e - 1); r = substr(r, e + 6); i = r + 0
  s = index(r, ",\"s3\":\""); if (s == 0) next
  t = substr(r, s + 7); e = index(t, "\""); verdict = substr(t, 1, e - 1)
  d = ""; p = index(t, "\"detail\":\""); if (p > 0) { d = substr(t, p + 10); sub(/"\}$/, "", d) }
  gsub(/\t/, " ", d)
  print key "\t" i "\t" verdict "\t" d
}
