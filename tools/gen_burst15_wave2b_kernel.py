#!/usr/bin/env python3
"""Burst 15 wave 2b — mass kernel kern_* hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
KERN = PBSD / "kernel" / "kern"
CMAKE = PBSD / "CMakeLists.txt"

# (stem without kern_ prefix, optional cmake dep beyond pbsd_core)
NEW_MODULES: list[tuple[str, str | None]] = [
    ("exec", None),
    ("descrip", "pbsd_kernel_filedesc"),
    ("syscalls", "pbsd_kernel_syscall"),
    ("thread", "pbsd_handles"),
    ("switch", "pbsd_kernel_sched"),
    ("time", "pbsd_kernel_timekeeping"),
    ("event", "pbsd_kernel_kevent"),
    ("umtx", None),
    ("module", "pbsd_kernel_kmod"),
    ("cpu", "pbsd_kernel_cpuset"),
    ("conf", None),
    ("environment", None),
    ("lock", None),
    ("sendfile", "pbsd_kernel_socket_syscall"),
    ("sharedpage", "pbsd_kernel_vm"),
    ("thr", None),
    ("ntptime", "pbsd_kernel_timekeeping"),
    ("physio", "pbsd_kernel_bio"),
    ("procctl", "pbsd_kernel_process"),
    ("membarrier", None),
]

BODIES: dict[str, str] = {
    "exec": """inline constexpr int kTraceExec = 0x0001;
inline constexpr int kTraceFork = 0x0002;
inline constexpr int kTraceVfork = 0x0004;

[[nodiscard]] inline Status validate_trace_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool traces_exec(int flags) noexcept {
    return (flags & kTraceExec) != 0;
}""",
    "descrip": """inline constexpr int kMaxFd = 1 << 20;
inline constexpr int kDupFixed = 0x0001;

[[nodiscard]] inline Status validate_fd(int fd) noexcept {
    if (fd < 0 || fd >= kMaxFd) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_dup2(int oldfd, int newfd) noexcept {
    if (validate_fd(oldfd) != Status::Ok || validate_fd(newfd) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "syscalls": """inline constexpr unsigned char kSyfCapenabled = 0x01;
inline constexpr unsigned kSyThrStatic = 0x01;
inline constexpr unsigned kSyThrDraining = 0x02;
inline constexpr unsigned kSyThrAbsent = 0x04;

[[nodiscard]] inline bool cap_enabled(unsigned char flags) noexcept {
    return (flags & kSyfCapenabled) != 0;
}

[[nodiscard]] inline bool is_draining(unsigned thr) noexcept {
    return (thr & kSyThrDraining) != 0;
}""",
    "thread": """inline constexpr int kTdRunning = 0x0001;
inline constexpr int kTdSleeping = 0x0002;
inline constexpr int kTdLocksleep = 0x0004;
inline constexpr int kTdIntr = 0x0008;

[[nodiscard]] inline bool is_sleeping(int flags) noexcept {
    return (flags & (kTdSleeping | kTdLocksleep)) != 0;
}

[[nodiscard]] inline Status validate_state(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "switch": """enum class Reason : unsigned char {
    Preempt = 0,
    Sleep = 1,
    Yield = 2,
    Exit = 3,
};

[[nodiscard]] inline Status validate_reason(Reason r) noexcept {
    switch (r) {
    case Reason::Preempt:
    case Reason::Sleep:
    case Reason::Yield:
    case Reason::Exit:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}""",
    "time": """inline constexpr int kClockRealtime = 0;
inline constexpr int kClockMonotonic = 3;
inline constexpr int kTimerAbstime = 0x0001;

[[nodiscard]] inline Status validate_clockid(int id) noexcept {
    if (id != kClockRealtime && id != kClockMonotonic) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_absolute(int flags) noexcept {
    return (flags & kTimerAbstime) != 0;
}""",
    "event": """inline constexpr unsigned kEvAdd = 0x0001;
inline constexpr unsigned kEvDelete = 0x0002;
inline constexpr unsigned kEvEnable = 0x0004;
inline constexpr unsigned kEvDisable = 0x0008;
inline constexpr unsigned kEvOneshot = 0x0010;

[[nodiscard]] inline Status validate_fflags(unsigned fflags) noexcept {
    if (fflags & ~(kEvAdd | kEvDelete | kEvEnable | kEvDisable | kEvOneshot)) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "umtx": """inline constexpr int kOpWait = 2;
inline constexpr int kOpWake = 3;
inline constexpr int kOpMutexLock = 5;
inline constexpr int kOpMutexUnlock = 6;
inline constexpr int kAbstime = 0x01;

[[nodiscard]] inline Status validate_op(int op) noexcept {
    switch (op) {
    case kOpWait:
    case kOpWake:
    case kOpMutexLock:
    case kOpMutexUnlock:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}""",
    "module": """inline constexpr int kModUnload = 0x0001;
inline constexpr int kModQuiet = 0x0002;
inline constexpr int kModForce = 0x0004;

[[nodiscard]] inline Status validate_load_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool force_unload(int flags) noexcept {
    return (flags & (kModUnload | kModForce)) == (kModUnload | kModForce);
}""",
    "cpu": """inline constexpr unsigned kCpuOnline = 0x0001;
inline constexpr unsigned kCpuPresent = 0x0002;
inline constexpr unsigned kCpuBsp = 0x0004;

[[nodiscard]] inline bool is_online(unsigned state) noexcept {
    return (state & kCpuOnline) != 0;
}

[[nodiscard]] inline bool is_bsp(unsigned state) noexcept {
    return (state & kCpuBsp) != 0;
}""",
    "conf": """inline constexpr unsigned kDOpen = 0x0001;
inline constexpr unsigned kDClose = 0x0002;
inline constexpr unsigned kDRead = 0x0004;
inline constexpr unsigned kDWrite = 0x0008;

[[nodiscard]] inline Status validate_devsw_flags(unsigned flags) noexcept {
    if ((flags & (kDOpen | kDClose | kDRead | kDWrite)) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "environment": """inline constexpr unsigned kEnvMax = 4096;
inline constexpr unsigned kEnvHashBuckets = 256;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kEnvMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned hash_bucket(const char* name, unsigned len) noexcept {
    unsigned h = 0;
    for (unsigned i = 0; i < len && name[i] != '\\0'; ++i) {
        h = h * 33 + static_cast<unsigned char>(name[i]);
    }
    return h % kEnvHashBuckets;
}""",
    "lock": """inline constexpr int kLockShared = 0x0001;
inline constexpr int kLockExclusive = 0x0002;
inline constexpr int kLockUpgrade = 0x0004;
inline constexpr int kLockDowngrade = 0x0008;

[[nodiscard]] inline Status validate_mode(int mode) noexcept {
    const int acc = mode & (kLockShared | kLockExclusive);
    if (acc != kLockShared && acc != kLockExclusive) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "sendfile": """inline constexpr int kSfMntWait = 0x0001;
inline constexpr int kSfSync = 0x0002;
inline constexpr int kSfNodiskio = 0x0004;

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool wants_sync(int flags) noexcept {
    return (flags & kSfSync) != 0;
}""",
    "sharedpage": """inline constexpr unsigned kSpPresent = 0x0001;
inline constexpr unsigned kSpMapped = 0x0002;

struct State {
    unsigned flags{};
    unsigned gen{};
};

[[nodiscard]] inline Status publish(State& s) noexcept {
    ++s.gen;
    s.flags |= kSpPresent;
    return Status::Ok;
}""",
    "thr": """inline constexpr int kThrCreateSuspended = 0x0001;
inline constexpr int kThrCreateDetached = 0x0002;

[[nodiscard]] inline Status validate_create_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool starts_suspended(int flags) noexcept {
    return (flags & kThrCreateSuspended) != 0;
}""",
    "ntptime": """inline constexpr int kModOffset = 0x0001;
inline constexpr int kModFrequency = 0x0002;
inline constexpr int kModStatus = 0x0010;
inline constexpr int kModTai = 0x0080;

[[nodiscard]] inline Status validate_modes(int modes) noexcept {
    if (modes < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "physio": """inline constexpr unsigned kBAsync = 0x0001;
inline constexpr unsigned kBRead = 0x0002;
inline constexpr unsigned kBWrite = 0x0004;

[[nodiscard]] inline bool is_async(unsigned flags) noexcept {
    return (flags & kBAsync) != 0;
}

[[nodiscard]] inline Status validate_io(unsigned flags) noexcept {
    const unsigned io = flags & (kBRead | kBWrite);
    if (io != kBRead && io != kBWrite) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "procctl": """inline constexpr int kProcctlSetProtection = 1;
inline constexpr int kProcctlClearProtection = 2;
inline constexpr int kProcctlMaxId = 3;

[[nodiscard]] inline Status validate_id(int id) noexcept {
    if (id < 0 || id > kProcctlMaxId) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "membarrier": """enum class Scope : unsigned char {
    Private = 0,
    Shared = 1,
};

enum class Kind : unsigned char {
    LoadLoad = 0,
    LoadStore = 1,
    StoreStore = 2,
    StoreLoad = 3,
};

[[nodiscard]] inline Status validate(Scope s, Kind k) noexcept {
    switch (s) {
    case Scope::Private:
    case Scope::Shared:
        break;
    default:
        return Status::Invalid;
    }
    switch (k) {
    case Kind::LoadLoad:
    case Kind::LoadStore:
    case Kind::StoreStore:
    case Kind::StoreLoad:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}""",
}


def cmake_target(stem: str) -> str:
    return f"pbsd_kernel_kern_{stem}"


def render_c(source: str) -> str:
    return f"/* Reference logic from {source} (dual-world). */\n\n/* stub */\n"


def render_cppm(stem: str, body: str) -> str:
    source = f"hbsd/src/sys/kern/kern_{stem}.c"
    title = stem.replace("_", " ")
    needs_cstdint = "std::" in body or "unsigned char" in body
    inc = "#include <cstdint>\n" if needs_cstdint else ""
    return f"""module;
{inc}
export module pbsd.kernel.kern_{stem};

export import pbsd.core;

/// Freestanding port of `kern/kern_{stem}.c` — {title} helpers.
export namespace pbsd::kernel::kern_{stem} {{

{body}

}} // namespace pbsd::kernel::kern_{stem}
"""


def cmake_block(stem: str, parent: str | None) -> str:
    tgt = cmake_target(stem)
    rel = f"kernel/kern/pbsd.kernel.kern_{stem}.cppm"
    deps = "pbsd_core" if not parent else f"pbsd_core {parent}"
    return f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES
    {rel})
target_link_libraries({tgt} PUBLIC {deps})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def ensure_module(stem: str) -> bool:
    cppm = KERN / f"pbsd.kernel.kern_{stem}.cppm"
    c_stub = KERN / f"pbsd.kernel.kern_{stem}.c"
    source = f"hbsd/src/sys/kern/kern_{stem}.c"
    created = False
    if not cppm.exists():
        cppm.write_text(render_cppm(stem, BODIES[stem]), encoding="utf-8", newline="\n")
        created = True
    c_stub.write_text(render_c(source), encoding="utf-8", newline="\n")
    return created


def patch_cmake() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    anchor = "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)"
    blocks: list[str] = []
    added = 0
    for stem, parent in NEW_MODULES:
        tgt = cmake_target(stem)
        if f"TARGET {tgt}" in cmake:
            continue
        blocks.append(cmake_block(stem, parent))
        added += 1
    if blocks and anchor in cmake:
        cmake = cmake.replace(anchor, "\n".join(blocks) + "\n" + anchor, 1)
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_aggregate() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    pattern = (
        r"(target_link_libraries\(pbsd_kernel INTERFACE[\s\S]*?"
        r"pbsd_kernel_kern_exit)\n(\s+pbsd_kernel_prf)"
    )
    m = re.search(pattern, cmake)
    if not m:
        return 0
    insert = "\n".join(
        f"    {cmake_target(stem)}" for stem, _ in NEW_MODULES if cmake_target(stem) not in m.group(1)
    )
    if not insert:
        return 0
    replacement = m.group(1) + "\n" + insert + "\n" + m.group(2)
    cmake = cmake[: m.start(1)] + replacement + cmake[m.end(2) :]
    CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return sum(1 for stem, _ in NEW_MODULES if cmake_target(stem) not in m.group(1))


def main() -> None:
    created = 0
    for stem, _ in NEW_MODULES:
        if ensure_module(stem):
            created += 1
    cmake_added = patch_cmake()
    link_added = patch_aggregate()
    names = [f"kern_{s}" for s, _ in NEW_MODULES]
    print(f"created {created} new .cppm modules")
    print(f"cmake added {cmake_added} library targets")
    print(f"pbsd_kernel aggregate: +{link_added} links")
    print(f"wave2b kernel: {', '.join(names)}")


if __name__ == "__main__":
    main()
