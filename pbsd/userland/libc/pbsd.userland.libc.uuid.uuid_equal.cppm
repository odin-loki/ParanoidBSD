module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_equal;

export import pbsd.core;

/// uuid_equal from hbsd/src/lib/libc/uuid/uuid_equal.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_equal_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
