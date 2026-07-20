module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.ptsname;

export import pbsd.core;

/// ptsname from hbsd/src/lib/libc/stdlib/ptsname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ptsname_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
