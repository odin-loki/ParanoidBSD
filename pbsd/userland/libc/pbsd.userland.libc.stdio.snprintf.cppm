module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.snprintf;

export import pbsd.core;

/// snprintf from hbsd/src/lib/libc/stdio/snprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status snprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
