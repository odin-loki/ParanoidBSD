module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vscanf;

export import pbsd.core;

/// vscanf from hbsd/src/lib/libc/stdio/vscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
