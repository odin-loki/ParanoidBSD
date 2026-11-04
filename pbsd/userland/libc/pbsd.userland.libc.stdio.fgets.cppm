module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fgets;

export import pbsd.core;

/// fgets from hbsd/src/lib/libc/stdio/fgets.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgets_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
