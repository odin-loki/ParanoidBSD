module;

export module pbsd.userland.libc.gen.siglist;

export import pbsd.core;

/// siglist/sys_signame from hbsd/src/lib/libc/gen/siglist.c
export namespace pbsd::userland::libc {

inline constexpr int kNsigs = 33;

inline constexpr const char* kSigname[kNsigs] = {
    "Signal 0",
    "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL",
    "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG", "STOP", "TSTP",
    "CONT", "CHLD", "TTIN", "TTOU", "IO", "XCPU", "XFSZ", "VTALRM", "PROF",
    "WINCH", "INFO", "USR1", "USR2",
};

[[nodiscard]] inline char const* signame(int sig) noexcept {
    if (sig < 0 || sig >= kNsigs) {
        return nullptr;
    }
    return kSigname[sig];
}

[[nodiscard]] inline char const* sigdescr(int sig) noexcept {
    switch (sig) {
    case 1: return "Hangup";
    case 2: return "Interrupt";
    case 9: return "Killed";
    case 15: return "Terminated";
    default: return signame(sig);
    }
}

} // namespace pbsd::userland::libc
