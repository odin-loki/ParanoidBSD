module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strdup;

export import pbsd.core;

/// strdup from hbsd/src/lib/libc/string/strdup.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strdup_src(const char* s) noexcept {
    if (s == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
