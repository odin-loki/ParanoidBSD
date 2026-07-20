module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strcat;

export import pbsd.core;

/// strcat from hbsd/src/lib/libc/string/strcat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strcat_dst(char* dst, const char* src) noexcept {
    if (dst == nullptr || src == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
