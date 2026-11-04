module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vfscanf;

export import pbsd.core;

/// vfscanf from hbsd/src/lib/libc/stdio/vfscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vfscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
