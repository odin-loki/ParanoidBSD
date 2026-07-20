module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pwrite;

export import pbsd.core;

/// pwrite from hbsd/src/lib/libc/gen/pwrite.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> pwrite_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}

} // namespace pbsd::userland::libc
