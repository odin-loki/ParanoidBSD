module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vfprintf;

export import pbsd.core;

/// vfprintf from hbsd/src/lib/libc/stdio/vfprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vfprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
