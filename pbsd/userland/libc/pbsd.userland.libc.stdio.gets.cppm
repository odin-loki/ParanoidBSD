module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.gets;

export import pbsd.core;

/// gets from hbsd/src/lib/libc/stdio/gets.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gets_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
