module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getaddrinfo;

export import pbsd.core;

/// getaddrinfo from hbsd/src/lib/libc/net/getaddrinfo.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getaddrinfo_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
