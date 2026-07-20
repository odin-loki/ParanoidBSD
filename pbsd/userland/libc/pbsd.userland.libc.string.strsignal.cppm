module;

#include <cstddef>

export module pbsd.userland.libc.string.strsignal;

/// strsignal from hbsd/src/lib/libc/string/strsignal.c
export namespace pbsd::userland::libc {

inline constexpr const char* kSignalNames[] = {
    "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE",
    "KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG",
};

[[nodiscard]] inline const char* strsignal(int sig) noexcept {
    if (sig < 1 || sig > 15) {
        return nullptr;
    }
    return kSignalNames[sig - 1];
}

[[nodiscard]] inline bool valid_signal(int sig) noexcept {
    return sig >= 1 && sig <= 15;
}

} // namespace pbsd::userland::libc
