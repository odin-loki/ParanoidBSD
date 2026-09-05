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
# Run: sh tools/ci/test_init_swap.sh

set -eu

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SCRIPT="$ROOT/tools/ci/build_boot_image.sh"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"

mkdir -p bin fakeroot/sbin fakeroot/rescue fakeroot/boot out
: > out/fake.img
echo "dynamic-init-with-pie-safestack-cfi" > fakeroot/sbin/init
echo "crunched-static-init"                > fakeroot/rescue/init
echo 'vfs.mountroot.timeout="10"'          > fakeroot/boot/loader.conf

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
printf '#!/bin/sh\nexit 0\n'                    > bin/umount
printf '#!/bin/sh\necho "(df stub)"\n'          > bin/df
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
    [ "$rc" = "$2" ] || note FAIL "expected exit $2, got $rc"
}

echo "== extracting"
( INIT_SWAP=rescue; export INIT_SWAP; run swap-only 0 )
MNT="$(cat "$WORK/mnt.swap-only" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/sbin/init.pbsd" ] && note "ok  " "/sbin/init.pbsd exists" \
        || note FAIL "/sbin/init.pbsd missing"
    [ ! -f "$MNT/sbin/init" ] && note "ok  " "/sbin/init is gone" \
        || note FAIL "/sbin/init still there"
    [ -f "$MNT/rescue/init" ] && note "ok  " "/rescue/init untouched" \
        || note FAIL "/rescue/init missing"
    grep -q boot_verbose "$MNT/boot/loader.conf" 2>/dev/null \
        && note FAIL "loader.conf edited when it was not asked for" \
        || note "ok  " "loader.conf left alone"
}

( INIT_SWAP=rescue; LOADER_CONF_EXTRA='boot_verbose="YES"'
  export INIT_SWAP LOADER_CONF_EXTRA; run both 0 )
MNT="$(cat "$WORK/mnt.both" 2>/dev/null || true)"
[ -n "$MNT" ] && {
    [ -f "$MNT/sbin/init.pbsd" ] && note "ok  " "both: init moved" \
        || note FAIL "both: init not moved"
    grep -q 'boot_verbose="YES"' "$MNT/boot/loader.conf" \
        && note "ok  " "both: loader.conf appended" \
        || note FAIL "both: loader.conf not appended"
}

( run neither 0 )
[ -f "$WORK/mnt.neither" ] \
    && note FAIL "mounted the image with nothing to do" \
    || note "ok  " "nothing asked for, image never mounted"

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
