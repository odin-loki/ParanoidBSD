module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.nsdispatch;

export import pbsd.core;

/// nsdispatch from hbsd/src/lib/libc/net/nsdispatch.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nsdispatch_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
