#!/usr/bin/env bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Wave 0 CI driver: clang-tidy L1 notes, SI harness, inventory scripts.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

echo "== ParanoidBSD Wave 0 analyser CI =="

# --- L1 clang-tidy (non-blocking notes) --------------------------------------
if command -v clang-tidy >/dev/null 2>&1; then
    if [[ -f compile_commands.json ]]; then
        echo "-- clang-tidy L1 (pbsd/, first 24 TUs) --"
        mapfile -t _tidy_targets < <(find pbsd -type f \( -name '*.cpp' -o -name '*.cppm' \) | sort | head -24)
        tidy_rc=0
        for f in "${_tidy_targets[@]}"; do
            clang-tidy -p "$ROOT" "$f" --quiet 2>/dev/null || tidy_rc=1
        done
        if [[ $tidy_rc -ne 0 ]]; then
            echo "clang-tidy: notes emitted (non-fatal in Wave 0)"
        else
            echo "clang-tidy: clean on sampled TUs"
        fi
    else
        echo "SKIP clang-tidy: compile_commands.json missing (run cmake -B build -S pbsd)"
    fi
else
    echo "SKIP clang-tidy: not installed"
fi

# --- SI harness (hosted CMake) -----------------------------------------------
if command -v cmake >/dev/null 2>&1 && command -v ninja >/dev/null 2>&1; then
    echo "-- SI harness (cmake + ctest) --"
    cmake -B build/wave0 -S pbsd -G Ninja -DCMAKE_BUILD_TYPE=Debug
    cmake --build build/wave0 --target pbsd_si_harness pbsd_cxx_hello_smoke
    ctest --test-dir build/wave0 --output-on-failure -R 'si_harness|cxx_hello_smoke'
elif command -v cmake >/dev/null 2>&1; then
    echo "-- SI harness (cmake + ctest, Unix Makefiles) --"
    cmake -B build/wave0 -S pbsd -DCMAKE_BUILD_TYPE=Debug
    cmake --build build/wave0 --target pbsd_si_harness pbsd_cxx_hello_smoke
    ctest --test-dir build/wave0 --output-on-failure -R 'si_harness|cxx_hello_smoke'
else
    echo "SKIP SI harness: cmake not installed"
fi

# --- Inventory scripts -------------------------------------------------------
echo "-- C source inventory --"
python3 tools/inventory_c_sources.py
python3 tools/wave_purge_c_check.py

echo "== Wave 0 CI complete =="
