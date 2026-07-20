export module pbsd.kernel.syscall;

export import pbsd.core;

/// Wave 4 — FreeBSD/HBSD syscall number subset (sys/syscall.h) for dual-link tables.
export namespace pbsd::kernel::syscall {

inline constexpr int kSysExit = 1;
inline constexpr int kSysFork = 2;
inline constexpr int kSysRead = 3;
inline constexpr int kSysWrite = 4;
inline constexpr int kSysOpen = 5;
inline constexpr int kSysClose = 6;
inline constexpr int kSysWait4 = 7;
inline constexpr int kSysLink = 9;
inline constexpr int kSysUnlink = 10;
inline constexpr int kSysChdir = 12;
inline constexpr int kSysFchdir = 13;
inline constexpr int kSysMknod = 14;
inline constexpr int kSysChmod = 15;
inline constexpr int kSysChown = 16;
inline constexpr int kSysBreak = 17;
inline constexpr int kSysGetpid = 20;
inline constexpr int kSysMount = 21;
inline constexpr int kSysUnmount = 22;
inline constexpr int kSysSetuid = 23;
inline constexpr int kSysGetuid = 24;
inline constexpr int kSysGeteuid = 25;
inline constexpr int kSysPtrace = 26;
inline constexpr int kSysRecvmsg = 27;
inline constexpr int kSysSendmsg = 28;
inline constexpr int kSysRecvfrom = 29;
inline constexpr int kSysAccept = 30;
inline constexpr int kSysGetpeername = 31;
inline constexpr int kSysGetsockname = 32;
inline constexpr int kSysAccess = 33;
inline constexpr int kSysChflags = 34;
inline constexpr int kSysFchflags = 35;
inline constexpr int kSysSync = 36;
inline constexpr int kSysKill = 37;
inline constexpr int kSysGetppid = 39;
inline constexpr int kSysDup = 41;
inline constexpr int kSysPipe = 42;
inline constexpr int kSysGetegid = 43;
inline constexpr int kSysProfil = 44;
inline constexpr int kSysKtrace = 45;
inline constexpr int kSysGetgid = 47;
inline constexpr int kSysGetlogin = 49;
inline constexpr int kSysSetlogin = 50;
inline constexpr int kSysAcct = 51;
inline constexpr int kSysSigaltstack = 53;
inline constexpr int kSysIoctl = 54;
inline constexpr int kSysReboot = 55;
inline constexpr int kSysRevoke = 56;
inline constexpr int kSysSymlink = 57;
inline constexpr int kSysReadlink = 58;
inline constexpr int kSysExecve = 59;
inline constexpr int kSysUmask = 60;
inline constexpr int kSysChroot = 61;
inline constexpr int kSysMmap = 197;
inline constexpr int kSysMunmap = 73;
inline constexpr int kSysMprotect = 74;
inline constexpr int kSysCapRightsLimit = 533;
inline constexpr int kSysCapEnter = 516;
inline constexpr int kSysCapGetmode = 517;
inline constexpr int kSysJail = 338;
inline constexpr int kSysJailGet = 506;
inline constexpr int kSysJailSet = 507;
inline constexpr int kSysJailRemove = 508;
inline constexpr int kSysJailAttach = 436;
inline constexpr int kSysClockGettime = 232;
inline constexpr int kSysClockSettime = 233;
inline constexpr int kSysClockGetres = 234;
inline constexpr int kSysClockNanosleep = 244;

struct SyscallEntry {
    int         number;
    const char* name;
};

inline constexpr SyscallEntry kSyscallTable[] = {
    {kSysRead,            "read"},
    {kSysWrite,           "write"},
    {kSysOpen,            "open"},
    {kSysClose,           "close"},
    {kSysMmap,            "mmap"},
    {kSysKtrace,          "ktrace"},
    {kSysCapEnter,        "cap_enter"},
    {kSysCapRightsLimit,  "cap_rights_limit"},
    {kSysJail,            "jail"},
    {kSysJailGet,         "jail_get"},
    {kSysJailSet,         "jail_set"},
    {kSysClockGettime,    "clock_gettime"},
    {kSysClockNanosleep,  "clock_nanosleep"},
};

[[nodiscard]] inline unsigned syscall_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kSyscallTable) / sizeof(kSyscallTable[0]));
}

[[nodiscard]] inline Result<int> syscall_number(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    for (const auto& e : kSyscallTable) {
        const char* a = name;
        const char* b = e.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') {
            return result_ok(e.number);
        }
    }
    return result_err<int>(Status::NotFound);
}

[[nodiscard]] constexpr bool is_jail_syscall(int n) noexcept {
    return n == kSysJail || n == kSysJailGet || n == kSysJailSet || n == kSysJailRemove
        || n == kSysJailAttach;
}

[[nodiscard]] constexpr bool is_time_syscall(int n) noexcept {
    return n == kSysClockGettime || n == kSysClockSettime || n == kSysClockGetres
        || n == kSysClockNanosleep;
}

[[nodiscard]] constexpr bool is_capsicum_syscall(int n) noexcept {
    return n == kSysCapRightsLimit || n == kSysCapEnter || n == kSysCapGetmode;
}

[[nodiscard]] constexpr bool is_file_syscall(int n) noexcept {
    return n == kSysOpen || n == kSysClose || n == kSysRead || n == kSysWrite
        || n == kSysAccess || n == kSysChmod || n == kSysChown;
}

} // namespace pbsd::kernel::syscall
