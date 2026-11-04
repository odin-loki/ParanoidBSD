module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_create;

export import pbsd.core;

/// uuid_create from hbsd/src/lib/libc/uuid/uuid_create.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_create_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
