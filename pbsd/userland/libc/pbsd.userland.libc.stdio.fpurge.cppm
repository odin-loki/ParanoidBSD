module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fpurge;

export import pbsd.core;

/// fpurge from hbsd/src/lib/libc/stdio/fpurge.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fpurge_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
