module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.name6;

export import pbsd.core;

/// name6 from hbsd/src/lib/libc/net/name6.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status name6_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
