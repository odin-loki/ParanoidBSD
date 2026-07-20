module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fprintf;

export import pbsd.core;

/// fprintf from hbsd/src/lib/libc/stdio/fprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
