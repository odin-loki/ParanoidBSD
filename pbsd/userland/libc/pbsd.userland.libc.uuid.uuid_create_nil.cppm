module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_create_nil;

export import pbsd.core;

/// uuid_create_nil from hbsd/src/lib/libc/uuid/uuid_create_nil.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_create_nil_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
