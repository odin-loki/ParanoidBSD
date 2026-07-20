module;
#include <cstddef>

export module pbsd.userland.libc.gen.psignal;

export import pbsd.core;

/// psignal from hbsd/src/lib/libc/gen/psignal.c
export namespace pbsd::userland::libc {

inline constexpr const char* kSysSiglist[] = {
    "Hangup", "Interrupt", "Quit", "Illegal instruction", "Trace/BPT trap",
    "Abort", "Emulator trap", "Arithmetic exception", "Killed", "Bus error",
    "Segmentation fault", "Bad system call", "Broken pipe", "Alarm clock",
    "Terminated", "Urgent I/O condition",
};

[[nodiscard]] inline const char* signal_name(int sig) noexcept {
    if (sig < 0 || sig >= static_cast<int>(sizeof(kSysSiglist) / sizeof(kSysSiglist[0]))) {
        return "Unknown signal";
    }
    return kSysSiglist[sig];
}

[[nodiscard]] inline std::size_t message_length(const char* prefix, int sig) noexcept {
    std::size_t total = 0;
    if (prefix != nullptr && *prefix != '\0') {
        for (const char* p = prefix; *p != '\0'; ++p) {
            ++total;
        }
        total += 2;
    }
    for (const char* p = signal_name(sig); *p != '\0'; ++p) {
        ++total;
    }
    return total + 1;
}

} // namespace pbsd::userland::libc
