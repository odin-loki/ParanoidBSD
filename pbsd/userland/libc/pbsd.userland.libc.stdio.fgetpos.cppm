module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fgetpos;

export import pbsd.core;

/// fgetpos from hbsd/src/lib/libc/stdio/fgetpos.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgetpos_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
