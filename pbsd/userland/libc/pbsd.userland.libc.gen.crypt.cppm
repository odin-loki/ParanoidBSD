module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.crypt;

export import pbsd.core;

/// crypt from hbsd/src/lib/libc/gen/crypt.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status crypt_validate(const char* key, const char* salt) noexcept {
    if (key == nullptr || salt == nullptr || salt[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
