module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.nss_compat;

export import pbsd.core;

/// nss_compat from hbsd/src/lib/libc/net/nss_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nss_compat_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
