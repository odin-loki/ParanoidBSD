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
set -euo pipefail
[ "$(id -u)" -eq 0 ] || { echo "run as root"; exit 1; }
: "${PBSD_PUBKEY:?set PBSD_PUBKEY to the runner ssh public key}"
PBSD_IS_MIRROR="${PBSD_IS_MIRROR:-1}"
PBSD_MIRROR="${PBSD_MIRROR:-/srv/pbsd/pbsd.git}"
export DEBIAN_FRONTEND=noninteractive

echo "== packages (toolchain, QEMU, PRISM runtime)"
apt-get update -q
apt-get install -y -q build-essential clang lld llvm bmake bison flex m4 cmake ninja-build ccache git \
  qemu-system-x86 qemu-utils openjdk-17-jre-headless python3 rsync curl jq xz-utils wireguard-tools \
  util-linux coreutils procps
# PRISM: prebuilt Linux release into /opt/prism (version pinned; change PBSD_PRISM_VER to upgrade).
PV="${PBSD_PRISM_VER:-4.8.1}"
if ! command -v prism >/dev/null; then
  curl -fsSL "https://github.com/prismmodelchecker/prism/releases/download/v$PV/prism-$PV-linux64-x86.tar.gz" -o /tmp/prism.tgz \
    && mkdir -p /opt/prism && tar -xzf /tmp/prism.tgz -C /opt/prism --strip-components=1 \
    && (cd /opt/prism && ./install.sh >/dev/null) && ln -sf /opt/prism/bin/prism /usr/local/bin/prism \
    || echo "WARNING: PRISM download failed; proof gates will exit 3 until it is installed"
fi

echo "== user pbsd and ssh key"
id pbsd >/dev/null 2>&1 || useradd -m -s /bin/bash pbsd
install -d -m 700 -o pbsd -g pbsd /home/pbsd/.ssh
grep -qxF "$PBSD_PUBKEY" /home/pbsd/.ssh/authorized_keys 2>/dev/null || echo "$PBSD_PUBKEY" >> /home/pbsd/.ssh/authorized_keys
chown pbsd:pbsd /home/pbsd/.ssh/authorized_keys; chmod 600 /home/pbsd/.ssh/authorized_keys
usermod -aG kvm pbsd 2>/dev/null || true
# Key-only ssh.
sed -i 's/^#\?PasswordAuthentication.*/PasswordAuthentication no/' /etc/ssh/sshd_config
grep -q '^MaxStartups 200' /etc/ssh/sshd_config || echo 'MaxStartups 200:30:400' >> /etc/ssh/sshd_config
grep -q '^MaxSessions 200' /etc/ssh/sshd_config || echo 'MaxSessions 200' >> /etc/ssh/sshd_config
systemctl reload ssh 2>/dev/null || systemctl reload sshd 2>/dev/null || true

echo "== tmpfs workdir (the gates run in RAM; nothing on this box needs to survive)"
ram_gb=$(awk '/MemTotal/ {printf "%d", $2/1048576}' /proc/meminfo)
sz="${PBSD_TMPFS_GB:-$(( ram_gb * 70 / 100 ))}"
mkdir -p /run/pbsd
mountpoint -q /run/pbsd || mount -t tmpfs -o "size=${sz}G,mode=0755" tmpfs /run/pbsd
grep -q ' /run/pbsd ' /etc/fstab || echo "tmpfs /run/pbsd tmpfs size=${sz}G,mode=0755 0 0" >> /etc/fstab
install -d -o pbsd -g pbsd /run/pbsd/wt /run/pbsd/ccache /run/pbsd/proof-cache /run/pbsd/tmp
su pbsd -c "CCACHE_DIR=/run/pbsd/ccache ccache -M 40G >/dev/null"

echo "== git mirror"
if [ "$PBSD_IS_MIRROR" = 1 ]; then
  install -d -o pbsd -g pbsd /srv/pbsd
  [ -d /srv/pbsd/pbsd.git ] || su pbsd -c "git init -q --bare /srv/pbsd/pbsd.git && git -C /srv/pbsd/pbsd.git config receive.denyNonFastForwards false && git -C /srv/pbsd/pbsd.git config gc.auto 0"
fi

echo "== gate entry point"
install -d /opt/pbsd/bin /opt/pbsd/gates /etc/pbsd
here="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" 2>/dev/null && pwd || echo /nonexistent)"
if [ -f "$here/../gates/remote-gate.sh" ]; then
  install -m 755 "$here/../gates/remote-gate.sh" /opt/pbsd/bin/pbsd-gate
  install -m 755 "$here"/../gates/*.sh /opt/pbsd/gates/
fi
cat > /etc/pbsd/worker.env <<ENV
PBSD_MIRROR=$PBSD_MIRROR
PBSD_JOBS=2
ENV

echo "== WireGuard (optional)"
if [ -n "${PBSD_WG_CONF:-}" ]; then
  umask 077; printf '%s\n' "$PBSD_WG_CONF" > /etc/wireguard/wg0.conf
  systemctl enable --now wg-quick@wg0
fi

cores=$(lscpu -p=CORE,SOCKET | grep -v '^#' | sort -u | wc -l)
slots=$(( (cores - 4) / 2 )); [ $slots -lt 1 ] && slots=1
line="$(hostname) $slots"
echo "== done: $cores physical cores, ${ram_gb} GB RAM, tmpfs ${sz} GB"
echo "workers.txt line (use your ssh alias for the host):  $line"
if [ -n "${PBSD_RUNNER:-}" ]; then
  su pbsd -c "ssh -o BatchMode=yes -o StrictHostKeyChecking=accept-new $PBSD_RUNNER 'mkdir -p ~/.pbsd && grep -q \"^$(hostname) \" ~/.pbsd/workers.txt 2>/dev/null || echo \"$line\" >> ~/.pbsd/workers.txt'" \
    && echo "registered with $PBSD_RUNNER" || echo "self-registration failed; add the line by hand"
fi
