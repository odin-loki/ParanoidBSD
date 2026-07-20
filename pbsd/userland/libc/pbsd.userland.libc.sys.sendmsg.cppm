module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sendmsg;

export import pbsd.core;

/// sendmsg from hbsd/src/lib/libc/sys/sendmsg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sendmsg_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
