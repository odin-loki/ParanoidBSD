module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fwide;

export import pbsd.core;

/// fwide from hbsd/src/lib/libc/stdio/fwide.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fwide_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
