module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.wprintf;

export import pbsd.core;

/// wprintf from hbsd/src/lib/libc/stdio/wprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
