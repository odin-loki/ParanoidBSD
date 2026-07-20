module;
#include <cstddef>

export module pbsd.userland.libc.sys.read;

export import pbsd.core;

/// read from hbsd/src/lib/libc/sys/read.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> read_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}

} // namespace pbsd::userland::libc
