module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.devname_compat11;

export import pbsd.core;

/// devname_compat11 from hbsd/src/lib/libc/gen/devname_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status devname_compat11_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
