module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.free_sized;

export import pbsd.core;

/// free_sized from hbsd/src/lib/libc/stdlib/malloc/free_sized.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status free_sized_ptr(void* p, std::size_t size) noexcept {
    (void)p; (void)size;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
