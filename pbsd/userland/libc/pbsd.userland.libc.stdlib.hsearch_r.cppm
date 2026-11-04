module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.hsearch_r;

export import pbsd.core;

/// hsearch_r from hbsd/src/lib/libc/stdlib/hsearch_r.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status hsearch_r_item(const char* item) noexcept {
    if (item == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
