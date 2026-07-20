module;
#include <cstddef>

export module pbsd.userland.yes;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.capsicum.helpers;

/// Port of hbsd/src/usr.bin/yes/yes.c — expletive assembly (logic-only).
export namespace pbsd::userland::usr_bin::yes {

inline constexpr const char kDefaultExp[] = "y\n";
inline constexpr std::size_t kDefaultExpLen = 2;
inline constexpr std::size_t kOptBuf = 8190;

[[nodiscard]] inline std::size_t build_expletive(char* buf, std::size_t cap,
                                                 char* const* argv) noexcept {
    if (buf == nullptr || cap < 2) {
        return 0;
    }
    if (argv == nullptr || argv[0] == nullptr) {
        if (cap >= kDefaultExpLen) {
            for (std::size_t i = 0; i < kDefaultExpLen; ++i) {
                buf[i] = kDefaultExp[i];
            }
            return kDefaultExpLen;
        }
        return 0;
    }

    char* pos = buf;
    char* end = buf + cap;
    while (argv[0] != nullptr && pos < end) {
        if (pos > buf) {
            *pos++ = ' ';
        }
        const char* exp = argv[0];
        while (*exp != '\0' && pos < end) {
            *pos++ = *exp++;
        }
        ++argv;
    }
    if (pos > end - 2) {
        pos = end - 2;
    }
    *pos++ = '\n';
    return static_cast<std::size_t>(pos - buf);
}

[[nodiscard]] inline std::size_t optimal_buflen(std::size_t explen) noexcept {
    if (explen == 0) {
        return 0;
    }
    std::size_t buflen = explen;
    while (buflen < kOptBuf) {
        buflen += buflen;
    }
    if (explen < kOptBuf && buflen > kOptBuf) {
        buflen = kOptBuf;
    }
    return buflen;
}

} // namespace pbsd::userland::usr_bin::yes
