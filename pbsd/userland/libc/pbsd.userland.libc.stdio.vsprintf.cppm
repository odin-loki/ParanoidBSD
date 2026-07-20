module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vsprintf;

export import pbsd.core;

/// vsprintf from hbsd/src/lib/libc/stdio/vsprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vsprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
