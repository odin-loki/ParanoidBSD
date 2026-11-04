#!/usr/bin/env bash
# QEMU boot test. Boots the image the build produced and waits for the boot marker on the serial
# console. Override with PBSD_QEMU_IMAGE / PBSD_QEMU_MARKER once the image path is fixed.
set -uo pipefail
wt="$1"
img="${PBSD_QEMU_IMAGE:-$(find "$wt" -maxdepth 4 -name '*.img' -newer "$wt/.git" 2>/dev/null | head -1)}"
marker="${PBSD_QEMU_MARKER:-login:}"
[ -n "$img" ] && [ -f "$img" ] || { echo "no boot image found (set PBSD_QEMU_IMAGE)"; exit 1; }
accel=tcg; [ -w /dev/kvm ] && accel=kvm
log="$(mktemp -p "${TMPDIR:-/tmp}" qemu.XXXX.log)"
timeout "${PBSD_QEMU_BOOT_S:-600}" qemu-system-x86_64 -accel "$accel" -smp 2 -m "${PBSD_QEMU_MEM:-2048}" \
  -nographic -no-reboot -serial "file:$log" -monitor none \
  -drive "file=$img,format=raw,snapshot=on" &
pid=$!
while kill -0 $pid 2>/dev/null; do
  if grep -q "$marker" "$log" 2>/dev/null; then kill $pid; echo "booted: found '$marker'"; rm -f "$log"; exit 0; fi
  if grep -Eq 'panic|fatal trap' "$log" 2>/dev/null; then kill $pid; echo "kernel panic"; tail -60 "$log"; exit 1; fi
  sleep 2
done
echo "QEMU exited or timed out before '$marker'"; tail -60 "$log"; exit 1
