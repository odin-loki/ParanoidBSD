module;
#include <cstddef>

export module pbsd.userland.libc.sys.write;

export import pbsd.core;

/// write from hbsd/src/lib/libc/sys/write.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> write_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}

} // namespace pbsd::userland::libc
