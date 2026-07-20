module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.send;

export import pbsd.core;

/// send from hbsd/src/lib/libc/sys/send.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status send_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
