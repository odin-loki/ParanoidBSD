#!/usr/bin/env bash
# On the PC (WSL2 Ubuntu 24.04), as your normal user: build PRISM and the analyzers it runs. Idempotent.
# Worked through by hand on 6 Nov 2026; every step below was needed.
#   tools/cloud/build-prism.sh            # clones ~/PRISM if missing
# Needs (once, as root; pc-tools.sh installs them):
#   apt-get install -y libboost-all-dev bison flex libz3-dev libcurl4-openssl-dev llvm-18-dev libclang-18-dev libzstd-dev \
#     libfmt-dev nlohmann-json3-dev libyaml-cpp-dev libgmp-dev libedit-dev libimmer-dev libc6-dev-i386 gcc-multilib
# Why each piece:
#   - PRISM needs its pinned clang 23.1.4 (scripts/fetch_llvm.py, user space). Clang 18 fails: its __cpp_concepts is 201907,
#     libstdc++ 14 needs 202002 for std::expected (FORGE uses it). Clang 19+ is required.
#   - `prism-deps tool` only FETCHES and sha256-verifies the pinned source ("no automatic build recipe"); it does not build.
#     So cppcheck, CBMC and ESBMC are built here with the manifest's build_hint commands and copied to <prefix>/bin,
#     where PRISM looks (~/.prism/tools/<name>/<commit>/bin/<name>, or $PRISM_TOOLS_DIR).
#   - FORGE: PRISM's CMake fetches odin-loki/FORGE main. Until 6 Nov 2026 its grammar parser had no {m,n} repetition (every PRISM grammar uses
#     it), its memory governor crashed at exit with a CUDA runtime, and autodiff read freed memory; PRISM resolved source paths against the
#     cwd, used a fixed model seed, mis-parsed primed theorem names and copied Lake projects without their packages. All of these are merged
#     (FORGE #69 #70 #71, PRISM #40 #41 #42), and a fresh build from main was checked: it proves a Mathlib lemma on the GPU. FORGE_DIR=<path>
#     still builds against a local FORGE checkout.
#   - ESBMC 8.5 supports LLVM 18..22: point it at LLVM 18. Its libc models need the i386 libc headers.
set -euo pipefail
P="${PRISM_DIR:-$HOME/PRISM}"; J="${PBSD_BUILD_JOBS:-$(nproc)}"
[ -d "$P/.git" ] || git clone -q https://github.com/odin-loki/PRISM "$P"
cd "$P"
L="$HOME/.cache/prism/tools/llvm-23.1.4/bin"
[ -x "$L/clang++" ] || python3 scripts/fetch_llvm.py
if [ ! -x build/prism ] || [ ! -x build/prism-deps ]; then
  FORGE_ARGS=(); [ -n "${FORGE_DIR:-}" ] && FORGE_ARGS=(-DFORGE_SOURCE_DIR="$FORGE_DIR")   # see the note above on the grammar fix
  cmake -B build -G Ninja "${FORGE_ARGS[@]}" -DCMAKE_C_COMPILER="$L/clang" -DCMAKE_CXX_COMPILER="$L/clang++" -DPRISM_LLVM_PINNED_BIN="$L" \
    -DCMAKE_BUILD_TYPE=Release -DPRISM_CUDA=OFF -DPRISM_QT=OFF -DPRISM_Z3=ON -DPRISM_TESTS=OFF
  cmake --build build --target prism prism-deps -j"$J"      # FORGE is fetched by this CMake
fi
./build/prism --version
T="${PRISM_TOOLS_DIR:-$HOME/.prism/tools}"
./build/prism-deps tool esbmc cbmc cppcheck --tools-dir "$T"
dirof() { ls -d "$T/$1"/*/ | head -1; }
p="$(dirof cppcheck)"; [ -x "$p/bin/cppcheck" ] || { (cd "$p/src" && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$J") \
  && mkdir -p "$p/bin" && cp "$p/src/build/bin/cppcheck" "$p/bin/" && cp -r "$p/src/cfg" "$p/bin/"; }
p="$(dirof cbmc)"; [ -x "$p/bin/cbmc" ] || { (cd "$p/src" && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DWITH_JBMC=OFF && cmake --build build --target cbmc -j"$J") \
  && mkdir -p "$p/bin" && cp "$p/src/build/bin/cbmc" "$p/bin/"; }
p="$(dirof esbmc)"; [ -x "$p/bin/esbmc" ] || { (cd "$p/src" && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_Z3=ON \
    -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm -DClang_DIR=/usr/lib/llvm-18/lib/cmake/clang && cmake --build build -j"$J") \
  && mkdir -p "$p/bin" && cp "$p/src/build/src/esbmc/esbmc" "$p/bin/"; }
for t in esbmc cbmc cppcheck; do printf '  %-10s %s\n' "$t" "$(ls "$T"/$t/*/bin/$t 2>/dev/null | head -1 || echo MISSING)"; done
echo "export PRISM_BIN=$P/build/prism   # and PRISM_PROVER_FPK=/path/to/goedel-prover.fpk"
