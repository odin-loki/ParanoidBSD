module;

#include <cstring>

export module pbsd.userland.libc.string.strcoll;

/// strcoll from hbsd/src/lib/libc/string/strcoll.c (C locale passthrough)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strcoll(const char* s1, const char* s2) noexcept {
    if (s1 == nullptr || s2 == nullptr) {
        return 0;
    }
    return std::strcmp(s1, s2);
}

} // namespace pbsd::userland::libc
