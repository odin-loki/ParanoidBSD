#!/bin/sh
# llama-server for the local Lean prover (the runner's lean_prove tool; AGENTS.md section 10).
# A Lean-specialised model, for example Goedel-Prover-V2-32B or DeepSeek-Prover-V2, fully on one GPU.
# At Q4_K_M a 32B model is about 20 GB, which fits the RTX 3090 (24 GB) with a 32k context.
#   lean-prover.sh            # serves http://127.0.0.1:8081/v1, the default lean_prover_url
# If no GGUF of the model is published, convert it with llama.cpp's convert_hf_to_gguf.py and quantise.
set -eu
MODELS=${PBSD_PROVER_MODELS:-/models/lean-prover}
QUANT=${1:-Q4_K_M}
SERVER=${LLAMA_SERVER:-llama-server}
PORT=${PORT:-8081}; SLOTS=${SLOTS:-2}; CTX=${CTX_PER_SLOT:-32768}
MODEL=$(find "$MODELS" -name "*${QUANT}*.gguf" | sort | head -n1)
[ -n "$MODEL" ] || { echo "no $QUANT GGUF under $MODELS" >&2; exit 1; }
# CUDA_VISIBLE_DEVICES picks the GPU (0 = RTX 3090 on the P920; 1 = the A4000 for a smaller 8B prover).
exec "$SERVER" -m "$MODEL" --host 127.0.0.1 --port "$PORT" --parallel "$SLOTS" --ctx-size $((SLOTS * CTX)) \
  -ngl 999 -fa on --jinja --metrics --cache-type-k q8_0 --cache-type-v q8_0
