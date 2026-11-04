module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fsetpos;

export import pbsd.core;

/// fsetpos from hbsd/src/lib/libc/stdio/fsetpos.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fsetpos_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
