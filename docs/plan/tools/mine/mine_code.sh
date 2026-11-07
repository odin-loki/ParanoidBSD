#!/bin/bash
# mine_code.sh <corpus> <root-dir> <out.jsonl> <max-chunks> [file-list]   Chunk C/C++ files (<=300 lines, cut at blank lines) and mine each chunk with GLM-5.3-Flash.
# Resumable (done chunks are skipped); 8 calls in parallel; output is appended to the repository folder as it arrives. Cost: about $0.0005-0.001 per chunk.
HERE=$(cd "$(dirname "$0")" && pwd); CORPUS=$1; ROOT=$2; OUT=$3; MAX=${4:-20}; LIST=${5:-}
mkdir -p "$(dirname "$OUT")"
if [ -n "$LIST" ]; then shuf --random-source=<(yes 42) "$LIST"; else find "$ROOT" -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.cc' \) -size +1k -size -60k | sort; fi |
while read -r f; do
  awk -v f="$f" 'BEGIN{s=1;n=0} {n++} (n-s>=250 && $0=="") || (n-s>=300){print f, s, n; s=n+1} END{if(n>=s)print f, s, n}' "$f"
done | head -n "$MAX" | xargs -P 8 -L 1 bash -c "$HERE/mine_chunk.sh $CORPUS \$0 \$1 \$2 $OUT"
echo "chunks mined: $(wc -l < "$OUT") ; tokens in/out: $(jq -s '[.[].usage.prompt_tokens]|add' "$OUT") / $(jq -s '[.[].usage.completion_tokens]|add' "$OUT")"
