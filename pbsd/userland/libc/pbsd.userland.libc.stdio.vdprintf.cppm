module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vdprintf;

export import pbsd.core;

/// vdprintf from hbsd/src/lib/libc/stdio/vdprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vdprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
