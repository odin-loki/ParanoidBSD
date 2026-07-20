module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.open_memstream;

export import pbsd.core;

/// open_memstream from hbsd/src/lib/libc/stdio/open_memstream.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status open_memstream_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
