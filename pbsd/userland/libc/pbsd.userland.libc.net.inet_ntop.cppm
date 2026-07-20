module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_ntop;

export import pbsd.core;

/// inet_ntop from hbsd/src/lib/libc/net/inet_ntop.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_ntop_validate(int af) noexcept {
    if (af != 2 && af != 28) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
