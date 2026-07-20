module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.auth_none;

export import pbsd.core;

/// auth_none from hbsd/src/lib/libc/rpc/auth_none.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status auth_none_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
