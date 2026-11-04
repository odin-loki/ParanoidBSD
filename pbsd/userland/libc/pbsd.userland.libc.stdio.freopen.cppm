module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.freopen;

export import pbsd.core;

/// freopen from hbsd/src/lib/libc/stdio/freopen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status freopen_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
