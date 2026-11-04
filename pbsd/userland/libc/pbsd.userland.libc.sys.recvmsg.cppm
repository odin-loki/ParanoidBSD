module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.recvmsg;

export import pbsd.core;

/// recvmsg from hbsd/src/lib/libc/sys/recvmsg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status recvmsg_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
