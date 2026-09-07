#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The build-system readers in includes.py, against inputs and the tree.

Every one of these exists because the tool had a rule where the build
system had an answer, and each was found by a translation unit that came
back ERROR and looked exactly like a clean one:

  the language standard   -std=c17 against a tree that says gnu17, so
                          `typeof' was not a keyword and every vendored
                          Linux driver that allocates a struct failed
  makeoptions             a compile-with naming a variable defined in an
                          architecture's own conf/, not in sys/conf
  files.* outside conf/   sys/powerpc/conf/dpaa/files.dpaa
  unquoted compile-with   `compile-with ${ZSTD_C}', twenty-one lines of
                          sys/conf/files, no quotes
  kern.pre.mk             LINUXKPI_INCLUDES, previously a hand-written
                          copy of three flags
  cpu lines               `cpu BOOKE_E500', which DEFAULTS does not carry
  .if in a module         a block the build always takes, from a variable
                          set fifty lines above it
  -D at all               an -I makes a header findable; a -D decides
                          what is in it

They are load-bearing now, so they are checked. A reader that silently
stops reading gives back the same zero it gave before it was written.
"""
from __future__ import annotations
import os, sys, tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import includes  # noqa: E402

fails: list[str] = []


def check(name: str, got, want) -> None:
    if got == want:
        print(f"  ok   {name}")
    else:
        print(f"  FAIL {name}\n         got  {got!r}\n         want {want!r}")
        fails.append(name)


def check_that(name: str, cond: bool, why: str = "") -> None:
    if cond:
        print(f"  ok   {name}")
    else:
        print(f"  FAIL {name}{'  ' + why if why else ''}")
        fails.append(name)


print("== the language standard is the one the tree sets")
SRC = includes.SRC
check("kernel C", includes.lang_flags(SRC / "sys/kern/kern_malloc.c"),
      ["-xc", "-std=gnu17"])
check("userland C", includes.lang_flags(SRC / "lib/libc/gen/err.c"),
      ["-xc", "-std=gnu17"])
check("kernel C++", includes.lang_flags(SRC / "sys/kern/x.cpp"),
      ["-xc++", "-std=c++23", "-fno-exceptions", "-fno-rtti"])
check("userland C++", includes.lang_flags(SRC / "lib/libc/gen/x.cpp"),
      ["-xc++", "-std=gnu++17", "-fno-exceptions", "-fno-rtti"])
check("a .cpp forced to C for CBMC",
      includes.lang_flags(SRC / "sys/kern/x.cpp", "sys/kern/x.cpp",
                          as_c=True),
      ["-xc", "-std=gnu17"])
# The point of reading them rather than naming them: gnu17 must be what
# the makefiles say, not what this test says.
for mk, var, want in (("sys/conf/kern.mk", "CSTD", "gnu17"),
                      ("share/mk/bsd.sys.mk", "CSTD", "gnu17"),
                      ("sys/conf/kern.mk", "CXXSTD", "c++23")):
    text = (SRC / mk).read_text(errors="replace")
    check_that(f"{mk} still says {var}?= {want}",
               any(line.strip().startswith(var)
                   and line.split("=")[-1].strip() == want
                   for line in text.splitlines()
                   if line.strip().startswith(var + "?=")
                   or line.strip().startswith(var + "=")),
               "if the tree changed it, lang_flags follows and this "
               "expectation is what is stale")

# ...and it must be READ, not defaulted. Break the read and this is the
# only check that notices: STD_DEFAULT happens to agree with the tree
# today, so every other expectation above passes on the fallback.
_saved_sys, _saved_src = includes.SYS, includes.SRC
_fake = Path(tempfile.mkdtemp())
(_fake / "sys" / "conf").mkdir(parents=True)
(_fake / "sys" / "conf" / "kern.mk").write_text(
    "# a comment\n.if ${COMPILER_TYPE} == \"gcc\"\nCSTD?=\tc89\n.endif\n"
    "CSTD?=\tgnu99\nCXXSTD?=\tc++26\n")
includes.SYS, includes.SRC = _fake / "sys", _fake
includes._std.cache_clear()
check("CSTD comes from the file, not the fallback",
      includes._std("kernel-c"), "gnu99")
check("CXXSTD too", includes._std("kernel-c++"), "c++26")
check_that("and a conditional CSTD is not taken",
           includes._std("kernel-c") != "c89")
# ...and the fallback, for a tree where the makefile is absent. It is a
# last resort and it still has to say what the tree says, or a missing
# file silently changes the language.
includes.SYS = _fake / "nothing" / "sys"
includes.SRC = _fake / "nothing"
includes._std.cache_clear()
check("the fallback C standard", includes._std("kernel-c"), "gnu17")
check("the fallback userland C++", includes._std("user-c++"), "gnu++17")

includes.SYS, includes.SRC = _saved_sys, _saved_src
includes._std.cache_clear()

check("STD_LINE reads a plain assignment",
      includes.STD_LINE.match("CSTD=	gnu17").groups(), ("CSTD", "gnu17"))
check("STD_LINE reads a ?= assignment",
      includes.STD_LINE.match("CXXSTD?=	c++23").groups(),
      ("CXXSTD", "c++23"))
check_that("STD_LINE does not match a comment",
           includes.STD_LINE.match("# CSTD?= c89") is None)

print("\n== a compile-with, quoted or not")
tmp = Path(tempfile.mkdtemp())
srcs = {
    'a.c   optional x compile-with "${NORMAL_C} -I/usr/include"': "/usr/include",
    "b.c   optional x compile-with ${SOME_CMD}": None,
    "c.c   optional x standard": None,
}
for line, _ in srcs.items():
    m = includes.FILES_COMPILE.match(line)
    if line.startswith("a.c"):
        check_that("quoted compile-with matches", m is not None)
        check("...and yields the command", m and m.group("dq"),
              "${NORMAL_C} -I/usr/include")
    elif line.startswith("b.c"):
        check_that("bare compile-with matches", m is not None)
        check("...and yields the variable", m and m.group("bare"),
              "${SOME_CMD}")
    else:
        check_that("a line with no compile-with does not match", m is None)

print("\n== -I merge, -D first-wins")
# `-I.' is the kernel build directory. Resolved against the analyser's
# own cwd it is a real directory, so it passes is_dir() and quietly puts
# whatever happens to be there on the include path.
check("a relative -I is dropped",
      includes._kernel_dirs("-I. -I.. -I/usr/include", {}),
      ["-I/usr/include"])
check("a later -D of the same name is dropped",
      includes._dedupe_defines(["-D_KERNEL", "-DFOO=1", "-I/a", "-DFOO=2",
                                "-DBAR", "-I/a"]),
      ["-D_KERNEL", "-DFOO=1", "-I/a", "-DBAR", "-I/a"])
check("a function-like -D is keyed on its name too",
      includes._dedupe_defines(["-D__has_c_attribute(x)=0",
                                "-D__has_c_attribute(x)=1"]),
      ["-D__has_c_attribute(x)=0"])

print("\n== a module Makefile's .if, three-valued")
V = {"PM": "1", "OFF": "0", "NAME": "yes"}
for expr, want in (
        ("defined(PM)", True),
        ("defined(NOPE)", False),
        ("${PM} > 0", True),
        ("${OFF} > 0", False),
        ("defined(PM) && ${PM} > 0", True),
        ("defined(OFF) && ${OFF} > 0", False),
        ("${NAME} == \"yes\"", True),
        ("${NAME} != \"yes\"", False),
        # Undecidable: a variable nothing in this file set, and a
        # modifier expression. Both must be None, not False and not
        # True - None skips the block, which is the old behaviour.
        ("${UNSET} > 0", None),
        ("${KERN_OPTS:MDEV_PCI}", None),
        ("defined(PM) && ${UNSET} > 0", None),
        ("defined(NOPE) && ${UNSET} > 0", False),
        ("defined(PM) || ${UNSET} > 0", True),
):
    check(f".if {expr}", includes._mk_cond(expr, V), want)

print("\n== against the real tree")
kp = includes.kern_pre_vars()
check_that("kern.pre.mk gives LINUXKPI_INCLUDES",
           "LINUXKPI_INCLUDES" in kp and "linuxkpi" in kp["LINUXKPI_INCLUDES"])
check_that("kern.pre.mk gives ZSTD_C",
           "ZSTD_C" in kp and "contrib/zstd/lib/freebsd" in kp["ZSTD_C"])
check_that("kern.pre.mk's conditional assignments are NOT taken",
           "ZSTD_DECOMPRESS_BLOCK_FLAGS" not in kp,
           "it is inside .if ${COMPILER_TYPE} == \"gcc\"")

cpu = includes.files_cpu_index()
check("the DPAA ethernet's cpu set",
      cpu.get("sys/dev/dpaa/portals_common.c"), ("BOOKE", "BOOKE_E500"))
check_that("and it is an intersection, not a union",
           all(len(v) <= 2 for v in cpu.values()),
           "a file built by configs that disagree keeps only what they "
           "all declare")

by_file, by_src, by_dir = includes.kernel_flag_index()
ncsw = by_file.get("sys/contrib/ncsw/etc/error.c", ())
check_that("the DPAA compile-with expanded",
           any("contrib/ncsw/inc" in f for f in ncsw),
           "makeoptions DPAA_COMPILE_CMD in sys/powerpc/conf/dpaa")
check_that("...including the linuxkpi it opens with",
           any("linuxkpi" in f for f in ncsw),
           "${LINUXKPI_C} is ${NORMAL_C} ${LINUXKPI_INCLUDES}")
zstd = by_file.get("sys/contrib/zstd/lib/common/error_private.c", ())
check_that("the unquoted ${ZSTD_C} expanded",
           any("contrib/zstd/lib/freebsd" in f for f in zstd),
           "sys/conf/files:648, no quotes")

iwl = by_dir.get("sys/contrib/dev/iwlwifi", ())
for want in ("-DCONFIG_IWLMVM=1", "-DCONFIG_PM"):
    check_that(f"iwlwifi gets {want}", want in iwl,
               "the second is inside a .if the Makefile itself decides")
check_that("iwlwifi does not get -DCONFIG_IWLWIFI_DEBUGFS",
           "-DCONFIG_IWLWIFI_DEBUGFS" not in iwl,
           "IWLWIFI_DEBUGFS=0, so that block is dead")
check_that("no -I resolves to the analyser's own directory",
           all(not f.startswith("-I.") for f in iwl))

# bmake pulls ${.CURDIR}/../Makefile.inc in through bsd.init.mk, so a
# submodule Makefile can open `.PATH: ${COMMONDIR}' with COMMONDIR
# defined a directory up. mt76 does exactly that, and without the chain
# the .PATH resolved to nothing and all 135 of its files found none of
# their own headers.
# The architecture an `optional' clause implies. A HINT: arch_of() must
# NOT use it, because "no amd64 config declares this device" is not
# "amd64 cannot build this file".
opt = includes.files_opt_arch_index()
check("the Alpine HAL is ARM, both widths",
      opt.get("sys/contrib/alpine-hal/al_hal_iofic.c"),
      ("aarch64", "armv7"))
check_that("an option is matched case-insensitively, as config(8) does",
           "armv7" in (opt.get("sys/dev/gpio/gpioregulator.c") or ()),
           "`options FDT' in a kernel config and `optional ... fdt' in "
           "sys/conf/files are the same option; keying on the spelling "
           "left fdt resolving to riscv64 alone")
check_that("a disjunction is a union, not an intersection",
           opt.get("sys/dev/mii/e1000phy.c") is None,
           "`optional miibus | e1000phy' - miibus is declared by all six, "
           "and intersecting made a PHY driver look like ARM code")

# The -D a file's own `optional' clause implies. Note that the rule for
# the ARCHITECTURE above is the opposite of the rule for the OPTIONS
# here, and deliberately: for the architecture, any alternative that
# builds the file is an architecture that builds the file, so union;
# for the options, each alternative is a DIFFERENT configuration, and
# defining one alternative's options is asserting a configuration the
# file may never be built in.
# A module that builds objects OUTSIDE SRCS. Seven Makefiles in the tree
# have an OBJS line, and they are the ones with per-file instruction-set
# flags: sys/modules/blake2 reaches its ten SIMD implementations through
# SRCS_IN, OBJS and a `.for' rule, so no SRCS line names them.
_bs = includes.kernel_flag_index("amd64")[1]
_avx = _bs.get("sys/crypto/blake2/blake2b-avx.c") or ()
check_that("OBJS: a module's .for sources are named",
           bool(_avx),
           "sys/modules/blake2's ten SIMD files are `SRCS_IN' and "
           "`OBJS+= ${SRCS_IN:S/.c/.o/g}', which no reading of SRCS finds")
check_that("CFLAGS.<file>: with its own -mavx",
           "-mavx" in _avx,
           "CFLAGS.blake2b-avx.c is `-DSUFFIX=_avx -msse2 -mssse3 "
           "-msse4.1 -mavx' and the intrinsics do not compile without it")
check_that("...and the rule's own -D",
           "-D_MM_MALLOC_H_INCLUDED" in _avx,
           "the .for rule's command line carries it, and it is in no "
           "variable bmake will hand back")
_fl = includes.include_flags(includes.SRC /
                             "sys/crypto/blake2/blake2b-avx.c", "amd64")
check_that("...and ${CFLAGS:N-nostdinc} takes -nostdinc back off",
           "-nostdinc" not in _fl,
           "clang's own <mm_malloc.h> calls malloc() and free(), and the "
           "rule says it wants the standard headers")
check_that("a file NOT built by that rule keeps -nostdinc",
           "-nostdinc" in includes.include_flags(
               includes.SRC / "sys/kern/kern_exec.c", "amd64"),
           "the marker must not leak to the rest of the kernel")

# config(8)'s other two sources of macros. A DEVICE becomes DEV_<NAME>
# where sys/conf/options declares one, whether the device is named by a
# file's own `optional' clause or by the architecture's DEFAULTS.
check_that("a device's DEV_<name> is defined",
           "-DDEV_ACPI" in (includes.files_option_defines()
                            .get("sys/dev/gpio/pl061_acpi.c") or ()),
           "`optional pl061 gpio acpi' and sys/arm64/include/intr.h:45 "
           "guards ACPI_GPIO_XREF with `#ifdef DEV_ACPI' - the token is "
           "acpi, the macro is DEV_ACPI, and a lookup on the name alone "
           "misses it")
check_that("DEFAULTS' device lines too",
           "-DDEV_ISA" in includes.defaults_options("amd64"),
           "no amd64 config declares `device isa' - DEFAULTS:10 does, "
           "for all of them - and sys/x86/isa/atrtc.c is `standard' with "
           "its <isa/isavar.h> inside `#ifdef DEV_ISA' and four uses "
           "outside it")
check_that("...and only where options* declares the macro",
           not any(f == "-DDEV_MEM" for f in includes.defaults_options("amd64")),
           "`device mem' is in every DEFAULTS and sys/conf/options "
           "declares no DEV_MEM; inventing one would be this tool "
           "asserting a macro config(8) never writes")

# sys/conf/Makefile.<arch>, which the kernel build reads for every file.
# All six add -I$S/contrib/libfdt, and libfdt's own headers include
# <fdt.h> with ANGLE brackets, so the directory itself has to be on the
# path. Five translation units failed on exactly that.
_amk = includes.arch_makefile_flags("aarch64")
check_that("Makefile.<arch> puts libfdt on the path",
           any(f.endswith("/sys/contrib/libfdt") for f in _amk),
           "sys/contrib/libfdt/libfdt.h:55 is `#include <fdt.h>' and the "
           "file is sys/contrib/libfdt/fdt.h - nothing else in the build "
           "adds that directory")
check_that("...and arm64's device-tree headers with it",
           any(f.endswith("/sys/contrib/device-tree/include") for f in _amk),
           "Makefile.arm64:26 adds both")
check_that("amd64 does not get the device-tree headers",
           not any(f.endswith("/sys/contrib/device-tree/include")
                   for f in includes.arch_makefile_flags("amd64")),
           "Makefile.amd64:32 adds libfdt alone; a per-architecture file "
           "read once for all six would say otherwise")

_od = includes.files_option_defines()
check_that("a single alternative gives all its options",
           _od.get("sys/dev/random/fenestrasX/fx_brng.c") ==
           ("-DRANDOM_FENESTRASX",),
           "`optional !random_loadable random_fenestrasx' is one "
           "configuration and sys/sys/vdso.h:96 declares "
           "fxrng_push_seed_generation() inside `#ifdef "
           "RANDOM_FENESTRASX'")
check_that("a disjunction gives only what its alternatives share",
           _od.get("sys/netinet/tcp_ratelimit.c") == ("-DRATELIMIT",),
           "`optional ratelimit inet | ratelimit inet6' - both name "
           "ratelimit and only one names inet, so -DINET would be a "
           "guess at which of the two kernels this is")
check_that("...and nothing when they share nothing",
           _od.get("sys/xdr/xdr.c") is None,
           "`optional xdr | krpc | nfslockd | nfscl | nfsd | zfs'. "
           "Taking the first alternative cost six regressions in sweep "
           "11: XDR is a declared option nothing sets (options:489) and "
           "sys/rpc/xdr.h uses XDR as a TYPE, so `#define XDR 1' makes "
           "every declaration in that header a syntax error")
check("arch_of does not use the hint",
      includes.arch_of("sys/dev/nvmem/nvmem.c"), "amd64")
check_that("...even though the hint has an opinion",
           opt.get("sys/dev/nvmem/nvmem.c") is not None,
           "nvmem is declared only by the three FDT architectures and "
           "the file compiles clean as amd64")

# The index is per architecture, because a module Makefile says
# ${MACHINE_CPUARCH} and means it.
for a, want in (("amd64", "cddl/dev/dtrace/x86"),
                ("aarch64", "cddl/dev/dtrace/aarch64")):
    _bf, _bs, _bd = includes.kernel_flag_index(a)
    key = "sys/cddl/dev/dtrace/" + ("amd64" if a == "amd64" else "aarch64")
    check_that(f"{a}'s dtrace gets {want}",
               any(f.endswith(want) for f in _bd.get(key, ())),
               "ARCHDIR= ${MACHINE_CPUARCH}, and a .PATH built from it")
_bd64 = includes.kernel_flag_index("aarch64")[2]
check_that("...and arm64 does NOT get the x86 one",
           not any(f.endswith("cddl/dev/dtrace/x86")
                   for f in _bd64.get("sys/cddl/dev/dtrace/aarch64", ())),
           "the .if that adds it tests MACHINE_CPUARCH")

mt76 = by_dir.get("sys/contrib/dev/mediatek/mt76", ())
check_that("a submodule's .PATH from the parent Makefile.inc",
           any(f.endswith("/mediatek/mt76") for f in mt76),
           "COMMONDIR is in sys/modules/mt76/Makefile.inc")
check_that("...and the .inc's own CFLAGS with it",
           "-DCONFIG_ARCH_DMA_ADDR_T_64BIT" in mt76)
# A variable set inside a condition the tool cannot decide is taken; one
# inside a condition it can decide as FALSE is not. iwlwifi sets
# IWLWIFI_CONFIG_ACPI=1 inside `.if ${KERN_OPTS:MDEV_ACPI}' (undecidable,
# and true on every amd64 and arm64 config), and mt76's Makefile.inc sets
# MT76_ACPI?=0 at depth 0.
check_that("a variable from an undecidable block is taken",
           "-DCONFIG_ACPI" in iwl,
           "and it is what makes iwlwifi's fw/acpi.c compile")
check_that("...but a definitely-false block's is not",
           "-DCONFIG_ACPI" not in by_dir.get("sys/contrib/dev/mediatek/mt76",
                                             ()),
           "MT76_ACPI?= 0")
rtw = by_dir.get("sys/contrib/dev/rtw88", ())
check_that("rtw88 gets its DEBUGFS, which is 1", "-DCONFIG_RTW88_DEBUGFS" in rtw)
for off in ("-DCONFIG_RTW88_USB", "-DCONFIG_RTW88_LEDS", "-DCONFIG_PM"):
    check_that(f"...and not {off}, which is 0", off not in rtw)

mt7615 = by_dir.get("sys/contrib/dev/mediatek/mt76/mt7615", ())
check_that("a per-driver .PATH resolves ${MT76_DRIVER_NAME}",
           any(f.endswith("/mt76/mt7615") for f in mt7615),
           "DEVDIR is ${COMMONDIR}/${MT76_DRIVER_NAME}, and the name is "
           "set on line 1 of the submodule's own Makefile")

# The whole point, end to end: one real translation unit's flags.
rel = "sys/contrib/dev/iwlwifi/mvm/rxmq.c"
fl = includes.include_flags(SRC / rel, includes.arch_of(rel))
names = [f[2:].split("=")[0].split("(")[0] for f in fl if f.startswith("-D")]
check("no macro is defined twice on one command line",
      sorted(n for n in set(names) if names.count(n) > 1), [])

print("\n== the generated headers sys/conf/files describes")
# The recipes in sys/conf/files are read and RUN. What must hold: the
# ones whose variables all resolve produce a non-empty header, the ones
# needing a kernel configuration's answer produce nothing rather than
# something wrong, and the directory is on the include path of a kernel
# translation unit that needs one.
_gen = includes.gen_headers()
_made = set(os.listdir(_gen))
for _h in ("bhnd_nvram_map.h", "snd_fxdiv_gen.h", "feeder_eq_gen.h",
           "usbdevs.h", "miidevs.h"):
    check_that(f"{_h} is generated", _h in _made)
    if _h in _made:
        check_that(f"{_h} is not empty",
                   (Path(_gen) / _h).stat().st_size > 0,
                   "an empty header satisfies the #include and then "
                   "declares nothing, which reports as a wall of "
                   "unrelated errors")
for _h in ("font.h", "kbdmuxmap.h", "fdt_static_dtb.h"):
    check_that(f"{_h} is NOT invented", _h not in _made,
               "its recipe needs a kernel configuration's answer, so "
               "producing it here would be a guess")
_fl = includes.include_flags(SRC / "sys/dev/sound/pcm/feeder_eq.c", "amd64")
check_that("the generated-header directory is on a kernel file's -I",
           f"-I{_gen}" in _fl)

print()
if fails:
    print(f"{len(fails)} check(s) failed")
    sys.exit(1)
print("all checks passed")
