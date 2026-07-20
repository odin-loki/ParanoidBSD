module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.crypt_client;

export import pbsd.core;

/// crypt_client from hbsd/src/lib/libc/rpc/crypt_client.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status crypt_client_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
