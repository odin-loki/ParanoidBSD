module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.rthdr;

export import pbsd.core;

/// rthdr from hbsd/src/lib/libc/net/rthdr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rthdr_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
