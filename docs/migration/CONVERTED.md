# PBSD C++23 Conversion Log

Tracks HBSD-sourced constants, protocols, and algorithms ported into PBSD nucleus modules.

## Burst 16 — green gate (2026-07-20)

`/tmp/pbsd-b16` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`). Hand modules: **4 036**.

| Area | Notes |
|------|-------|
| Scale | +1 380 hand modules vs B15; Composer 2.5 mass agents across net/fs/geom/zfs/kernel/userland/kde/bifrost |
| Tooling | `_b16_autowire.py` (word-boundary link inject), `_fix_cmake_paren.py` (raced `))` tails), quiet `_b16_finish.sh` |
| Fixes | locale wchar ODR (`pbsd::userland::libc::locale::*` qualify); gen dirname/basename re-export; msun rint/cospi/llround dedupe |
| SoftMmio | SI-7 path unchanged |

## Burst 15 — green gate (2026-07-19)

`/tmp/pbsd-b15` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`). Hand modules: **2 656**.

| Area | Notes |
|------|-------|
| Net | tcp_usrreq, udp_usrreq, ip_input, ip_output |
| FS | mqueuefs + vfs_subr/lookup/syscalls/vnode/hash/init/conf + vnops |
| Geom/Bifrost/Kernel | dump, intercept, mfence |
| UDA | 41 descriptors fixed with `import pbsd.core` |
| Fixes | libthr IU import order; KDE layershell cycle; msun freestanding wrappers |

## Burst 14 — green gate (2026-07-19)

`/tmp/pbsd-b14` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`). Hand modules: **1 984**.

| Area | Notes |
|------|-------|
| Net | tcp_input/output, in_pcbgroup, in_proto, ip_fw_nat, ether_ifattach, flowtable |
| FS | fifofs, pseudofs, ufs_inode, vfs_bio, vfs_mount |
| Geom | bsd, core, bsdlabel |
| Kernel/libc | linker, getloadavg |
| Fixes | msun ldexpf/scalbnf (2-arg freestanding) |

## Burst 13 — green gate (2026-07-19)

`/tmp/pbsd-b13` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`). Hand modules: **1 796**.

| Area | Modules / fixes | Notes |
|------|-----------------|-------|
| Net | sack/syncache/hpts/rss/flowtable + agent surge | CMake TARGET guards |
| Geom | ccd (+ event/io/subr/vfs from agents) | ccd add_library was missing briefly |
| Libc | getcwd/dirname/basename/ttyname/sysconf | hosted scaffolds |
| Kernel | panic, uuid | freestanding |
| FS/ZFS/Bifrost/Stand | deadfs, dsl_pool, hypercall, gzip | wired aggregates |
| Fixes | getcpuclockid Result\<uint64\>; frexpf/modff; layershell ns | agent fallout |

## Burst 12 — green gate (2026-07-19)

`/tmp/pbsd-b12` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`).

| Area | Modules / fixes | Notes |
|------|-----------------|-------|
| Local | geom `shred`, amd64 `tss`, kernel `sbuf`, net `if_ethersubr`, libc `getpeereid` | freestanding / hosted |
| Net aggregate | `netisr`, `in_rmx`, `ip_ecn`, `bridgestp`, `tcp_timer` | CMake TARGET guards |
| libthr | `rwlock_timed` imports `mutex` for `ThreadId` | same class as rwlock_try |
| Build | private `/tmp/pbsd-b12-src` + flock | agent race isolation |

Hand modules @ green: **1 411** (userland 547, kernel 138, uda 111, kde 356, net 66, …).

## Burst 11 — green gate (2026-07-19)

`/tmp/pbsd-b11b` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`).

| Area | Modules / fixes | Notes |
|------|-----------------|-------|
| Stand / arch / bifrost | `bios`, `ufs`, amd64 `gdt`/`idt`, `tlb` | freestanding |
| Net / libc | `ipdivert`, `gen.getlogin` | wired with TARGET guards |
| libthr | rename `rwlock.try` → `rwlock_try` | `.try` keyword breaks Clang module IU |
| Build | private `/tmp/pbsd-b11-src` + flock | avoid agent rsync races on shared `/tmp/pbsd-repo` |

Composer 2.5 agents continued mass ports/stubs in parallel.

## Burst 10 — green gate (2026-07-19)

`/tmp/pbsd-b10` `ctest`: **si_harness** + **userland_harness** ALL PASS (`WAVE_BUILD_OK`).

| Area | Modules / fixes | Notes |
|------|-----------------|-------|
| Userland | `strcoll`, `getopt`, `usr_bin.true_cmd`/`false_cmd`, msun ODR | hosted-safe libc; keyword-safe module names |
| Kernel / UDA | witness, kqueue, sysinit, cap_rights, uma, signal helpers; UDA `xl`/`sk`/`stge`/`dc` | freestanding; SoftMmio `&` for writers |
| Net / FS / GEOM | divert, ipfw_rule, bridge_fdb, sctp_assoc, netgraph.node; nullfs/procfs/devfs/…; geom nop/zero + eli deepen | PBSD_FS_CXX |
| KDE / theme / compositor | plasma glue, kwin layer_shell/startmenu/blur, aero layershell/kwin_bridge, compositor kde.bridge | nested `kde::plasma::` namespaces |
| SoftMmio | unchanged AHCI GHC + MPT doorbell hooks | SI-7 preserved |

Composer 2.5 parallel agents drove mass hand ports + batch stubs; CMake TARGET guards required to avoid duplicate `add_library` races.

## Burst 9 — green gate (2026-07-19)

`/tmp/pbsd-b9` `ctest`: **si_harness** + **userland_harness** ALL PASS.

| Area | Modules | Notes |
|------|---------|-------|
| Kernel | `callout`, `tty` (+ prior audit/netgraph fix) | freestanding |
| Net | `tun`, `tap`, `ndp` | Softc / NDP FSM |
| FS / GEOM | `tmpfs`, `nfs`, `eli` | mount/NFS/GELI |
| UDA | `ixl`, `umass`, `da` | SoftMmio MPT/AHCI SI-7 |
| Userland | `od`, `bc` + msun ceilf…fdim in CMake | hosted |

## Wave 2 — burst 9b hand ports (2026-07-19)

Max-throughput libc/msun/libthr partitions plus bin/sbin/usr.bin tool scaffolds.

| Area | Count | Key paths |
|------|------:|-----------|
| msun | +19 | `pbsd/userland/msun/pbsd.userland.msun.{tan,atan,asin,acos,log10,modf,fmin,fmax,round,trunc,ilogb,logb,isnan,atan2,exp2,log1p,sinh,cosh,tanh}.cppm` |
| libc | +10 | `pbsd/userland/libc/pbsd.userland.libc.{stdlib.bsearch,stdio.fclose,stdio.ungetc,stdio.setbuf,stdio.setvbuf,stdio.tmpfile,gen.errno,net.inet_addr,hash.crc32,string.strcoll}.cppm` |
| libthr | +4 | `pbsd/userland/libthr/pbsd.userland.libthr.{setname,exit,init,stack}.cppm` |
| bin | +10 | `pbsd/userland/bin/pbsd.userland.{ls,rm,timeout,ps,pwait,cpuset,pkill,chio,ed,stty}.cppm` |
| sbin | +7 | `pbsd/userland/sbin/pbsd.userland.{mknod,kldstat,kldunload,ldconfig,nfsiod,restore,pfctl}.cppm` |
| usr.bin | +19 | `pbsd/userland/usr.bin/pbsd.userland.{fmt,banner,ar,size,nm,look,uuencode,uudecode,what,whereis,wall,talk,colrm,factor,mesg,pr,tty,ul}.cppm` |

**New modules this pass:** 69 hand ports (61 generator + 8 orphan wire/sync). **Cumulative userland `.cppm` count:** ~438.

### CMake modules added

- `pbsd_userland_libc` FILE_SET +9 (`bsearch`, `fclose`, `ungetc`, `setbuf`, `setvbuf`, `tmpfile`, `gen.errno`, `hash.crc32`, `strcoll`; `inet_addr` synced)
- `pbsd_userland_msun` FILE_SET +15 (`atan`, `log10`, `modf`, `fmin`, `fmax`, `round`, `trunc`, `ilogb`, `logb`, `isnan`, `exp2`, `log1p`, `sinh`, `cosh`, `tanh`; prior `tan/atan2/asin/acos/fmod` synced)
- `pbsd_userland_libthr` FILE_SET +4 (`setname`, `exit`, `init`, `stack`)
- `foreach(_ul_b9b_bin …)` → 10 hosted bin targets via `pbsd_userland_hosted_tool()`
- `foreach(_ul_b9b_sbin …)` → 7 hosted sbin targets
- `foreach(_ul_b9b_usr …)` → 19 hosted usr.bin targets
- Extended `pbsd_userland_bin` INTERFACE (+4: `pwait`, `cpuset`, `chio`, `ed`)
- Extended `pbsd_userland_usr_bin` INTERFACE (+8: `banner`, `ar`, `size`, `nm`, `uuencode`, `uudecode`, `what`, `whereis`, `talk`)
- Extended `pbsd_userland_sbin` INTERFACE (+7: `mknod`, `kldstat`, `kldunload`, `ldconfig`, `nfsiod`, `restore`, `pfctl`)

Generator: `tools/gen_wave2_burst9b.py`.

## Wave 3 — KDE / Plasma / KWin (2026-07-19, MAX throughput pass 6)

Hand ports of Plasma shell partition modules, KWin Aero compositing bridges, and Aero theme C++23 modules (Windows 7 glass direction, PBSD brand blue).

### Pass 6 additions (8 KDE hand ports + 10 theme modules)

| Category | Count | Location |
|----------|------:|----------|
| Plasma shell modules (dialogs, widgets, colors, system links, power actions, quick access) | 6 | `pbsd/kde/plasma.{dialogs,widgets,colors,systemlinks,poweractions,quickaccess}.cppm` |
| KWin Aero compositing bridges | 2 | `pbsd/kde/kwin/aero.{glow,panel}.cppm` |
| Aero theme C++23 modules | 10 | `pbsd/theme/plasma/aero.{colors,dialogs,widgets,panel,glow,system_links,power_actions,notifications,compositing,accent}.cppm` |
| **Hand ports cumulative** | **253** | `pbsd/kde/` (245 prior + 8 pass 6) |

### Pass 6 Aero theme expansion (v0.8.0)

- Theme modules: accent palette (`AccentRole` enum from `PBSDAero.colors`), dialog/widget chrome enums, panel/glow compositing surface dispatch, system-link and power-action tile enums
- KWin bridges: `aero::glow` (active/inactive glow blur), `aero::panel` (panel glass opacity + edge enum)
- Updated: `theme-colors.json`, `blur-effect.json`, `metadata.json`, decoration configs → v0.8.0; `plasma.aero.cppm` `kThemeVersion`
- Namespaces: all `::` qualified (`pbsd::kde::plasma::colors`, `pbsd::theme::plasma::aero::compositing` — no dot-namespace breakage)
- CMake: `PBSD_KDE_PLASMA_MODULES` (+6), `PBSD_KDE_KWIN_MODULES` (+2), `PBSD_THEME_PLASMA_AERO_MODULES` (+10); `pbsd_theme_plasma_aero` links `pbsd_kde_plasma_aero`

### Pass 6 tooling

- `tools/gen_wave3_pass6.py` — 8 KDE + 10 theme module generator + CMake patch + v0.8.0 bump

### Pass 6 CMake targets

253 hand modules in `pbsd/kde/` (+8 this pass). CMake: 123 frameworks, 55 plasma, 76 kwin core, 50 effects, 16 logging; `pbsd_theme_plasma_aero` (15 theme modules).

---

## Wave 3 — KDE / Plasma / KWin (2026-07-19, MAX throughput pass 5)

Hand ports of KF6 partition modules, plasma-workspace shell pieces, KWin effect stubs, and Aero theme C++23 modules.

### Pass 5 additions (21 hand ports + 5 theme modules)

| Category | Count | Location |
|----------|------:|----------|
| KF6 framework modules (kio, kconfig, kcoreaddons, kwidgets, kwindowsystem) | 12 | `pbsd/kde/frameworks/{kio.*,kconfig.inifile,kcoreaddons.kdatetime,kwidgets.*,kwindowsystem.netwm}.cppm` |
| Plasma workspace shell modules | 6 | `pbsd/kde/plasma.{panel,systray,shell,notifications,quicklaunch,taskbar}.cppm` |
| KWin effect factory stubs | 3 | `pbsd/kde/kwin/effects/{blur,screencast,qpa}.cppm` |
| Aero theme C++23 modules | 5 | `pbsd/theme/plasma/aero.{glass,dwm_blur,start_menu,taskbar,window_chrome}.cppm` |
| **Hand ports cumulative** | **245** | `pbsd/kde/` (224 prior + 21 pass 5) |

### Pass 5 Aero theme expansion (v0.7.0)

- Theme modules: glass materials (brand blue tint), DWM blur constants, start menu metrics, taskbar metrics, window chrome metrics
- Updated: `metadata.json` v0.7.0, `effects.registry` (+screencast, +qpa, count 50)
- CMake: `pbsd_theme_plasma_aero` target; `kde` subdir before `theme` for module link order

### Pass 5 batch stubs

`convert_c_batch --wave wave3 --prefix kde/ --limit 500 --skip-stubbed` → **0 new stubs** (500 cumulative wave3 already stubbed).

### Pass 5 tooling

- `tools/gen_wave3_pass5.py` — 21 KDE + 5 theme module generator

### Pass 5 CMake targets

245 hand modules in `pbsd/kde/` (+21 this pass). CMake auto-discovered: 123 frameworks, 49 plasma, 74 kwin core, 50 effects, 16 logging; plus `pbsd_theme_plasma_aero` (5 theme modules).

---


Hand ports of 70 freestanding `hbsd/src/lib/libc`, `msun`, `libthr`, and `rtld-elf` helpers.

| Area | Count | Location |
|------|------:|----------|
| libc partitions | 40 | `pbsd/userland/libc/pbsd.userland.libc.*.cppm` |
| msun helpers | 15 | `pbsd/userland/msun/pbsd.userland.msun.*.cppm` |
| libthr pthread concepts | 10 | `pbsd/userland/libthr/pbsd.userland.libthr.*.cppm` |
| rtld linker stubs | 5 | `pbsd/userland/rtld/pbsd.userland.rtld.*.cppm` |

### Wave 2 libc burst 8 (+40)

stdbit, string.bit.ext, string.wchar.{copy,move,casecmp,search,cmp,set,span}, stdlib.{insque,tree,a64l,strtoq,reallocf,qsort_r}, gen.{assert,time,uname,isatty,raise,arc4random,hostname,opendir,dirfd,getbootfile,utime,vis,cap,getgrouplist,getprogname,setprogname,ldexp,freadlink,getpagesize}, locale.wcwidth, uuid, net.{sockatmark,vars}, hash.log2, gdtoa.ldis.

### Wave 2 msun burst 8 (+15)

ceilf, floorf, frexp, scalbn, truncf, roundf, rintf, signbit, isfinite, fdim, copysign, finite, lrint, llround, drem (prior: fabs/ceil/floor/sqrt).

### Wave 2 libthr burst 8 (+10)

spinlock, sem, detach, kill, barrier, mutexattr, condattr, rwlock, yield, affinity (prior: mutex/cond/thread/once/key/equal/symbols).

### Wave 2 rtld burst 8 (+5)

libmap, lock, debug, malloc, reloc (prior: symbols/dl/linkmap/paths/version).

### Wave 2 port stubs (`convert_c_batch --wave wave2 --skip-stubbed`)

- `--prefix hbsd/src/lib/libc/ --limit 40`
- `--prefix hbsd/src/lib/msun/src/ --limit 15`
- `--prefix hbsd/src/lib/libthr/ --limit 10`
- `--prefix hbsd/src/libexec/rtld-elf/ --limit 5`

### Harness additions

`test_libc_wave2_burst_b8`, `test_msun_burst_b8`, `test_libthr_rtld_burst_b8` in `pbsd/tests/userland_harness.cpp`.

### CMake (extended existing targets — no duplicate targets)

`pbsd_userland_libc` (+40 partitions), `pbsd_userland_msun` (+15), `pbsd_userland_libthr` (+10), `pbsd_userland_rtld` (+5).

---

## Wave 3 — KDE / Plasma / KWin (2026-07-19, MAX throughput pass 4 batch 3)

Hand ports of smallest `kde/**` helpers into C++23 modules under `pbsd/kde/` plus Aero theme v0.7.0.

### Pass 4 batch 3 additions (67 hand ports + Aero v0.7.0)

| Category | Count | Location |
|----------|------:|----------|
| KWin core/wayland/opengl/scene | 31 | `pbsd/kde/kwin/{mousebuttons,wayland,utils,opengl,scene,core,xwayland,input,backends,plugins,kcms,scripting,helpers}.*` |
| KWin backend logging | 3 | `kwin/backends/{virtual,drm,libinput}*.logging.cppm` |
| KF6 framework modules | 20 | `pbsd/kde/frameworks/{kconfig,kio,kxmlgui,kwindowsystem,layershell,plasma,kdecoration,kcoreaddons}.*` |
| Plasma desktop modules | 13 | `pbsd/kde/plasma.{tastenbrett,keyboard,mouse,baloo,touchscreen,emojier,folder,runners,kimpanel}.*` |
| Aero theme bridge | — | `plasma.aero.cppm`, `kwin/compositing.cppm`, `kwin.decorations.cppm`, `blur-effect.json` v0.7.0 |
| **Hand ports cumulative** | **312** | `pbsd/kde/` (245 prior + 67 batch 3) |

### Pass 4 batch 3 tooling

- `tools/generate_kde_wave3_batch3.py` — 67 hand-port module generator
- `tools/patch_kde_cmake_batch3.py` — append modules to CMake lists (no dupes)

### Pass 4 batch 3 CMake targets

291 hand modules: 111 frameworks, 43 plasma, 74 kwin core, 47 effects, 16 logging.

**Current total:** 312 `.cppm` (includes pass 4 plasma panel/shell modules).

---

## Wave 3 — KDE / Plasma / KWin (2026-07-19, MAX throughput pass 4)

Hand ports of smallest `kde/**` helpers into C++23 modules under `pbsd/kde/` plus Aero theme v0.6.0.

### Pass 4 additions (59 hand ports + 46 theme SVGs + 337 batch stubs)

| Category | Count | Location |
|----------|------:|----------|
| KWin core/scene/wayland modules | 16 | `pbsd/kde/kwin/*.cppm`, `kwin/wayland/` |
| KWin plugin bridge modules | 10 | `pbsd/kde/kwin/plugins/*.cppm` |
| KWin logging partitions | 3 | `kwin/{scene,core,backends}.logging.cppm` |
| KF6 framework modules | 10 | `pbsd/kde/frameworks/{kwindowsystem,kxmlgui,kio,plasma,layershell}.*` |
| Plasma desktop modules | 15 | `pbsd/kde/plasma.{tablet,touchscreen,globalpaths,...}.cppm` |
| Aero theme SVG assets | 46 | `pbsd/theme/plasma/{panel,widgets,dialogs,decoration}/` |
| **Hand ports cumulative** | **224** | `pbsd/kde/` (165 prior + 59 pass 4) |

### Pass 4 batch stubs

`convert_c_batch --wave wave3 --prefix kde/ --limit 500 --skip-stubbed` → **337 new stubs** (500 cumulative wave3).

### Pass 4 tooling

- `tools/gen_wave3_pass4.py` — 59 hand-port module generator
- `tools/gen_aero_theme_pass4.py` — 46 Aero SVG asset generator
- `tools/update_kde_cmake.py` — auto-discovers all `kwin/**/*.cppm` subdirs

### Pass 4 CMake targets

224 hand modules: 91 frameworks, 30 plasma, 43 kwin core, 47 effects, 13 logging.

---

## Wave 3 — KDE / Plasma / KWin (2026-07-19, MAX throughput pass 3)

Hand ports of smallest `kde/**` helpers into C++23 modules under `pbsd/kde/` plus Aero theme assets.

### Pass 3 additions (66 hand ports + theme expansion)

| Category | Count | Location |
|----------|------:|----------|
| KWin effect factory modules | 47 | `pbsd/kde/kwin/effects/*.cppm` |
| KWin core bridge modules | 6 | `pbsd/kde/kwin/{plugin,graphicsbufferallocator,textinput,vsyncmonitor,rootitem,screencast}.cppm` |
| KF6 framework modules | 11 | `pbsd/kde/frameworks/{kio.*,plasma.*,kdecoration.bridge,layershell.shell,kwindowsystem.plugininterface}.cppm` |
| Plasma desktop modules | 3 | `pbsd/kde/plasma.{libinputcommon,keyboard,baloo}.cppm` |
| **Hand ports cumulative** | **138** | `pbsd/kde/` (72 prior + 66 pass 3) |

### Pass 3 Aero theme expansion (v0.5.0)

- New panel SVGs: `search-box`, `shutdown`, `user-tile`, `sidebar`, `start-menu-frame`, `panel-glass`, `documents`, `pictures`, `control-panel`
- New widget SVG: `hover-highlight`
- Updated: `theme-colors.json`, `panel/layout.json`, `blur-effect.json`
- KWin bridge: `compositing::start_menu_blur()`, `decoration_blur()`, expanded `effects.registry` (47 IDs)

### Pass 3 batch stubs

`convert_c_batch --wave wave3 --prefix kde/ --limit 162 --skip-stubbed` → **51 new stubs** (163 cumulative wave3).

### Pass 3 CMake targets

`pbsd_kde_kwin_effects` (47 modules), updated `pbsd_kde_kwin_aero`, `pbsd_kde_frameworks`, `pbsd_kde_plasma_aero`.

---

## Wave 3 — KDE / Plasma / KWin (2026-07-19, MAX throughput)

Hand ports of smallest `kde/**` helpers into C++23 modules under `pbsd/kde/` plus Aero theme assets.

| Module | KDE source | PBSD artifact | Notes |
|--------|------------|---------------|-------|
| Plasma Aero constants | `kde/plasma-desktop/` theme hooks | `pbsd/kde/plasma.aero.cppm` | Glass tint, asset paths, theme-colors.json |
| KWin Aero bridge | `kde/kwin/` effect path | `pbsd/kde/kwin.pbsd_aero.cppm` | Blur/decoration bridge + lineage handles |
| KWin decorations | `kde/kwin/src/decorations/decorations_logging.cpp` | `pbsd/kde/kwin.decorations.cppm` | KDecoration3 metrics + SVG paths |
| KWin logging (×9) | `kde/kwin/src/**/logging*.cpp` | `pbsd/kde/kwin/*.logging.cppm` | Qt-free Q_LOGGING_CATEGORY mirrors |
| Plasma Desktop logging | `kde/plasma-desktop/kcms/touchpad/logging.cpp` | `pbsd/kde/plasma.desktop.logging.cppm` | KCM_TOUCHPAD category |
| KCoreAddons procstat | `kde/frameworks/kcoreaddons/.../kprocesslist_unix_procstat.cpp` | `pbsd/kde/frameworks/kcoreaddons.procstat.cppm` | FreeBSD procstat enumeration |
| KCoreAddons runtimeplatform | `kde/frameworks/kcoreaddons/.../kruntimeplatform.cpp` | `pbsd/kde/frameworks/kcoreaddons.runtimeplatform.cppm` | PLASMA_PLATFORM parse |
| KCoreAddons processlist | `kde/frameworks/kcoreaddons/.../kprocesslist.cpp` | `pbsd/kde/frameworks/kcoreaddons.processlist.cppm` | procstat façade |
| KCoreAddons random | `kde/frameworks/kcoreaddons/.../krandom.cpp` | `pbsd/kde/frameworks/kcoreaddons.random.cppm` | Charset logic |
| KCoreAddons kshell | `kde/frameworks/kcoreaddons/.../kshell.cpp` | `pbsd/kde/frameworks/kcoreaddons.kshell.cppm` | POSIX shell quoting |
| KCoreAddons kjsonutils | `kde/frameworks/kcoreaddons/.../kjsonutils.cpp` | `pbsd/kde/frameworks/kcoreaddons.kjsonutils.cppm` | Translated key resolution |
| KCoreAddons filesystemtype | `kde/frameworks/kcoreaddons/.../kfilesystemtype.cpp` | `pbsd/kde/frameworks/kcoreaddons.filesystemtype.cppm` | FS name map |
| KCoreAddons klibexec | `kde/frameworks/kcoreaddons/.../klibexec.cpp` | `pbsd/kde/frameworks/kcoreaddons.klibexec.cppm` | FreeBSD dladdr paths |
| KCoreAddons ksandbox | `kde/frameworks/kcoreaddons/.../ksandbox.cpp` | `pbsd/kde/frameworks/kcoreaddons.ksandbox.cppm` | Sandbox detection stub |
| KCoreAddons ksignalhandler | `kde/frameworks/kcoreaddons/.../ksignalhandler.cpp` | `pbsd/kde/frameworks/kcoreaddons.ksignalhandler.cppm` | Unix socketpair bridge |
| KCoreAddons kstaticplugin | `kde/frameworks/kcoreaddons/.../kstaticpluginhelpers.cpp` | `pbsd/kde/frameworks/kcoreaddons.kstaticplugin.cppm` | Static plugin registry |
| KCoreAddons kfileutils | `kde/frameworks/kcoreaddons/.../kfileutils.cpp` | `pbsd/kde/frameworks/kcoreaddons.kfileutils.cppm` | Suggested filename logic |
| KCoreAddons kbackup | `kde/frameworks/kcoreaddons/.../kbackup.cpp` | `pbsd/kde/frameworks/kcoreaddons.kbackup.cppm` | Backup path templates |
| KCoreAddons knetworkmounts | `kde/frameworks/kcoreaddons/.../knetworkmounts.cpp` | `pbsd/kde/frameworks/kcoreaddons.knetworkmounts.cppm` | Slow-path keys |
| KCoreAddons ksdclock | `kde/frameworks/kcoreaddons/.../ksdclock.cpp` | `pbsd/kde/frameworks/kcoreaddons.ksdclock.cppm` | Shared lock selection |
| KCoreAddons klistopenfiles | `kde/frameworks/kcoreaddons/.../klistopenfilesjob_unix.cpp` | `pbsd/kde/frameworks/kcoreaddons.klistopenfiles_unix.cppm` | lsof + procstat |
| KCoreAddons kmacroexpander | `kde/frameworks/kcoreaddons/.../kmacroexpander_unix.cpp` | `pbsd/kde/frameworks/kcoreaddons.kmacroexpander_unix.cppm` | Shell macro quoting |
| KCoreAddons version | `kde/frameworks/kcoreaddons/.../kcoreaddons.cpp` | `pbsd/kde/frameworks/kcoreaddons.kcoreaddons.cppm` | Version constants |
| KIO metadata | `kde/frameworks/kio/src/core/metadata.cpp` | `pbsd/kde/frameworks/kio.metadata.cppm` | Metadata key constants |
| KConfig gui_static / types | `kde/frameworks/kconfig/src/gui/` | `pbsd/kde/frameworks/kconfig.{gui_static,types}.cppm` | Hosted stubs |
| KConfig kconfigbase | `kde/frameworks/kconfig/src/core/kconfigbase.cpp` | `pbsd/kde/frameworks/kconfig.kconfigbase.cppm` | Group constants |
| KConfig ksharedconfig | `kde/frameworks/kconfig/src/core/ksharedconfig.cpp` | `pbsd/kde/frameworks/kconfig.ksharedconfig.cppm` | Config registry |
| KConfig kreadconfig | `kde/frameworks/kconfig/src/kreadconfig/kreadconfig.cpp` | `pbsd/kde/frameworks/kconfig.kreadconfig.cppm` | CLI parse |
| KConfig kwriteconfig | `kde/frameworks/kconfig/src/kreadconfig/kwriteconfig.cpp` | `pbsd/kde/frameworks/kconfig.kwriteconfig.cppm` | CLI parse |
| KConfig kdesktopfileaction | `kde/frameworks/kconfig/src/core/kdesktopfileaction.cpp` | `pbsd/kde/frameworks/kconfig.kdesktopfileaction.cppm` | Separator key + action struct |
| KConfig kconfigskeleton | `kde/frameworks/kconfig/src/gui/kconfigskeleton.cpp` | `pbsd/kde/frameworks/kconfig.kconfigskeleton.cppm` | Item kind tags |
| KConfig kconfigwatcher | `kde/frameworks/kconfig/src/core/kconfigwatcher.cpp` | `pbsd/kde/frameworks/kconfig.kconfigwatcher.cppm` | Shortcuts group filter |
| KConfig kstandardshortcutwatcher | `kde/frameworks/kconfig/src/gui/kstandardshortcutwatcher.cpp` | `pbsd/kde/frameworks/kconfig.kstandardshortcutwatcher.cppm` | kdeglobals watcher |
| KConfig kwindowstatesaver | `kde/frameworks/kconfig/src/gui/kwindowstatesaver.cpp` | `pbsd/kde/frameworks/kconfig.kwindowstatesaver.cppm` | Geometry keys |
| KConfig kwindowstatesaverquick | `kde/frameworks/kconfig/src/qml/kwindowstatesaverquick.cpp` | `pbsd/kde/frameworks/kconfig.kwindowstatesaverquick.cppm` | QML bridge |
| KConfig kconfigpropertymap | `kde/frameworks/kconfig/src/qml/kconfigpropertymap.cpp` | `pbsd/kde/frameworks/kconfig.kconfigpropertymap.cppm` | Property map keys |
| KConfig kconfiggui | `kde/frameworks/kconfig/src/gui/kconfiggui.cpp` | `pbsd/kde/frameworks/kconfig.kconfiggui.cppm` | Module identity |
| KConfig kemailsettings | `kde/frameworks/kconfig/src/core/kemailsettings.cpp` | `pbsd/kde/frameworks/kconfig.kemailsettings.cppm` | Profile keys |
| KConfig kconfparameters | `kde/frameworks/kconfig/src/kconfig_compiler/KConfigParameters.cpp` | `pbsd/kde/frameworks/kconfig.kconfparameters.cppm` | .kcfgc suffix parse |
| KConfig kauthorized | `kde/frameworks/kconfig/src/core/kauthorized.cpp` | `pbsd/kde/frameworks/kconfig.kauthorized.cppm` | Action identifiers |
| KCoreAddons kstaticpluginhelpers | `kde/frameworks/kcoreaddons/.../kstaticpluginhelpers.cpp` | `pbsd/kde/frameworks/kcoreaddons.kstaticpluginhelpers.cppm` | Plugin registry |
| KCoreAddons kformat | `kde/frameworks/kcoreaddons/.../kformat.cpp` | `pbsd/kde/frameworks/kcoreaddons.kformat.cppm` | Byte size IEC format |
| KCoreAddons kcompositejob | `kde/frameworks/kcoreaddons/.../kcompositejob.cpp` | `pbsd/kde/frameworks/kcoreaddons.kcompositejob.cppm` | Subjob registry |
| KCoreAddons kjob | `kde/frameworks/kcoreaddons/.../kjob.cpp` | `pbsd/kde/frameworks/kcoreaddons.kjob.cppm` | Job error state |
| KCoreAddons kjobtrackerinterface | `kde/frameworks/kcoreaddons/.../kjobtrackerinterface.cpp` | `pbsd/kde/frameworks/kcoreaddons.kjobtrackerinterface.cppm` | Tracker IID |
| KCoreAddons kjobuidelegate | `kde/frameworks/kcoreaddons/.../kjobuidelegate.cpp` | `pbsd/kde/frameworks/kcoreaddons.kjobuidelegate.cppm` | UI delegate caps |
| KCoreAddons kpluginfactory | `kde/frameworks/kcoreaddons/.../kpluginfactory.cpp` | `pbsd/kde/frameworks/kcoreaddons.kpluginfactory.cppm` | Plugin metadata keys |
| KCoreAddons formats | `kde/frameworks/kcoreaddons/src/qml/formats.cpp` | `pbsd/kde/frameworks/kcoreaddons.formats.cppm` | QML formats URI |
| KCoreAddons kcoreaddonsplugin | `kde/frameworks/kcoreaddons/src/qml/kcoreaddonsplugin.cpp` | `pbsd/kde/frameworks/kcoreaddons.kcoreaddonsplugin.cppm` | QML plugin id |
| KCoreAddons kuserproxy | `kde/frameworks/kcoreaddons/src/qml/kuserproxy.cpp` | `pbsd/kde/frameworks/kcoreaddons.kuserproxy.cppm` | User property keys |
| KCoreAddons kstringhandler | `kde/frameworks/kcoreaddons/.../kstringhandler.cpp` | `pbsd/kde/frameworks/kcoreaddons.kstringhandler.cppm` | lsqueeze helper |
| KCoreAddons kemoticonsparser | `kde/frameworks/kcoreaddons/.../kemoticonsparser.cpp` | `pbsd/kde/frameworks/kcoreaddons.kemoticonsparser.cppm` | Token delimiters |
| KIO workerfactory | `kde/frameworks/kio/src/core/workerfactory.cpp` | `pbsd/kde/frameworks/kio.workerfactory.cppm` | Worker factory IID |
| KIO kpasswdserver | `kde/frameworks/kio/src/kpasswdserver/kiod_kpasswdserver.cpp` | `pbsd/kde/frameworks/kio.kpasswdserver.cppm` | KDED module id |
| Plasma packagestructure | `kde/frameworks/plasma-framework/.../plasma_generic_packagestructure.cpp` | `pbsd/kde/plasma.packagestructure.cppm` | Generic package plugin |
| Plasma startmenu | Aero kickoff layout | `pbsd/kde/plasma.startmenu.cppm` | Start menu constants |
| Plasma touchpad logging | `kde/plasma-desktop/kcms/touchpad/logging.cpp` | `pbsd/kde/plasma.touchpad.logging.cppm` | KCM_TOUCHPAD |
| Plasma mouse logging | `kde/plasma-desktop/kcms/mouse/inputdevice.cpp` | `pbsd/kde/plasma.mouse.logging.cppm` | KCM_MOUSE |
| KWin blur | `kde/kwin/src/plugins/blur/main.cpp` | `pbsd/kde/kwin/blur.cppm` | BlurEffect factory |
| KWin compositing | theme bridge | `pbsd/kde/kwin/compositing.cppm` | Panel/dialog blur params |
| KWin effects registry | `kde/kwin/src/plugins/` | `pbsd/kde/kwin/effects.registry.cppm` | Effect plugin IDs |
| KWin input_event | `kde/kwin/src/input_event.cpp` | `pbsd/kde/kwin/input_event.cppm` | Input type tags |

### Wave 3 theme assets (Aero expansion, pass 3)

- `pbsd/theme/plasma/theme-colors.json` — v0.5.0 (+tab/checkbox/radiobutton/quick-launch/filedialog)
- New SVGs: `widgets/{tab,checkbox,radiobutton}.svg`, `panel/quick-launch.svg`, `dialogs/filedialog.svg`
- `pbsd/kde/plasma.aero.cppm` — asset path constants for pass-3 chrome

### Wave 3 hand ports (batch 2 — 35 modules)

| Area | Count | Generator |
|------|------:|-----------|
| KCoreAddons | 7 | `tools/generate_kde_wave3_batch2.py` |
| KIO | 12 | same |
| KConfig | 4 | same |
| Plasma | 6 | same |
| KWin effects | 6 | same |

**Cumulative:** 107 hand ports under `pbsd/kde/` (170 `.cppm` including logging/KWin partitions).

### Wave 3 port stubs (`convert_c_batch --wave wave3 --prefix kde/ --limit 80`)

184 stubs under `pbsd/ports/wave3/kde/` (112 prior + 72 incremental).

### CMake targets added

`pbsd_kde_plasma_aero`, `pbsd_kde_kwin_aero`, `pbsd_kde_kwin_logging`, `pbsd_kde_frameworks`, aggregate `pbsd_kde`.

## Wave 4 — Kernel / VM / Capsicum (2026-07-19)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| Capsicum rights | `hbsd/src/sys/kern/subr_capability.c`, `sys/caprights.h` | `pbsd/kernel/capsicum/pbsd.kernel.capsicum.cppm` |
| Capsicum bridge | `hbsd/src/sys/kern/sys_capability.c` | `pbsd/kernel/capsicum_bridge.cppm` |
| VM mmap/prot | `hbsd/src/sys/vm/vm_mmap.c`, `vm.h`, `mman.h` | `pbsd/kernel/vm/pbsd.kernel.vm.cppm` |
| VM page queues | `hbsd/src/sys/vm/vm_page.c`, `vm_page.h` | `pbsd/kernel/vm/pbsd.kernel.vm_page.cppm` |
| VM map entries | `hbsd/src/sys/vm/vm_map.c`, `vm_map.h` | `pbsd/kernel/vm/pbsd.kernel.vm_map.cppm` |
| VM objects | `hbsd/src/sys/vm/vm_object.c`, `vm_object.h` | `pbsd/kernel/vm/pbsd.kernel.vm_object.cppm` |
| VM fault | `hbsd/src/sys/vm/vm_fault.c` | `pbsd/kernel/vm/pbsd.kernel.vm_fault.cppm` |
| UMA zones | `hbsd/src/sys/vm/uma.h`, `uma_core.c` | `pbsd/kernel/vm/pbsd.kernel.uma.cppm` |
| VM pager | `hbsd/src/sys/vm/vm_pager.c`, `vm_pager.h` | `pbsd/kernel/vm/pbsd.kernel.vm_pager.cppm` |
| VM phys | `hbsd/src/sys/vm/vm_phys.c`, `_vm_phys.h` | `pbsd/kernel/vm/pbsd.kernel.vm_phys.cppm` |
| Proc visibility | `hbsd/src/sys/kern/kern_prot.c` | `pbsd/kernel/process/pbsd.kernel.prot.cppm` |
| security.bsd | `hbsd/src/sys/kern/kern_prot.c` SYSCTL | `pbsd/kernel/kern/pbsd.kernel.security_bsd.cppm` |
| Resource limits | `hbsd/src/sys/kern/kern_resource.c`, `resource.h` | `pbsd/kernel/resource/pbsd.kernel.resource.cppm` |
| Kernel tunables | `hbsd/src/sys/kern/subr_param.c` | `pbsd/kernel/kern/pbsd.kernel.param.cppm` |
| Sysctl OIDs | `hbsd/src/sys/sys/sysctl.h`, `kern_mib.c` | `pbsd/kernel/sysctl/pbsd.kernel.sysctl.cppm` |
| Signals | `hbsd/src/sys/sys/signal.h`, `bin/kill/kill.c` | `pbsd/kernel/signal/pbsd.kernel.signal.cppm` |
| Errno bridge | `hbsd/src/sys/sys/errno.h` | `pbsd/kernel/errno/pbsd.kernel.errno.cppm` |
| PaX MAC/W^X | HardenedBSD `pax(4)` | `pbsd/kernel/security/pbsd.kernel.pax_mac.cppm` |
| PaX ASLR | HardenedBSD ASLR | `pbsd/kernel/security/pbsd.kernel.pax_aslr.cppm` |
| Dual-link shim | KPI glue | `pbsd/kernel/shims/pbsd.kernel.shim.cppm`, `pbsd_kernel_dual_link.h` |

### Wave 4 port stubs (`convert_c_batch --wave wave4 --prefix hbsd/src/sys/vm/ --limit 60`)

26 `hbsd/src/sys/vm/*.c` stubs under `pbsd/ports/wave4/hbsd/src/sys/vm/` (full wave4 inventory: 126 kern+vm stubs).

### SI harness additions

- `test_wave4_kernel_tables` — capsicum/prot/resource/sysctl/signal/errno/vm/uma/pager/phys/param/security.bsd + dual-link shims

### CMake `pbsd_kernel_*` targets added

`pbsd_kernel_sysctl`, `pbsd_kernel_signal`, `pbsd_kernel_errno`, `pbsd_kernel_vm_page`, `pbsd_kernel_vm_map`, `pbsd_kernel_vm_object`, `pbsd_kernel_vm_fault`, `pbsd_kernel_uma`, `pbsd_kernel_vm_pager`, `pbsd_kernel_vm_phys`, `pbsd_kernel_param`, `pbsd_kernel_security_bsd` — all propagate `PUBLIC ${PBSD_FS_CXX}`; `pbsd_kernel` INTERFACE aggregates Wave 4 modules.

## Wave 4 — Kernel core burst (2026-07-19, expanded)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| sysctl OID tables | `hbsd/src/sys/sys/sysctl.h`, `kern/subr_sysctl.c` | `pbsd/kernel/sysctl/pbsd.kernel.sysctl.cppm` (+ kern/vm/hw sub-tables) |
| syscall numbers | `hbsd/src/sys/sys/syscall.h` | `pbsd/kernel/syscall/pbsd.kernel.syscall.cppm` |
| ucred helpers | `hbsd/src/sys/sys/ucred.h`, `kern/kern_prot.c` | `pbsd/kernel/cred/pbsd.kernel.ucred.cppm` |
| jail kernel side | `hbsd/src/sys/sys/jail.h`, `kern/kern_jail.c` | `pbsd/kernel/jail/pbsd.kernel.jail.cppm` |
| timekeeping | `hbsd/src/sys/sys/clock.h`, `kern/kern_time.c` | `pbsd/kernel/kern/pbsd.kernel.timekeeping.cppm` |
| mutex concepts | `hbsd/src/sys/sys/mutex.h` | `pbsd/kernel/sync/pbsd.kernel.mutex.cppm` |
| turnstile stubs | `hbsd/src/sys/sys/turnstile.h` | `pbsd/kernel/sync/pbsd.kernel.turnstile.cppm` |
| ktrace flags | `hbsd/src/sys/sys/ktrace.h` | `pbsd/kernel/kern/pbsd.kernel.ktrace.cppm` |

### Wave 4 port stubs (`convert_c_batch --wave wave4 --prefix hbsd/src/sys/kern --limit 20`)

20 smallest `hbsd/src/sys/kern/*.c` stubs under `pbsd/ports/wave4/`.

### SI-7 / wave4 harness additions

- Extended `test_wave4_kernel_tables` for sysctl sub-OIDs, syscall table, ucred/jail/timekeeping/mutex/turnstile/ktrace
- `test_si7_ahci_cam`, `test_si7_nvme_io`, `test_si7_igb_vf`, `test_si7_uart_pci`, `test_si7_acpi_video`

### CMake kernel modules added

`pbsd_kernel_syscall`, `pbsd_kernel_ucred`, `pbsd_kernel_jail`, `pbsd_kernel_timekeeping`, `pbsd_kernel_mutex`, `pbsd_kernel_turnstile`, `pbsd_kernel_ktrace`

## Wave 4 — I/O path burst (2026-07-19)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| Syscall arg structs | `hbsd/src/sys/sys/sysproto.h` | `pbsd/kernel/syscall/pbsd.kernel.syscall_args.cppm` |
| kevent filters | `hbsd/src/sys/sys/event.h` | `pbsd/kernel/kevent/pbsd.kernel.kevent.cppm` |
| poll flags | `hbsd/src/sys/sys/poll.h` | `pbsd/kernel/poll/pbsd.kernel.poll.cppm` |
| Socket syscalls | `hbsd/src/sys/kern/uipc_syscalls.c`, `syscall.h` | `pbsd/kernel/socket/pbsd.kernel.socket_syscall.cppm` |
| pipe/pipe2 | `hbsd/src/sys/kern/sys_pipe.c` | `pbsd/kernel/pipe/pbsd.kernel.pipe.cppm` |
| FIFO/mkfifo | `hbsd/src/sys/kern/vfs_syscalls.c` | `pbsd/kernel/fifo/pbsd.kernel.fifo.cppm` |
| Vnode types (kernel) | `hbsd/src/sys/sys/vnode.h` | `pbsd/kernel/vnode/pbsd.kernel.vnode.cppm` |
| namei flags | `hbsd/src/sys/sys/namei.h` | `pbsd/kernel/namei/pbsd.kernel.namei.cppm` |
| filedesc | `hbsd/src/sys/sys/filedesc.h` | `pbsd/kernel/filedesc/pbsd.kernel.filedesc.cppm` |
| selinfo | `hbsd/src/sys/sys/selinfo.h` | `pbsd/kernel/selinfo/pbsd.kernel.selinfo.cppm` |
| select(2)/pselect(2) | `hbsd/src/sys/sys/select.h`, `kern/subr_select.c` | `pbsd/kernel/select/pbsd.kernel.select.cppm` |
| socket state/options | `hbsd/src/sys/sys/socket.h`, `socketvar.h` | `pbsd/kernel/socket/pbsd.kernel.socket_ops.cppm` |
| aio(2)/lio_listio | `hbsd/src/sys/sys/aio.h`, `kern/kern_aio.c` | `pbsd/kernel/aio/pbsd.kernel.aio.cppm` |
| mbuf flags/types | `hbsd/src/sys/sys/mbuf.h` | `pbsd/kernel/mbuf/pbsd.kernel.mbuf.cppm` |
| TCP pcb FSM/flags | `hbsd/src/sys/netinet/tcp_var.h`, `tcp_fsm.h` | `pbsd/kernel/pcb/pbsd.kernel.pcb.cppm` |
| inpcb flags (kernel) | `hbsd/src/sys/netinet/in_pcb.h` | `pbsd/kernel/inpcb/pbsd.kernel.inpcb.cppm` |

### Wave 4 port stubs (`convert_c_batch --wave wave4 --prefix hbsd/src/sys/kern --limit 30`)

30 smallest `hbsd/src/sys/kern/*.c` stubs under `pbsd/ports/wave4/` (poll/kevent/socket/pipe/fifo path).

### SI harness additions

- Extended `test_wave4_kernel_tables` for syscall_args, kevent, poll, socket_syscall, vnode, pipe, fifo, namei, filedesc, selinfo
- `test_wave4_io_socket` — select, aio, mbuf, pcb, inpcb, socket_ops

### CMake kernel modules added

`pbsd_kernel_syscall_args`, `pbsd_kernel_kevent`, `pbsd_kernel_poll`, `pbsd_kernel_select`, `pbsd_kernel_socket_syscall`, `pbsd_kernel_socket_ops`, `pbsd_kernel_vnode`, `pbsd_kernel_pipe`, `pbsd_kernel_fifo`, `pbsd_kernel_namei`, `pbsd_kernel_filedesc`, `pbsd_kernel_selinfo`, `pbsd_kernel_aio`, `pbsd_kernel_mbuf`, `pbsd_kernel_pcb`, `pbsd_kernel_inpcb`

## Wave 4 — Net stack burst (2026-07-19)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| Kernel route RTF_* | `hbsd/src/sys/net/route.h`, `net/route.c` | `pbsd/kernel/route/pbsd.kernel.route.cppm` |
| Kernel ifnet link state | `hbsd/src/sys/net/if.h`, `net/if_var.h` | `pbsd/kernel/ifnet/pbsd.kernel.ifnet.cppm` |
| BPF filter insns | `hbsd/src/sys/net/bpf.h` | `pbsd/kernel/bpf/pbsd.kernel.bpf.cppm` |
| Netgraph node stubs | `hbsd/src/sys/netgraph/netgraph.h` | `pbsd/kernel/netgraph/pbsd.kernel.netgraph.cppm` |
| Audit triggers/events | `hbsd/src/sys/security/audit/audit.h`, `bsm/audit.h` | `pbsd/kernel/audit/pbsd.kernel.audit.cppm` |
| Socket buffer limits | `hbsd/src/sys/sys/socketvar.h` | `pbsd/kernel/sockbuf/pbsd.kernel.sockbuf.cppm` |
| Protocol domains | `hbsd/src/sys/sys/domain.h` | `pbsd/kernel/domain/pbsd.kernel.domain.cppm` |
| protosw flags | `hbsd/src/sys/sys/protosw.h` | `pbsd/kernel/protosw/pbsd.kernel.protosw.cppm` |
| sockio ioctls | `hbsd/src/sys/sys/sockio.h` | `pbsd/kernel/sockio/pbsd.kernel.sockio.cppm` |
| rtentry kernel | `hbsd/src/sys/net/route.h` | `pbsd/kernel/rtentry/pbsd.kernel.rtentry.cppm` |
| CARP/VRRP | `hbsd/src/sys/netinet/ip_carp.h` | `pbsd/kernel/net/pbsd.kernel.carp.cppm` |
| GIF tunnel | `hbsd/src/sys/net/if_gif.h` | `pbsd/kernel/net/pbsd.kernel.gif.cppm` |
| GRE header | `hbsd/src/sys/net/if_gre.h` | `pbsd/kernel/net/pbsd.kernel.gre.cppm` |
| if_bridge ioctls | `hbsd/src/sys/net/if_bridgevar.h` | `pbsd/kernel/net/pbsd.kernel.bridge.cppm` |
| VLAN tags | `hbsd/src/sys/net/if_vlan_var.h` | `pbsd/kernel/net/pbsd.kernel.vlan.cppm` |
| lagg (LAG) | `hbsd/src/sys/net/if_lagg.h` | `pbsd/kernel/net/pbsd.kernel.lagg.cppm` |
| if_media | `hbsd/src/sys/net/if_media.h` | `pbsd/kernel/net/pbsd.kernel.if_media.cppm` |
| if_clone | `hbsd/src/sys/net/if_clone.h` | `pbsd/kernel/net/pbsd.kernel.if_clone.cppm` |
| pfil hooks | `hbsd/src/sys/net/pfil.h` | `pbsd/kernel/net/pbsd.kernel.pfil.cppm` |
| random entropy sources | `hbsd/src/sys/sys/random.h` | `pbsd/kernel/random/pbsd.kernel.random.cppm` |
| entropy KPI | `hbsd/src/sys/dev/random/randomdev.h` | `pbsd/kernel/random/pbsd.kernel.entropy.cppm` |
| opencrypto KPI | `hbsd/src/sys/opencrypto/cryptodev.h` | `pbsd/kernel/crypto/pbsd.kernel.crypto.cppm` |
| GEOM kernel | `hbsd/src/sys/geom/geom.h` | `pbsd/kernel/geom/pbsd.kernel.geom.cppm` |
| CAM kernel | `hbsd/src/sys/cam/cam.h` | `pbsd/kernel/cam/pbsd.kernel.cam.cppm` |

Deepened: `audit` (tokens/pipe), `bpf` (tap/mode), `netgraph` (msg/hook flags).

### Wave 4 port stubs (`convert_c_batch --wave wave4`)

- netgraph: `--prefix hbsd/src/sys/netgraph --limit 20`
- netinet carp: `--prefix hbsd/src/sys/netinet/ --limit 10`
- net if_*: `--prefix hbsd/src/sys/net/ --limit 30`
- geom: `--prefix hbsd/src/sys/geom/ --limit 20`
- cam: `--prefix hbsd/src/sys/cam/ --limit 20`
- opencrypto: `--prefix hbsd/src/sys/opencrypto/ --limit 20`

### SI harness additions

- Extended `test_wave4_net_stack` — carp, gif, gre, bridge, vlan, lagg, random, entropy, crypto, geom, cam, pfil, audit tokens, bpf tap

### CMake kernel modules added

`pbsd_kernel_carp`, `pbsd_kernel_gif`, `pbsd_kernel_gre`, `pbsd_kernel_bridge`, `pbsd_kernel_vlan`, `pbsd_kernel_lagg`, `pbsd_kernel_if_media`, `pbsd_kernel_if_clone`, `pbsd_kernel_pfil`, `pbsd_kernel_random`, `pbsd_kernel_entropy`, `pbsd_kernel_crypto`, `pbsd_kernel_geom`, `pbsd_kernel_cam` (deduped duplicate `pbsd_kernel_bpf`/`pbsd_kernel_audit` targets)

## Wave 6–9 burst (2026-07-19)

### Wave 6 — ICMP / ifnet / TCP FSM / vnode / ZFS spa

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| ICMP types/codes | `hbsd/src/sys/netinet/ip_icmp.h` | `pbsd/net/pbsd.net.icmp.cppm` |
| ifnet IFF_* | `hbsd/src/sys/net/if.h` | `pbsd/net/pbsd.net.ifnet.cppm` |
| TCP FSM TCPS_* | `hbsd/src/sys/netinet/tcp_fsm.h` | `pbsd/net/pbsd.net.tcp.cppm` (`State`, `validate_state_transition`) |
| Vnode vtype/vstate/VV_* | `hbsd/src/sys/sys/vnode.h` | `pbsd/fs/pbsd.fs.vop.cppm` |
| SPA feature GUIDs | `hbsd/src/sys/contrib/openzfs/include/zfeature_common.h` | `pbsd/zfs/pbsd.zfs.features.cppm` (expanded table) |
| Route RTF_* | `hbsd/src/sys/net/route.h` | `pbsd/net/pbsd.net.route.cppm` |
| INP_* socket flags | `hbsd/src/sys/netinet/in_pcb.h` | `pbsd/net/pbsd.net.in_pcb.cppm` |
| namei lookup ops | `hbsd/src/sys/sys/namei.h` | `pbsd/fs/pbsd.fs.namei.cppm` |

CMake: `pbsd_net_route`, `pbsd_net_in_pcb`, `pbsd_fs_namei`. SI harness: expanded `test_wave6_net_fs_geom_zfs`.

### Wave 7 — EFI protocols / bootinfo / MSRs

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| EFI protocol GUIDs | `hbsd/src/stand/efi/include/*.h` | `pbsd/stand/pbsd.stand.efi.protocols.cppm` |
| bootinfo / bootargs | `hbsd/src/stand/i386/common/bootargs.h`, `stand/*/bootinfo*.c` | `pbsd/stand/pbsd.stand.bootinfo.cppm` |
| amd64 MSRs | `hbsd/src/sys/x86/include/specialreg.h` | `pbsd/arch/amd64/pbsd.arch.amd64.msr.cppm` (`kMsrTable`, `validate_msr_access`) |

CMake: `pbsd_stand_efi_protocols`, `pbsd_stand_bootinfo`. SI harness: expanded `test_wave7_stand_arch`.

### Wave 8 — Wayland xdg / BIFROST nested virt

| Module | Source | PBSD artifact |
|--------|--------|---------------|
| xdg positioner/decoration/wm_capabilities | xdg-shell stable | `pbsd/compositor/pbsd.compositor.wayland.cppm` |
| wlr-layer-shell | layer-shell-qt / wlr-layer-shell-unstable-v1 | `pbsd/compositor/pbsd.compositor.wayland.layer.cppm` |
| Nested VMX/SVM stubs | `hbsd amd64/vmm` | `pbsd/bifrost/pbsd.bifrost.nested.cppm` |
| EPT/NPT violation stubs | `hbsd amd64/vmm/intel/ept.h`, `amd/npt.h` | `pbsd/bifrost/pbsd.bifrost.ept.cppm` |

CMake: `pbsd_bifrost_nested`, `pbsd_bifrost_ept`, `pbsd_compositor_wayland_layer`. SI harness: expanded `test_wave8_compositor`.

### Wave 9 — Purge metrics + stubs

| Item | Artifact |
|------|----------|
| Purge JSON metrics | `tools/wave_purge_c_check.py --metrics` |
| Wave status rollup | `docs/migration/WAVE_STATUS.md` |
| Contrib port stubs | `pbsd/ports/wave9/` (batch via `convert_c_batch --wave wave9`) |

### Wave 6–9 partition pass 3 (+18 modules, 2026-07-19)

| Domain | New modules |
|--------|-------------|
| net | `pbsd.net.altq`, `pbsd.net.in6`, `pbsd.net.gre`, `pbsd.net.epair` |
| fs | `pbsd.fs.tmpfs`, `pbsd.fs.pipefs`, `pbsd.fs.extattr` |
| geom | `pbsd.geom.stripe`, `pbsd.geom.concat` |
| stand | `pbsd.stand.env`, `pbsd.stand.devinfo`, `pbsd.stand.zfsboot` |
| arch | `pbsd.arch.amd64.exception`, `pbsd.arch.arm64.cpufunc` |
| bifrost | `pbsd.bifrost.ioapic`, `pbsd.bifrost.lapic`, `pbsd.bifrost.vpid` |
| compositor | `pbsd.compositor.drm` |

CMake: `pbsd_net_{altq,in6,gre,epair}`, `pbsd_fs_{tmpfs,pipefs,extattr}`, `pbsd_geom_{stripe,concat}`, `pbsd_stand_{env,devinfo,zfsboot}`, `pbsd_arch_amd64_exception`, `pbsd_arch_arm64_cpufunc`, `pbsd_bifrost_{ioapic,lapic,vpid}`, `pbsd_compositor_drm`. SI harness: partition smoke in `test_wave6/7/8`.

## Wave 5 — UDA descriptors + engines (2026-07-19)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| igb 82575 | `hbsd/src/sys/dev/e1000/e1000_82575.h`, `if_em.c` | `pbsd/uda/descriptors/igb.cppm` |
| em 82571 | `hbsd/src/sys/dev/e1000/e1000_hw.h`, `if_em.c` | `pbsd/uda/descriptors/em.cppm` |
| em 82574L | `hbsd/src/sys/dev/e1000/e1000_hw.h` | `pbsd/uda/descriptors/em_82574.cppm` |
| re RTL8168 | `hbsd/src/sys/dev/re/if_re.c`, `rl/if_rlreg.h` | `pbsd/uda/descriptors/re.cppm` |
| igc I225 | `hbsd/src/sys/dev/igc/igc_regs.h`, `igc_hw.h` | `pbsd/uda/descriptors/igc.cppm` |
| HDA controller | `hbsd/src/sys/dev/sound/pci/hda/hdac_reg.h` | `pbsd/uda/descriptors/hda.cppm` |
| ICH SMBus | `hbsd/src/sys/dev/ichsmb/ichsmb_reg.h` | `pbsd/uda/descriptors/smbus.cppm` |
| ICH watchdog | `hbsd/src/sys/dev/ichwd/ichwd.h` | `pbsd/uda/descriptors/ichwd.cppm` |
| ACPI thermal | `hbsd/src/sys/dev/acpica/acpi_thermal.c` | `pbsd/uda/descriptors/acpi_thermal.cppm` |
| vtnet MQ | `hbsd/src/sys/dev/virtio/network/if_vtnetvar.h` | `pbsd/uda/descriptors/vtnet_mq.cppm` |
| VirtIO GPU | `hbsd/src/sys/dev/virtio/gpu/virtio_gpu.h` | `pbsd/uda/descriptors/virtio_gpu.cppm` |
| VirtIO entropy | `hbsd/src/sys/dev/virtio/random/virtio_random.c`, `virtio_ids.h` | `pbsd/uda/descriptors/virtio_random.cppm` |
| VirtIO balloon | `hbsd/src/sys/dev/virtio/balloon/virtio_balloon.h` | `pbsd/uda/descriptors/virtio_balloon.cppm` |
| NVMe admin | `hbsd/src/sys/dev/nvme/nvme.h` (AQA/ASQ/ACQ, opcodes) | `pbsd/uda/descriptors/nvme_admin.cppm` |
| ixgbe 82599 | `hbsd/src/sys/dev/ixgbe/ixgbe_type.h` | `pbsd/uda/descriptors/ixgbe.cppm` |
| UHCI | `hbsd/src/sys/dev/usb/controller/uhcireg.h` | `pbsd/uda/descriptors/uhci.cppm` |
| EHCI | `hbsd/src/sys/dev/usb/controller/ehcireg.h` | `pbsd/uda/descriptors/ehci.cppm` |
| NS8250 UART | `hbsd/src/sys/dev/ic/ns16550.h` | `pbsd/uda/descriptors/ns8250.cppm` |
| CHV GPIO | `hbsd/src/sys/dev/gpio/chvgpio_reg.h` | `pbsd/uda/descriptors/gpio.cppm` |
| ACPI button | `hbsd/src/sys/dev/acpica/acpi_button.c` | `pbsd/uda/descriptors/acpi_button.cppm` |
| Display engine | virtio-gpu request path | `pbsd/uda/engines/pbsd.uda.engine.display.cppm` |
| Sensor engine | entropy/balloon/gpio/button paths | `pbsd/uda/engines/pbsd.uda.engine.sensor.cppm` |
| Input engine | UART tx/rx + HID | `pbsd/uda/engines/pbsd.uda.engine.input.cppm` |
| Block engine | NVMe IO opcode queue, AHCI FIS path | `pbsd/uda/engines/pbsd.uda.engine.block.cppm` |
| Net engine | TSO submit, link status, RX csum counters | `pbsd/uda/engines/pbsd.uda.engine.net.cppm` |
| Input engine | HDA stream queue | `pbsd/uda/engines/pbsd.uda.engine.input.cppm` |
| Sensor engine | thermal, watchdog pet, SMBus txn | `pbsd/uda/engines/pbsd.uda.engine.sensor.cppm` |
| AHCI CAM port0 | `hbsd/src/sys/dev/ahci/ahci.h`, `ahci.c` | `pbsd/uda/descriptors/ahci_cam.cppm` |
| NVMe IO qpair | `hbsd/src/sys/dev/nvme/nvme_private.h` | `pbsd/uda/descriptors/nvme_io.cppm` |
| igb VF 82576 | `hbsd/src/sys/dev/e1000/e1000_vf.h` | `pbsd/uda/descriptors/igb_vf.cppm` |
| UART PCI SOL | `hbsd/src/sys/dev/uart/uart_bus_pci.c` | `pbsd/uda/descriptors/uart_pci.cppm` |
| ACPI video | `hbsd/src/sys/dev/acpica/acpi_video.c` | `pbsd/uda/descriptors/acpi_video.cppm` |
| SDHCI host | `hbsd/src/sys/dev/sdhci/sdhci.h` | `pbsd/uda/descriptors/sdhci.cppm` |
| MPT LSI CAM | `hbsd/src/sys/dev/mpt/mpt_reg.h`, `mpt_cam.c` | `pbsd/uda/descriptors/mpt_cam.cppm` |
| bge BCM5700 | `hbsd/src/sys/dev/bge/if_bgereg.h` | `pbsd/uda/descriptors/bge.cppm` |
| ATA PCI channel | `hbsd/src/sys/dev/ata/ata-pci.h`, `ata-all.h` | `pbsd/uda/descriptors/ata_pci.cppm` |
| msk Yukon2 | `hbsd/src/sys/dev/msk/if_mskreg.h` | `pbsd/uda/descriptors/msk.cppm` |

Deepened register programs (burst): `ahci`, `nvme`, `igb`, `ns8250`, `e1000`.
Deepened register programs (pass 2): `em`, `em_82574`, `re`, `igc`, `ixgbe`, `ahci_cam`, `nvme_io` (+ `probe_mmio` on `e1000`/`igb`).
Deepened register programs (pass 3): `xhci`, `hda`, `smbus`, `ichwd`, `uhci`, `gpio`, `virtio_common`, `vtnet_mq`, `usbhid`, `acpi_thermal`.
Engine deepening (pass 3): block `submit_trim`, net `set_mtu`/rx batch, sensor `submit_thermal_batch`, display `submit_flush`, CAM CCB sense data.

### CAM integration (`pbsd/uda/cam/`)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| CAM status codes | `hbsd/src/sys/cam/cam.h` | `pbsd/uda/cam/pbsd.uda.cam.status.cppm` |
| CAM CCB | `hbsd/src/sys/cam/cam_ccb.h` | `pbsd/uda/cam/pbsd.uda.cam.ccb.cppm` |
| XPT transport | `hbsd/src/sys/cam/cam_xpt.c` | `pbsd/uda/cam/pbsd.uda.cam.xpt.cppm` |
| CAM aggregate | `hbsd/src/sys/cam/` | `pbsd/uda/cam/pbsd.uda.cam.cppm` |
| VirtIO-SCSI CAM | `hbsd/src/sys/dev/virtio/scsi/` | `pbsd/uda/descriptors/virtio_scsi_cam.cppm` |

Engine deepening: block CAM IO queue, net multicast/bpf tap, display create2d/transfer, input HID batch/modifiers, sensor GPIO edge queue; pass 3: block `submit_mpt_io`, net `submit_rx_batch`/`complete_tx`, CAM abort/rescan/async, `kAhciCamIssueCmd`.

### Wave 5 port stubs (`convert_c_batch --wave wave5`)

- virtio tree: 18 stubs under `pbsd/ports/wave5/hbsd/src/sys/dev/virtio/`
- e1000/em/igb: `hbsd/src/sys/dev/e1000/` (limit 40)
- re/rl: `hbsd/src/sys/dev/re`, `hbsd/src/sys/dev/rl` (limit 20)
- igc: `hbsd/src/sys/dev/igc/` (limit 20)
- hda: `hbsd/src/sys/dev/sound/pci/hda/` (limit 20)
- ichsmb/ichwd: `hbsd/src/sys/dev/ichsmb`, `hbsd/src/sys/dev/ichwd` (limit 20)
- acpica thermal: `hbsd/src/sys/dev/acpica/acpi_thermal.c`
- vtnet: `hbsd/src/sys/dev/virtio/network/` (limit 10)
- ahci cam path: `hbsd/src/sys/dev/ahci/` (limit 10)
- nvme io: `hbsd/src/sys/dev/nvme/` (limit 10)
- e1000 vf: `hbsd/src/sys/dev/e1000/e1000_vf.h`, `e1000_api.c` (limit 10)
- uart pci: `hbsd/src/sys/dev/uart/` (limit 10)
- acpi video: `hbsd/src/sys/dev/acpica/acpi_video.c`
- sdhci: `hbsd/src/sys/dev/sdhci/` (limit 10)
- mpt cam: `hbsd/src/sys/dev/mpt/` (limit 10)
- bge: `hbsd/src/sys/dev/bge/` (limit 10)
- ata pci: `hbsd/src/sys/dev/ata/` (limit 10)
- msk: `hbsd/src/sys/dev/msk/` (limit 10)

### SI-7 harness additions

- `test_si7_igb`, `test_si7_em`, `test_si7_em_82574`, `test_si7_re`, `test_si7_igc`
- `test_si7_hda`, `test_si7_smbus`, `test_si7_ichwd`, `test_si7_acpi_thermal`, `test_si7_vtnet_mq`
- `test_si7_virtio_gpu`, `test_si7_virtio_random`, `test_si7_virtio_balloon`
- `test_si7_nvme_admin`, `test_si7_ixgbe`, `test_si7_uhci`, `test_si7_ehci`
- `test_si7_ns8250`, `test_si7_gpio`, `test_si7_acpi_button`
- `test_si7_ahci_cam`, `test_si7_nvme_io`, `test_si7_igb_vf`, `test_si7_uart_pci`, `test_si7_acpi_video`
- `test_si7_cam_integration`, `test_si7_engine_deepening`
- `test_si7_new_descriptors` — sdhci, mpt_cam, bge, ata_pci, msk

### CMake `pbsd_uda` INTERFACE targets added

`pbsd_uda_igb`, `pbsd_uda_em`, `pbsd_uda_em_82574`, `pbsd_uda_re`, `pbsd_uda_igc`, `pbsd_uda_hda`, `pbsd_uda_smbus`, `pbsd_uda_ichwd`, `pbsd_uda_acpi_thermal`, `pbsd_uda_vtnet_mq`, `pbsd_uda_virtio_gpu`, `pbsd_uda_virtio_random`, `pbsd_uda_virtio_balloon`, `pbsd_uda_nvme_admin`, `pbsd_uda_ixgbe`, `pbsd_uda_uhci`, `pbsd_uda_ehci`, `pbsd_uda_ns8250`, `pbsd_uda_gpio`, `pbsd_uda_acpi_button`, `pbsd_uda_ahci_cam`, `pbsd_uda_nvme_io`, `pbsd_uda_igb_vf`, `pbsd_uda_uart_pci`, `pbsd_uda_acpi_video`, `pbsd_uda_sdhci`, `pbsd_uda_mpt_cam`, `pbsd_uda_bge`, `pbsd_uda_ata_pci`, `pbsd_uda_msk`, `pbsd_uda_cam_status`, `pbsd_uda_cam_ccb`, `pbsd_uda_cam_xpt`, `pbsd_uda_cam`, `pbsd_uda_virtio_scsi_cam`

## Wave 6 — Net / FS / GEOM / ZFS (2026-07-19, expanded)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| Socket options | `hbsd/src/sys/sys/socket.h`, `sockopt.h`, `kern/uipc_socket.c` | `pbsd/net/pbsd.net.sockopt.cppm`, `setsockopt()` in `pbsd.net.cppm` |
| Netinet constants | `hbsd/src/sys/netinet/in.h` | `pbsd/net/pbsd.net.netinet.cppm` |
| TCP options | `hbsd/src/sys/netinet/tcp.h` | `pbsd/net/pbsd.net.tcp.cppm` |
| UDP options | `hbsd/src/sys/netinet/udp.h` | `pbsd/net/pbsd.net.udp.cppm` |
| VFS ops | `hbsd/src/sys/kern/vnode_if.src` | `pbsd/fs/pbsd.fs.vop.cppm`, `vop_dispatch()` in `pbsd.fs.cppm` |
| GEOM ctl | `hbsd/src/sys/geom/geom_ctl.h`, `geom.h` | `pbsd/geom/pbsd.geom.ctl.cppm`, `ioctl_ctl()` in `pbsd.geom.cppm` |
| ZFS ioctl | `hbsd/src/sys/contrib/openzfs/include/sys/fs/zfs.h`, `zfs_ioctl.h` | `pbsd/zfs/pbsd.zfs.ioctl.cppm`, `ioctl()` in `pbsd.zfs.cppm` |
| ZFS features | `hbsd/src/sys/contrib/openzfs/include/zfeature_common.h` | `pbsd/zfs/pbsd.zfs.features.cppm`, `require_feature()` in `pbsd.zfs.cppm` |
| IPv4 header / TOS / DSCP | `hbsd/src/sys/netinet/ip.h` | `pbsd/net/pbsd.net.ip.cppm` |
| Mount flags | `hbsd/src/sys/sys/mount.h` | `pbsd/fs/pbsd.fs.mount.cppm` |

### Wave 6 port stubs (incremental)

| Batch | Count | Prefix |
|-------|------:|--------|
| netinet (prior) | 40 | `hbsd/src/sys/netinet` |
| geom | 35 | `hbsd/src/sys/geom` |
| fs | 35 | `hbsd/src/sys/fs` |
| openzfs module | 30 | `hbsd/src/sys/contrib/openzfs/module` |

**Total wave6 stubs:** 180 under `pbsd/ports/wave6/` (+40 net incremental).

## Wave 7 — Stand / Arch (2026-07-19, expanded)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| PE/COFF | `hbsd/src/stand/efi/include/amd64/pe.h` | `pbsd/stand/pbsd.stand.pe.cppm`, `BootParams` in `pbsd.stand.cppm` |
| EFI boot | `hbsd/src/stand/efi/include`, `stand/common/` | `pbsd/stand/pbsd.stand.efi.cppm` |
| amd64 MSR/VMX | `hbsd/src/sys/x86/include/specialreg.h`, `amd64/vmm/intel/vmcs.h` | `pbsd/arch/amd64/pbsd.arch.amd64.msr.cppm` |
| arm64 sysreg | `hbsd/src/sys/arm64/include/armreg.h`, `hypervisor.h`, `arm64/include/vmm.h` | `pbsd/arch/arm64/pbsd.arch.arm64.sysreg.cppm` |
| Loader commands / boot flags | `hbsd/src/stand/common/*.c` | `pbsd/stand/pbsd.stand.common.cppm` |

### Wave 7 port stubs (incremental)

| Batch | Count | Prefix |
|-------|------:|--------|
| stand (prior) | 40 | `hbsd/src/stand` |
| amd64 | 25 | `hbsd/src/sys/amd64` |
| arm64 | 25 | `hbsd/src/sys/arm64` |
| stand/common | 24 | `hbsd/src/stand/common` |

**Total wave7 stubs:** 114 under `pbsd/ports/wave7/`.

## Wave 8 — BIFROST / Compositor (2026-07-19, expanded)

| Module | HBSD / spec source | PBSD artifact |
|--------|-------------------|---------------|
| VM exits | `amd64/vmm/intel/vmcs.h`, `amd/vmcb.h`, `amd64/include/vmm.h` | `pbsd/bifrost/pbsd.bifrost.hypervisor.cppm`, `dispatch_exit()` |
| virtio-mmio PV | `hbsd/src/sys/dev/virtio/mmio/virtio_mmio.h` | `pbsd/bifrost/pbsd.bifrost.virtio.cppm` |
| Wayland wire | wayland.xml core + xdg-shell | `pbsd/compositor/pbsd.compositor.wayland.cppm` |
| Aero theme | `pbsd/theme/plasma/blur-effect.json` | `load_aero_json_fragment()`, `apply_aero_to_surface()` in `pbsd.compositor.cppm` |

Expanded pass 2: VMX/SVM exit table (+6 entries), virtio-mmio status/PV GPU, Wayland pointer/data-device opcodes.

## Wave 9 — Purge / pkg (2026-07-19)

| Item | Artifact |
|------|----------|
| Purge exceptions | expanded `docs/migration/c_purge_exceptions.txt` (28 vendor/asm/firmware patterns) |
| Purge tooling | `tools/wave_purge_c_check.py` (`--json`, category breakdown, batch stub counts) |
| Batch tooling | `convert_c_batch --skip-stubbed` for incremental contrib batches |
| Contrib stubs | 200 under `pbsd/ports/wave9/` (`--prefix hbsd/src/contrib --limit 200 --skip-stubbed`) |
| Atomic pkg | `pbsd/pkg/pbsd.pkg.cppm`, `test_wave9_pkg()` in SI harness |

## SI harness additions (2026-07-19)

- **SI-8** expanded: VmHandle create/deny, MMIO grant bounds, VMX/SVM exit dispatch, lineage revoke on `destroy_vm`
- **Wave 6–8** smoke tests in `pbsd/tests/si_harness.cpp`
- **Wave 9** `test_wave9_pkg()` — AtomicUpdater begin/stage/commit

## CMake modules added

`pbsd_net_sockopt`, `pbsd_net_netinet`, `pbsd_net_tcp`, `pbsd_net_udp`, `pbsd_net_ip`, `pbsd_fs_vop`, `pbsd_fs_mount`, `pbsd_geom_ctl`, `pbsd_zfs_ioctl`, `pbsd_zfs_features`, `pbsd_stand_pe`, `pbsd_stand_efi`, `pbsd_stand_common`, `pbsd_arch_amd64_msr`, `pbsd_arch_arm64_sysreg`, `pbsd_bifrost_hypervisor`, `pbsd_compositor_wayland`, `pbsd_pkg`

## Wave 2 — Userland bin utilities (2026-07-19)

Hand ports of smallest `hbsd/src/bin/*` helpers into nucleus modules (logic-only; Capsicum/hosted syscalls deferred to callers).

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| sleep | `hbsd/src/bin/sleep/sleep.c` | `pbsd/userland/bin/pbsd.userland.sleep.cppm` | Interval parse (`1.5s`, `2m`, …) |
| rmdir | `hbsd/src/bin/rmdir/rmdir.c` | `pbsd/userland/bin/pbsd.userland.rmdir.cppm` | `-p`/`-v`, trailing-slash trim |
| mkdir | `hbsd/src/bin/mkdir/mkdir.c` | `pbsd/userland/bin/pbsd.userland.mkdir.cppm` | `-p`/`-m`; imports `pbsd.userland.util.filemode` |
| cat | `hbsd/src/bin/cat/cat.c` | `pbsd/userland/bin/pbsd.userland.cat.cppm` | getopt-style flags, cooked-mode helpers |
| true | `hbsd/src/bin/true/true.c` | `pbsd/userland/bin/pbsd.userland.true.cppm` | Module `pbsd.userland.true_cmd` (C++ keyword) |
| false | `hbsd/src/bin/false/false.c` | `pbsd/userland/bin/pbsd.userland.false.cppm` | Module `pbsd.userland.false_cmd` (C++ keyword) |

CMake: all six wired into `pbsd_userland_bin` INTERFACE in `pbsd/userland/CMakeLists.txt`. Smoke coverage in `pbsd/tests/userland_harness.cpp` (`test_sleep_parse`, `test_mkdir_parse`, `test_rmdir_parse`, `test_cat_parse`, `test_true_false`).

## Wave 2 — libc / jail / casper / libutil / capsicum (2026-07-19, expanded)

Hand ports from `hbsd/src/lib/libc`, `libjail`, `libcasper`, `libutil`, `libcapsicum`.

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| libc memory | `string/{memset,memcpy,bzero,memmove}.c`, `bcopy.c` | `pbsd/userland/libc/pbsd.userland.libc.memory.cppm` | Word-aligned freestanding helpers |
| libc string | `string/{strlen,strcmp,strncmp,strcpy,strlcpy,strchr,...}.c` | `pbsd.userland.libc.string*.cppm` | Compare + copy + search partitions |
| libc gen | `gen/{dirname,basename}.c` | `pbsd/userland/libc/pbsd.userland.libc.gen.cppm` | Path component helpers |
| libc stdlib | `stdlib/{strtol,atoi}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.cppm` | C-locale strtol (no xlocale) |
| libc umbrella | — | `pbsd/userland/libc/pbsd.userland.libc.cppm` | Re-exports memory/string/stdlib |
| libjail | `libjail/jail.c`, `jail_getid.c`, `jail.h` | `pbsd/userland/jail/pbsd.userland.jail*.cppm` | param enum/jailsys, noname/kvname, jid 0 |
| libcasper | `libcasper/services/*` | `pbsd/userland/casper/pbsd.userland.casper.*.cppm` | 8-service registry + fileargs name |
| libutil | `libutil/mntopts.c` | `pbsd/userland/util/pbsd.userland.util.mntopts.cppm` | getmntopts flag parsing |
| libcapsicum | `libcapsicum/capsicum_helpers.h` | `pbsd/userland/capsicum/pbsd.userland.capsicum.rights.cppm` | Stream ioctl table, CAP bit map |

### Wave 2 port stubs (`convert_c_batch --wave wave2 --prefix hbsd/src/lib/libc/ --limit 100`)

100 smallest `hbsd/src/lib/libc/**/*.c` stubs under `pbsd/ports/wave2/`.

### Harness additions

- `test_libc_memory`, `test_libc_string`, `test_libc_stdlib`
- `test_capsicum_rights`, `test_jail_name_helpers`, `test_util_mntopts`, `test_casper_registry`

### CMake modules added

`pbsd_userland_libc` (+ string/stdlib partitions), `pbsd_userland_capsicum_rights`, `pbsd_userland_util_mntopts`; `pbsd_userland_jail` links `pbsd_userland_libc`.

## Wave 2/4 additions (burst 3)

| Source | PBSD artifact |
|--------|---------------|
| `bin/chflags` | `pbsd.userland.chflags` |
| `sbin/sysctl` helpers | `pbsd.userland.sysctl` |
| `sys/errno.h` bridge | `pbsd.kernel.errno` |
| `sys/signal.h` tables | `pbsd.kernel.signal` (str2sig/sig2str) |

## Wave 2 usr.bin + syscall (burst 4)

| Source | PBSD artifact |
|--------|---------------|
| `usr.bin/yes` | `pbsd.userland.yes` |
| `usr.bin/basename` | `pbsd.userland.basename` |
| `usr.bin/dirname` | `pbsd.userland.dirname` |
| `usr.bin/uname` | `pbsd.userland.uname` |
| `usr.bin/wc` | `pbsd.userland.wc` |
| `sys/syscall.h` subset | `pbsd.kernel.syscall` |

## Burst 5

| Source | PBSD artifact |
|--------|---------------|
| `usr.bin/head` | `pbsd.userland.head` |
| `usr.bin/tee` | `pbsd.userland.tee` |
| `usr.bin/seq` | `pbsd.userland.seq` |
| `usr.bin/env` | `pbsd.userland.env` |
| `sys/event.h` | `pbsd.kernel.kevent` |

## Burst 6

| Source | PBSD artifact |
|--------|---------------|
| `usr.bin/tr` | `pbsd.userland.tr` |
| `usr.bin/cut` | `pbsd.userland.cut` |
| `usr.bin/uniq` | `pbsd.userland.uniq` |
| `libthr` concepts | `pbsd.userland.libthr` |
| `sys/mbuf.h` | `pbsd.kernel.mbuf` |
| `sys/poll.h` | `pbsd.kernel.poll` |

## Burst 7

| Source | PBSD artifact |
|--------|---------------|
| `usr.bin/grep` | `pbsd.userland.grep` |
| `usr.bin/xargs` | `pbsd.userland.xargs` |
| `libc/ctype` | `pbsd.userland.libc.ctype` |
| `sys/namei.h` | `pbsd.kernel.namei` |
| `net/bpf.h` | `pbsd.kernel.bpf` |

## Burst 8

| Source | PBSD artifact |
|--------|---------------|
| `usr.bin/sed` | `pbsd.userland.sed` |
| `usr.bin/find` | `pbsd.userland.find` |
| `sys/audit.h` | `pbsd.kernel.audit` |
| `net/if_vlan` | `pbsd.net.vlan` |

## Wave 2 — Userland bin/sbin hand ports (2026-07-19, burst 4)

Promoted 12 Wave 2 port stubs (and priority utilities without stubs) into real C++23 modules under `pbsd/userland/bin/` and `pbsd/userland/sbin/`.

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| chflags | `hbsd/src/bin/chflags/chflags.c` | `pbsd/userland/bin/pbsd.userland.chflags.cppm` | FTS/flag parse, octal + symbolic |
| dd | `hbsd/src/bin/dd/args.c` | `pbsd/userland/bin/pbsd.userland.dd.cppm` | JCL operand + numeric postfix parse |
| getfacl | `hbsd/src/bin/getfacl/getfacl.c` | `pbsd/userland/bin/pbsd.userland.getfacl.cppm` | `-dhnqsv` option parse |
| setfacl | `hbsd/src/bin/setfacl/{setfacl.c,util.c}` | `pbsd/userland/bin/pbsd.userland.setfacl.cppm` | ACL brand/op helpers, FTS flags |
| sync | `hbsd/src/bin/sync/sync.c` | `pbsd/userland/bin/pbsd.userland.sync.cppm` | Trivial Status/Result wrapper |
| uuidgen | `hbsd/src/bin/uuidgen/uuidgen.c` | `pbsd/userland/bin/pbsd.userland.uuidgen.cppm` | Capsicum-first sandbox entry |
| realpath | `hbsd/src/bin/realpath/realpath.c` | `pbsd/userland/bin/pbsd.userland.realpath.cppm` | `-q` parse |
| kenv | `hbsd/src/bin/kenv/kenv.c` | `pbsd/userland/bin/pbsd.userland.kenv.cppm` | dump/get/set/unset mode parse |
| ls cmp | `hbsd/src/bin/ls/cmp.c` | `pbsd/userland/bin/pbsd.userland.ls.cmp.cppm` | FTS sort comparators |
| stty util | `hbsd/src/bin/stty/util.c` | `pbsd/userland/bin/pbsd.userland.stty.util.cppm` | stdout redirect warning helper |
| chown | `hbsd/src/usr.sbin/chown/chown.c` | `pbsd/userland/sbin/pbsd.userland.chown.cppm` | owner:group parse, FTS options |
| df | `hbsd/src/bin/df/df.c` | `pbsd/userland/sbin/pbsd.userland.df.cppm` | Flag parse, `fsbtoblk`, human units |

### Wave 2 port stubs (`convert_c_batch --wave wave2 --prefix hbsd/src/usr.bin/ --limit 60`)

60 smallest `hbsd/src/usr.bin/**/*.c` stubs under `pbsd/ports/wave2/`.

### Harness additions

`test_chflags_parse`, `test_dd_parse`, `test_getfacl_parse`, `test_setfacl_helpers`, `test_chown_parse`, `test_df_parse`, `test_sync_run`, `test_uuidgen_parse`, `test_realpath_parse`, `test_kenv_parse`, `test_ls_cmp`, `test_stty_util`

### CMake modules added

`pbsd_userland_chflags`, `pbsd_userland_dd`, `pbsd_userland_getfacl`, `pbsd_userland_setfacl`, `pbsd_userland_sync`, `pbsd_userland_uuidgen`, `pbsd_userland_realpath`, `pbsd_userland_kenv`, `pbsd_userland_ls_cmp`, `pbsd_userland_stty_util`, `pbsd_userland_chown`, `pbsd_userland_df`; new `pbsd_userland_sbin` INTERFACE aggregates sbin targets.

## Wave 2 — usr.bin / sbin hand ports (2026-07-19, burst 5)

Hand ports of 17 `hbsd/src/usr.bin/*` utilities and 4 `hbsd/src/sbin/*` parse stubs into `pbsd/userland/usr.bin/` and `pbsd/userland/sbin/`.

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| yes | `hbsd/src/usr.bin/yes/yes.c` | `pbsd/userland/usr.bin/pbsd.userland.yes.cppm` | Expletive assembly, OPTBUF sizing |
| basename | `hbsd/src/usr.bin/basename/basename.c` | `pbsd/userland/usr.bin/pbsd.userland.basename.cppm` | `-a`/`-s`, suffix strip |
| dirname | `hbsd/src/usr.bin/dirname/dirname.c` | `pbsd/userland/usr.bin/pbsd.userland.dirname.cppm` | Path parent extraction |
| uname | `hbsd/src/usr.bin/uname/uname.c` | `pbsd/userland/usr.bin/pbsd.userland.uname.cppm` | Flag parse, field formatting |
| printenv | `hbsd/src/usr.bin/printenv/printenv.c` | `pbsd/userland/usr.bin/pbsd.userland.printenv.cppm` | Env lookup |
| env | `hbsd/src/usr.bin/env/env.c` | `pbsd/userland/usr.bin/pbsd.userland.env.cppm` | `-0CiLPSUuv` parse |
| which | `hbsd/src/usr.bin/which/which.c` | `pbsd/userland/usr.bin/pbsd.userland.which.cppm` | PATH walk helpers |
| time | `hbsd/src/usr.bin/time/time.c` | `pbsd/userland/usr.bin/pbsd.userland.time.cppm` | Module `pbsd.userland.time_cmd` |
| touch | `hbsd/src/usr.bin/touch/touch.c` | `pbsd/userland/usr.bin/pbsd.userland.touch.cppm` | `-Aacdhmrt` parse |
| truncate | `hbsd/src/usr.bin/truncate/truncate.c` | `pbsd/userland/usr.bin/pbsd.userland.truncate.cppm` | `-cdrs` + expand_number |
| id | `hbsd/src/usr.bin/id/id.c` | `pbsd/userland/usr.bin/pbsd.userland.id.cppm` | id/whoami/groups modes |
| head | `hbsd/src/usr.bin/head/head.c` | `pbsd/userland/usr.bin/pbsd.userland.head.cppm` | `-n`/`-c`/`-q`/`-v` |
| tail | `hbsd/src/usr.bin/tail/tail.c` | `pbsd/userland/usr.bin/pbsd.userland.tail.cppm` | `-Fbcfnqrv`, offset style |
| wc | `hbsd/src/usr.bin/wc/wc.c` | `pbsd/userland/usr.bin/pbsd.userland.wc.cppm` | `-clmwL`, default counts |
| tee | `hbsd/src/usr.bin/tee/tee.c` | `pbsd/userland/usr.bin/pbsd.userland.tee.cppm` | `-ai` parse |
| mktemp | `hbsd/src/usr.bin/mktemp/mktemp.c` | `pbsd/userland/usr.bin/pbsd.userland.mktemp.cppm` | `-dpqtu`, template check |
| printf | `hbsd/src/usr.bin/printf/printf.c` | `pbsd/userland/usr.bin/pbsd.userland.printf.cppm` | Directive/escape helpers |
| ifconfig | `hbsd/src/sbin/ifconfig/ifconfig.c` | `pbsd/userland/sbin/pbsd.userland.ifconfig.cppm` | Leading flag parse stub |
| mount | `hbsd/src/sbin/mount/mount.c` | `pbsd/userland/sbin/pbsd.userland.mount.cppm` | `-adFfLlno:prt:uvw`, ro detect |
| route | `hbsd/src/sbin/route/route.c` | `pbsd/userland/sbin/pbsd.userland.route.cppm` | `-46nqdtvj`, keyword index |
| install | `hbsd/src/stand/common/install.c` | `pbsd/userland/sbin/pbsd.userland.install.cppm` | setpath/csv helpers |

### Wave 2 port stubs (`convert_c_batch --wave wave2`)

- `--prefix hbsd/src/usr.bin/ --limit 80` (80 smallest `.c` stubs)
- `--prefix hbsd/src/sbin/ --limit 40` (40 smallest `.c` stubs)

### Harness additions

`test_usr_bin_wave2`, `test_sbin_wave2` in `pbsd/tests/userland_harness.cpp`.

### CMake modules added

`pbsd_userland_usr_bin` INTERFACE; extended `pbsd_userland_sbin` with `pbsd_userland_yes`, `pbsd_userland_basename`, `pbsd_userland_dirname`, `pbsd_userland_uname`, `pbsd_userland_printenv`, `pbsd_userland_env`, `pbsd_userland_which`, `pbsd_userland_time_cmd`, `pbsd_userland_touch`, `pbsd_userland_truncate`, `pbsd_userland_id`, `pbsd_userland_head`, `pbsd_userland_tail`, `pbsd_userland_wc`, `pbsd_userland_tee`, `pbsd_userland_mktemp`, `pbsd_userland_printf`, `pbsd_userland_ifconfig`, `pbsd_userland_mount`, `pbsd_userland_route`, `pbsd_userland_install`.

## Wave 2 — libc / libutil burst (2026-07-19, throughput)

Hand ports of core `hbsd/src/lib/libc` string/stdlib/gen + expanded libutil.

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| libc string search | `string/{strchr,strrchr,strchrnul}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.search.cppm` | Word-scan strchrnul + strchr/strrchr |
| libc string copy | `string/{strcpy,strncpy,strcat,strlcpy,strnlen}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.copy.cppm` | BSD/Nuxi copy helpers |
| libc memory compare | `string/{memcmp,memchr}.c` | `pbsd/userland/libc/pbsd.userland.libc.memory.cppm` | Added to memory partition |
| libc stdlib convert | `stdlib/{atol,strtoul,abs,labs}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.cppm` | C-locale, no xlocale |
| libc stdlib sort | `stdlib/{qsort,bsearch}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.sort.cppm` | Bentley–McIlroy qsort |
| libc gen path | `gen/{dirname,basename}.c` | `pbsd/userland/libc/pbsd.userland.libc.gen.cppm` | In-place libgen helpers |
| libutil uucplock | `libutil/uucplock.c` | `pbsd/userland/util/pbsd.userland.util.uucplock.cppm` | Lock codes + uu_lockerr |
| libutil login_times | `libutil/login_times.c` | `pbsd/userland/util/pbsd.userland.util.login_times.cppm` | parse_lt/in_ltm |

### Wave 2 port stubs (hand-port batch, 18 TU)

`convert_c_batch --wave wave2` for `strchr`, `strrchr`, `strcat`, `strcpy`, `strncpy`, `memcmp`, `memchr`, `strlcpy`, `strnlen`, `strchrnul`, `atol`, `strtoul`, `abs`, `labs`, `qsort`, `bsearch`, `dirname`, `basename` under `pbsd/ports/wave2/hbsd/src/lib/libc/`.

### Harness additions

- `test_libc_memory_compare`, `test_libc_sort`, `test_libc_gen`
- Expanded `test_libc_string`, `test_libc_stdlib`
- `test_util_uucplock`, `test_util_login_times`

### CMake modules added

`pbsd.userland.libc.string.search`, `pbsd.userland.libc.string.copy`, `pbsd.userland.libc.stdlib.sort`, `pbsd.userland.libc.gen`; `pbsd.userland.util.uucplock`, `pbsd.userland.util.login_times`; `pbsd_userland_util` links `pbsd_userland_libc`.

## Wave 2 — usr.bin/sbin/bin burst 6 (2026-07-19, MAX throughput)

Hand ports of 16 new utilities plus wiring for 20 previously unlinked modules under `pbsd/userland/`.

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| tr | `hbsd/src/usr.bin/tr/tr.c` | `pbsd/userland/usr.bin/pbsd.userland.tr.cppm` | Map table, -cds parse |
| cut | `hbsd/src/usr.bin/cut/cut.c` | `pbsd/userland/usr.bin/pbsd.userland.cut.cppm` | Field list bitmask |
| sort | `hbsd/src/usr.bin/sort/sort.c` | `pbsd/userland/usr.bin/pbsd.userland.sort.cppm` | -cunMfhRs flags |
| uniq | `hbsd/src/usr.bin/uniq/uniq.c` | `pbsd/userland/usr.bin/pbsd.userland.uniq.cppm` | -cdu, skip -f/-s |
| cmp | `hbsd/src/usr.bin/cmp/cmp.c` | `pbsd/userland/usr.bin/pbsd.userland.cmp.cppm` | -ils, skip spec |
| comm | `hbsd/src/usr.bin/comm/comm.c` | `pbsd/userland/usr.bin/pbsd.userland.comm.cppm` | Column suppress |
| jot | `hbsd/src/usr.bin/jot/jot.c` | `pbsd/userland/usr.bin/pbsd.userland.jot.cppm` | -bcnr, getprec |
| logger | `hbsd/src/usr.bin/logger/logger.c` | `pbsd/userland/usr.bin/pbsd.userland.logger.cppm` | Priority name table |
| script | `hbsd/src/usr.bin/script/script.c` | `pbsd/userland/usr.bin/pbsd.userland.script.cppm` | -Ffqre stamp |
| calendar | `hbsd/src/usr.bin/calendar/calendar.c` | `pbsd/userland/usr.bin/pbsd.userland.calendar.cppm` | Month token parse |
| du | `hbsd/src/usr.bin/du/du.c` | `pbsd/userland/usr.bin/pbsd.userland.du.cppm` | -ahksx, human suffix |
| find | `hbsd/src/usr.bin/find/find.c` | `pbsd/userland/usr.bin/pbsd.userland.find.cppm` | Primary/operator scaffold |
| xargs | `hbsd/src/usr.bin/xargs/xargs.c` | `pbsd/userland/usr.bin/pbsd.userland.xargs.cppm` | -0ILJnrs |
| grep | `hbsd/src/usr.bin/grep/grep.c` | `pbsd/userland/usr.bin/pbsd.userland.grep.cppm` | Fixed-string match helper |
| sed | `hbsd/src/usr.bin/sed/sed.c` | `pbsd/userland/usr.bin/pbsd.userland.sed.cppm` | Command/address scaffold |
| awk | `hbsd/src/usr.bin/awk/` | `pbsd/userland/usr.bin/pbsd.userland.awk.cppm` | Field-split stub |
| patch | `hbsd/src/usr.bin/patch/patch.c` | `pbsd/userland/usr.bin/pbsd.userland.patch.cppm` | Hunk header parse |
| gzip | `hbsd/src/usr.bin/gzip/gzip.c` | `pbsd/userland/usr.bin/pbsd.userland.gzip.cppm` | Magic + level flags |
| tar | `hbsd/src/bin/tar/` | `pbsd/userland/bin/pbsd.userland.tar.cppm` | c/x/t/r op parse |
| ssh | `hbsd/src/usr.bin/ssh/` | `pbsd/userland/usr.bin/pbsd.userland.ssh.cppm` | Client flag stub |
| fetch | `hbsd/src/usr.bin/fetch/` | `pbsd/userland/usr.bin/pbsd.userland.fetch.cppm` | URL scheme helper |
| pkg | `hbsd/src/usr.sbin/pkg/` | `pbsd/userland/sbin/pbsd.userland.pkg_cmd.cppm` | Subcommand registry |
| newsyslog | `hbsd/src/usr.sbin/newsyslog/` | `pbsd/userland/sbin/pbsd.userland.newsyslog.cppm` | Conf-line parse |
| cron | `hbsd/src/usr.sbin/cron/` | `pbsd/userland/sbin/pbsd.userland.cron.cppm` | Schedule field scaffold |
| pw | `hbsd/src/usr.sbin/pw/` | `pbsd/userland/sbin/pbsd.userland.pw.cppm` | useradd/mod subcmds |
| vipw | `hbsd/src/usr.sbin/vipw/` | `pbsd/userland/sbin/pbsd.userland.vipw.cppm` | Passwd line validate |
| diff | `hbsd/src/usr.bin/diff/diff.c` | `pbsd/userland/usr.bin/pbsd.userland.diff.cppm` | Algorithm select |
| join/paste/nl/fold/split/expand/tsort/rev | `hbsd/src/usr.bin/*` | `pbsd/userland/usr.bin/pbsd.userland.*.cppm` | Text pipeline helpers |
| umount/ping | `hbsd/src/sbin/` | `pbsd/userland/sbin/pbsd.userland.*.cppm` | sbin flag parse |

**New modules this pass:** 16 (`du`, `find`, `xargs`, `grep`, `sed`, `awk`, `patch`, `gzip`, `tar`, `ssh`, `fetch`, `pkg_cmd`, `newsyslog`, `cron`, `pw`, `vipw`).

### Wave 2 port stubs (`convert_c_batch --wave wave2`)

- `--prefix hbsd/src/usr.bin/ --limit 30`
- `--prefix hbsd/src/sbin/ --limit 15`
- `--prefix hbsd/src/bin/ --limit 15`

### Harness additions

`test_usr_bin_wave2_burst6`, extended `test_sbin_wave2` in `pbsd/tests/userland_harness.cpp`.

### CMake modules added

36 targets wired into `pbsd_userland_usr_bin`, `pbsd_userland_sbin`, `pbsd_userland_bin` INTERFACE aggregates.

## Wave 2 — libc/msun expansion (2026-07-19, MAX throughput)

Hand ports of 22 freestanding `hbsd/src/lib/libc` modules + 4 tiny `hbsd/src/lib/msun` helpers.

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| libc string span | `string/{strspn,strcspn}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.span.cppm` | Bitmap charset scan |
| libc string find | `string/strpbrk.c` | `pbsd/userland/libc/pbsd.userland.libc.string.find.cppm` | First-of-set scan |
| libc string search ext | `string/{strstr,strnstr}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.search.ext.cppm` | Substring search |
| libc string case | `string/strcasecmp.c` | `pbsd/userland/libc/pbsd.userland.libc.string.case.cppm` | C-locale case compare |
| libc string concat | `string/{strncat,strlcat}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.concat.cppm` | Bounded concat |
| libc string stp | `string/{stpcpy,stpncpy}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.stp.cppm` | Stp copy helpers |
| libc string bcmp | `string/bcmp.c` | `pbsd/userland/libc/pbsd.userland.libc.string.bcmp.cppm` | Legacy byte compare |
| libc string memext | `string/{memrchr,memmem}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.memext.cppm` | Reverse/sub-buffer search |
| libc string bit | `string/{ffs,ffsl,fls,flsl}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.bit.cppm` | Find first/last set |
| libc string secure | `string/timingsafe_{bcmp,memcmp}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.secure.cppm` | Constant-time compare |
| libc string token | `string/strsep.c` | `pbsd/userland/libc/pbsd.userland.libc.string.token.cppm` | Delimiter tokenize |
| libc string swab | `string/swab.c` | `pbsd/userland/libc/pbsd.userland.libc.string.swab.cppm` | 16-bit byte swap |
| libc string memccpy | `string/memccpy.c` | `pbsd/userland/libc/pbsd.userland.libc.string.memccpy.cppm` | Copy until char |
| libc string strverscmp | `string/strverscmp.c` | `pbsd/userland/libc/pbsd.userland.libc.string.strverscmp.cppm` | Version sort compare |
| libc string explicit | `string/memset_explicit.c` | `pbsd/userland/libc/pbsd.userland.libc.string.explicit.cppm` | Volatile zero fill |
| libc stdlib convert ext | `stdlib/{strtoll,strtoull}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.convert.ext.cppm` | 64-bit strtol family |
| libc stdlib div | `stdlib/{div,ldiv,lldiv}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.div.cppm` | Quotient/remainder tuples |
| libc stdlib rand | `stdlib/rand.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.rand.cppm` | Park–Miller LCG |
| libc stdlib strtod | `stdlib/strtod.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.strtod.cppm` | Decimal parse subset |
| libc stdlib imax | `stdlib/imaxabs.c` + stdint helpers | `pbsd/userland/libc/pbsd.userland.libc.stdlib.imax.cppm` | imaxabs/min/max |
| libc ctype | `locale/ctype.c` | `pbsd/userland/libc/pbsd.userland.libc.ctype.cppm` | C-locale wrappers |
| libc gen ftok | `gen/ftok.c` | `pbsd/userland/libc/pbsd.userland.libc.gen.ftok.cppm` | Key compose (logic-only) |
| msun fabs | `msun/src/s_fabs.c` | `pbsd/userland/msun/pbsd.userland.msun.fabs.cppm` | IEEE754 sign clear |
| msun ceil | `msun/src/s_ceil.c` | `pbsd/userland/msun/pbsd.userland.msun.ceil.cppm` | Bit-twiddle round up |
| msun floor | `msun/src/s_floor.c` | `pbsd/userland/msun/pbsd.userland.msun.floor.cppm` | Bit-twiddle round down |
| msun sqrt | `msun/src/e_sqrt.c` | `pbsd/userland/msun/pbsd.userland.msun.sqrt.cppm` | Newton sqrt (portable) |
| msun umbrella | — | `pbsd/userland/msun/pbsd.userland.msun.cppm` | Re-exports tiny libm |

### Wave 2 port stubs (`convert_c_batch --wave wave2 --prefix hbsd/src/lib/libc/ --prefix hbsd/src/lib/msun/src/ --limit 120`)

62 new stubs under `pbsd/ports/wave2/hbsd/src/lib/libc/` and `pbsd/ports/wave2/hbsd/src/lib/msun/src/`.

### Harness additions

- `test_libc_wave2_burst` — span/find/search/case/concat/stp/bcmp/memext/bit/secure/token/swab/memccpy/strverscmp/convert/div/rand/strtod/ctype/imax/explicit
- `test_msun_tiny` — fabs/ceil/floor/sqrt smoke

### CMake modules added

`pbsd_userland_libc` extended with 22 partitions; new `pbsd_userland_msun`; aggregate `pbsd_userland` links both.

## Wave 2 — usr.bin / sbin hand ports (2026-07-19, burst 6)

Hand ports of 20 `hbsd/src/usr.bin/*` text utilities and 2 `hbsd/src/sbin/*` parse stubs into `pbsd/userland/usr.bin/` and `pbsd/userland/sbin/`.

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| tr | `hbsd/src/usr.bin/tr/tr.c` | `pbsd/userland/usr.bin/pbsd.userland.tr.cppm` | `-Ccdsu`, string count validation |
| cut | `hbsd/src/usr.bin/cut/cut.c` | `pbsd/userland/usr.bin/pbsd.userland.cut.cppm` | `-bcdfnsw`, mode conflict checks |
| sort | `hbsd/src/usr.bin/sort/sort.c` | `pbsd/userland/usr.bin/pbsd.userland.sort.cppm` | Flag/key helpers, `-cu` conflict |
| uniq | `hbsd/src/usr.bin/uniq/uniq.c` | `pbsd/userland/usr.bin/pbsd.userland.uniq.cppm` | `-D::cdifu`, dup format enum |
| comm | `hbsd/src/usr.bin/comm/comm.c` | `pbsd/userland/usr.bin/pbsd.userland.comm.cppm` | `-123i`, tab offset helper |
| cmp | `hbsd/src/usr.bin/cmp/cmp.c` | `pbsd/userland/usr.bin/pbsd.userland.cmp.cppm` | `-blns`, `parse_iskipspec` |
| diff | `hbsd/src/usr.bin/diff/diff.c` | `pbsd/userland/usr.bin/pbsd.userland.diff.cppm` | Algorithm names, context flags |
| seq | `hbsd/src/usr.bin/seq/seq.c` | `pbsd/userland/usr.bin/pbsd.userland.seq.cppm` | `-fwst`, numeric token check |
| jot | `hbsd/src/usr.bin/jot/jot.c` | `pbsd/userland/usr.bin/pbsd.userland.jot.cppm` | `-bcnprsw`, `getprec` |
| calendar | `hbsd/src/usr.bin/calendar/calendar.c` | `pbsd/userland/usr.bin/pbsd.userland.calendar.cppm` | `-aABDFflt`, month token stub |
| logger | `hbsd/src/usr.bin/logger/logger.c` | `pbsd/userland/usr.bin/pbsd.userland.logger.cppm` | Priority decode, `-ptfh` |
| script | `hbsd/src/usr.bin/script/script.c` | `pbsd/userland/usr.bin/pbsd.userland.script.cppm` | `-FfqreTa`, stamp direction |
| join | `hbsd/src/usr.bin/join/join.c` | `pbsd/userland/usr.bin/pbsd.userland.join.cppm` | `-12teoi`, default fields |
| paste | `hbsd/src/usr.bin/paste/paste.c` | `pbsd/userland/usr.bin/pbsd.userland.paste.cppm` | `-sd`, sequential mode |
| nl | `hbsd/src/usr.bin/nl/nl.c` | `pbsd/userland/usr.bin/pbsd.userland.nl.cppm` | `-bfhps`, numbering types |
| fold | `hbsd/src/usr.bin/fold/fold.c` | `pbsd/userland/usr.bin/pbsd.userland.fold.cppm` | `-bsw`, default width 80 |
| split | `hbsd/src/usr.bin/split/split.c` | `pbsd/userland/usr.bin/pbsd.userland.split.cppm` | `-abClC`, expand_number |
| expand | `hbsd/src/usr.bin/expand/expand.c` | `pbsd/userland/usr.bin/pbsd.userland.expand.cppm` | Tabstop parse, `next_tab` |
| tsort | `hbsd/src/usr.bin/tsort/tsort.c` | `pbsd/userland/usr.bin/pbsd.userland.tsort.cppm` | `-dlq`, edge token check |
| rev | `hbsd/src/usr.bin/rev/rev.c` | `pbsd/userland/usr.bin/pbsd.userland.rev.cppm` | No flags, line length |
| umount | `hbsd/src/sbin/umount/umount.c` | `pbsd/userland/sbin/pbsd.userland.umount.cppm` | `-AadF:fhNntv`, force flags |
| ping | `hbsd/src/sbin/ping/ping.c` | `pbsd/userland/sbin/pbsd.userland.ping.cppm` | `-4AcdfinmqsvW`, count/size parse |

### Wave 2 port stubs (`convert_c_batch --wave wave2 --skip-stubbed`)

- `--prefix hbsd/src/usr.bin/ --limit 40` (remaining usr.bin stubs)
- `--prefix hbsd/src/sbin/ --limit 20` (remaining sbin stubs)

### Harness additions

`test_usr_bin_wave2_burst6`, extended `test_sbin_wave2` (`umount`, `ping`) in `pbsd/tests/userland_harness.cpp`.

### CMake modules added

`pbsd_userland_tr`, `pbsd_userland_cut`, `pbsd_userland_sort`, `pbsd_userland_uniq`, `pbsd_userland_comm`, `pbsd_userland_cmp`, `pbsd_userland_diff`, `pbsd_userland_seq`, `pbsd_userland_jot`, `pbsd_userland_calendar`, `pbsd_userland_logger`, `pbsd_userland_script`, `pbsd_userland_join`, `pbsd_userland_paste`, `pbsd_userland_nl`, `pbsd_userland_fold`, `pbsd_userland_split`, `pbsd_userland_expand`, `pbsd_userland_tsort`, `pbsd_userland_rev`, `pbsd_userland_umount`, `pbsd_userland_ping`; extended `pbsd_userland_usr_bin` / `pbsd_userland_sbin` INTERFACE aggregates.
## Wave 2 — Userland batch ports (2026-07-19)

| hbsd source | PBSD module / stub | Notes |
|-------------|-------------------|-------|
| `hbsd/src/lib/libc/string/strspn.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strcspn.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strpbrk.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strstr.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strnstr.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strcasecmp.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strncat.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strlcat.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/stpcpy.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/stpncpy.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/bcmp.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/memrchr.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/memmem.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/ffs.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/fls.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/timingsafe_memcmp.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/swab.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/memccpy.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strverscmp.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/memset_explicit.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdlib/strtoll.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdlib/strtoull.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdlib/div.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdlib/rand.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdlib/strtod.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdlib/imaxabs.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/gen/ftok.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/msun/src/s_fabs.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/msun/src/s_ceil.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/msun/src/s_floor.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/msun/src/e_sqrt.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/strsep.c` | `(hand port)` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/string/timingsafe_bcmp.c` | `pbsd.port.wave2.hbsd.src.lib.libc.string.timingsafe_bcmp` | hand port wave2 libc/msun burst |
| `hbsd/src/lib/libc/stdbit/stdc_count_ones.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libc/string/ffsll.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libc/string/wcpcpy.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libc/stdlib/insque.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libc/stdlib/a64l.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libc/gen/uname.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libc/uuid/uuid_create_nil.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/msun/src/s_ceilf.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/msun/src/s_fdim.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libthr/thread/thr_spinlock.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/lib/libthr/thread/thr_barrier.c` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/libexec/rtld-elf/Symbol.map` | `(hand port)` | hand port wave2 burst 8 |
| `hbsd/src/usr.bin/true/true.c` | `pbsd.userland.usr_bin.true` | usr.bin true(1) re-exports bin/true_cmd |
| `hbsd/src/usr.bin/false/false.c` | `pbsd.userland.usr_bin.false` | usr.bin false(1) re-exports bin/false_cmd |
| `hbsd/src/usr.bin/getopt/getopt.c` | `pbsd.userland.getopt` | getopt(1) parse/format helpers |
| `hbsd/src/bin/echo/echo.c` | `pbsd.userland.echo` | hand port in pbsd/userland/bin |

## Wave 2 — libc stdio/locale/stdlib + libthr + rtld (2026-07-19, burst 7)

Hand ports of 37 libc partitions, 7 libthr pthread concept modules, and 5 rtld dynamic-linker symbol stubs.

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| libc stdio | `stdio/{feof,ferror,fileno,putc,getc,...}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdio*.cppm` | 18 IoFile concept helpers |
| libc ctype ext | `locale/ctype.c` | `pbsd/userland/libc/pbsd.userland.libc.ctype.ext.cppm` | iscntrl/ispunct/isprint/isgraph/isblank |
| libc locale | `locale/{setlocale,localeconv,mblen,...}.c` | `pbsd/userland/libc/pbsd.userland.libc.locale*.cppm` | C-locale stubs (8 modules) |
| libc stdlib | `stdlib/{atof,strtonum,getopt,getenv,...}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.*.cppm` | 11 remaining stdlib helpers |
| libthr mutex | `libthr/thread/thr_mutex.c` | `pbsd/userland/libthr/pbsd.userland.libthr.mutex.cppm` | Mutex/lock/unlock/trylock |
| libthr cond | `libthr/thread/thr_cond.c` | `pbsd/userland/libthr/pbsd.userland.libthr.cond.cppm` | Cond wait/signal/broadcast |
| libthr thread | `libthr/thread/thr_{create,self,join}.c` | `pbsd/userland/libthr/pbsd.userland.libthr.thread.cppm` | ThreadId + attr concepts |
| libthr once | `libthr/thread/thr_once.c` | `pbsd/userland/libthr/pbsd.userland.libthr.once.cppm` | pthread_once stub |
| libthr key | `libthr/thread/thr_spec.c` | `pbsd/userland/libthr/pbsd.userland.libthr.key.cppm` | TSS key create/set/get |
| libthr equal | `libthr/thread/thr_equal.c` | `pbsd/userland/libthr/pbsd.userland.libthr.equal.cppm` | thread_equal |
| libthr symbols | `libthr/thread/thr_symbols.c` | `pbsd/userland/libthr/pbsd.userland.libthr.symbols.cppm` | Debugger offset constants |
| libthr umbrella | — | `pbsd/userland/libthr/pbsd.userland.libthr.cppm` | Re-exports 7 partitions |
| rtld symbols | `libexec/rtld-elf/Symbol.map` | `pbsd/userland/rtld/pbsd.userland.rtld.symbols.cppm` | Public/private symbol table |
| rtld dl | `libexec/rtld-elf/rtld.c` | `pbsd/userland/rtld/pbsd.userland.rtld.dl.cppm` | dlopen/dlsym/dlclose concepts |
| rtld linkmap | `libexec/rtld-elf/map_object.c` | `pbsd/userland/rtld/pbsd.userland.rtld.linkmap.cppm` | LinkMap/ObjEntry struct |
| rtld paths | `libexec/rtld-elf/rtld_paths.h` | `pbsd/userland/rtld/pbsd.userland.rtld.paths.cppm` | Default lib/rtld paths |
| rtld version | `libexec/rtld-elf/rtld.c` | `pbsd/userland/rtld/pbsd.userland.rtld.version.cppm` | FreeBSD version constants |
| rtld umbrella | — | `pbsd/userland/rtld/pbsd.userland.rtld.cppm` | Re-exports rtld partitions |

### Wave 2 port stubs (`convert_c_batch --wave wave2 --skip-stubbed`)

- `--prefix hbsd/src/lib/libc/stdio/ --prefix hbsd/src/lib/libc/locale/ --prefix hbsd/src/lib/libthr/ --prefix hbsd/src/libexec/rtld-elf/ --limit 80` → 74 stubs

### Harness additions

`test_libc_wave2_burst_b6`, `test_libthr_rtld_burst_b6` in `pbsd/tests/userland_harness.cpp`.

### CMake modules added

`pbsd_userland_libc` extended with 37 partitions; new `pbsd_userland_libthr`, `pbsd_userland_rtld`; aggregate `pbsd_userland` links both.

## Wave 2 — usr.bin / sbin / usr.sbin burst 8 (2026-07-19, MAX throughput)

Hand ports of 38 priority utilities + 8 pipeline helpers into `pbsd/userland/` (logic-only parse/subcommand scaffolds).

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| bzip2 / xz / compress | `usr.bin/{bzip2,xz,compress}/` | `pbsd/userland/usr.bin/pbsd.userland.*.cppm` | Flag/magic helpers |
| curl / scp / rsync | `contrib/` + `crypto/openssh/scp.c` | `pbsd/userland/usr.bin/pbsd.userland.{curl,scp,rsync}.cppm` | Network stubs |
| hexdump / stat / cksum / netstat / sockstat | `usr.bin/*` | `pbsd/userland/usr.bin/pbsd.userland.*.cppm` | Flag parse |
| sysrc / service / services | `usr.sbin/{sysrc,service,services_mkdb}` | `pbsd/userland/usr.sbin/pbsd.userland.*.cppm` | Subcommand/flag scaffolds |
| bectl / camcontrol / geom / gpart | `sbin/{bectl,camcontrol,geom}/` | `pbsd/userland/sbin/pbsd.userland.*.cppm` | Subcommand registry |
| mdconfig / dumpon / savecore | `sbin/*` | `pbsd/userland/sbin/pbsd.userland.*.cppm` | Attach/dump helpers |
| fsck / fsck_ffs / fsck_msdosfs | `sbin/fsck*` | `pbsd/userland/sbin/pbsd.userland.fsck*.cppm` | Preen/type flags |
| newfs / newfs_msdos / tunefs / growfs | `sbin/*` | `pbsd/userland/sbin/pbsd.userland.*.cppm` | mkfs/tune scaffolds |
| zfs / zpool | `sys/contrib/openzfs/cmd/` | `pbsd/userland/sbin/pbsd.userland.{zfs,zpool}.cppm` | CLI verb tables |
| dmesg / dumpfs / fdisk / bsdlabel | `sbin/*` | `pbsd/userland/sbin/pbsd.userland.*.cppm` | Flag parse |
| swapon / shutdown / reboot / kldload | `sbin/*` | `pbsd/userland/sbin/pbsd.userland.*.cppm` | Admin flag parse |
| find/grep/sed/awk/xargs helpers | `usr.bin/*` | `pbsd/userland/usr.bin/pbsd.userland.*.{helpers,context,addr,fields,quote}.cppm` | Pipeline sub-modules |
| periodic / syslogd / rcorder | `usr.sbin/*`, `sbin/rcorder` | helper modules | Run/facility/dependency parse |

**New modules this pass:** 46 (38 utilities + 8 helpers). **Cumulative userland `.cppm` count:** 361.

### Wave 2 port stubs (`convert_c_batch --wave wave2 --skip-stubbed`)

- `--prefix hbsd/src/usr.bin/ --prefix hbsd/src/sbin/ --prefix hbsd/src/usr.sbin/ --prefix hbsd/src/crypto/openssh/ --prefix hbsd/src/sys/contrib/openzfs/cmd/ --limit 50` → incremental (660 stubbed cumulative wave2)

### Harness additions

`test_usr_bin_wave2_burst8`, `test_sbin_wave2_burst8` in `pbsd/tests/userland_harness.cpp`.

### CMake targets added

`pbsd_userland_{bzip2,xz,curl,scp,rsync,compress,hexdump,stat,cksum,netstat,sockstat,sysrc,service,services,bectl,camcontrol,geom,gpart,mdconfig,dumpon,savecore,fsck_ffs,fsck_msdosfs,newfs_msdos,tunefs,growfs,dmesg,dumpfs,fdisk,bsdlabel,swapon,shutdown,reboot,kldload}` plus burst-8 helper targets via `PBSD_UL_B8_*` aggregate lists in `pbsd/userland/CMakeLists.txt`.

---

## Wave 2 — libc/msun/libthr/rtld expansion (2026-07-19, burst 8)

Hand ports of 30 libc partitions, 10 msun/libm helpers, 8 libthr pthread modules, and expanded rtld symbol version tables.

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| libc stdbit | `stdbit/stdc_count_{ones,zeros}.c`, `stdc_has_single_bit.c` | `pbsd/userland/libc/pbsd.userland.libc.stdbit.cppm` | C23 popcount helpers |
| libc string bit ext | `string/{ffsll,flsll}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.bit.ext.cppm` | 64-bit ffs/fls |
| libc wchar | `string/{wcpcpy,wcpncpy,wmemmove,wmemcpy,wcscasecmp,wcsncasecmp,wcslen,wcsrchr}.c` | `pbsd/userland/libc/pbsd.userland.libc.string.wchar.*.cppm` | 4 wchar partitions |
| libc stdlib queue | `stdlib/{insque,remque}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.insque.cppm` | Circular queue insert/remove |
| libc stdlib tree | `stdlib/tree.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.tree.cppm` | tfind/tsearch/twalk/tdelete |
| libc stdlib a64l | `stdlib/{a64l,l64a}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.a64l.cppm` | Radix-64 encode/decode |
| libc stdlib strtoq | `stdlib/{strtoq,strtouq}.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.strtoq.cppm` | strtoll alias wrappers |
| libc stdlib reallocf | `stdlib/reallocf.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.reallocf.cppm` | Free-on-failure realloc |
| libc gen | `gen/{assert,time,uname,isatty,raise,arc4random,hostname,opendir,dirfd,getbootfile,utime,vis,cap_sandboxed,getgrouplist}.c` | `pbsd/userland/libc/pbsd.userland.libc.gen.*.cppm` | 14 gen partitions |
| libc uuid | `uuid/{uuid_create,uuid_create_nil,uuid_hash}.c` | `pbsd/userland/libc/pbsd.userland.libc.uuid.cppm` | Nil/hash helpers |
| libc net | `net/{sockatmark,vars}.c` | `pbsd/userland/libc/pbsd.userland.libc.net.*.cppm` | h_errno + sockatmark |
| libc hash | `db/hash/hash_log2.c` | `pbsd/userland/libc/pbsd.userland.libc.hash.log2.cppm` | Log2 bucket sizing |
| libc gdtoa | `gdtoa/machdep_ldisd.c` | `pbsd/userland/libc/pbsd.userland.libc.gdtoa.ldis.cppm` | IEEE exponent extract |
| msun ceilf | `msun/src/s_ceilf.c` | `pbsd/userland/msun/pbsd.userland.msun.ceilf.cppm` | Float ceil bit-twiddle |
| msun floorf | `msun/src/s_floorf.c` | `pbsd/userland/msun/pbsd.userland.msun.floorf.cppm` | Float floor bit-twiddle |
| msun frexp | `msun/src/s_frexp.c` | `pbsd/userland/msun/pbsd.userland.msun.frexp.cppm` | frexp/frexpf |
| msun scalbn | `msun/src/s_scalbn.c` | `pbsd/userland/msun/pbsd.userland.msun.scalbn.cppm` | ldexp wrappers |
| msun truncf | `msun/src/s_truncf.c` | `pbsd/userland/msun/pbsd.userland.msun.truncf.cppm` | trunc via ceil/floor |
| msun roundf | `msun/src/s_roundf.c` | `pbsd/userland/msun/pbsd.userland.msun.roundf.cppm` | std::round wrapper |
| msun rintf | `msun/src/s_rintf.c` | `pbsd/userland/msun/pbsd.userland.msun.rintf.cppm` | nearbyint wrapper |
| msun signbit | `msun/src/s_signbit.c` | `pbsd/userland/msun/pbsd.userland.msun.signbit.cppm` | Sign bit extract |
| msun isfinite | `msun/src/s_isfinite.c` | `pbsd/userland/msun/pbsd.userland.msun.isfinite.cppm` | Finite test |
| msun fdim | `msun/src/s_fdim.c` | `pbsd/userland/msun/pbsd.userland.msun.fdim.cppm` | Positive difference |
| msun umbrella | — | `pbsd/userland/msun/pbsd.userland.msun.cppm` | Re-exports 14 libm modules |
| libthr spinlock | `libthr/thread/thr_spinlock.c` | `pbsd/userland/libthr/pbsd.userland.libthr.spinlock.cppm` | SpinLock lock/unlock |
| libthr sem | `libthr/thread/thr_sem.c` | `pbsd/userland/libthr/pbsd.userland.libthr.sem.cppm` | Semaphore post/wait |
| libthr detach | `libthr/thread/thr_detach.c` | `pbsd/userland/libthr/pbsd.userland.libthr.detach.cppm` | thread_detach |
| libthr kill | `libthr/thread/thr_kill.c` | `pbsd/userland/libthr/pbsd.userland.libthr.kill.cppm` | thread_kill stub |
| libthr barrier | `libthr/thread/thr_barrier.c` | `pbsd/userland/libthr/pbsd.userland.libthr.barrier.cppm` | Barrier wait |
| libthr mutexattr | `libthr/thread/thr_mutexattr.c` | `pbsd/userland/libthr/pbsd.userland.libthr.mutexattr.cppm` | MutexAttr type |
| libthr condattr | `libthr/thread/thr_condattr.c` | `pbsd/userland/libthr/pbsd.userland.libthr.condattr.cppm` | CondAttr pshared |
| libthr rwlock | `libthr/thread/thr_rwlock*.c` | `pbsd/userland/libthr/pbsd.userland.libthr.rwlock.cppm` | RwLock rd/wr lock |
| libthr umbrella | — | `pbsd/userland/libthr/pbsd.userland.libthr.cppm` | Re-exports 15 partitions |
| rtld symbols | `libexec/rtld-elf/Symbol.map` | `pbsd/userland/rtld/pbsd.userland.rtld.symbols.cppm` | Version-set tables + lookup |

### Wave 2 port stubs (`convert_c_batch --wave wave2 --skip-stubbed`)

- `--prefix hbsd/src/lib/libc/stdbit/ --prefix hbsd/src/lib/libc/string/w* --prefix hbsd/src/lib/libc/stdlib/ --prefix hbsd/src/lib/libc/gen/ --prefix hbsd/src/lib/libc/uuid/ --prefix hbsd/src/lib/msun/src/ --prefix hbsd/src/lib/libthr/thread/thr_* --limit 120` → 23+ stubs

### Harness additions

`test_libc_msun_libthr_burst_b8` in `pbsd/tests/userland_harness.cpp`; extended `test_libthr_rtld_burst_b6` with version-set lookup.

### CMake modules added

`pbsd_userland_libc` +30 partitions; `pbsd_userland_msun` +10; `pbsd_userland_libthr` +8; aggregate `pbsd_userland` links `pbsd_userland_msun`.

## Wave 2 — usr.bin/sbin hand ports (2026-07-19, burst 8)

Hand ports of **38** utilities and helper partitions into `pbsd/userland/` (find/grep/sed/awk/xargs helpers, compression, network stubs, rc.d/sysrc/service, ZFS/ZPOOL wrappers).

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| find helpers | `hbsd/src/usr.bin/find/operator.c` | `pbsd/userland/usr.bin/pbsd.userland.find.helpers.cppm` | Boolean ops `-a/-o/!` |
| grep context | `hbsd/src/usr.bin/grep/grep.c` | `pbsd/userland/usr.bin/pbsd.userland.grep.context.cppm` | `-A/-B/-C` context |
| sed addr | `hbsd/src/usr.bin/sed/addr.c` | `pbsd/userland/usr.bin/pbsd.userland.sed.addr.cppm` | Address range parse |
| awk fields | `hbsd/src/usr.bin/awk/` | `pbsd/userland/usr.bin/pbsd.userland.awk.fields.cppm` | NF / field split |
| xargs quote | `hbsd/src/usr.bin/xargs/xargs.c` | `pbsd/userland/usr.bin/pbsd.userland.xargs.quote.cppm` | Shell quoting |
| bzip2 | `hbsd/src/usr.bin/bzip2/` | `pbsd/userland/usr.bin/pbsd.userland.bzip2.cppm` | Magic + level flags |
| compress | `hbsd/src/usr.bin/compress/` | `pbsd/userland/usr.bin/pbsd.userland.compress.cppm` | Legacy `.Z` |
| cpio | `hbsd/src/usr.bin/cpio/` | `pbsd/userland/usr.bin/pbsd.userland.cpio.cppm` | `-o/-i/-p` modes |
| xz | `hbsd/src/usr.bin/xz/` | `pbsd/userland/usr.bin/pbsd.userland.xz.cppm` | XZ magic + `-d/-k/-c` |
| curl | `hbsd/src/usr.bin/fetch/` (stub) | `pbsd/userland/usr.bin/pbsd.userland.curl.cppm` | HTTP client flag subset |
| scp | `hbsd/src/usr.bin/ssh/scp.c` | `pbsd/userland/usr.bin/pbsd.userland.scp.cppm` | `user@host:path` parse |
| ftp | `hbsd/src/usr.bin/ftp/` | `pbsd/userland/usr.bin/pbsd.userland.ftp.cppm` | Passive/active flags |
| file | `hbsd/src/usr.bin/file/` | `pbsd/userland/usr.bin/pbsd.userland.file.cppm` | ELF sniff + `-b/-i/-L` |
| hexdump | `hbsd/src/usr.bin/hexdump/` | `pbsd/userland/usr.bin/pbsd.userland.hexdump.cppm` | `-C` canonical |
| stat | `hbsd/src/usr.bin/stat/` | `pbsd/userland/usr.bin/pbsd.userland.stat.cppm` | `-f` format |
| strings | `hbsd/src/usr.bin/strings/` | `pbsd/userland/usr.bin/pbsd.userland.strings.cppm` | Min-length scan |
| cksum | `hbsd/src/usr.bin/cksum/` | `pbsd/userland/usr.bin/pbsd.userland.cksum.cppm` | POSIX polynomial |
| netstat | `hbsd/src/usr.bin/netstat/` | `pbsd/userland/usr.bin/pbsd.userland.netstat.cppm` | `-anp` parse |
| sockstat | `hbsd/src/usr.bin/sockstat/` | `pbsd/userland/usr.bin/pbsd.userland.sockstat.cppm` | `-4/-6/-l/-u` |
| vmstat | `hbsd/src/usr.bin/vmstat/` | `pbsd/userland/usr.bin/pbsd.userland.vmstat.cppm` | Interval/count |
| who | `hbsd/src/usr.bin/who/` | `pbsd/userland/usr.bin/pbsd.userland.who.cppm` | `-H/-q/-T` |
| w | `hbsd/src/usr.bin/w/` | `pbsd/userland/usr.bin/pbsd.userland.w_cmd.cppm` | Snapshot flags |
| ldd | `hbsd/src/usr.bin/ldd/` | `pbsd/userland/usr.bin/pbsd.userland.ldd.cppm` | Shared-object detect |
| iconv | `hbsd/src/usr.bin/iconv/` | `pbsd/userland/usr.bin/pbsd.userland.iconv.cppm` | `-f/-t` codesets |
| service | `hbsd/src/usr.sbin/service/` | `pbsd/userland/usr.sbin/pbsd.userland.service.cppm` | rc.d verbs |
| services | `hbsd/src/usr.sbin/services_mkdb/` | `pbsd/userland/usr.sbin/pbsd.userland.services.cppm` | services(5) triple |
| sysrc | `hbsd/src/usr.sbin/sysrc/` | `pbsd/userland/usr.sbin/pbsd.userland.sysrc.cppm` | rc.conf get/set |
| periodic | `hbsd/src/usr.sbin/periodic/` | `pbsd/userland/usr.sbin/pbsd.userland.periodic.cppm` | daily/weekly/monthly |
| syslogd | `hbsd/src/usr.sbin/syslogd/` | `pbsd/userland/usr.sbin/pbsd.userland.syslogd.cppm` | `-d/-F/-f/-s` |
| rcorder | `hbsd/src/sbin/rcorder/` | `pbsd/userland/sbin/pbsd.userland.rcorder.cppm` | PROVIDE/REQUIRE |
| bectl | `hbsd/src/sbin/bectl/` | `pbsd/userland/sbin/pbsd.userland.bectl.cppm` | Boot env subcmds |
| zfs | `hbsd/src/cddl/sbin/zfs/` | `pbsd/userland/sbin/pbsd.userland.zfs.cppm` | ZFS verb registry |
| zpool | `hbsd/src/cddl/sbin/zpool/` | `pbsd/userland/sbin/pbsd.userland.zpool.cppm` | ZPOOL verb registry |
| md5 | `hbsd/src/sbin/md5/` | `pbsd/userland/sbin/pbsd.userland.md5.cppm` | `-c/-p/-q` |
| fsck | `hbsd/src/sbin/fsck/` | `pbsd/userland/sbin/pbsd.userland.fsck.cppm` | Preen/force |
| geom | `hbsd/src/sbin/geom/` | `pbsd/userland/sbin/pbsd.userland.geom.cppm` | Class list stub |
| newfs | `hbsd/src/sbin/newfs/` | `pbsd/userland/sbin/pbsd.userland.newfs.cppm` | UFS `-O/-U/-j` |
| camcontrol | `hbsd/src/sbin/camcontrol/` | `pbsd/userland/sbin/pbsd.userland.camcontrol.cppm` | devlist/inquiry |

**New modules this pass:** 38 (`pbsd/userland/**/*.cppm` cumulative **249**).

### Wave 2 port stubs (`convert_c_batch --wave wave2`)

- `--prefix hbsd/src/usr.bin/ --prefix hbsd/src/sbin/ --prefix hbsd/src/usr.sbin/ --prefix hbsd/src/cddl/sbin/ --limit 50 --skip-stubbed`
- `--prefix hbsd/src/cddl/sbin/ --prefix hbsd/src/usr.bin/bzip2 --limit 40` (incremental cddl/compress tree)

Generator: `tools/generate_userland_wave2_burst8.py`.

### Harness additions

`test_usr_bin_wave2_burst8` in `pbsd/tests/userland_harness.cpp` (38 smoke checks).

### CMake modules added

38 targets via `pbsd_userland_hosted_tool()` burst-8 loop; extended `pbsd_userland_usr_bin` (+24) and `pbsd_userland_sbin` (+14) INTERFACE aggregates.

## Wave 2 — libc/libthr/msun module coverage (2026-07-19, burst 9)

Inventory sync + hand ports for regex/net/hash/gdtoa/locale/string/stdlib/stdio, libthr attr/cancel/join/sched/tls/signals, and msun trig/log/exp/pow/hypot/fma/nextafter/nan.

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| libc regex | `regex/regerror.c` | `pbsd/userland/libc/pbsd.userland.libc.regex*.cppm` | REG_* error table + regerror |
| libc net | `net/htonl.c` | `pbsd/userland/libc/pbsd.userland.libc.net.hton.cppm` | htons/htonl/ntohs/ntohl |
| libc hash | `db/hash/hash.c` | `pbsd/userland/libc/pbsd.userland.libc.hash.prime.cppm` | Prime bucket sizing |
| libc gdtoa | `gdtoa/machdep_ldisf.c` | `pbsd/userland/libc/pbsd.userland.libc.gdtoa.ldisf.cppm` | Float IEEE exponent |
| libc stdio | `stdio/fseek.c` | `pbsd/userland/libc/pbsd.userland.libc.stdio.fseek.cppm` | fseek/ftell/rewind stubs |
| libc string | `string/strerror.c` | `pbsd/userland/libc/pbsd.userland.libc.string.strerror.cppm` | POSIX errno subset |
| libc stdlib | `stdlib/abs.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.abs.cppm` | abs/labs/llabs |
| libc locale | `locale/wcwidth.c` | `pbsd/userland/libc/pbsd.userland.libc.locale.wcwidth.cppm` | ASCII wcwidth (wired) |
| libc gen | `gen/{getpagesize,getprogname,...}.c` | `pbsd/userland/libc/pbsd.userland.libc.gen.*.cppm` | 10 prior orphans wired to CMake |
| libthr attr | `libthr/thread/thr_attr.c` | `pbsd/userland/libthr/pbsd.userland.libthr.attr.cppm` | pthread_attr_* concepts |
| libthr cancel | `libthr/thread/thr_cancel.c` | `pbsd/userland/libthr/pbsd.userland.libthr.cancel.cppm` | cancel state/type/test |
| libthr join | `libthr/thread/thr_join.c` | `pbsd/userland/libthr/pbsd.userland.libthr.join.cppm` | pthread_join stub |
| libthr sched | `libthr/thread/thr_create.c` | `pbsd/userland/libthr/pbsd.userland.libthr.sched.cppm` | SchedPolicy/param get/set |
| libthr tls | `libthr/thread/thr_spec.c` | `pbsd/userland/libthr/pbsd.userland.libthr.tls.cppm` | Per-thread TSS slot array |
| libthr signals | `libthr/thread/thr_sig.c` | `pbsd/userland/libthr/pbsd.userland.libthr.signals.cppm` | SigSet mask block/unblock |
| msun sin/cos | `msun/src/{k_sin,k_cos,s_sin,s_cos}.c` | `pbsd/userland/msun/pbsd.userland.msun.{sin,cos}.cppm` | Taylor + phase shift |
| msun log/exp | `msun/src/{e_log,e_exp}.c` | `pbsd/userland/msun/pbsd.userland.msun.{log,exp}.cppm` | frexp + Taylor |
| msun pow | `msun/src/e_pow.c` | `pbsd/userland/msun/pbsd.userland.msun.pow.cppm` | exp(y*log(x)) |
| msun hypot | `msun/src/e_hypot.c` | `pbsd/userland/msun/pbsd.userland.msun.hypot.cppm` | Scaled sqrt |
| msun fma | `msun/src/s_fma.c` | `pbsd/userland/msun/pbsd.userland.msun.fma.cppm` | x*y+z stub |
| msun nextafter | `msun/src/s_nextafter.c` | `pbsd/userland/msun/pbsd.userland.msun.nextafter.cppm` | ULP step |
| msun nan | `msun/src/s_nan.c` | `pbsd/userland/msun/pbsd.userland.msun.nan.cppm` | Quiet NaN payload |
| libc umbrella | — | `pbsd/userland/libc/pbsd.userland.libc.cppm` | +18 export imports |
| libthr umbrella | — | `pbsd/userland/libthr/pbsd.userland.libthr.cppm` | +6 export imports (22 partitions) |
| msun umbrella | — | `pbsd/userland/msun/pbsd.userland.msun.cppm` | +9 export imports (28 partitions) |

**New modules this pass:** 23 hand ports; **CMake inventory:** libc 118, libthr 24, msun 29, rtld 11 — all `.cppm` on disk wired to FILE_SETs.

### CMake modules added

`pbsd_userland_libc` +8 partitions (+10 orphan sync); `pbsd_userland_libthr` +6; `pbsd_userland_msun` +9 (sin/cos/log/exp/pow/hypot/fma/nextafter/nan).

## Wave 4/5 — Kernel IPC/subsystems + UDA device burst (2026-07-19)

### Wave 4 — Kernel partitions (hand ports)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| TTY termios | `hbsd/src/sys/sys/tty.h`, `ttydefaults.h` | `pbsd/kernel/tty/pbsd.kernel.tty.cppm` |
| Console | `hbsd/src/sys/sys/cons.h`, `kern/tty_cons.c` | `pbsd/kernel/cons/pbsd.kernel.cons.cppm` |
| UNIX IPC | `hbsd/src/sys/sys/un.h`, `kern/uipc_*.c` | `pbsd/kernel/uipc/pbsd.kernel.uipc.cppm` |
| SysV shm | `hbsd/src/sys/sys/shm.h`, `kern/kern_shm.c` | `pbsd/kernel/sysvshm/pbsd.kernel.sysvshm.cppm` |
| SysV sem | `hbsd/src/sys/sys/sem.h`, `kern/kern_sem.c` | `pbsd/kernel/sysvsem/pbsd.kernel.sysvsem.cppm` |
| SysV msg | `hbsd/src/sys/sys/msg.h`, `kern/kern_msg.c` | `pbsd/kernel/sysvmsg/pbsd.kernel.sysvmsg.cppm` |
| POSIX shm | `hbsd/src/sys/sys/mman.h`, `kern/kern_posix_shm.c` | `pbsd/kernel/posix_shm/pbsd.kernel.posix_shm.cppm` |
| Accounting | `hbsd/src/sys/sys/acct.h`, `kern/kern_acct.c` | `pbsd/kernel/acct/pbsd.kernel.acct.cppm` |
| racct types | `hbsd/src/sys/sys/racct.h`, `kern/kern_racct.c` | `pbsd/kernel/racct/pbsd.kernel.racct.cppm` |
| rctl rules | `hbsd/src/sys/sys/rctl.h`, `kern/kern_rctl.c` | `pbsd/kernel/rctl/pbsd.kernel.rctl.cppm` |
| cpuset | `hbsd/src/sys/sys/cpuset.h`, `kern/kern_cpuset.c` | `pbsd/kernel/cpuset/pbsd.kernel.cpuset.cppm` |
| NUMA policy | `hbsd/src/sys/sys/numa.h`, `vm/vm_numa.c` | `pbsd/kernel/numa/pbsd.kernel.numa.cppm` |
| SMR tiers | `hbsd/src/sys/sys/smr.h`, `kern/subr_smr.c` | `pbsd/kernel/smr/pbsd.kernel.smr.cppm` |
| epoch tracker | `hbsd/src/sys/sys/epoch.h`, `kern/subr_epoch.c` | `pbsd/kernel/epoch/pbsd.kernel.epoch.cppm` |
| callout wheel | `hbsd/src/sys/sys/callout.h`, `kern/kern_timeout.c` | `pbsd/kernel/callout/pbsd.kernel.callout.cppm` |
| timeout API | `hbsd/src/sys/sys/timeout.h` | `pbsd/kernel/timeout/pbsd.kernel.timeout.cppm` |
| interrupt IRQ | `hbsd/src/sys/sys/interrupt.h`, `kern/kern_intr.c` | `pbsd/kernel/intr/pbsd.kernel.intr.cppm` |
| bus_dma tags | `hbsd/src/sys/sys/bus_dma.h`, `kern/subr_busdma.c` | `pbsd/kernel/busdma/pbsd.kernel.busdma.cppm` |

**New kernel modules this pass:** 18 (`pbsd.kernel.{tty,cons,uipc,sysvshm,sysvsem,sysvmsg,posix_shm,acct,racct,rctl,cpuset,numa,smr,epoch,callout,timeout,intr,busdma}`).

### Wave 5 — UDA descriptors (hand ports)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| ixl X710 | `hbsd/src/sys/dev/ixl/ixl_common.h` | `pbsd/uda/descriptors/ixl.cppm` |
| bnxt NetXtreme | `hbsd/src/sys/dev/bnxt/if_bnxt.h` | `pbsd/uda/descriptors/bnxt.cppm` |
| mlx5 ConnectX-4 | `hbsd/src/sys/dev/mlx5/mlx5_core.h` | `pbsd/uda/descriptors/mlx5.cppm` |
| oce OneConnect | `hbsd/src/sys/dev/oce/if_oce.h` | `pbsd/uda/descriptors/oce.cppm` |
| qlnxe QLE8242 | `hbsd/src/sys/dev/qlnx/qlnxe.h` | `pbsd/uda/descriptors/qlnxe.cppm` |
| cdce ECM | `hbsd/src/sys/dev/usb/net/if_cdce.c` | `pbsd/uda/descriptors/cdce.cppm` |
| urtwn RTL8188EU | `hbsd/src/sys/dev/usb/wlan/if_urtwnreg.h` | `pbsd/uda/descriptors/urtwn.cppm` |
| rtwn RTL8192CE | `hbsd/src/sys/dev/rtwn/if_rtwnreg.h` | `pbsd/uda/descriptors/rtwn.cppm` |
| iwn WM4965AG | `hbsd/src/sys/dev/iwn/if_iwnreg.h` | `pbsd/uda/descriptors/iwn.cppm` |
| iwm WM7260 | `hbsd/src/sys/dev/iwm/if_iwmreg.h` | `pbsd/uda/descriptors/iwm.cppm` |
| ath AR9280 | `hbsd/src/sys/dev/ath/athreg.h` | `pbsd/uda/descriptors/ath.cppm` |
| ral RT2560 | `hbsd/src/sys/dev/ral/rt2560reg.h` | `pbsd/uda/descriptors/ral.cppm` |
| run RT2870 | `hbsd/src/sys/dev/run/if_runreg.h` | `pbsd/uda/descriptors/run.cppm` |
| USB storage CBW/CSW | `hbsd/src/sys/dev/usb/storage/umass.c` | `pbsd/uda/descriptors/usb_storage.cppm` |
| umass SCSI | `hbsd/src/sys/dev/usb/storage/umass.c` | `pbsd/uda/descriptors/umass.cppm` |
| da SCSI direct | `hbsd/src/sys/cam/scsi/scsi_da.c` | `pbsd/uda/descriptors/da.cppm` |
| ada ATA direct | `hbsd/src/sys/cam/ata/ata_da.c` | `pbsd/uda/descriptors/ada.cppm` |
| cd SCSI optical | `hbsd/src/sys/cam/scsi/scsi_cd.c` | `pbsd/uda/descriptors/cd.cppm` |
| pass passthrough | `hbsd/src/sys/cam/scsi/scsi_pass.c` | `pbsd/uda/descriptors/pass.cppm` |
| ses enclosure | `hbsd/src/sys/cam/scsi/scsi_ses.c` | `pbsd/uda/descriptors/ses.cppm` |
| SMART ATA | `hbsd/src/sys/cam/ata/ata_all.h` | `pbsd/uda/descriptors/smart.cppm` |
| ACPI battery | `hbsd/src/sys/dev/acpica/acpi_battery.c` | `pbsd/uda/descriptors/acpi_battery.cppm` |
| ACPI lid | `hbsd/src/sys/dev/acpica/acpi_lid.c` | `pbsd/uda/descriptors/acpi_lid.cppm` |
| ACPI EC | `hbsd/src/sys/dev/acpica/acpi_ec.c` | `pbsd/uda/descriptors/acpi_ec.cppm` |

**New UDA modules this pass:** 24 (ixgbe/ahci pre-existing; +24 above).

### CMake modules added

Kernel: `pbsd_kernel_tty`, `pbsd_kernel_cons`, `pbsd_kernel_uipc`, `pbsd_kernel_sysvshm`, `pbsd_kernel_sysvsem`, `pbsd_kernel_sysvmsg`, `pbsd_kernel_posix_shm`, `pbsd_kernel_acct`, `pbsd_kernel_racct`, `pbsd_kernel_rctl`, `pbsd_kernel_cpuset`, `pbsd_kernel_numa`, `pbsd_kernel_smr`, `pbsd_kernel_epoch`, `pbsd_kernel_callout`, `pbsd_kernel_timeout`, `pbsd_kernel_intr`, `pbsd_kernel_busdma` — all propagate `PUBLIC ${PBSD_FS_CXX}`; wired into `pbsd_kernel` INTERFACE aggregate.

UDA: `pbsd_uda_ixl`, `pbsd_uda_bnxt`, `pbsd_uda_mlx5`, `pbsd_uda_oce`, `pbsd_uda_qlnxe`, `pbsd_uda_cdce`, `pbsd_uda_urtwn`, `pbsd_uda_rtwn`, `pbsd_uda_iwn`, `pbsd_uda_iwm`, `pbsd_uda_ath`, `pbsd_uda_ral`, `pbsd_uda_run`, `pbsd_uda_usb_storage`, `pbsd_uda_umass`, `pbsd_uda_da`, `pbsd_uda_ada`, `pbsd_uda_cd`, `pbsd_uda_pass`, `pbsd_uda_ses`, `pbsd_uda_smart`, `pbsd_uda_acpi_battery`, `pbsd_uda_acpi_lid`, `pbsd_uda_acpi_ec` — wired into `pbsd_uda` INTERFACE aggregate.

## Wave 6 — Net / FS / GEOM / ZFS expanded (2026-07-19, burst)

Hand ports of HBSD header constants and small validation APIs into freestanding C++23 modules under `pbsd/net/`, `pbsd/fs/`, `pbsd/geom/`, `pbsd/zfs/`.

### Net modules (+14 new, 35 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| LAGG | `hbsd/src/sys/net/if_lagg.h` | `pbsd/net/pbsd.net.lagg.cppm` | Proto/port flags, provider count |
| GIF | `hbsd/src/sys/net/if_gif.h` | `pbsd/net/pbsd.net.gif.cppm` | MTU bounds, tunnel opts |
| TUN | `hbsd/src/sys/net/if_tun.h` | `pbsd/net/pbsd.net.tun.cppm` | MTU/MRU, `tuninfo` |
| TAP | `hbsd/src/sys/net/if_tap.h` | `pbsd/net/pbsd.net.tap.cppm` | MRU, vnet hdr |
| Loopback | `hbsd/src/sys/net/if.h` | `pbsd/net/pbsd.net.loop.cppm` | `IFF_LOOPBACK` |
| ipfw | `hbsd/src/sys/netinet/ip_fw.h` | `pbsd/net/pbsd.net.ipfw.cppm` | Rule/set limits |
| CARP | `hbsd/src/sys/netinet/ip_carp.h` | `pbsd/net/pbsd.net.carp.cppm` | Advertisement header |
| NDP | `hbsd/src/sys/netinet6/nd6.h` | `pbsd/net/pbsd.net.ndp.cppm` | LLINFO states |
| IPv6 header | `hbsd/src/sys/netinet/ip6.h` | `pbsd/net/pbsd.net.ip6.cppm` | Next-header, hop limit |
| UDP6 | `hbsd/src/sys/netinet/in.h` | `pbsd/net/pbsd.net.udp6.cppm` | Capsicum sockopt checks |
| TCP6 | `hbsd/src/sys/netinet/tcp.h` | `pbsd/net/pbsd.net.tcp6.cppm` | Capsicum sockopt checks |
| SCTP | `hbsd/src/sys/netinet/sctp.h` | `pbsd/net/pbsd.net.sctp.cppm` | Chunk types, header |
| netmap | `hbsd/src/sys/net/netmap.h` | `pbsd/net/pbsd.net.netmap.cppm` | API version |
| netgraph | `hbsd/src/sys/netgraph/ng_message.h` | `pbsd/net/pbsd.net.netgraph.cppm` | Name/path limits |

Also wired (pre-existing files): `gre`, `in6`, `epair`, `altq`, `bridge`, `pf`.

### FS modules (+6 new, 20 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| NFS | `hbsd/src/sys/fs/nfs/nfsproto.h` | `pbsd/fs/pbsd.fs.nfs.cppm` | Version/port constants |
| nullfs | `hbsd/src/sys/fs/nullfs/null.h` | `pbsd/fs/pbsd.fs.nullfs.cppm` | Mount flags |
| procfs | `hbsd/src/sys/fs/procfs/procfs.h` | `pbsd/fs/pbsd.fs.procfs.cppm` | Node types |
| devfs | `hbsd/src/sys/fs/devfs/devfs.h` | `pbsd/fs/pbsd.fs.devfs.cppm` | Magic, rule IDs |
| fdescfs | `hbsd/src/sys/fs/fdescfs/fdesc.h` | `pbsd/fs/pbsd.fs.fdescfs.cppm` | Mount/node IDs |
| linprocfs | `hbsd/src/sys/compat/linprocfs/linprocfs.c` | `pbsd/fs/pbsd.fs.linprocfs.cppm` | Linux `/proc` nodes |
| fusefs | `hbsd/src/sys/fs/fuse/fuse.h` | `pbsd/fs/pbsd.fs.fusefs.cppm` | Protocol version, init flags |

Also wired: `tmpfs`, `pipefs`, `extattr`, `ufs`.

### GEOM modules (+4 new, 15 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| RAID3 | `hbsd/src/sys/geom/raid3/g_raid3.h` | `pbsd/geom/pbsd.geom.raid3.cppm` | Version, width |
| ELI | `hbsd/src/sys/geom/eli/g_eli.h` | `pbsd/geom/pbsd.geom.eli.cppm` | Algo/flag enums |
| JOURNAL | `hbsd/src/sys/geom/journal/g_journal.h` | `pbsd/geom/pbsd.geom.journal.cppm` | State machine |
| GATE | `hbsd/src/sys/geom/gate/g_gate.h` | `pbsd/geom/pbsd.geom.gate.cppm` | Version, ctl cmds |
| MULTIPATH | `hbsd/src/sys/geom/multipath/g_multipath.h` | `pbsd/geom/pbsd.geom.multipath.cppm` | Provider count |

Also wired: `concat`, `stripe`, `mirror`, `label`, `part`.

### ZFS modules (+1 new, 11 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| zio_checksum | `hbsd/src/sys/contrib/openzfs/include/sys/zio_checksum.h` | `pbsd/zfs/pbsd.zfs.zio_checksum.cppm` | Algorithm table, flags |

Existing: `spa`, `vdev`, `zio`, `dsl`, `arc`, `dmu`, `dnode`, `features`, `props`, `ioctl`.

### Harness additions

Expanded `test_wave6_net_fs_geom_zfs` in `pbsd/tests/si_harness.cpp` (+30 smoke checks for new partitions).

### CMake modules added

All targets use `PUBLIC ${PBSD_FS_CXX}` and link into aggregate `pbsd_net` (35), `pbsd_fs` (20), `pbsd_geom` (15), `pbsd_zfs` (11):

Net: `pbsd_net_lagg`, `pbsd_net_gif`, `pbsd_net_tun`, `pbsd_net_tap`, `pbsd_net_loop`, `pbsd_net_ipfw`, `pbsd_net_carp`, `pbsd_net_ndp`, `pbsd_net_ip6`, `pbsd_net_udp6`, `pbsd_net_tcp6`, `pbsd_net_sctp`, `pbsd_net_netmap`, `pbsd_net_netgraph` (+ wired `gre`, `in6`, `epair`, `altq`).

FS: `pbsd_fs_nullfs`, `pbsd_fs_procfs`, `pbsd_fs_devfs`, `pbsd_fs_fdescfs`, `pbsd_fs_linprocfs`, `pbsd_fs_fusefs` (+ wired `tmpfs`, `pipefs`, `extattr`, `nfs`).

GEOM: `pbsd_geom_raid3`, `pbsd_geom_journal`, `pbsd_geom_gate`, `pbsd_geom_multipath` (+ wired `concat`, `stripe`, `eli`).

ZFS: `pbsd_zfs_zio_checksum`.

Generator: `tools/gen_wave6_burst.py`.

## Wave 2 — usr.bin high-value tools (2026-07-19, burst 9)

Hand ports of 17 priority `usr.bin` utilities from HBSD C sources into `pbsd/userland/usr.bin/` (logic-only flag/constant scaffolds).

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| od | `usr.bin/hexdump/odsyntax.c` | `pbsd/userland/usr.bin/pbsd.userland.od.cppm` | Legacy od `-A` address base + flag set |
| col | `usr.bin/col/col.c` | `pbsd/userland/usr.bin/pbsd.userland.col.cppm` | `-bfhl:px`, BUFFER_MARGIN |
| column | `usr.bin/column/column.c` | `pbsd/userland/usr.bin/pbsd.userland.column.cppm` | `-c:l:s:tx`, default 80 cols |
| rs | `usr.bin/rs/rs.cc` | `pbsd/userland/usr.bin/pbsd.userland.rs.cppm` | TRANSPOSE/MTRANSPOSE flag bits |
| units | `usr.bin/units/units.c` | `pbsd/userland/usr.bin/pbsd.userland.units.cppm` | UNITSFILE, MAXUNITS, `-ehf:o:qtv` |
| bc | `contrib/bc/src/{bc.c,args.c}` | `pbsd/userland/usr.bin/pbsd.userland.bc.cppm` | gh-bc `-i/-g/-l/-q` flags |
| dc | `contrib/bc/src/dc.c` | `pbsd/userland/usr.bin/pbsd.userland.dc.cppm` | Desk calc flag subset |
| make | `contrib/bmake/main.c` | `pbsd/userland/usr.bin/pbsd.userland.make.cppm` | bmake optspecs `BC:D:I:J:NST:V:WXd:ef:ij:km:nqrstv:w` |
| yacc | `contrib/byacc/main.c` | `pbsd/userland/usr.bin/pbsd.userland.yacc.cppm` | `-Bb:dghH:ilLo:Pp:rstVvy` |
| lex | `usr.bin/lex/initparse.h` | `pbsd/userland/usr.bin/pbsd.userland.lex.cppm` | Token IDs + `-8bcfIlnpsStv` |
| m4 | `usr.bin/m4/{main.c,mdef.h,pathnames.h}` | `pbsd/userland/usr.bin/pbsd.userland.m4.cppm` | MACROTYPE/DEFINETYPE, `/tmp/m4.*` |
| apply | `usr.bin/apply/apply.c` | `pbsd/userland/usr.bin/pbsd.userland.apply.cppm` | Magic `%`, `-a`, `-d`, `-N` |
| chpass | `usr.bin/chpass/{chpass.c,chpass.h}` | `pbsd/userland/usr.bin/pbsd.userland.chpass.cppm` | Field enum E_SHELL, `-a:p:s:e:loy` |
| passwd | `usr.bin/passwd/passwd.c` | `pbsd/userland/usr.bin/pbsd.userland.passwd.cppm` | PAM `-d:h:loy` |
| login | `usr.bin/login/{login.c,pathnames.h}` | `pbsd/userland/usr.bin/pbsd.userland.login.cppm` | `.hushlogin`, motd, `-fh:p` |
| su | `usr.bin/su/su.c` | `pbsd/userland/usr.bin/pbsd.userland.su.cppm` | PAM `-flmsc:` login style |
| sudo | BSD sudo(8) (not in HBSD tree) | `pbsd/userland/usr.bin/pbsd.userland.sudo.cppm` | Typical `-Ab:CEeilnPsuVv` scaffold |

**New modules this pass:** 17. **Cumulative userland `.cppm` count:** 378 (+17).

### CMake modules added

- `foreach(_ul_b9 IN ITEMS od col column rs units bc dc make yacc lex m4 apply chpass passwd login su sudo)` → 17 `pbsd_userland_*` targets
- Extended `pbsd_userland_usr_bin` INTERFACE (+17)
- Extended `pbsd_userland_sbin` INTERFACE (+17): wired pre-existing `gpart mdconfig dumpon savecore fsck_ffs fsck_msdosfs newfs_msdos tunefs growfs dmesg dumpfs fdisk bsdlabel swapon shutdown reboot kldload` from `_ul_sbin8` loop
- `pbsd_userland_msun` FILE_SET confirmed complete (14 libm partitions + umbrella)

## Burst 9 continuation — kernel sync + UDA NIC burst (2026-07-19)

Expanded freestanding kernel and UDA descriptor coverage; wired Wave 4/5 kernel partitions into `pbsd_kernel` INTERFACE aggregate (previously CMake targets only).

### Kernel modules (+6 new, 94 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| sx | `hbsd/src/sys/sys/sx.h` | `pbsd/kernel/sync/pbsd.kernel.sx.cppm` | Shared/exclusive lock flags |
| rwlock | `hbsd/src/sys/sys/rwlock.h` | `pbsd/kernel/sync/pbsd.kernel.rwlock.cppm` | Reader/writer lock word |
| condvar | `hbsd/src/sys/sys/condvar.h` | `pbsd/kernel/sync/pbsd.kernel.condvar.cppm` | cv waiters stub |
| taskqueue | `hbsd/src/sys/sys/taskqueue.h` | `pbsd/kernel/kern/pbsd.kernel.taskqueue.cppm` | FAIL_IF_* enqueue flags |
| uio | `hbsd/src/sys/sys/uio.h`, `_uio.h` | `pbsd/kernel/kern/pbsd.kernel.uio.cppm` | UIO_MAXIOV, seg/rw enums |
| lockf | `hbsd/src/sys/sys/lockf.h`, `fcntl.h` | `pbsd/kernel/kern/pbsd.kernel.lockf.cppm` | F_RDLCK/F_WRLCK range |

Also wired into `pbsd_kernel` INTERFACE: `tty`, `cons`, `uipc`, `sysvshm`, `sysvsem`, `sysvmsg`, `posix_shm`, `acct`, `racct`, `rctl`, `cpuset`, `numa`, `smr`, `epoch`, `callout`, `timeout`, `intr`, `busdma`.

### UDA descriptors (+4 new, 69 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| ale | `hbsd/src/sys/dev/ale/if_alereg.h` | `pbsd/uda/descriptors/ale.cppm` | AR81xx MAC_CFG init |
| axe | `hbsd/src/sys/dev/usb/net/if_axereg.h` | `pbsd/uda/descriptors/axe.cppm` | AX88772 SW reset |
| fxp | `hbsd/src/sys/dev/fxp/if_fxpreg.h` | `pbsd/uda/descriptors/fxp.cppm` | i82559 port reset |
| vmx | `hbsd/src/sys/dev/vmware/vmxnet3/if_vmxreg.h` | `pbsd/uda/descriptors/vmx.cppm` | VMXNET3 enable cmd |

Prior burst-9 UDA (`ixl`, `umass`, `da`) unchanged; SoftMmio AHCI/MPT hooks in `pbsd.uda.interp.cppm` untouched.

### Harness additions

- `test_wave5_kernel_ipc` — callout/tty/cons/uipc/sysvshm/intr/busdma + sync/uio/lockf smoke
- `test_si7_burst9_descriptors` — ixl, umass, da, ale, axe, fxp, vmx descriptor validation

### CMake modules added

Kernel: `pbsd_kernel_sx`, `pbsd_kernel_rwlock`, `pbsd_kernel_condvar`, `pbsd_kernel_taskqueue`, `pbsd_kernel_uio`, `pbsd_kernel_lockf` — all `PUBLIC ${PBSD_FS_CXX}`; extended `pbsd_kernel` INTERFACE (+24 partitions).

UDA: `pbsd_uda_ale`, `pbsd_uda_axe`, `pbsd_uda_fxp`, `pbsd_uda_vmx` — wired into `pbsd_uda` INTERFACE aggregate.

## Wave 2 — bin/sbin/usr.bin + libc/libthr/msun (2026-07-19, burst 10)

Hand ports continuing after burst-9 GREEN gate. 28 new C++23 modules under `pbsd/userland/`.

### bin/ (+6)

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| ls | `bin/ls/{ls.c,ls.h}` | `pbsd/userland/bin/pbsd.userland.ls.cppm` | HUMANVALSTR_LEN, getopt_long flag set |
| ps | `bin/ps/{ps.c,ps.h}` | `pbsd/userland/bin/pbsd.userland.ps.cppm` | PS_ARGS, keyword VarFlag bits |
| rm | `bin/rm/rm.c` | `pbsd/userland/bin/pbsd.userland.rm.cppm` | `-dfiIPRrvWx`, unlink mode, FTS |
| timeout | `bin/timeout/timeout.c` | `pbsd/userland/bin/pbsd.userland.timeout.cppm` | EXIT_124..127, `+fhk:ps:v` |
| stty | `bin/stty/{stty.c,stty.h}` | `pbsd/userland/bin/pbsd.userland.stty.cppm` | FMT enum, `-aef:g`, LINELENGTH |
| pkill | `bin/pkill/pkill.c` | `pbsd/userland/bin/pbsd.userland.pkill.cppm` | Process match opt string |

### usr.bin/ (+9)

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| colrm | `usr.bin/colrm/colrm.c` | `pbsd/userland/usr.bin/pbsd.userland.colrm.cppm` | TAB=8, column range |
| factor | `usr.bin/factor/factor.c` | `pbsd/userland/usr.bin/pbsd.userland.factor.cppm` | `-h`, trial division |
| fmt | `usr.bin/fmt/fmt.c` | `pbsd/userland/usr.bin/pbsd.userland.fmt.cppm` | goal=65, max=goal+10 |
| look | `usr.bin/look/look.c` | `pbsd/userland/usr.bin/pbsd.userland.look.cppm` | `+adft:`, binary compare |
| mesg | `usr.bin/mesg/mesg.c` | `pbsd/userland/usr.bin/pbsd.userland.mesg.cppm` | y/n group write bit |
| pr | `usr.bin/pr/{pr.c,pr.h}` | `pbsd/userland/usr.bin/pbsd.userland.pr.cppm` | LINES=66, PGWD=72 defaults |
| tty | `usr.bin/tty/tty.c` | `pbsd/userland/usr.bin/pbsd.userland.tty.cppm` | `-s`, exit status |
| ul | `usr.bin/ul/ul.c` | `pbsd/userland/usr.bin/pbsd.userland.ul.cppm` | `-it:T:`, CHAR mode bits |
| wall | `usr.bin/wall/wall.c` | `pbsd/userland/usr.bin/pbsd.userland.wall.cppm` | `-g:n`, group broadcast |

### sbin/ (+3)

| Utility | HBSD source | PBSD artifact | Notes |
|---------|-------------|---------------|-------|
| kldstat | `sbin/kldstat/kldstat.c` | `pbsd/userland/sbin/pbsd.userland.kldstat.cppm` | `-dhi:m:n:qv`, PTR_WIDTH |
| kldunload | `sbin/kldunload/kldunload.c` | `pbsd/userland/sbin/pbsd.userland.kldunload.cppm` | `-finv`, OPT_ID/FORCE |
| pfctl | `sbin/pfctl/pfctl.c` | `pbsd/userland/sbin/pbsd.userland.pfctl.cppm` | PF opt string scaffold |

### libc (+3 partitions)

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| inet | `lib/libc/inet/inet_addr.c` | `pbsd/userland/libc/pbsd.userland.libc.net.inet_addr.cppm` | inet_aton/inet_addr |
| string | `lib/libc/string/strtok.c` | `pbsd/userland/libc/pbsd.userland.libc.string.strtok_r.cppm` | Reentrant tokenizer |
| stdlib | `lib/libc/stdlib/getenv.c` | `pbsd/userland/libc/pbsd.userland.libc.stdlib.setenv.cppm` | Name/assignment validation |

### libthr (+2 partitions)

| Area | HBSD source | PBSD artifact | Notes |
|------|-------------|---------------|-------|
| concurrency | `lib/libthr/thread/thr_concurrency.c` | `pbsd/userland/libthr/pbsd.userland.libthr.concurrency.cppm` | get/setconcurrency |
| cleanup | `lib/libthr/thread/thr_clean.c` | `pbsd/userland/libthr/pbsd.userland.libthr.cleanup.cppm` | cleanup push/pop stack |

### msun (+5 partitions)

| Function | HBSD source | PBSD artifact | Notes |
|----------|-------------|---------------|-------|
| tan | `lib/msun/src/s_tan.c` | `pbsd/userland/msun/pbsd.userland.msun.tan.cppm` | sin/cos ratio |
| atan2 | `lib/msun/src/e_atan2.c` | `pbsd/userland/msun/pbsd.userland.msun.atan2.cppm` | Quadrant pi constants |
| fmod | `lib/msun/src/e_fmod.c` | `pbsd/userland/msun/pbsd.userland.msun.fmod.cppm` | Shift-subtract stub |
| asin | `lib/msun/src/e_asin.c` | `pbsd/userland/msun/pbsd.userland.msun.asin.cppm` | Taylor on \|x\|<1 |
| acos | `lib/msun/src/e_acos.c` | `pbsd/userland/msun/pbsd.userland.msun.acos.cppm` | pi/2 - asin |

**New modules this pass:** 28. **Cumulative userland `.cppm` count:** 406 (+28).

### CMake modules added

- `foreach(_ul_bin10 IN ITEMS ls ps rm timeout stty pkill)` → 6 `pbsd_userland_*` targets → `pbsd_userland_bin` (+6)
- `foreach(_ul_b10 IN ITEMS colrm factor fmt look mesg pr tty ul wall)` → 9 targets → `pbsd_userland_usr_bin` (+9)
- `foreach(_ul_sbin10 IN ITEMS kldstat kldunload pfctl)` → 3 targets → `pbsd_userland_sbin` (+3)
- `pbsd_userland_libc` FILE_SET (+3: inet_addr, strtok_r, setenv)
- `pbsd_userland_libthr` FILE_SET (+2: concurrency, cleanup)
- `pbsd_userland_msun` FILE_SET (+5: tan, atan2, fmod, asin, acos)
- Umbrella exports updated: `pbsd.userland.libc`, `pbsd.userland.libthr`, `pbsd.userland.msun`

## Wave 4/5 — Kernel sync/I/O + UDA driver burst (2026-07-19, pass 2)

### Wave 4 — Kernel partitions (hand ports)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| rmlock | `hbsd/src/sys/sys/rmlock.h` | `pbsd/kernel/sync/pbsd.kernel.rmlock.cppm` |
| bio | `hbsd/src/sys/sys/bio.h` | `pbsd/kernel/io/pbsd.kernel.bio.cppm` |
| disk | `hbsd/src/sys/sys/disk.h` | `pbsd/kernel/io/pbsd.kernel.disk.cppm` |
| buf | `hbsd/src/sys/sys/buf.h` | `pbsd/kernel/io/pbsd.kernel.buf.cppm` |
| eventhandler | `hbsd/src/sys/sys/eventhandler.h` | `pbsd/kernel/kern/pbsd.kernel.eventhandler.cppm` |
| refcnt | `hbsd/src/sys/sys/refcount.h` | `pbsd/kernel/kern/pbsd.kernel.refcnt.cppm` |
| kobj | `hbsd/src/sys/sys/kobj.h` | `pbsd/kernel/kern/pbsd.kernel.kobj.cppm` |
| pci | `hbsd/src/sys/sys/pciio.h` | `pbsd/kernel/kern/pbsd.kernel.pci.cppm` |
| devstat | `hbsd/src/sys/sys/devstat.h` | `pbsd/kernel/kern/pbsd.kernel.devstat.cppm` |
| sx | `hbsd/src/sys/sys/sx.h` | `pbsd/kernel/sync/pbsd.kernel.sx.cppm` |
| rwlock | `hbsd/src/sys/sys/rwlock.h` | `pbsd/kernel/sync/pbsd.kernel.rwlock.cppm` |
| condvar | `hbsd/src/sys/sys/condvar.h` | `pbsd/kernel/sync/pbsd.kernel.condvar.cppm` |
| taskqueue | `hbsd/src/sys/sys/taskqueue.h` | `pbsd/kernel/kern/pbsd.kernel.taskqueue.cppm` |
| uio | `hbsd/src/sys/sys/uio.h` | `pbsd/kernel/kern/pbsd.kernel.uio.cppm` |
| lockf | `hbsd/src/sys/sys/lockf.h` | `pbsd/kernel/kern/pbsd.kernel.lockf.cppm` |
| sleepq | `hbsd/src/sys/sys/sleepqueue.h` | `pbsd/kernel/sleepq/pbsd.kernel.sleepq.cppm` |

**New kernel modules this pass:** 16 (`pbsd.kernel.{rmlock,bio,disk,buf,eventhandler,refcnt,kobj,pci,devstat,sx,rwlock,condvar,taskqueue,uio,lockf,sleepq}`).

Also synced prior pass modules into `pbsd_kernel` aggregate: `{callout,tty,cons,uipc,sysvshm,sysvsem,sysvmsg,posix_shm,acct,racct,rctl,cpuset,numa,smr,epoch,timeout,intr,busdma}`.

### Wave 5 — UDA descriptors (hand ports)

| Module | HBSD source | PBSD artifact |
|--------|-------------|---------------|
| vr Rhine | `hbsd/src/sys/dev/vr/if_vrreg.h` | `pbsd/uda/descriptors/vr.cppm` |
| sis 900 | `hbsd/src/sys/dev/sis/if_sisreg.h` | `pbsd/uda/descriptors/sis.cppm` |
| bce NetXtreme II | `hbsd/src/sys/dev/bce/if_bcereg.h` | `pbsd/uda/descriptors/bce.cppm` |
| nfe MCP55 | `hbsd/src/sys/dev/nfe/if_nfereg.h` | `pbsd/uda/descriptors/nfe.cppm` |
| jme JMC250 | `hbsd/src/sys/dev/jme/if_jmereg.h` | `pbsd/uda/descriptors/jme.cppm` |
| vge Rhine II | `hbsd/src/sys/dev/vge/if_vgereg.h` | `pbsd/uda/descriptors/vge.cppm` |
| bwn BCM4306 | `hbsd/src/sys/dev/bwn/if_bwnreg.h` | `pbsd/uda/descriptors/bwn.cppm` |
| rum RT2573 | `hbsd/src/sys/dev/usb/wlan/if_rumreg.h` | `pbsd/uda/descriptors/rum.cppm` |
| wpi 4965AGN | `hbsd/src/sys/dev/wpi/if_wpireg.h` | `pbsd/uda/descriptors/wpi.cppm` |
| mwl 88W8363 | `hbsd/src/sys/dev/mwl/mwlreg.h` | `pbsd/uda/descriptors/mwl.cppm` |
| zyd ZD1211 | `hbsd/src/sys/dev/usb/wlan/if_zydreg.h` | `pbsd/uda/descriptors/zyd.cppm` |
| fxp 82559 | `hbsd/src/sys/dev/fxp/if_fxpreg.h` | `pbsd/uda/descriptors/fxp.cppm` |
| axe AX88772 | `hbsd/src/sys/dev/usb/net/if_axereg.h` | `pbsd/uda/descriptors/axe.cppm` |
| ale AR81xx | `hbsd/src/sys/dev/ale/if_alereg.h` | `pbsd/uda/descriptors/ale.cppm` |
| virtio-input | `hbsd/src/sys/dev/virtio/input/` | `pbsd/uda/descriptors/virtio_input.cppm` |
| vmxnet3 | `hbsd/src/sys/dev/vmware/vmxnet3/if_vmxreg.h` | `pbsd/uda/descriptors/vmx.cppm` |

**New UDA modules this pass:** 16 (11 new + 5 orphan sync).

### CMake modules added

Kernel: `pbsd_kernel_rmlock`, `pbsd_kernel_bio`, `pbsd_kernel_disk`, `pbsd_kernel_buf`, `pbsd_kernel_eventhandler`, `pbsd_kernel_refcnt`, `pbsd_kernel_kobj`, `pbsd_kernel_pci`, `pbsd_kernel_devstat`, `pbsd_kernel_sx`, `pbsd_kernel_rwlock`, `pbsd_kernel_condvar`, `pbsd_kernel_taskqueue`, `pbsd_kernel_uio`, `pbsd_kernel_lockf`, `pbsd_kernel_sleepq` — all propagate `PUBLIC ${PBSD_FS_CXX}`; wired into `pbsd_kernel` INTERFACE aggregate.

UDA: `pbsd_uda_vr`, `pbsd_uda_sis`, `pbsd_uda_bce`, `pbsd_uda_nfe`, `pbsd_uda_jme`, `pbsd_uda_vge`, `pbsd_uda_bwn`, `pbsd_uda_rum`, `pbsd_uda_wpi`, `pbsd_uda_mwl`, `pbsd_uda_zyd`, `pbsd_uda_fxp`, `pbsd_uda_axe`, `pbsd_uda_ale`, `pbsd_uda_virtio_input`, `pbsd_uda_vmx` — wired into `pbsd_uda` INTERFACE aggregate.

Generator: `tools/gen_wave45_burst.py`. SoftMmio hooks in `pbsd.uda.interp.cppm` unchanged (SI-7).

---

## Wave 6 — Net / FS / GEOM / ZFS pass 3 (2026-07-19, MAX throughput)

Hand ports of HBSD header constants into freestanding C++23 modules. All targets use `PUBLIC ${PBSD_FS_CXX}`.

### Net modules (+5 new, 46 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| DLT | `hbsd/src/sys/net/dlt.h` | `pbsd/net/pbsd.net.dlt.cppm` | libpcap link types 0–10 |
| IFM media | `hbsd/src/sys/net/if_media.h` | `pbsd/net/pbsd.net.media.cppm` | IFM_ETHER subtypes, FDX/HDX opts |
| pfil | `hbsd/src/sys/net/pfil.h` | `pbsd/net/pbsd.net.pfil.cppm` | PFIL_IN/OUT/FWD, hook types |
| LACP | `hbsd/src/sys/net/ieee8023ad_lacp.h` | `pbsd/net/pbsd.net.lacp.cppm` | State bits, timer indices |
| if_clone | `hbsd/src/sys/net/if_clone.h` | `pbsd/net/pbsd.net.clone.cppm` | Autounit, name length |

Also present (prior passes): `vxlan`, `ipsec`, `stf`, `wg`, `pfsync`.

### FS modules (+2 new, 28 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| ext2fs | `hbsd/src/sys/fs/ext2fs/ext2fs.h` | `pbsd/fs/pbsd.fs.ext2fs.cppm` | E2FS_MAGIC 0xEF53, rev flags |
| smbfs | `hbsd/src/sys/fs/smbfs/smbfs.h` | `pbsd/fs/pbsd.fs.smbfs.cppm` | Mount flags, version 101012 |

Also present: `msdosfs`, `cd9660`, `unionfs`, `isofs`, `autofs`.

### GEOM modules (+2 new, 21 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| RAID | `hbsd/src/sys/geom/raid/g_raid.h` | `pbsd/geom/pbsd.geom.raid.cppm` | G_RAID_VERSION, device flags |
| MOUNTVER | `hbsd/src/sys/geom/mountver/g_mountver.h` | `pbsd/geom/pbsd.geom.mountver.cppm` | Version 4, `.mountver` suffix |

Also present: `cache`, `shsec`, `virstor`.

### ZFS modules (+2 new, 17 total `.cppm`)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| zio_flag | `hbsd/src/sys/contrib/openzfs/include/sys/zio.h` | `pbsd/zfs/pbsd.zfs.zio_flag.cppm` | ZIO_FLAG_* bits, failure modes |
| compress | `hbsd/src/sys/contrib/openzfs/include/sys/zio.h` | `pbsd/zfs/pbsd.zfs.compress.cppm` | ZIO_COMPRESS_* algorithms |

Also present: `zap`, `zil`, `l2arc`.

### Quick wins — stand / arch / bifrost (+3)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| stand net | `hbsd/src/stand/libsa/net.h` | `pbsd/stand/pbsd.stand.net.cppm` | Boot ARP/RARP/BOOTP/TFTP protos |
| amd64 CR0 | `hbsd/src/sys/x86/include/specialreg.h` | `pbsd/arch/amd64/pbsd.arch.amd64.cr.cppm` | CR0_PE/PG/WP validate |
| VM exit | `hbsd/src/sys/amd64/include/vmm.h` | `pbsd/bifrost/pbsd.bifrost.exit.cppm` | VM_EXITCODE_* dispatch table |

### Harness additions

Expanded `test_wave6_net_fs_geom_zfs` (+14 smoke checks), `test_wave7_stand_arch` (+2), `test_wave8_compositor` (+2) in `pbsd/tests/si_harness.cpp`.

### CMake modules added

Net: `pbsd_net_dlt`, `pbsd_net_media`, `pbsd_net_pfil`, `pbsd_net_lacp`, `pbsd_net_clone` → `pbsd_net` aggregate.

FS: `pbsd_fs_ext2fs`, `pbsd_fs_smbfs` → `pbsd_fs` aggregate.

GEOM: `pbsd_geom_raid`, `pbsd_geom_mountver` → `pbsd_geom` aggregate.

ZFS: `pbsd_zfs_zio_flag`, `pbsd_zfs_compress` → `pbsd_zfs` aggregate.

Stand: `pbsd_stand_net` → `pbsd_stand` aggregate.

Arch: `pbsd_arch_amd64_cr` → `pbsd_arch_amd64` aggregate.

BIFROST: `pbsd_bifrost_exit` → `pbsd_bifrost` aggregate.

Generator: `tools/gen_wave6_pass3.py`.

**Cumulative Wave 6 partition counts:** net 46, fs 28, geom 21, zfs 17 `.cppm` files.

## Wave 6/9 — Net / FS / GEOM / ZFS / KDE / Theme burst 9 (2026-07-19)

Hand ports + CMake inventory sync for burst-9 green gate expansion. All namespaces use `::` segment syntax (e.g. `pbsd::kde::plasma::volume`, not dotted forms).

### Net modules (+3 new, +2 wired orphans → 41 CMake targets)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| STF | `sys/net/if_stf.h` | `pbsd/net/pbsd.net.stf.cppm` | 6to4 prefix validation |
| WireGuard | `sys/dev/wg/if_wg.h` | `pbsd/net/pbsd.net.wg.cppm` | Key size, allowed-ip flags |
| PFSYNC | `sys/net/pfvar.h`, `netinet/in.h` | `pbsd/net/pbsd.net.pfsync.cppm` | Protocol version, action enum |

Also wired (pre-existing on disk): `ipsec`, `vxlan`.

### FS modules (+2 new, +3 wired orphans → 25 CMake targets)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| isofs | `sys/fs/isofs/cd9660/iso.h` | `pbsd/fs/pbsd.fs.isofs.cppm` | Primary volume descriptor |
| autofs | `sys/fs/autofs/autofs.h` | `pbsd/fs/pbsd.fs.autofs.cppm` | Map type / request enums |

Also wired (pre-existing on disk): `msdosfs`, `cd9660`, `unionfs`.

### GEOM modules (+3 new → 18 CMake targets)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| CACHE | `sys/geom/cache/g_cache.h` | `pbsd/geom/pbsd.geom.cache.cppm` | Manual/automatic type |
| SHSEC | `sys/geom/shsec/g_shsec.h` | `pbsd/geom/pbsd.geom.shsec.cppm` | State machine |
| VIRSTOR | `sys/geom/virstor/g_virstor.h` | `pbsd/geom/pbsd.geom.virstor.cppm` | Chunk size validation |

### ZFS modules (+3 new → 14 CMake targets)

| Module | HBSD source | PBSD artifact | Notes |
|--------|-------------|---------------|-------|
| zap | `contrib/openzfs/include/sys/zap.h` | `pbsd/zfs/pbsd.zfs.zap.cppm` | Name/value limits |
| zil | `contrib/openzfs/include/sys/zil.h` | `pbsd/zfs/pbsd.zfs.zil.cppm` | Block types |
| l2arc | `contrib/openzfs/include/sys/arc_impl.h` | `pbsd/zfs/pbsd.zfs.l2arc.cppm` | Dev size, flags |

### KDE modules (+6 new hand ports)

| Module | Upstream | PBSD artifact |
|--------|----------|---------------|
| KColorScheme | `kguiaddons/.../kcolorscheme.cpp` | `pbsd/kde/frameworks/kguiaddons.kcolorscheme.cppm` |
| Kirigami units | `kirigami/src/units.cpp` | `pbsd/kde/frameworks/kirigami.units.cppm` |
| Plasma volume | `plasma-workspace/applets/volume/` | `pbsd/kde/plasma.volume.cppm` |
| Media controller | `plasma-workspace/applets/mediacontroller/` | `pbsd/kde/plasma.mediacontroller.cppm` |
| KWin fade | `kwin/src/plugins/fade/` | `pbsd/kde/kwin/effects/fade.cppm` |
| KWin dialogparent | `kwin/src/plugins/dialogparent/` | `pbsd/kde/kwin/effects/dialogparent.cppm` |

### Theme modules (+4 new, 3 wired to CMake)

| Module | PBSD artifact | Notes |
|--------|---------------|-------|
| aero.panel | `pbsd/theme/plasma/aero.panel.cppm` | Panel chrome metrics |
| aero.notification | `pbsd/theme/plasma/aero.notification.cppm` | Notification popup metrics |
| aero.dialog | `pbsd/theme/plasma/aero.dialog.cppm` | Dialog frame metrics |
| aero.icons | `pbsd/theme/plasma/aero.icons.cppm` | Context icon sizing |

Fixed pre-existing theme imports: `aero.compositing`, `aero.glow` now depend only on `pbsd.kde.plasma.aero`.

### Harness additions

- `test_wave6_net_fs_geom_zfs`: +18 burst-9 smoke checks (stf/wg/pfsync/ipsec/vxlan, msdosfs/cd9660/unionfs/isofs/autofs, geom cache/shsec/virstor, zap/zil/l2arc)
- `test_wave9_kde_theme`: plasma aero + volume + fade + theme panel/icons smoke (6 checks)
- SI harness links `pbsd_kde` + `pbsd_theme` INTERFACE aggregates

### CMake modules added

Net: `pbsd_net_stf`, `pbsd_net_wg`, `pbsd_net_pfsync` (+ wired `pbsd_net_ipsec`, `pbsd_net_vxlan`).

FS: `pbsd_fs_isofs`, `pbsd_fs_autofs` (+ wired `pbsd_fs_msdosfs`, `pbsd_fs_cd9660`, `pbsd_fs_unionfs`).

GEOM: `pbsd_geom_cache`, `pbsd_geom_shsec`, `pbsd_geom_virstor`.

ZFS: `pbsd_zfs_zap`, `pbsd_zfs_zil`, `pbsd_zfs_l2arc`.

KDE: extended `PBSD_KDE_FRAMEWORKS_MODULES` (+2), `PBSD_KDE_PLASMA_MODULES` (+2), `PBSD_KDE_KWIN_EFFECT_MODULES` (+2).

Theme: extended `PBSD_THEME_PLASMA_AERO_MODULES` (+3: `aero.dialog`, `aero.notification`, `aero.icons`; `aero.panel` pre-existing).

Umbrella re-exports updated: `pbsd.net.cppm`, `pbsd.fs.cppm`, `pbsd.geom.cppm`, `pbsd.zfs.cppm`.

Build fixes: `pbsd_userland_libc` → `pbsd_userland_hosted` link; `vge.cppm` `kPciVendorVIA` typo; harness `sx`/`rwlock`/`stand::net` qualification.

Generator: `tools/gen_wave9_burst.py`.

**New modules this pass:** 21 hand ports; **CMake wired:** +8 new targets, +5 orphan sync. **`si_harness`:** ALL PASS (`/tmp/pbsd-b9`).

