module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.linkaddr;

export import pbsd.core;

/// linkaddr from hbsd/src/lib/libc/net/linkaddr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status linkaddr_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
