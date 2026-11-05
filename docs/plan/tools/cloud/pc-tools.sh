#!/usr/bin/env bash
# On the PC (WSL2 Ubuntu 24.04), once: the gate toolchain, proof tools (PRISM adapters ESBMC/CBMC, Lean, Apalache), pinned gate scripts and the
# read-only Linux and Wine reference trees. Idempotent.
#   sudo LINUX_TAG=<newest longterm tag on kernel.org> WINE_TAG=<wine stable tag> PBSD_USER=$USER bash pc-tools.sh
# The same tags must be given to every cloud worker (cloud-init.yaml), so all gates test against one reference.
set -euo pipefail
[ "$(id -u)" -eq 0 ] || { echo "run with sudo"; exit 1; }
u="${PBSD_USER:-${SUDO_USER:-odin}}"; home="$(getent passwd "$u" | cut -d: -f6)"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export DEBIAN_FRONTEND=noninteractive
apt-get update -q
apt-get install -y -q build-essential clang lld llvm cmake ninja-build ccache git jq curl unzip xz-utils numactl bubblewrap \
  qemu-system-x86 qemu-system-arm qemu-system-misc qemu-utils openjdk-21-jre-headless z3 python3 python3-yaml \
  libcurl4-openssl-dev nlohmann-json3-dev libsqlite3-dev libnuma-dev sqlite3 tmux
# GCC 16 for the C++26 contract runtime build: from the distribution if it has it, else build or use a toolchain PPA (P0-07 pins it).
apt-get install -y -q g++-16 2>/dev/null || echo "NOTE: g++-16 not in this distribution's archive; P0-07 installs it"
gh_latest() { curl -fsSL "https://api.github.com/repos/$1/releases/latest" | jq -r --arg re "$2" '.assets[] | select(.name | test($re)) | .browser_download_url' | head -1; }
command -v esbmc >/dev/null || { f="${ESBMC_URL:-$(gh_latest esbmc/esbmc 'linux.*\.zip$')}"; curl -fsSL "$f" -o /tmp/esbmc.zip && mkdir -p /opt/esbmc && unzip -qo /tmp/esbmc.zip -d /opt/esbmc && ln -sf "$(find /opt/esbmc -name esbmc -type f -perm -u+x | head -1)" /usr/local/bin/esbmc; } || echo "WARNING: ESBMC not installed"
command -v cbmc >/dev/null || { f="${CBMC_DEB_URL:-$(gh_latest diffblue/cbmc 'ubuntu-24.04.*\.deb$')}"; curl -fsSL "$f" -o /tmp/cbmc.deb && apt-get install -y -q /tmp/cbmc.deb; } || apt-get install -y -q cbmc || echo "WARNING: CBMC not installed"
command -v apalache-mc >/dev/null || { f="${APALACHE_URL:-$(gh_latest apalache-mc/apalache 'apalache.*\.tgz$')}"; curl -fsSL "$f" -o /tmp/apalache.tgz && mkdir -p /opt/apalache && tar -xzf /tmp/apalache.tgz -C /opt/apalache --strip-components=1 && ln -sf /opt/apalache/bin/apalache-mc /usr/local/bin/apalache-mc; } || echo "WARNING: Apalache not installed"
[ -x "$home/.elan/bin/elan" ] || su "$u" -c 'curl -fsSL https://raw.githubusercontent.com/leanprover/elan/master/elan-init.sh | sh -s -- -y --default-toolchain none' || echo "WARNING: elan not installed"
ln -sf "$home/.elan/bin/lake" /usr/local/bin/lake 2>/dev/null || true; ln -sf "$home/.elan/bin/lean" /usr/local/bin/lean 2>/dev/null || true
# Pinned gate scripts (the runner's gates_dir) and the task list the integrator gate reads.
install -d /opt/pbsd/gates; install -m 755 "$here"/../gates/*.sh /opt/pbsd/gates/; install -m 644 "$here/../../tasks.json" /opt/pbsd/tasks.json
# Reference trees under ~/pbsd-ref (ref_roots in the runner config).
install -d -o "$u" "$home/pbsd-ref"
[ -n "${LINUX_TAG:-}" ] && [ ! -d "$home/pbsd-ref/linux" ] && su "$u" -c "git clone -q --depth 1 --branch '$LINUX_TAG' https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git '$home/pbsd-ref/linux'"
[ -n "${WINE_TAG:-}" ] && [ ! -d "$home/pbsd-ref/wine" ] && su "$u" -c "git clone -q --depth 1 --branch '$WINE_TAG' https://gitlab.winehq.org/wine/wine.git '$home/pbsd-ref/wine'"
# PRISM (odin-loki/PRISM) is built by you with its own scripts (Clang 19+, FORGE is fetched by its CMake):
#   git clone https://github.com/odin-loki/PRISM ~/PRISM && cd ~/PRISM && scripts/wsl_build.sh
# then point PRISM_BIN at build/prism, and PRISM_PROVER_FPK at your Goedel-Prover .fpk (tested on FORGE).
[ -x "${PRISM_BIN:-$home/PRISM/build/prism}" ] && echo "  PRISM found: ${PRISM_BIN:-$home/PRISM/build/prism}" || echo "  WARNING: PRISM not built yet (see the note above)"
for t in esbmc cbmc apalache-mc lake qemu-system-x86_64; do printf '  %-20s %s\n' "$t" "$(command -v $t || echo MISSING)"; done
[ -w /dev/kvm ] && echo "  KVM available" || echo "  WARNING: no /dev/kvm: enable nested virtualization for WSL2 (.wslconfig nestedVirtualization=true) or QEMU gates will be slow"
