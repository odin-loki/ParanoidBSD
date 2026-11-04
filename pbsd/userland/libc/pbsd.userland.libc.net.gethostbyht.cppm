module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.gethostbyht;

export import pbsd.core;

/// gethostbyht from hbsd/src/lib/libc/net/gethostbyht.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gethostbyht_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
