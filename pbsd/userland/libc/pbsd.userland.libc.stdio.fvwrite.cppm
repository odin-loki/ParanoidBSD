module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fvwrite;

export import pbsd.core;

/// fvwrite from hbsd/src/lib/libc/stdio/fvwrite.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fvwrite_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
