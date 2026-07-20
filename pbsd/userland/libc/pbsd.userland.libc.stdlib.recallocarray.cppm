module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.recallocarray;

export import pbsd.core;

/// recallocarray from hbsd/src/lib/libc/stdlib/recallocarray.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status recallocarray_count(std::size_t n, std::size_t size) noexcept {
    if (n == 0 || size == 0) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
