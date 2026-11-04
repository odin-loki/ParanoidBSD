module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fwalk;

export import pbsd.core;

/// fwalk from hbsd/src/lib/libc/stdio/fwalk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fwalk_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
