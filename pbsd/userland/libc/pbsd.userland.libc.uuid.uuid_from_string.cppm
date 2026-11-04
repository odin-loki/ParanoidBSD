module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_from_string;

export import pbsd.core;

/// uuid_from_string from hbsd/src/lib/libc/uuid/uuid_from_string.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_from_string_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
