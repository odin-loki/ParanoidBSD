module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.rpc.des_crypt;

export import pbsd.core;

/// des_crypt from hbsd/src/lib/libc/rpc/des_crypt.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status des_crypt_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
