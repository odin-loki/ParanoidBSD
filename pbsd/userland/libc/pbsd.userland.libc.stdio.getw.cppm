module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.getw;

export import pbsd.core;

/// getw from hbsd/src/lib/libc/stdio/getw.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getw_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
