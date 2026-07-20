module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.recv;

export import pbsd.core;

/// recv from hbsd/src/lib/libc/sys/recv.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status recv_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
