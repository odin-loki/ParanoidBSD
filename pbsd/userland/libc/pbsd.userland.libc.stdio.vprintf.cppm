module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vprintf;

export import pbsd.core;

/// vprintf from hbsd/src/lib/libc/stdio/vprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
