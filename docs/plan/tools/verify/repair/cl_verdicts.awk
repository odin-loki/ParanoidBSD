function esc(x) { gsub(/\\/, "\\\\", x); gsub(/"/, "\\\"", x); return x }
      match($0, /: (AGREE|DISAGREE)( |:|$)/) { n=substr($0, 1, RSTART-1); rest=substr($0, RSTART+2); v=rest; sub(/[ :].*/, "", v); r=""; if (v == "DISAGREE") { r=rest; sub(/^DISAGREE:? ?/, "", r) }
        printf "%s{\"name\":\"%s\",\"verdict\":\"%s\",\"reason\":\"%s\"}", (c++ ? "," : ""), esc(n), tolower(v), esc(r) }
