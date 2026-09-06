#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Exercise build_boot_image.sh's image-editing block against a stub image.
#
# That block runs at the end of a fifty-minute build, on FreeBSD, against a
# real disk image, with mdconfig and gpart and mount. Everything about it is
# hard to reach and expensive to get wrong: run 26 spent fifty minutes
# learning the memstick is MBR and not GPT, because the test before it used
# a GPT layout that had been made up.
#
# So the block is extracted VERBATIM from the script - not reimplemented -
# and run here against stub mdconfig, gpart, mount, umount and df whose
# output is the output those commands really produced in run 26 and 27.
#
# The stub image has NO /rescue, because the real one does not: release/
# Makefile's disc1 target passes MK_RESCUE=no to installworld. The first
# version of this test wrote fakeroot/rescue/init itself and then asserted
# that same file was still there, which is not a test of anything - it
# supplied the precondition and then checked it had not been eaten. Run 33
# spent fifty-one minutes on `FAIL no /rescue/init in the image`.
#
# Run: sh tools/ci/test_init_swap.sh

set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SCRIPT="$ROOT/tools/ci/build_boot_image.sh"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"

# The image, as disc1 really builds it: /sbin/init, no /rescue at all, and
# the installer's distribution tarballs taking up most of the room.
mkdir -p bin fakeroot/sbin fakeroot/boot fakeroot/usr/freebsd-dist out
: > out/fake.img
echo "dynamic-init-with-pie-safestack-cfi" > fakeroot/sbin/init
echo 'vfs.mountroot.timeout="10"'          > fakeroot/boot/loader.conf
echo "base payload"   > fakeroot/usr/freebsd-dist/base.txz
echo "kernel payload" > fakeroot/usr/freebsd-dist/kernel.txz
echo "MANIFEST"       > fakeroot/usr/freebsd-dist/MANIFEST

# The staged distribution tree distributeworld leaves in the objdir: one
# crunched binary and a HARD LINK per program name. The links are the point -
# a copy that does not preserve them writes the binary once per name.
mkdir -p objrelease/dist/base/rescue
echo "crunched-static-rescue-binary" > objrelease/dist/base/rescue/rescue
for p in init sh ls cat mount; do
    ln objrelease/dist/base/rescue/rescue "objrelease/dist/base/rescue/$p"
done

cat > bin/mdconfig <<'EOF'
#!/bin/sh
case "$1" in -a) echo md0 ;; *) : ;; esac
EOF
# The real call is `gpart show -p <name>`, with a bare name and no /dev.
# Getting that wrong is what made the first version of this test report
# "no UFS filesystem" against a script that was working.
cat > bin/gpart <<'EOF'
#!/bin/sh
case "$3" in
md0)   printf '=>      1  2302010    md0  MBR  (1.1G)\n        1   131050  md0s1  efi  (64M)\n   131051  2170960  md0s2  freebsd  [active]  (1.0G)\n' ;;
md0s2) printf '=>       0  2170960  md0s2  BSD  (1.0G)\n         0  2170960  md0s2a  freebsd-ufs  (1.0G)\n' ;;
esac
EOF
cat > bin/mount <<'EOF'
#!/bin/sh
cp -a "$FAKEROOT"/. "$2"/ && echo "$2" > "$MOUNTED_AT"
EOF
# df -k <dir>: the block reads field 4 of the LAST line as free kilobytes.
# DF_FREE drives it, and once the tarballs are gone it reports DF_FREE_AFTER,
# which is how the not-enough-room path gets exercised.
cat > bin/df <<'EOF'
#!/bin/sh
free="${DF_FREE:-1048576}"
for d in "$@"; do
    case "$d" in -*) continue ;; esac
    [ -e "$d/usr/freebsd-dist/base.txz" ] || free="${DF_FREE_AFTER:-$free}"
done
printf 'Filesystem 1024-blocks Used Avail Capacity Mounted on\n'
printf '/dev/md0s2a 1058560 1000000 %s 91%%%% %s\n' "$free" "${1##-*}"
EOF
printf '#!/bin/sh\nexit 0\n' > bin/umount
chmod +x bin/*

# The block itself, lifted out of the real script by its own comment markers.
python3 - "$SCRIPT" <<'PY'
import pathlib, sys
src = pathlib.Path(sys.argv[1]).read_text()
a = src.index("    # INIT_SWAP=rescue moves /sbin/init aside")
tail = '        echo "   detached /dev/$MD"\n    fi\n'
b = src.index(tail, a) + len(tail)
pathlib.Path("block.sh").write_text(
    'set -eu\nREPOROOT="$PWD"\nOUT="fake.img"\n' + src[a:b])
print(f"   extracted {src[a:b].count(chr(10))} lines verbatim")
PY

PATH="$WORK/bin:$PATH"
FAKEROOT="$WORK/fakeroot"
export PATH FAKEROOT

fail=0
note() { printf '  %s %s\n' "$1" "$2"; [ "$1" = FAIL ] && fail=1; return 0; }

run() {   # run <name> <expected-exit>; env comes from the caller
    MOUNTED_AT="$WORK/mnt.$1"
    export MOUNTED_AT
    rm -f "$MOUNTED_AT"
    set +e
    OUTPUT="$(sh block.sh 2>&1)"; rc=$?
    set -e
    printf '\n== %s (exit %s)\n' "$1" "$rc"
    printf '%s\n' "$OUTPUT" | sed 's/^/     /'
    # run is called inside a subshell so the caller can set the environment
    # without leaking it; $OUTPUT does not survive that, so put it on disk.
    printf '%s\n' "$OUTPUT" > "$WORK/out.$1"
    [ "$rc" = "$2" ] || note FAIL "expected exit $2, got $rc"
}

inode() { ls -i "$1" | awk '{print $1}'; }

echo "== extracting"

# 1. The real case: no /rescue in the image, a staged tree in the objdir.
( INIT_SWAP=rescue; OBJRELEASE="$WORK/objrelease"
  export INIT_SWAP OBJRELEASE; run swap-only 0 )
MNT="$(cat "$WORK/mnt.swap-only" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/sbin/init.pbsd" ] && note "ok  " "/sbin/init.pbsd exists" \
        || note FAIL "/sbin/init.pbsd missing"
    [ ! -f "$MNT/sbin/init" ] && note "ok  " "/sbin/init is gone" \
        || note FAIL "/sbin/init still there"
    [ -f "$MNT/rescue/init" ] && note "ok  " "/rescue/init installed" \
        || note FAIL "/rescue/init was not installed"
    if [ -f "$MNT/rescue/init" ] && [ -f "$MNT/rescue/rescue" ] && \
            [ "$(inode "$MNT/rescue/init")" = "$(inode "$MNT/rescue/rescue")" ]
    then note "ok  " "/rescue/init is a hard link to the crunched binary"
    else note FAIL "/rescue/init is a separate copy, not a hard link"
    fi
    [ -f "$MNT/usr/freebsd-dist/base.txz" ] \
        && note "ok  " "room was there; tarballs left alone" \
        || note FAIL "removed the tarballs with room to spare"
    grep -q boot_verbose "$MNT/boot/loader.conf" 2>/dev/null \
        && note FAIL "loader.conf edited when it was not asked for" \
        || note "ok  " "loader.conf left alone"
}

# 2. No staged tree either. This is run 33, and it must say where it looked.
( INIT_SWAP=rescue; export INIT_SWAP; run no-source 1 )
MNT="$(cat "$WORK/mnt.no-source" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/sbin/init" ] && note "ok  " "no source: image left intact" \
        || note FAIL "no source: image was modified anyway"
}
grep -q "dist/base/rescue" "$WORK/out.no-source" \
    && note "ok  " "no source: the message names where it looked" \
    || note FAIL "no source: the message does not name where it looked"

# 3. Not enough room. The installer payload is what gives way.
( INIT_SWAP=rescue; OBJRELEASE="$WORK/objrelease"
  DF_FREE=8; DF_FREE_AFTER=1048576
  export INIT_SWAP OBJRELEASE DF_FREE DF_FREE_AFTER; run tight 0 )
MNT="$(cat "$WORK/mnt.tight" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/rescue/init" ] && note "ok  " "tight: /rescue/init installed" \
        || note FAIL "tight: /rescue/init was not installed"
    [ ! -f "$MNT/usr/freebsd-dist/base.txz" ] \
        && note "ok  " "tight: distribution tarballs removed" \
        || note FAIL "tight: tarballs still there"
    [ -f "$MNT/usr/freebsd-dist/MANIFEST" ] \
        && note "ok  " "tight: only the *.txz went" \
        || note FAIL "tight: took more than the tarballs"
}

# 4. RESCUE_DIST wins over OBJRELEASE.
mkdir -p override/rescue
echo "override-binary" > override/rescue/rescue
ln override/rescue/rescue override/rescue/init
( INIT_SWAP=rescue; OBJRELEASE="$WORK/objrelease"
  RESCUE_DIST="$WORK/override/rescue"
  export INIT_SWAP OBJRELEASE RESCUE_DIST; run override 0 )
MNT="$(cat "$WORK/mnt.override" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    grep -q override-binary "$MNT/rescue/init" 2>/dev/null \
        && note "ok  " "RESCUE_DIST takes precedence" \
        || note FAIL "RESCUE_DIST ignored"
}

# 5. Swap and loader.conf together.
( INIT_SWAP=rescue; OBJRELEASE="$WORK/objrelease"
  LOADER_CONF_EXTRA='boot_verbose="YES"'
  export INIT_SWAP OBJRELEASE LOADER_CONF_EXTRA; run both 0 )
MNT="$(cat "$WORK/mnt.both" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/sbin/init.pbsd" ] && note "ok  " "both: init moved" \
        || note FAIL "both: init not moved"
    grep -q 'boot_verbose="YES"' "$MNT/boot/loader.conf" \
        && note "ok  " "both: loader.conf appended" \
        || note FAIL "both: loader.conf not appended"
}

# 6. Nothing asked for.
( run neither 0 )
[ -f "$WORK/mnt.neither" ] \
    && note FAIL "mounted the image with nothing to do" \
    || note "ok  " "nothing asked for, image never mounted"

# 7. A value that is neither.
( INIT_SWAP=banana; export INIT_SWAP; run bad-value 1 )
MNT="$(cat "$WORK/mnt.bad-value" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/sbin/init" ] && note "ok  " "bad value: image left intact" \
        || note FAIL "bad value: image was modified anyway"
}

echo
if [ "$fail" = 0 ]; then
    echo "all checks passed"
else
    echo "FAILURES"
fi
exit "$fail"
