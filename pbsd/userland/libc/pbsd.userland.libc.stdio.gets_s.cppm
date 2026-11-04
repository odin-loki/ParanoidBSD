module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.gets_s;

export import pbsd.core;

/// gets_s from hbsd/src/lib/libc/stdio/gets_s.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gets_s_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
