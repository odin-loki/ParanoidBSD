module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.open_wmemstream;

export import pbsd.core;

/// open_wmemstream from hbsd/src/lib/libc/stdio/open_wmemstream.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status open_wmemstream_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
