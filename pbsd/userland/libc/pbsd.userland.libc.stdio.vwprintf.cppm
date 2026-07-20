module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vwprintf;

export import pbsd.core;

/// vwprintf from hbsd/src/lib/libc/stdio/vwprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vwprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
