module;

#include <cstddef>

export module pbsd.userland.libc.gen.strmode;

export import pbsd.core;

/// strmode from hbsd/src/lib/libc/gen/strmode.c
export namespace pbsd::userland::libc {

inline constexpr const char kTypeChars[] = "?-pldcbs-?";

[[nodiscard]] inline void strmode(unsigned mode, char* buf) noexcept {
    if (buf == nullptr) {
        return;
    }
    const unsigned type = (mode >> 12) & 017U;
    char* p = buf;
    *p++ = type < 8 ? kTypeChars[type] : '?';
    *p++ = (mode & 0400U) ? 'r' : '-';
    *p++ = (mode & 0200U) ? 'w' : '-';
    *p++ = (mode & 0100U) ? 'x' : '-';
    *p++ = (mode & 0040U) ? 'r' : '-';
    *p++ = (mode & 0020U) ? 'w' : '-';
    *p++ = (mode & 0010U) ? 'x' : '-';
    *p++ = (mode & 0004U) ? 'r' : '-';
    *p++ = (mode & 0002U) ? 'w' : '-';
    *p++ = (mode & 0001U) ? 'x' : '-';
    *p = '\0';
}

} // namespace pbsd::userland::libc
