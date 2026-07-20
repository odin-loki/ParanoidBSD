module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.findfp;

export import pbsd.core;

/// findfp from hbsd/src/lib/libc/stdio/findfp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status findfp_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
