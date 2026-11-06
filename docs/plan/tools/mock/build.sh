#!/usr/bin/env bash
set -e
cd "$GATE_WT"
n=0
while IFS= read -r f; do
  clang++ -std=c++23 -fsyntax-only "$f"
  n=$((n+1))
done < <(find pbsd tests -name '*.cpp' 2>/dev/null)
echo "mock build: $n translation units ok"
