#!/usr/bin/env bash
# PBSD cloud gate worker bootstrap (Ubuntu 24.04 / Debian 12). Run as root, once per machine:
#   curl -fsSL <raw url>/bootstrap.sh | sudo PBSD_PUBKEY="ssh-ed25519 AAAA... odin@p920" bash
# or pass it as user-data via cloud-init.yaml. Idempotent: safe to run again after a spot restart.
# Env:
#   PBSD_PUBKEY    (required) the PC's runner key, allowed to log in as user pbsd
#   PBSD_MIRROR    where workers fetch task commits (default: this box's /srv/pbsd/pbsd.git)
#   PBSD_IS_MIRROR 1 = this box also hosts the bare git mirror (default 1 on the first worker)
#   PBSD_TMPFS_GB  size of the RAM workdir (default: 70% of RAM)
#   PBSD_WG_CONF   optional WireGuard config contents for wg0 (see wg-worker.conf.example)
#   PBSD_RUNNER    optional ssh target on the PC (over WireGuard) to self-register, e.g. odin@10.66.0.1
#   LINUX_TAG, WINE_TAG   reference trees for differential tests: the same tags as on the PC
#   APALACHE_URL   release download (default: the latest release; P0-07 pins it). ESBMC, CBMC and cppcheck are not installed here:
#                  mirror-setup.sh copies the PC's PRISM-built (pinned) copies to /opt/pbsd/prism-tools
set -euo pipefail
[ "$(id -u)" -eq 0 ] || { echo "run as root"; exit 1; }
: "${PBSD_PUBKEY:?set PBSD_PUBKEY to the runner ssh public key}"
PBSD_IS_MIRROR="${PBSD_IS_MIRROR:-1}"
PBSD_MIRROR="${PBSD_MIRROR:-/srv/pbsd/pbsd.git}"
here="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" 2>/dev/null && pwd || echo /nonexistent)"
export DEBIAN_FRONTEND=noninteractive

echo "== packages (toolchain, QEMU for amd64/arm64/riscv64, solvers, Java for Apalache)"
apt-get update -q
apt-get install -y -q build-essential clang lld llvm bmake bison flex m4 cmake ninja-build ccache git jq curl unzip xz-utils \
  qemu-system-x86 qemu-system-arm qemu-system-misc qemu-utils openjdk-21-jre-headless z3 python3 rsync wireguard-tools \
  iptables bubblewrap util-linux coreutils procps

echo "== proof tools (AGENTS.md section 10). A missing tool makes its gates exit 3 (retried), never pass"
gh_latest() {  # gh_latest <owner/repo> <asset regex>: newest matching release asset
  curl -fsSL "https://api.github.com/repos/$1/releases/latest" | jq -r --arg re "$2" '.assets[] | select(.name | test($re)) | .browser_download_url' | head -1
}
if ! command -v apalache-mc >/dev/null; then
  u="${APALACHE_URL:-https://github.com/apalache-mc/apalache/releases/latest/download/apalache.tgz}"   # not the API: it 403s when rate limited
  if [ -n "$u" ] && curl -fsSL "$u" -o /tmp/apalache.tgz; then
    mkdir -p /opt/apalache && tar -xzf /tmp/apalache.tgz -C /opt/apalache --strip-components=1 && ln -sf /opt/apalache/bin/apalache-mc /usr/local/bin/apalache-mc
  else echo "WARNING: Apalache not installed"; fi
fi

echo "== user pbsd and ssh key"
id pbsd >/dev/null 2>&1 || useradd -m -s /bin/bash pbsd
install -d -m 700 -o pbsd -g pbsd /home/pbsd/.ssh
grep -qxF "$PBSD_PUBKEY" /home/pbsd/.ssh/authorized_keys 2>/dev/null || echo "$PBSD_PUBKEY" >> /home/pbsd/.ssh/authorized_keys
chown pbsd:pbsd /home/pbsd/.ssh/authorized_keys; chmod 600 /home/pbsd/.ssh/authorized_keys
usermod -aG kvm pbsd 2>/dev/null || true
# Lean 4 through elan; the toolchain version comes from proofs/lean-toolchain in the repository (P0-07).
if [ ! -x /home/pbsd/.elan/bin/elan ]; then
  su pbsd -c 'curl -fsSL https://raw.githubusercontent.com/leanprover/elan/master/elan-init.sh | sh -s -- -y --default-toolchain none' \
    || echo "WARNING: elan not installed; Lean model gates will exit 3"
fi
ln -sf /home/pbsd/.elan/bin/lake /usr/local/bin/lake 2>/dev/null || true
ln -sf /home/pbsd/.elan/bin/lean /usr/local/bin/lean 2>/dev/null || true
# Key-only ssh, enough sessions for ~100 multiplexed gates.
sed -i 's/^#\?PasswordAuthentication.*/PasswordAuthentication no/' /etc/ssh/sshd_config
grep -q '^MaxStartups 200' /etc/ssh/sshd_config || echo 'MaxStartups 200:30:400' >> /etc/ssh/sshd_config
grep -q '^MaxSessions 200' /etc/ssh/sshd_config || echo 'MaxSessions 200' >> /etc/ssh/sshd_config
systemctl reload ssh 2>/dev/null || systemctl reload sshd 2>/dev/null || true

echo "== isolation for agent-written code"
# Gates run in a network namespace without network (remote-gate.sh); Ubuntu 24.04 restricts
# unprivileged user namespaces, so allow them on this dedicated box. No cloud credentials live here,
# and user pbsd cannot reach the instance metadata service.
sysctl -qw kernel.apparmor_restrict_unprivileged_userns=0 2>/dev/null || true
echo 'kernel.apparmor_restrict_unprivileged_userns=0' > /etc/sysctl.d/60-pbsd-userns.conf
iptables -C OUTPUT -d 169.254.169.254 -m owner --uid-owner pbsd -j REJECT 2>/dev/null \
  || iptables -A OUTPUT -d 169.254.169.254 -m owner --uid-owner pbsd -j REJECT || true

echo "== tmpfs workdir (the gates run in RAM; nothing on this box needs to survive)"
ram_gb=$(awk '/MemTotal/ {printf "%d", $2/1048576}' /proc/meminfo)
sz="${PBSD_TMPFS_GB:-$(( ram_gb * 70 / 100 ))}"
mkdir -p /run/pbsd
mountpoint -q /run/pbsd || mount -t tmpfs -o "size=${sz}G,mode=0755" tmpfs /run/pbsd
grep -q ' /run/pbsd ' /etc/fstab || echo "tmpfs /run/pbsd tmpfs size=${sz}G,mode=0755 0 0" >> /etc/fstab
install -d -o pbsd -g pbsd /run/pbsd/wt /run/pbsd/ccache /run/pbsd/proof-cache /run/pbsd/tmp /run/pbsd/build
su pbsd -c "CCACHE_DIR=/run/pbsd/ccache ccache -M 40G >/dev/null"

echo "== git mirror"
if [ "$PBSD_IS_MIRROR" = 1 ]; then
  install -d -o pbsd -g pbsd /srv/pbsd
  [ -d /srv/pbsd/pbsd.git ] || su pbsd -c "git init -q --bare /srv/pbsd/pbsd.git && git -C /srv/pbsd/pbsd.git config receive.denyNonFastForwards false && git -C /srv/pbsd/pbsd.git config gc.auto 0"
fi

echo "== reference trees (read-only, for differential tests)"
install -d -o pbsd -g pbsd /srv/ref
[ -n "${LINUX_TAG:-}" ] && [ ! -d /srv/ref/linux ] && { su pbsd -c "git clone -q --depth 1 --branch '$LINUX_TAG' https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git /srv/ref/linux" || echo "WARNING: Linux $LINUX_TAG clone failed"; }
[ -n "${WINE_TAG:-}" ] && [ ! -d /srv/ref/wine ] && { su pbsd -c "git clone -q --depth 1 --branch '$WINE_TAG' https://gitlab.winehq.org/wine/wine.git /srv/ref/wine" || echo "WARNING: Wine $WINE_TAG clone failed"; }

echo "== pinned gate scripts and entry points (agents cannot change these)"
install -d /opt/pbsd/bin /opt/pbsd/gates /etc/pbsd
install -d -o pbsd -g pbsd /opt/pbsd/prism /opt/pbsd/prism-tools    # mirror-setup.sh copies the PC's PRISM build and its tools here
if [ -f "$here/../gates/remote-gate.sh" ]; then
  install -m 755 "$here/../gates/remote-gate.sh" /opt/pbsd/bin/pbsd-gate
  install -m 755 "$here"/../gates/*.sh /opt/pbsd/gates/
  [ -f "$here/../../tasks.json" ] && install -m 644 "$here/../../tasks.json" /opt/pbsd/tasks.json
fi
# hbsd/ and kde/ reference trees come from the mirror once the PC has seeded it (mirror-setup.sh calls this).
cat > /opt/pbsd/bin/pbsd-ref-setup <<'REF'
#!/usr/bin/env bash
# Run as pbsd after the mirror is seeded: read-only hbsd/ and kde/ checkouts under /srv/ref.
set -euo pipefail
. /etc/pbsd/worker.env
ref="${1:-pbsd-base}"
[ -d /srv/ref/.pbsd ] || git clone -q --no-checkout "$PBSD_MIRROR" /srv/ref/.pbsd
git -C /srv/ref/.pbsd fetch -q origin "$ref" && git -C /srv/ref/.pbsd sparse-checkout set hbsd kde && git -C /srv/ref/.pbsd checkout -q --detach FETCH_HEAD
ln -sfn /srv/ref/.pbsd/hbsd /srv/ref/hbsd; ln -sfn /srv/ref/.pbsd/kde /srv/ref/kde
echo "reference trees ready at /srv/ref ($ref)"
REF
chmod 755 /opt/pbsd/bin/pbsd-ref-setup
cat > /etc/pbsd/worker.env <<ENV
PBSD_MIRROR=$PBSD_MIRROR
PBSD_JOBS=2
PBSD_REF=/srv/ref
PBSD_TASKS=/opt/pbsd/tasks.json
PRISM_BIN=/opt/pbsd/prism/prism
PRISM_TOOLS_DIR=/opt/pbsd/prism-tools
ENV

echo "== WireGuard (optional)"
if [ -n "${PBSD_WG_CONF:-}" ]; then
  umask 077; printf '%s\n' "$PBSD_WG_CONF" > /etc/wireguard/wg0.conf
  systemctl enable --now wg-quick@wg0
fi

cores=$(lscpu -p=CORE,SOCKET | grep -v '^#' | sort -u | wc -l)
slots=$(( (cores - 4) / 2 )); [ $slots -lt 1 ] && slots=1
kvm=""; [ -e /dev/kvm ] && kvm=" kvm"
line="$(hostname) $slots$kvm"
echo "== done: $cores physical cores, ${ram_gb} GB RAM, tmpfs ${sz} GB, KVM ${kvm:- none (QEMU gates stay on the PC)}"
for t in apalache-mc lake qemu-system-x86_64; do printf '  %-20s %s\n' "$t" "$(command -v $t || echo MISSING)"; done
[ -x /opt/pbsd/prism/prism ] || echo "  PRISM not yet on this worker: run tools/cloud/mirror-setup.sh from the PC (copies it)"
echo "workers.txt line (use your ssh alias for the host):  $line"
if [ -n "${PBSD_RUNNER:-}" ]; then
  su pbsd -c "ssh -o BatchMode=yes -o StrictHostKeyChecking=accept-new $PBSD_RUNNER 'mkdir -p ~/.pbsd && grep -q \"^$(hostname) \" ~/.pbsd/workers.txt 2>/dev/null || echo \"$line\" >> ~/.pbsd/workers.txt'" \
    && echo "registered with $PBSD_RUNNER" || echo "self-registration failed; add the line by hand"
fi
