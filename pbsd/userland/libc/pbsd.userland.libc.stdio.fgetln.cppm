module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fgetln;

export import pbsd.core;

/// fgetln from hbsd/src/lib/libc/stdio/fgetln.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgetln_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
