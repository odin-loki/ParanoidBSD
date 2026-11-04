module;

#include <cstddef>

export module pbsd.userland.killall;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/killall/killall.c — signal name helpers.
export namespace pbsd::userland::usr_bin::killall {

inline constexpr const char* kSignalNames[] = {
    "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE",
    "KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG",
};

namespace detail {
[[nodiscard]] inline char to_lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}
} // namespace detail

[[nodiscard]] inline int signal_index(const char* name) noexcept {
    if (name == nullptr) {
        return -1;
    }
    for (std::size_t i = 0; i < sizeof(kSignalNames) / sizeof(kSignalNames[0]); ++i) {
        const char* sig = kSignalNames[i];
        std::size_t j = 0;
        while (name[j] != '\0' && sig[j] != '\0' &&
               detail::to_lower(name[j]) == detail::to_lower(sig[j])) {
            ++j;
        }
        if (name[j] == '\0' && sig[j] == '\0') {
            return static_cast<int>(i + 1);
        }
    }
    return -1;
}

[[nodiscard]] inline Result<int> parse_signal_option(const char* opt) noexcept {
    if (opt == nullptr || *opt != '-') {
        return result_err<int>(Status::Invalid);
    }
    ++opt;
    if (*opt == '\0') {
        return result_err<int>(Status::Invalid);
    }
    const int idx = signal_index(opt);
    if (idx < 0) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(idx);
}

} // namespace pbsd::userland::usr_bin::killall
