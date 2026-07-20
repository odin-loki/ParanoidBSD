module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strcasestr;

export import pbsd.core;

/// strcasestr from hbsd/src/lib/libc/string/strcasestr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strcasestr_find(const char* hay, const char* needle) noexcept {
    (void)hay; (void)needle;
    return nullptr;
}

} // namespace pbsd::userland::libc
