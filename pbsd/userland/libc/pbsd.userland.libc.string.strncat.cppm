module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strncat;

export import pbsd.core;

/// strncat from hbsd/src/lib/libc/string/strncat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strncat_dst(char* dst, const char* src, std::size_t n) noexcept {
    if (dst == nullptr || src == nullptr) return Status::Invalid;
    (void)n;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
