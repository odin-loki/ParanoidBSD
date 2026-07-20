module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_is_nil;

export import pbsd.core;

/// uuid_is_nil from hbsd/src/lib/libc/uuid/uuid_is_nil.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_is_nil_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
