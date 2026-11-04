# SPEC: Kernel C++23 ABI (Wave 0)

**Status:** Frozen for Wave 0–4  
**Port context:** ParanoidBSD full C++23 port — HardenedBSD kernel and kmods gain freestanding C++23 via `NORMAL_CXX`.

## Toolchain variables

| Variable | Location | Default |
|----------|----------|---------|
| `CXXSTD` | `hbsd/src/sys/conf/kern.mk` | `c++23` |
| `NORMAL_CXX` | `hbsd/src/sys/conf/kern.mk` | freestanding flag bundle |
| `CXX` | `hbsd/src/sys/conf/kmod.mk` | `${CC}` when `.cpp` in `SRCS` |

## Flags (freestanding)

`NORMAL_CXX` expands to:

```
-std=c++23 -fno-exceptions -fno-rtti -ffreestanding
-fno-threadsafe-statics -fno-use-cxa-atexit
```

Plus arch flags where applicable:

- amd64: `-mcmodel=kernel -mno-red-zone`
- aarch64: `-mgeneral-regs-only`
- When `MK_SSP=no`: `-fno-stack-protector`

C TUs continue to use `CSTD=gnu17` and `CWARNFLAGS`. C++ kmod TUs inherit `CFLAGS` minus `-std=*`, then append `NORMAL_CXX`.

## Dual-link pattern

ParanoidBSD kmods and kernel glue compile **C and C++23 TUs into one `.ko`**. Boundaries are explicit `extern "C"` — no exceptions, no RTTI across the edge.

```
┌─────────────────┐     extern "C"      ┌──────────────────┐
│  pbsd_*.c       │ ──────────────────► │  pbsd_*.cpp      │
│  (KPI / mod glue)│   pbsd_cxx_*()     │  (C++23 logic)   │
└─────────────────┘                     └──────────────────┘
        ▲                                         │
        │         extern "C"                      │
        └──────── pbsd_*_c_*() ◄──────────────────┘
```

Rules:

1. C++ TUs may call C KPI via headers wrapped in `extern "C" { }`.
2. C callers use `extern "C"` entry points exported from C++ TUs (`pbsd_cxx_*`).
3. C++ may call C helpers exported from `.c` TUs (`pbsd_*_c_*`).
4. No exceptions cross the boundary. Errors are integer / `pbsd::Status` codes.
5. Shared declarations live in a `.h` with `#ifdef __cplusplus extern "C"` guards.

## Modules in kernel

Prefer `.cppm` for PBSD nucleus; kmods may use `.cpp` + imported BMIs once module BMI wiring lands (Wave 4). Until then, `.cpp` + `#include` of thin C ABI headers is the supported path.

## Build integration

- Kernel config: `hbsd/src/sys/conf/kern.mk` defines `NORMAL_CXX`.
- Kmods: `hbsd/src/sys/conf/kmod.mk` includes `<bsd.suffixes.mk>` and sets `CXXFLAGS` when `SRCS` contains C++ sources.
- Hosted CI: `pbsd/CMakeLists.txt` mirrors freestanding flags on nucleus libraries.

## Proof artifact

| Artifact | Purpose |
|----------|---------|
| `pbsd/kernel/pbsd_cxx_hello.{h,cpp,c}` | Dual-link source (C↔C++) |
| `hbsd/src/sys/modules/pbsd_cxx_hello/Makefile` | In-tree kmod hook using `NORMAL_CXX` |
| `pbsd/kernel/CMakeLists.txt` | Hosted static lib for CI |

Load test on HardenedBSD host:

```sh
make -C sys/modules/pbsd_cxx_hello
kldload pbsd_cxx_hello
kldunload pbsd_cxx_hello
```
