#!/bin/sh
# llama-server for GLM-5.3-Flash on Linux (native boot or WSL2 with the model on a Linux filesystem).
#   glm-server.sh IQ2_XXS | Q4_K_M
# pbsd_run restarts it through server_cmd; this script only starts it once.
set -eu
QUANT=${1:-Q4_K_M}
MODELS=${PBSD_MODELS:-/models/GLM-5.3-Flash}
SERVER=${LLAMA_SERVER:-llama-server}
PORT=${PORT:-8080}; CTX=${CTX_PER_SLOT:-32768}
MODEL=$(find "$MODELS" -name "*${QUANT}*.gguf" | sort | head -n1)
[ -n "$MODEL" ] || { echo "no $QUANT GGUF under $MODELS" >&2; exit 1; }
case "$QUANT" in
  IQ2_XXS) SLOTS=${SLOTS:-2}; THREADS=30; MEM="" ;;      # about 85 GB: mmap, does not fit 64 GB
  Q4_K_M)  SLOTS=${SLOTS:-4}; THREADS=24; MEM="--mlock" ;; # about 195 GB: in RAM
esac
# NUMA: drop the page cache once so the weights are spread over both sockets, then
# --numa distribute spreads threads and memory across nodes 0 and 1.
[ -w /proc/sys/vm/drop_caches ] && sync && echo 3 > /proc/sys/vm/drop_caches || true
mkdir -p "$MODELS/slots"
exec numactl --interleave=all "$SERVER" -m "$MODEL" --host 127.0.0.1 --port "$PORT" \
  --parallel "$SLOTS" --ctx-size $((SLOTS * CTX)) --numa distribute \
  --threads "$THREADS" --threads-batch 32 --batch-size 2048 --ubatch-size 512 \
  -ngl 999 --n-cpu-moe 999 -fa on --jinja --cache-reuse 256 --metrics \
  --cache-type-k q8_0 --cache-type-v q8_0 --slot-save-path "$MODELS/slots" $MEM
