module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_compare;

export import pbsd.core;

/// uuid_compare from hbsd/src/lib/libc/uuid/uuid_compare.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_compare_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
