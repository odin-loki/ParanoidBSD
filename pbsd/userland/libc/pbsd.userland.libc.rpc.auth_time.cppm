module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.auth_time;

export import pbsd.core;

/// auth_time from hbsd/src/lib/libc/rpc/auth_time.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status auth_time_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
