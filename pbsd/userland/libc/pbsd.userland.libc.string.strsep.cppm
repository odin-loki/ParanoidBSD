module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strsep;

export import pbsd.core;

/// strsep from hbsd/src/lib/libc/string/strsep.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strsep_span(char** stringp, const char* delim) noexcept {
    if (stringp == nullptr || *stringp == nullptr) return nullptr;
    (void)delim;
    return *stringp;
}

} // namespace pbsd::userland::libc
