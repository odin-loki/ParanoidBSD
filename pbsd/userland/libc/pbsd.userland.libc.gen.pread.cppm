module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pread;

export import pbsd.core;

/// pread from hbsd/src/lib/libc/gen/pread.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> pread_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}

} // namespace pbsd::userland::libc
