module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_hash;

export import pbsd.core;

/// uuid_hash from hbsd/src/lib/libc/uuid/uuid_hash.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_hash_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
