module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strtok;

export import pbsd.core;

/// strtok from hbsd/src/lib/libc/string/strtok.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strtok_span(char* s, const char* delim) noexcept {
    (void)s; (void)delim;
    return nullptr;
}

} // namespace pbsd::userland::libc
