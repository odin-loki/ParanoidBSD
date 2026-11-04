module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.scanf;

export import pbsd.core;

/// scanf from hbsd/src/lib/libc/stdio/scanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status scanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
