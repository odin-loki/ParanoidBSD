module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.printf;

export import pbsd.core;

/// printf from hbsd/src/lib/libc/stdio/printf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status printf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
