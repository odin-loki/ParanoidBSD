module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vfwprintf;

export import pbsd.core;

/// vfwprintf from hbsd/src/lib/libc/stdio/vfwprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vfwprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
