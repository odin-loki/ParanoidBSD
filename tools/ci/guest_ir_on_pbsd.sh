#!/bin/sh
# Run inside the PBSD vm.ufs.raw guest after 9p is mounted.
# Do not force-reinstall llvm21: a 6G root fills during extract.
export PATH="/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"
sysctl hardening.insecure_kmod=1 || true
sysctl hardening.pax.mprotect.status=0 || true
df -h /
unset PYTHONHOME PYTHONPATH
cd /tmp
if python3 -c 'import encodings'; then
  echo PKG_SKIP
else
  echo PKG_PY_BEGIN
  ASSUME_ALWAYS_YES=yes pkg install -yf python312 python3
  echo PKG_PY_END
fi
if command -v clang21 >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then
  ln -sf /usr/local/bin/clang21 /usr/local/bin/clang
  ln -sf /usr/local/bin/clang++21 /usr/local/bin/clang++
  ln -sf /usr/local/bin/clang-cpp21 /usr/local/bin/clang-cpp
fi
export PBSD_ROOT=/mnt/pbsd
export PBSD_ORACLE_OUT=/mnt/out/ir-oracle-pbsd.jsonl
export PYTHONHASHSEED=0
python3 --version
python3 -c "import encodings; print('encodings_ok', encodings.__file__)"
clang --version | head -1 || clang21 --version | head -1 || true
python3 /mnt/out/guest-ir-oracle.py
echo IR_ORACLE_DONE
sync
