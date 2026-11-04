module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.dprintf;

export import pbsd.core;

/// dprintf from hbsd/src/lib/libc/stdio/dprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status dprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
