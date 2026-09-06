# Running PBSD locally, on Linux or WSL

Everything in this repository except the disk-image targets runs on a
Linux box. `sh tools/setup_local.sh --install` gets the packages and
`sh tools/setup_local.sh` proves each one does its job.

That script does not check that a package installed. It plants a signed
overflow and requires CBMC to report it, plants four defects and requires
the analyser to find all four, requires UBSan to trap `1 << 31` and exit
non-zero, and runs every gate in the repository. A tool that cannot see
reports zero findings, and this session has hit that three separate times
— `-Wno-everything` hiding a diagnostic, an empty translation unit
comparing equal to itself, `-analyzer-disable-all-checks` silencing the
analyser while it appeared to run. **A green number from a check that was
not running is worse than a red one.**

## What runs here, and what does not

| | |
|---|---|
| model checking, static analysis, every gate | **here**, and fast |
| cross-build of world and kernel | **here**, `tools/build/make.py` |
| `memstick.img`, `disc1.iso`, `vm.ufs.raw` | needs FreeBSD — `makefs` and `mkimg` want the host |

The last row is the only thing that has to stay in CI, and only because of
two tools. Everything that decides whether the tree is *correct* is local.

## Core count is the whole story

The CI runner is 4 cores and a `buildworld` there takes about 45 minutes.
Every number in this repository — 50-minute feedback loops, `--ir-limit`,
timeouts, why a batch is 25 files and not 500 — is downstream of that.

On 64 cores the same work is a different activity:

| | 4 cores (CI) | 64 cores |
|---|---|---|
| `buildworld` + `buildkernel` | ~45 min | **~4–6 min** |
| CBMC over `lib/libc` + `lib/msun`, sound set | ~7 min | **~30 s** |
| CBMC over the `POINTER` class (2,367 functions) | ~3 h | **~12 min** |
| the whole 114,217-function ledger | days | **hours** |

So pass `-j$(nproc)` and `--jobs $(nproc)` everywhere:

```sh
# cross-build the world and kernel
cd hbsd/src
MAKEOBJDIRPREFIX=/tmp/pbsd-obj \
XCC=/usr/bin/clang XCXX=/usr/bin/clang++ XLD=/usr/bin/ld.lld \
python3 tools/build/make.py --host-bindir=/usr/bin -j"$(nproc)" \
    TARGET=amd64 TARGET_ARCH=amd64 \
    SRCCONF="$PWD/../src.conf.pbsd" buildworld buildkernel

# model check everything, both classes
python3 tools/verify/classify.py   --scope lib/libc --scope lib/msun \
    --jobs "$(nproc)" --out /tmp/classes.json
python3 tools/verify/cbmc_driver.py --scope lib/libc --scope lib/msun \
    --classes /tmp/classes.json --jobs "$(nproc)" --out /tmp/ub.jsonl
python3 tools/verify/cbmc_driver.py --scope lib/libc --scope lib/msun \
    --classes /tmp/classes.json --allow POINTER --null-depth 3 \
    --jobs "$(nproc)" --out /tmp/ptr.jsonl

# and the kernel, which has its own header universe
python3 tools/verify/classify.py --scope sys --jobs "$(nproc)" \
    --out /tmp/classes_sys.json
python3 tools/verify/cbmc_driver.py --scope sys \
    --classes /tmp/classes_sys.json --jobs "$(nproc)" --out /tmp/ksys.jsonl

# the second instrument - path-sensitive and interprocedural where CBMC
# is exhaustive and modular. They disagree by construction.
python3 tools/verify/analyze.py --scope lib/libc --scope sys \
    --jobs "$(nproc)" --out /tmp/an.jsonl
```

## Images locally, if you want them

With 64 cores a FreeBSD guest is cheap, and it removes the last CI
dependency:

```sh
qemu-system-x86_64 -m 8G -smp 16 -accel kvm \
    -drive file=FreeBSD-15.0-amd64.qcow2,if=virtio \
    -nographic
```

WSL2 supports nested virtualisation, so `-accel kvm` works there after
enabling it in `.wslconfig`. Clone the tree inside the guest and
`sh tools/ci/build_boot_image.sh memstick` is the same script CI runs.

## Reading a result

`tools/verify/README.md` has the five calibrations that make the output
mean anything, and they are not optional — without them a run over
`lib/libc` reports roughly two thousand findings and essentially all of
them are noise. The short version:

- `PROVED` and `BOUNDED` are different claims and are never merged.
- `PROVED-ASSUMING` is a `POINTER` result: a proof *given* that the
  pointer arguments were valid objects.
- a `static` function's findings need its callers; an **exported**
  function's whole signature domain is the contract.
- floating-point division by zero is defined, and `lib/msun` depends on it.
- an unmodelled extern's return value is as unconstrained as a pointer
  parameter.

Confirm anything you intend to fix with UBSan first. Every entry in
`docs/security/UB_FINDINGS.md` was.
