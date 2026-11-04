module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.asprintf;

export import pbsd.core;

/// asprintf from hbsd/src/lib/libc/stdio/asprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status asprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
