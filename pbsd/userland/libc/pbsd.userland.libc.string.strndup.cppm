module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strndup;

export import pbsd.core;

/// strndup from hbsd/src/lib/libc/string/strndup.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strndup_src(const char* s, std::size_t n) noexcept {
    if (s == nullptr || n == 0) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
