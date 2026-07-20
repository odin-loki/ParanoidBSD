module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fcloseall;

export import pbsd.core;

/// fcloseall from hbsd/src/lib/libc/stdio/fcloseall.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fcloseall_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
