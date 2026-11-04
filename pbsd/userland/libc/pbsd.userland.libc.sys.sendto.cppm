module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sendto;

export import pbsd.core;

/// sendto from hbsd/src/lib/libc/sys/sendto.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sendto_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
