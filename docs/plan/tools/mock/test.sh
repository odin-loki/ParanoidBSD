#!/usr/bin/env bash
set -e
cd "$GATE_WT"
n=0
while IFS= read -r f; do
  clang++ -std=c++23 "$f" -o /tmp/mock_t_$$ && /tmp/mock_t_$$
  n=$((n+1))
done < <(find tests -name 'test.cpp' 2>/dev/null)
echo "mock tests: $n passed"
